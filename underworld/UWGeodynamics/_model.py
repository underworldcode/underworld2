from __future__ import print_function, absolute_import
import os
import sys
from collections import OrderedDict
import numpy as np
import h5py
import underworld as uw
import underworld.function as fn
from underworld.function.exception import SafeMaths as Safe
from mpi4py import MPI as _MPI
import underworld.UWGeodynamics.shapes as shapes
import underworld.UWGeodynamics.surfaceProcesses as surfaceProcesses
from . import rcParams
from underworld.scaling import dimensionalise
from underworld.scaling import non_dimensionalise as nd
from underworld.scaling import units as u
from .lithopress import Lithostatic_pressure
from ._utils import PressureSmoother, PassiveTracers
from ._rheology import Viscosity_limiter, Stress_limiter
from ._material import Material
from ._visugrid import Visugrid
from ._boundary_conditions import TemperatureBCs, HeatFlowBCs
from ._boundary_conditions import StressBCs, VelocityBCs
from ._mesh_advector import Mesh_advector
from ._frictional_boundary import FrictionBoundaries
from underworld.mesh import FeMesh_Cartesian
from underworld.swarm import Swarm
from underworld.mesh import MeshVariable
from underworld.swarm import SwarmVariable
from underworld.utils import _swarmvarschema
from datetime import datetime
from ._freesurface import FreeSurfaceProcessor
from ._remeshing import ReMesher
from ._density import Density

comm = _MPI.COMM_WORLD
rank = comm.rank
size = comm.size

_dim_gravity = {'[length]': 1.0, '[time]': -2.0}
_dim_time = {'[time]': 1.0}


class Model(Material):
    """UWGeodynamic Model Class"""

    def __init__(self, elementRes=(64, 64),
                 minCoord=(0., 0.), maxCoord=(64. * u.km, 64 * u.km),
                 name="Model", gravity=(0., -9.81 * u.m / u.s**2),
                 periodic=None, elementType="Q1/dQ0",
                 temperatureBCs=None, heatFlowBCs=None,
                 velocityBCs=None, stressBCs=None, materials=None,
                 outputDir="outputs", frictionalBCs=None,
                 surfaceProcesses=None, isostasy=None, visugrid=None):
        """Create a Model object

        Parameters
        ----------

            elementRes : tuple
                Resolution of the mesh in number of elements
                for each axis (degree of freedom)
            minCoord : tuple
                Minimum coordinates for each axis.
            maxCoord : tuple
                Maximum coordinates for each axis.
            name : str
                The Model name.
            gravity : tuple
                Acceleration due to gravity vector.
            periodic : tuple
                Mesh periodicity.
            elementType : str
                Type of finite element. "Q1/dQ0", "Q2/dQ0" are supported.
            temperatureBCs : TemperatureBCs
                Temperature Boundary Condition, must be object of type:
                TemperatureBCs
            heatFlowBCs : HeatFlowBCs
                Temperature Boundary Condition, must be object of type:
                TemperatureBCs
            velocityBCs : VelocityBCs
                Velocity Boundary Condtion, must be object of type VelocityBCs
            stressBCs : StressBCs
                Stress Boundary Condtion, must be object of type StressBCs
            materials : list
                List of materials, each material must be an object of type:
                Material
            outputDir : str
                Output Directory
            frictionalBCs : FrictionalBCs
                Frictional Boundary Conditions, must be object of type:
                FrictionBoundaries
            surfaceProcesses : SurfaceProcesses
                Surface Processes, must be an object of type:
                SurfaceProcesses
            isostasy : LecodeIsostasy
                Isostasy Solver
            visugrid : Visugrid
                Visugrid object
            advector : MeshAdvector
                Mesh advector object

        Examples
        --------

            >>> from underworld import UWGeodynamics as GEO
            >>> u = GEO.UnitRegistry
            >>> Model = Model = GEO.Model(
            ...        elementRes=(64, 64), minCoord=(0., 0.),
            ...        maxCoord=(64. * u.kilometer, 64. * u.kilometer))

        """

        super(Model, self).__init__()

        # Process __init__ arguments
        self.name = name

        self.minCoord = minCoord
        self.maxCoord = maxCoord
        self.top = maxCoord[-1]
        self.bottom = minCoord[-1]

        if not gravity:
            raise ValueError("""The gravity vector is missing""")
        if len(gravity) != len(self.minCoord):
            raise ValueError("""Your Model is {0}D but you have provided a {1}D
                             gravity vector, please
                             change""".format(len(self.minCoord),
                                              len(gravity)))

        self.gravity = gravity

        if not elementType:
            self.elementType = rcParams["element.type"]
        else:
            self.elementType = elementType

        self.elementRes = elementRes

        self.outputDir = outputDir

        # Compute model dimensions
        self.length = maxCoord[0] - minCoord[0]
        self.height = maxCoord[-1] - minCoord[-1]

        if len(maxCoord) == 3:
            self.width = maxCoord[1] - minCoord[1]

        if periodic:
            self.periodic = periodic
        else:
            periodic = tuple([False for val in maxCoord])
            self.periodic = periodic

        # Get non-dimensional extents along each axis
        minCoord = tuple([nd(val) for val in self.minCoord])
        maxCoord = tuple([nd(val) for val in self.maxCoord])

        # Initialize model mesh
        self.mesh = FeMesh_Cartesian(elementType=self.elementType,
                                     elementRes=self.elementRes,
                                     minCoord=minCoord,
                                     maxCoord=maxCoord,
                                     periodic=self.periodic)

        self.mesh_variables = OrderedDict()
        self.swarm_variables = OrderedDict()
        self.restart_variables = OrderedDict()

        # Add common mesh variables
        self.add_submesh_field("pressureField", nodeDofCount=1,
                               restart_variable=True)
        self.add_mesh_variable("velocityField", nodeDofCount=self.mesh.dim,
                               restart_variable=True)
        self.add_submesh_field("_strainRateField", nodeDofCount=1)

        # symmetric component of the gradient of the flow velocityField.
        self.strainRate = fn.tensor.symmetric(self.velocityField.fn_gradient)
        self._strainRate_2ndInvariant = None

        # Create the material swarm
        self.swarm = Swarm(mesh=self.mesh, particleEscape=True)
        self.swarm.allow_parallel_nn = True
        if self.mesh.dim == 2:
            particlesPerCell = rcParams["swarm.particles.per.cell.2D"]
        else:
            particlesPerCell = rcParams["swarm.particles.per.cell.3D"]

        self._swarmLayout = uw.swarm.layouts.PerCellSpaceFillerLayout(
            swarm=self.swarm,
            particlesPerCell=particlesPerCell)

        self.swarm.populate_using_layout(layout=self._swarmLayout)

        # timing and checkpointing
        self.checkpointID = 0
        self._ndtime = 0.0
        self.step = 0
        self.nlstep = 0
        self._dt = fn.misc.constant(0.)

        self.materials = list(materials) if materials is not None else list()
        self.materials.append(self)

        # Create a series of aliases for the boundary sets
        self.left_wall = self.mesh.specialSets["MinI_VertexSet"]
        self.right_wall = self.mesh.specialSets["MaxI_VertexSet"]

        if self.mesh.dim == 2:
            self.top_wall = self.mesh.specialSets["MaxJ_VertexSet"]
            self.bottom_wall = self.mesh.specialSets["MinJ_VertexSet"]
            self.front_wall = None
            self.back_wall = None
        else:
            self.front_wall = self.mesh.specialSets["MinJ_VertexSet"]
            self.back_wall = self.mesh.specialSets["MaxJ_VertexSet"]
            self.top_wall = self.mesh.specialSets["MaxK_VertexSet"]
            self.bottom_wall = self.mesh.specialSets["MinK_VertexSet"]

        # Boundary Conditions
        self.velocityBCs = velocityBCs
        self.stressBCs = stressBCs
        self.temperatureBCs = temperatureBCs
        self.heatFlowBCs = heatFlowBCs
        self.frictionalBCs = frictionalBCs
        self._isostasy = isostasy
        self.surfaceProcesses = surfaceProcesses

        self.pressSmoother = PressureSmoother(self.mesh, self.pressureField)

        # Passive Tracers
        # An ordered dict is required to ensure that all threads process
        # the same swarm at a time.
        self.passive_tracers = OrderedDict()

        # Visualisation
        self._visugrid = visugrid

        # Mesh advector
        self._advector = None
        self._solver = None
        self._rebuild_solver = False
        self._stokes_SLE = None

        # Initialise remaining attributes
        self.defaultStrainRate = 1e-15 / u.second
        self.minStrainRate = 1e-20 / u.second
        self._solution_exist = fn.misc.constant(False)
        self._temperatureDot = None
        self._temperature = None
        self.DiffusivityFn = None
        self.HeatProdFn = None
        self._freeSurface = False
        self._fssa_factor = None
        self._voronoi_swarm = None
        self._mesh_saved = False
        self._remesher = None
        self._initialize()

        self._viscosity_processor = _ViscosityFunction(self)
        self.callback_functions = OrderedDict()
        self.post_solve_functions = OrderedDict()
        self.pre_solve_functions = OrderedDict()

    @property
    def dt(self):
        return self._dt

    @dt.setter
    def dt(self, value):
        valFn = fn.Function.convert(value)
        self._dt.value = float(valFn.evaluate())

    def _initialize(self):
        """_initialize
        Model Initialisation
        """

        self.add_mesh_variable("tractionField", nodeDofCount=self.mesh.dim)

        self.swarm_advector = uw.systems.SwarmAdvector(
            swarm=self.swarm,
            velocityField=self.velocityField,
            order=2
        )

        if self.mesh.dim == 2:
            particlesPerCell = rcParams["popcontrol.particles.per.cell.2D"]
        else:
            particlesPerCell = rcParams["popcontrol.particles.per.cell.3D"]

        self.population_control = uw.swarm.PopulationControl(
            self.swarm,
            aggressive=rcParams["popcontrol.aggressive"],
            splitThreshold=rcParams["popcontrol.split.threshold"],
            maxSplits=rcParams["popcontrol.max.splits"],
            particlesPerCell=particlesPerCell)

        ## Add Common Swarm Variables
        # time dependent vars
        self.add_swarm_variable("materialField", dataType="int", count=1,
                                restart_variable=True, init_value=self.index)
        self.add_swarm_variable("plasticStrain", dataType="double", count=1,
                                restart_variable=True)
        self.add_swarm_variable("meltField", dataType="double", count=1, 
                                restart_variable=True)
        # DEPRECATE - change name 'timeField' is a poor description. This is a per particle
        # time variable.
        self.add_swarm_variable("timeField", dataType="double", count=1,
                                restart_variable=True)

        # non t dependent vars
        # ASSUMES density and viscosity are uw.fn dependent on other vars
        self.add_swarm_variable("_viscosityField", dataType="double", count=1)
        self.add_swarm_variable("_densityField", dataType="double", count=1)
        self.timeField.data[...] = 0.0
        self.materialField.data[...] = self.index

        if self.mesh.dim == 3:
            stress_dim = 6
        else:
            stress_dim = 3

        self.add_swarm_variable("_previousStressField", dataType="double",
                                count=stress_dim)
        self.add_swarm_variable("_stressTensor", dataType="double",
                                count=stress_dim, projected="submesh")
        self.add_swarm_variable("_stressField", dataType="double",
                                count=1, projected="submesh")

    def __getitem__(self, name):
        """__getitem__

        Return item with name=name from the class __dict__
        This allows the user to get the attributes of the model
        class as:
            Model["name"]

        Parameters
        ----------

            name : name of the attribute

        Returns
        -------
            Attribute of the Model instance.
            self.__dict__[name]
        """
        return self.__dict__[name]

    def _repr_html_(self):
        """_repr_html_

        HTML table describing the model.
        For integration with Jupyter notebook.
        """
        return _model_html_repr(self)

    @property
    def voronoi_swarm(self):
        return self._voronoi_swarm

    @voronoi_swarm.setter
    def voronoi_swarm(self, value):
        if not isinstance(value, (uw.Swarm, None)):
            raise ValueError("You must provide a swarm or None value")
        self._voronoi_swarm = value

    @property
    def time(self):
        """Model time"""
        return dimensionalise(self._ndtime, rcParams["time.SIunits"])

    @time.setter
    def time(self, value):
        """Model time"""
        self._ndtime = nd(value)

    @property
    def x(self):
        """x"""
        return fn.input()[0]

    @property
    def y(self):
        """y"""
        return fn.input()[1]

    @property
    def z(self):
        """z"""
        return fn.input()[2]

    @property
    def outputDir(self):
        """ Output Directory """
        return self._outputDir

    @outputDir.setter
    def outputDir(self, value):
        """ Output Directory """
        self._outputDir = value

    def remesh(self, x=None, y=None, z=None, reset=True):
        import warnings
        warnings.warn("This functionality is experimental")
        if not self._remesher:
            self._remesher = ReMesher(self, x, y, z, reset)
        self._remesher.x = x
        self._remesher.y = y
        self._remesher.z = z
        self._remesher.reset = reset
        self._remesher.remesh()

    def restart(self, step, restartDir=None):
        """Restart the Model from step using output in restartDir directory.

        Parameters
        ----------

            step : int
                step to restart from

            restartDir : path
                directory which contains the files to restart from

        """

        if not step:
            return
        restartDir = restartDir if restartDir else self.outputDir
        if not os.path.exists(restartDir):
            return
        if not os.listdir(restartDir):
            return
        _RestartFunction(self, restartDir).restart(step)

    def checkpoint(self, checkpointID, variables=None,
                   time=None, outputDir=None):
        _CheckpointFunction(self).checkpoint_all(checkpointID, variables,
                                                 time, outputDir)

    @property
    def projMaterialField(self):
        """ Material field projected on the mesh """
        self._materialFieldProjector.solve()
        return self._projMaterialField

    @property
    def projPlasticStrain(self):
        """ Plastic Strain Field projected on the mesh """
        self._plasticStrainProjector.solve()
        return self._projPlasticStrain

    @property
    def projTimeField(self):
        """ Time Field projected on the mesh """
        self._timeFieldProjector.solve()
        return self._projTimeField

    @property
    def projMeltField(self):
        """ Melt Field projected on the mesh """
        self._meltFieldProjector.solve()
        return self._projMeltField

    @property
    def strainRate_2ndInvariant(self):
        """ Strain Rate Field """
        self._strainRate_2ndInvariant = fn.tensor.second_invariant(
            self.strainRate
        )
        condition = [(self._solution_exist, self._strainRate_2ndInvariant),
                     (True, fn.misc.constant(nd(self.defaultStrainRate)))]
        self._strainRate_2ndInvariant = fn.branching.conditional(condition)
        
        condition = [(self._strainRate_2ndInvariant<=nd(self.minStrainRate),nd(self.minStrainRate)),
                     (True, self._strainRate_2ndInvariant)]
        self._strainRate_2ndInvariant = fn.branching.conditional(condition)
        
        return self._strainRate_2ndInvariant

    @property
    def strainRateField(self):
        """ Strain Rate Field """
        self._strainRateField.data[:] = (
            self.strainRate_2ndInvariant.evaluate(self.mesh.subMesh))
        return self._strainRateField

    @property
    def projViscosityField(self):
        """ Viscosity Field projected on the mesh """
        self.viscosityField.data[...] = self._viscosityFn.evaluate(self.swarm)
        self._viscosityFieldProjector.solve()
        return self._projViscosityField

    @property
    def viscosityField(self):
        """ Viscosity Field on particles """
        self._viscosityField.data[:] = self._viscosityFn.evaluate(self.swarm)
        return self._viscosityField

    @property
    def projStressTensor(self):
        """ Stress Tensor on mesh """
        self._stressTensor.data[...] = self._stressFn.evaluate(self.swarm)
        self._stressTensorProjector.solve()
        return self._projStressTensor

    @property
    def projStressField(self):
        """ Second Invariant of the Stress tensor projected on the submesh"""
        stress = fn.tensor.second_invariant(self._stressFn)
        self._stressField.data[...] = stress.evaluate(self.swarm)
        self._stressFieldProjector.solve()
        return self._projStressField

    @property
    def projDensityField(self):
        """ Density Field projected on the mesh """
        self.densityField.data[...] = self._densityFn.evaluate(self.swarm)
        self._densityFieldProjector.solve()
        return self._projDensityField

    @property
    def densityField(self):
        """ Density Field on particles """
        self._densityField.data[:] = self._densityFn.evaluate(self.swarm)
        return self._densityField

    @property
    def surfaceProcesses(self):
        """ Surface processes handler """
        return self._surfaceProcesses

    @surfaceProcesses.setter
    def surfaceProcesses(self, value):
        self._surfaceProcesses = value
        if value:
            self._surfaceProcesses.timeField = self.timeField
            self._surfaceProcesses.Model = self

    def set_temperatureBCs(self, left=None, right=None,
                           top=None, bottom=None,
                           front=None, back=None,
                           nodeSets=None, materials=None):

        """ Define temperature boundaries condition

        A condition can be a temperature (float, int, Pint Quantity) or
        an Underworld function which evaluates as a temperature.

        parameters
        ----------

            Model: (UWGeodynamics.Model)
                An UWGeodynamics Model (See UWGeodynamics.Model)

            left:
                Define conditions on the left side of the Model.
                Conditions are defined for each Model direction (x, y, [z])

            right:
                Define conditions on the right side of the Model.
                Conditions are defined for each Model direction (x, y, [z])

            top:
                Define conditions on the top side of the Model.
                Conditions are defined for each Model direction (x, y, [z])

            bottom:
                Define conditions on the bottom side of the Model.
                Conditions are defined for each Model direction (x, y, [z])

            nodeSets: list of tuples: [(nodes, condition)]
                List of node where to apply predefined condition.

                The nodes can be a list or a numpy array containing the
                local index of the nodes. It can also be an Underworld
                IndexSet. You can also pass an UWGeodynamics shape.

                The condition can be an Underworld Function, A Pint
                Quantity of a scalar.

            materials: list of tuples: [(Material, condition)]
                List of material on which to apply a condition.
                The materials must be UWGeodynamics Material objects.

                The condition can be an Underworld Function, A Pint
                Quantity of a scalar.

            order_wall_conditions: list of str, [left, right, top, bottom,
                front, back]
                Order in which the boundaries are processed.

            Only valid for 3D Models:

            front:
                Define conditions on the front side of the Model.
                Conditions are defined for each Model direction (x, y, [z])

            back:
                Define conditions on the front side of the Model.
                Conditions are defined for each Model direction (x, y, [z])

        examples:

        Setting the temperature at the top of a model to be 500kelvin at the top
        and 1600kelvin at the bottom:

        >>> from underworld import UWGeodynamics as GEO
        >>> u = GEO.u
        >>> Model = GEO.Model()

        >>> ign = Model.set_temperatureBCs(top=500. * u.degK, bottom=1600. * u.degK)


        You can of course define temperatures on the sidewalls:

        >>> from underworld import UWGeodynamics as GEO
        >>> u = GEO.u
        >>> Model = GEO.Model()

        >>> ign = Model.set_temperatureBCs(right=500. * u.degK, left=1600. * u.degK)
        ...

        Fix the temperature of a Material

        >>> from underworld import UWGeodynamics as GEO #doctest: +ELLIPSIS
        >>> u = GEO.u
        >>> Model = GEO.Model()
        >>> air = Model.add_material(name="sky")
        >>> ign = Model.set_temperatureBCs(top=500. * u.degK,
        ...                          bottom=-0.022 * u.milliwatt / u.metre**2,
        ...                          materials=[(air, 273.*u.degK)])
        ...

        Fix the temperature of internal nodes

        You can assign a temperature to a list of nodes by passing a list of node indices (global).

        >>> from underworld import UWGeodynamics as GEO #doctest: +ELLIPSIS
        >>> u = GEO.u
        >>> Model = GEO.Model()
        >>> nodes = [0, 1, 2]
        >>> ign = Model.set_temperatureBCs(top=500. * u.degK,
        ...                          bottom=-0.022 * u.milliwatt / u.metre**2,
        ...                          nodeSets=[(nodes, 273.*u.degK)])
        ...


        """
        if not self._temperature:
            self._init_temperature_variables()

        self._temperatureBCs = TemperatureBCs(self, left=left, right=right,
                                              top=top, bottom=bottom,
                                              back=back, front=front,
                                              nodeSets=nodeSets,
                                              materials=materials)
        return self._temperatureBCs.get_conditions()

    def set_heatFlowBCs(self, left=None, right=None,
                        top=None, bottom=None,
                        front=None, back=None,nodeSets=None, materials=None):

        """ Define heat flow boundaries condition

        A condition can be a heat flow (float, int, Pint Quantity) or
        an Underworld function which evaluates as a heat flow.

        parameters
        ----------

            Model: (UWGeodynamics.Model)
                An UWGeodynamics Model (See UWGeodynamics.Model)

            left:
                Define conditions on the left side of the Model.
                Conditions are defined for each Model direction (x, y, [z])

            right:
                Define conditions on the right side of the Model.
                Conditions are defined for each Model direction (x, y, [z])

            top:
                Define conditions on the top side of the Model.
                Conditions are defined for each Model direction (x, y, [z])

            bottom:
                Define conditions on the bottom side of the Model.
                Conditions are defined for each Model direction (x, y, [z])

            nodeSets: list of tuples: [(nodes, condition)]
                List of node where to apply predefined condition.

                The nodes can be a list or a numpy array containing the
                local index of the nodes. It can also be an Underworld
                IndexSet. You can also pass an UWGeodynamics shape.

                The condition can be an Underworld Function, A Pint
                Quantity of a scalar.

            materials: list of tuples: [(Material, condition)]
                List of material on which to apply a condition.
                The materials must be UWGeodynamics Material objects.

                The condition can be an Underworld Function, A Pint
                Quantity of a scalar.

            order_wall_conditions: list of str, [left, right, top, bottom,
                front, back]
                Order in which the boundaries are processed.

            Only valid for 3D Models:

            front:
                Define conditions on the front side of the Model.
                Conditions are defined for each Model direction (x, y, [z])

            back:
                Define conditions on the front side of the Model.
                Conditions are defined for each Model direction (x, y, [z])

        example:
        --------

        Heat Flux can be assign as follow:

        >>> from underworld import UWGeodynamics as GEO

        >>> u = GEO.u

        >>> Model = GEO.Model()
        >>> Material = Model.add_material(shape=GEO.shapes.Layer(top=Model.top,
        ...                                               bottom=Model.bottom))
        >>> Material.capacity = 1e3 * u.joule / (u.degK * u.kg)
        >>> Material.density  = 2e3 * u.kg * u.m**-3
        >>> ign = Model.set_heatFlowBCs(bottom=(-0.22 * u.milliwatt / u.metre**2,
        ...                               Material))
        ...
        """
        if not self._temperature:
            self._init_temperature_variables()
        
        self._heatFlowBCs = HeatFlowBCs(self, left=left, right=right,
                                        top=top, bottom=bottom,
                                        back=back, front=front,nodeSets=nodeSets,materials=materials)
        return self._heatFlowBCs.get_conditions()

    @property
    def temperatureBCs(self):
        if self._temperatureBCs:
            return self._temperatureBCs.get_conditions()
        else:
            return self._temperatureBCs

    @temperatureBCs.setter
    def temperatureBCs(self, value):
        self._temperatureBCs = value

    @property
    def heatFlowBCs(self):
        return self._heatFlowBCs.get_conditions()

    @heatFlowBCs.setter
    def heatFlowBCs(self, value):
        self._heatFlowBCs = value

    @property
    def velocityBCs(self):
        return self._velocityBCs.get_conditions()

    @velocityBCs.setter
    def velocityBCs(self, value):
        self._velocityBCs = value

    @property
    def stressBCs(self):
        return self._stressBCs.get_conditions()

    @stressBCs.setter
    def stressBCs(self, value):
        self._stressBCs = value

    @property
    def solver(self):
        # Get a solver
        # In some cases, it is necessary to rebuild the solver,
        # This can be done by switching the _rebuild_solver flag
        # to true.
        if not self._solver or self._rebuild_solver:
            if self._rebuild_solver and not self.step == 0:
                # Save current options in a dictionary
                options = _solver_options_dictionary(self._solver)
                # Rebuild solver
                self._solver = uw.systems.Solver(self.stokes_SLE)
                # Apply saved options on *new* solver
                _apply_saved_options_on_solver(self._solver, options)
            else:
                self._solver = uw.systems.Solver(self.stokes_SLE)
        return self._solver

    @property
    def temperature(self):
        """ Temperature Field """
        return self._temperature
    
    @temperature.setter
    def temperature(self, value):
        """ Temperature Field """
        if not self._temperature:
            self._init_temperature_variables()
        if isinstance(value, fn.Function):
            self._temperature.data[...] = value.evaluate(self.mesh)

    @property
    def _advdiffSystem(self):
        """ Advection Diffusion System """

        DiffusivityMap = {}
        for material in self.materials:
            if material.diffusivity:
                DiffusivityMap[material.index] = nd(material.diffusivity)

        self.DiffusivityFn = fn.branching.map(fn_key=self.materialField,
                                              mapping=DiffusivityMap,
                                              fn_default=nd(self.diffusivity))

        HeatProdMap = {}
        for material in self.materials:
            if all([material.density,
                    material.capacity,
                    material.radiogenicHeatProd]):

                HeatProdMap[material.index] = (
                    nd(material.radiogenicHeatProd) /
                    (self._densityFn * nd(material.capacity)))
            else:
                HeatProdMap[material.index] = 0.

        self.HeatProdFn = fn.branching.map(fn_key=self.materialField,
                                           mapping=HeatProdMap)

        # Add Viscous dissipation Heating
        if rcParams["shear.heating"]:
            stress = fn.tensor.second_invariant(self._stressFn)
            strain = self.strainRate_2ndInvariant
            self.HeatProdFn += stress * strain

        conditions = []
        conditions.append(self.temperatureBCs)
        if self._heatFlowBCs:
            conditions.append(self.heatFlowBCs)
        
        obj = uw.systems.AdvectionDiffusion(
                method=rcParams["advection.diffusion.method"],
                phiField=self.temperature,
                phiDotField=self._temperatureDot,
                velocityField=self.velocityField,
                fn_diffusivity=self.DiffusivityFn,
                fn_sourceTerm=self.HeatProdFn,
                conditions=conditions
        )

        return obj

    @property
    def _buoyancyFn(self):
        gravity = tuple([nd(val) for val in self.gravity])
        return self._densityFn * gravity

    @property
    def stokes_SLE(self):
        """ Stokes SLE """

        if any([material.viscosity for material in self.materials]):

            conditions = list()
            conditions.append(self.velocityBCs)

            if self._stressBCs:
                conditions.append(self.stressBCs)

            fssa = None
            if self._fssa_factor:
                fssa = self._fssa

            self._stokes_SLE = uw.systems.Stokes(
                velocityField=self.velocityField,
                pressureField=self.pressureField,
                conditions=conditions,
                voronoi_swarm=self.voronoi_swarm,
                _fn_fssa=fssa, 
                fn_viscosity=self._viscosityFn,
                fn_bodyforce=self._buoyancyFn,
                fn_stresshistory=self._elastic_stressFn,
                fn_one_on_lambda=self._lambdaFn)

        return self._stokes_SLE

    def _init_melt_fraction(self):
        """ Initialize the Melt Fraction Field """

        # Initialize Melt Fraction to material property
        meltFractionMap = {}
        for material in self.materials:
            if material.meltFraction:
                meltFractionMap[material.index] = material.meltFraction

        if meltFractionMap:
            InitFn = fn.branching.map(fn_key=self.materialField,
                                      mapping=meltFractionMap, fn_default=0.0)
            self.meltField.data[:] = InitFn.evaluate(self.swarm)

    def set_velocityBCs(self, left=None, right=None, top=None, bottom=None,
                        front=None, back=None, nodeSets=None,
                        order_wall_conditions=None):
        """ Set kinematic boundary conditions

        A condition can be a velocity (float, int, Pint Quantity) or
        an Underworld function which evaluates as a velocity.

        parameters
        ----------

        Model: (UWGeodynamics.Model)
            An UWGeodynamics Model (See UWGeodynamics.Model)

        left:(tuple)
            Define kinematic conditions on the left side of the Model.
            Conditions are defined for each Model direction (x, y, [z])

        right:(tuple)
            Define kinematic conditions on the right side of the Model.
            Conditions are defined for each Model direction (x, y, [z])

        top:(tuple)
            Define kinematic conditions on the top side of the Model.
            Conditions are defined for each Model direction (x, y, [z])

        bottom:(tuple)
            Define kinematic conditions on the bottom side of the Model.
            Conditions are defined for each Model direction (x, y, [z])

        nodeSets: list of tuples: [(nodes, condition)]
            List of node where to apply predefined condition.

            The nodes can be a list or a numpy array containing the
            local index of the nodes. It can also be an Underworld
            IndexSet. You can also pass an UWGeodynamics shape.

            The condition can be an Underworld Function, A Pint
            Quantity of a scalar.

        materials: list of tuples: [(Material, condition)]
            List of material on which to apply a condition.
            The materials must be UWGeodynamics Material objects.

            The condition can be an Underworld Function, A Pint
            Quantity of a scalar.

        order_wall_conditions: list of str, [left, right, top, bottom, front, back]
            Order in which the boundaries are processed.

        Only valid for 3D Models:

        front:(tuple) with length 2 in 2D, and length 3 in 3D.
            Define kinematic conditions on the front side of the Model.
            Conditions are defined for each Model direction (x, y, [z])

        back:(tuple) with length 2 in 2D, and length 3 in 3D.
            Define kinematic conditions on the front side of the Model.
            Conditions are defined for each Model direction (x, y, [z])

        examples:
        ---------

        The following example defines a (2x1) meter Underworld model with
        freeslip conditions on all the sides.

        >>> from underworld import UWGeodynamics as GEO
        >>> u = GEO.u
        >>> Model = GEO.Model(elementRes=(64, 64),
        ...                   minCoord=(-1. * u.meter, -50. * u.centimeter),
        ...                   maxCoord=(1. * u.meter, 50. * u.centimeter))
        >>> velocityBCs = Model.set_velocityBCs(left=[0, None],
        ...                                     right=[0,None],
        ...                                     top=[None,0],
        ...                                     bottom=[None, 0])
        """

        self._velocityBCs = VelocityBCs(
            self, left=left, right=right, top=top,
            bottom=bottom, front=front,
            back=back, nodeSets=nodeSets,
            order_wall_conditions=order_wall_conditions)
        return self._velocityBCs.get_conditions()

    set_kinematicBCs = set_velocityBCs

    def set_stressBCs(self, left=None, right=None, top=None, bottom=None,
                      front=None, back=None, nodeSets=None,
                      order_wall_conditions=None):
        """ Set stress boundary conditions

        A condition can be a stress (float, int, Pint value) or an
        Underworld function which evaluates as a stress.

        parameters
        ----------

            Model: (UWGeodynamics.Model)
                An UWGeodynamics Model (See UWGeodynamics.Model)

            left:(tuple)
                Define conditions on the left side of the Model.
                Conditions are defined for each Model direction (x, y, [z])

            right:(tuple)
                Define conditions on the right side of the Model.
                Conditions are defined for each Model direction (x, y, [z])

            top:(tuple)
                Define conditions on the top side of the Model.
                Conditions are defined for each Model direction (x, y, [z])

            bottom:(tuple)
                Define conditions on the bottom side of the Model.
                Conditions are defined for each Model direction (x, y, [z])

            nodeSets: list of tuples: [(nodes, condition)]
                List of node where to apply predefined condition.

                The nodes can be a list or a numpy array containing the
                local index of the nodes. It can also be an Underworld
                IndexSet. You can also pass an UWGeodynamics shape.

                The condition can be an Underworld Function, A Pint
                Quantity of a scalar.

            materials: list of tuples: [(Material, condition)]
                List of material on which to apply a condition.
                The materials must be UWGeodynamics Material objects.

                The condition can be an Underworld Function, A Pint
                Quantity of a scalar.

            order_wall_conditions: list of str, [left, right, top, bottom,
                front, back]
                Order in which the boundaries are processed.

            Only valid for 3D Models:

            front:(tuple) with length 2 in 2D, and length 3 in 3D.
                Define mechanical conditions on the front side of the Model.
                Conditions are defined for each Model direction (x, y, [z])

            back:(tuple) with length 2 in 2D, and length 3 in 3D.
                Define mechanical conditions on the front side of the Model.
                Conditions are defined for each Model direction (x, y, [z])

        """

        self._stressBCs = StressBCs(self, left=left,
                                    right=right, top=top,
                                    bottom=bottom, front=front,
                                    back=back, nodeSets=nodeSets,
                                    order_wall_conditions=order_wall_conditions)
        return self._stressBCs.get_conditions()

    def add_material(self, material=None, shape=None,
                     name="unknown", fill=True, reset=False):
        """ Add Material to the Model

        Parameters:
        -----------
            material:
                An UWGeodynamics material. If a material is created.
            shape:
                Shape of the material. See UWGeodynamics.shape
                Or Underworld function returning 0 or 1
            name:
                Material name
            reset: (bool)
                Reset the material Field before adding the new
                material. Default is False.

        """

        if reset:
            self.materialField.data[:] = self.index

        mat = material if material else Material()
        mat.name = material.name if (material and material.name) else name

        mat.Model = self
        mat.diffusivity = self.diffusivity
        mat.capacity = self.capacity
        mat.radiogenicHeatProd = self.radiogenicHeatProd

        if isinstance(shape, shapes.Layer):
            if self.mesh.dim == 3:
                shape = shapes.Layer3D(top=shape.top, bottom=shape.bottom)

        if hasattr(shape, "top"):
            mat.top = shape.top
        if hasattr(shape, "bottom"):
            mat.bottom = shape.bottom

        mat.shape = shape
        self.materials.reverse()
        self.materials.append(mat)
        self.materials.reverse()

        if mat.shape:
            condition = [(mat.shape, mat.index), (True, self.materialField)]
            func = fn.branching.conditional(condition)
            self.materialField.data[:] = func.evaluate(self.swarm)

        return mat

    def add_swarm_variable(self, name, dataType="double", count=1,
                           init_value=0., projected="mesh",
                           restart_variable=False, **kwargs):
        """Add a new swarm field to the model

        Parameters
        ----------

            name : str
                name of the swarm field
            dataType : str
                type of data to be recorded, default is "double"
            count : int
                degree of freedom, default is 1
            init_value : float
                default value of the field, default is to initialise
                the field to 0.
            projected : str
                the function creates a projector for each new
                swarm variable, you can choose to project on the "mesh" or
                "submesh"
            restart_variable: bool
                specifies if the variable is needed for a restart.

        Returns
        -------

            Swarm Variable
        """

        newField = self.swarm.add_variable(dataType, count, **kwargs)
        setattr(self, name, newField)
        newField.data[...] = init_value
        self.swarm_variables[name.strip("_")] = newField

        # Create mesh variable for projection
        if name.startswith("_"):
            proj_name = "_proj" + name[1].upper() + name[2:]
        else:
            proj_name = "_proj" + name[0].upper() + name[1:]

        if projected == "mesh":
            projected = self.add_mesh_variable(proj_name,
                                            nodeDofCount=count,
                                            dataType="double")
        else:
            projected = self.add_submesh_field(proj_name,
                                               nodeDofCount=count,
                                               dataType="double")

        # Create a projector
        if name.startswith("_"):
            projector_name = name + "Projector"
        else:
            projector_name = "_" + name + "Projector"
        if self.elementType.split("/")[0] != "Q2":
            voronoi_swarm = self.swarm
        else:
            voronoi_swarm = None
        projector = uw.utils.MeshVariable_Projection(projected,
                                                     newField,
                                                     voronoi_swarm=voronoi_swarm,
                                                     type=0)
        setattr(self, projector_name, projector)

        if restart_variable:
            self.restart_variables[name] = newField

        return newField

    def add_mesh_variable(self, name, nodeDofCount=1,
                          dataType="double", init_value=0.,
                          restart_variable=False, **kwargs):
        """Add a new mesh field to the model

        Parameters
        ----------

        name : str
            name of the mesh field
        nodeDofCount : int
            degree of freedom, default is 1
        dataType : str
            type of data to be recorded, default is "double"
        init_value : float
            default value of the field, default is to initialise the field to 0.
        restart_variable: bool,
            specifies if the variable is needed for a restart.

        Returns
        -------
        Mesh Variable
        """
        newField = self.mesh.add_variable(nodeDofCount, dataType, **kwargs)
        setattr(self, name, newField)
        newField.data[...] = init_value
        self.mesh_variables[name.strip("_")] = newField
        if restart_variable:
            self.restart_variables[name] = newField
        return newField

    def add_submesh_field(self, name, nodeDofCount=1,
                          dataType="double", init_value=0.,
                          restart_variable=False, **kwargs):
        """Add a new sub-mesh field to the model

        Parameters
        ----------

        name : str
            name of the mesh field
        nodeDofCount : int
            degree of freedom, default is 1
        dataType : str
            type of data to be recorded, default is "double"
        init_value : float
            default value of the field, default is to initialise the field to 0.
        restart_variable: bool,
            specifies if the variable is needed for a restart.

        Returns
        -------
        Mesh Variable
        """
        newField = MeshVariable(self.mesh.subMesh, nodeDofCount,
                                dataType, **kwargs)
        setattr(self, name, newField)
        newField.data[...] = init_value
        self.mesh_variables[name.strip("_")] = newField
        if restart_variable:
            self.restart_variables[name] = newField
        return newField

    @property
    def _densityFn(self):
        """Density Function Builder"""
        densityMap = {}
        for material in self.materials:

            density = material.density
            if isinstance(material.density, Density):
                density.temperatureField = self.temperature
                density.pressureField = self.pressureField
                densityMap[material.index] = density.effective_density()
            else:
                densityMap[material.index] = density

            if material.meltExpansion:
                fact = material.meltExpansion * self.meltField
                densityMap[material.index] = (
                    densityMap[material.index] * (1.0 - fact))

        return fn.branching.map(fn_key=self.materialField, mapping=densityMap)

    def set_frictional_boundary(self, right=None, left=None,
                                top=None, bottom=None, front=None,
                                back=None, thickness=2):
        """ Set Frictional Boundary conditions

        Frictional boundaries are implemented as a thin layer of frictional
        material along the chosen boundaries.

        Parameters:
        -----------
        Returns:
        --------
            Underworld mesh variable that maps the boundaries.
            (Values are set to 1 if the mesh element applies
            a frictional condition, 0 otherwise).
        """

        self.frictionalBCs = FrictionBoundaries(self, rightFriction=right,
                                                leftFriction=left,
                                                topFriction=top,
                                                bottomFriction=bottom,
                                                frontFriction=front,
                                                backFriction=back,
                                                thickness=thickness)

        return self.frictionalBCs

    @property
    def _viscosityFn(self):
        return self._viscosity_processor.get_effective_eta(
            rcParams["averaging.method"])

    @property
    def _stressFn(self):
        """Stress Function Builder"""
        viscousStressFn = self._viscous_stressFn()
        elasticStressFn = self._elastic_stressFn
        return  viscousStressFn + elasticStressFn

    def _viscous_stressFn(self):
        """Viscous Stress Function Builder"""
        return 2. * self.viscosityField * self.strainRate

    @property
    def _elastic_stressFn(self):
        """ Elastic Stress Function Builder"""
        if any([material.elasticity for material in self.materials]):
            stressMap = {}
            for material in self.materials:
                if material.elasticity:
                    ElasticityHandler = material.elasticity
                    ElasticityHandler.viscosity = self._viscosityFn
                    ElasticityHandler.previousStress = self._previousStressField
                    elasticStressFn = ElasticityHandler.elastic_stress
                else:
                    # Set elastic stress to zero if material
                    # has no elasticity
                    elasticStressFn = [0.0] * 3 if self.mesh.dim == 2 else [0.0] * 6
                stressMap[material.index] = elasticStressFn
            return (fn.branching.map(fn_key=self.materialField,
                                     mapping=stressMap))
        else:

            elasticStressFn = [0.0] * 3 if self.mesh.dim == 2 else [0.0] * 6
            return elasticStressFn

    def _update_stress_history(self, dt):
        """Update Previous Stress Field"""
        dt_e = []
        for material in self.materials:
            if material.elasticity:
                dt_e.append(nd(material.elasticity.observation_time))
        dt_e = np.array(dt_e).min()
        phi = dt / dt_e
        veStressFn_data = self._stressFn.evaluate(self.swarm)

        self._previousStressField.data[:] = (  
                                        phi  * veStressFn_data[:]
                                  + (1.-phi) * self._previousStressField.data[:]
                                             )

    def _phaseChangeFn(self):
        for material in self.materials:
            if material.phase_changes:
                for change in material.phase_changes:
                    obj = change
                    mask = obj.fn().evaluate(self.swarm)
                    conds = ((mask == 1) &
                             (self.materialField.data == material.index))
                    self.materialField.data[conds] = obj.result

    def solve_temperature_steady_state(self):
        """ Solve for steady state temperature

        Returns:
        --------
            Updated temperature Field
        """

        if self.materials:

            DiffusivityMap = {}
            for material in self.materials:
                if material.diffusivity:
                    DiffusivityMap[material.index] = nd(material.diffusivity)

            self.DiffusivityFn = fn.branching.map(
                fn_key=self.materialField,
                mapping=DiffusivityMap,
                fn_default=nd(self.diffusivity)
            )

            HeatProdMap = {}
            for material in self.materials:

                if all([material.density,
                        material.capacity,
                        material.radiogenicHeatProd]):

                    HeatProdMap[material.index] = (
                        nd(material.radiogenicHeatProd) /
                        self._densityFn  /
                        nd(material.capacity)
                    )

                else:
                    HeatProdMap[material.index] = 0.

                # Melt heating
                if material.latentHeatFusion and self.dt.value:
                    dynamicHeating = self._get_dynamic_heating(material)
                    HeatProdMap[material.index] += dynamicHeating

            self.HeatProdFn = fn.branching.map(fn_key=self.materialField,
                                               mapping=HeatProdMap)
        else:
            self.DiffusivityFn = fn.misc.constant(nd(self.diffusivity))
            self.HeatProdFn = fn.misc.constant(nd(self.radiogenicHeatProd))

        conditions = []
        conditions.append(self.temperatureBCs)
        if self._heatFlowBCs:
            conditions.append(self.heatFlowBCs)

        heatequation = uw.systems.SteadyStateHeat(
            temperatureField=self.temperature,
            fn_diffusivity=self.DiffusivityFn,
            fn_heating=self.HeatProdFn,
            conditions=conditions
        )

        heatsolver = uw.systems.Solver(heatequation)
        heatsolver.solve(nonLinearIterate=True)

        return self.temperature

    def initialize_pressure_to_lithostatic(self):
        """ Calculate the lithostatic Pressure Field

        Returns:
        --------
            Pressure Field (MeshVariable), array containing the pressures
            at the bottom of the Model
        """

        self.pressureField.data[...] = (
            self.lithostatic_pressureField.evaluate(self.mesh.subMesh)
        )
        self.pressSmoother.smooth()

        return self.pressureField

    @property
    def lithostatic_pressureField(self):
        """
        Returns:
        --------
            Lithostatic pressure field as a mesh variable

        """
        gravity = np.abs(nd(self.gravity[-1]))
        return Lithostatic_pressure(self.mesh, self._densityFn, gravity)

    def _calibrate_pressureField(self):
        """ Pressure Calibration callback function """

        surfaceArea = uw.utils.Integral(fn=1.0, mesh=self.mesh,
                                        integrationType='surface',
                                        surfaceIndexSet=self.top_wall)

        surfacepressureFieldIntegral = uw.utils.Integral(
            fn=self.pressureField,
            mesh=self.mesh,
            integrationType='surface',
            surfaceIndexSet=self.top_wall
        )
        area, = surfaceArea.evaluate()
        p0, = surfacepressureFieldIntegral.evaluate()
        offset = p0 / area
        self.pressureField.data[:] -= offset

    def _get_material_indices(self, material):
        """ Get mesh indices of a Material

        Parameter:
        ----------
            material:
                    The Material of interest
        Returns;
        --------
            underworld IndexSet

        """
        from decimal import Decimal
        mat = self.projMaterialField.evaluate(self.mesh)
        # Get nodes corresponding to material
        mask = (mat == Decimal(material.index))
        nodes = np.arange(0, self.mesh.nodesDomain)[mask.flatten()]
        return uw.mesh.FeMesh_IndexSet(self.mesh, topologicalIndex=0,
                                       size=self.mesh.nodesGlobal,
                                       fromObject=nodes)

    def solve(self):
        """ Solve Stokes """

        self.nlstep = 0

        if self.step == 0:
            self._curTolerance = rcParams["initial.nonlinear.tolerance"]
            minIterations = rcParams["initial.nonlinear.min.iterations"]
            maxIterations = rcParams["initial.nonlinear.max.iterations"]
        else:
            self._curTolerance = rcParams["nonlinear.tolerance"]
            minIterations = rcParams["nonlinear.min.iterations"]
            maxIterations = rcParams["nonlinear.max.iterations"]

        self.solver.solve(
            nonLinearIterate=True,
            nonLinearMinIterations=minIterations,
            nonLinearMaxIterations=maxIterations,
            callback_post_solve=self._callback_post_solve,
            nonLinearTolerance=self._curTolerance)

        self._solution_exist.value = True

        if rcParams["rebuild.solver"]:
            self._solver = False

    def _init_temperature_variables(self):
        self._temperature = MeshVariable(mesh=self.mesh,
                                         nodeDofCount=1)
        self._temperatureDot = MeshVariable(mesh=self.mesh,
                                            nodeDofCount=1)
        self._heatFlux = MeshVariable(mesh=self.mesh,
                                      nodeDofCount=1)
        self._temperatureDot.data[...] = 0.
        self._heatFlux.data[...] = 0.
        self.mesh_variables["temperature"] = self._temperature
        self.restart_variables["temperature"] = self._temperature

    def init_model(self, temperature="steady-state", pressure="lithostatic",
                   defaultStrainRate=1e-15 / u.second):
        """ Initialise the Temperature Field as steady state,
            Initialise the Pressure Field as Lithostatic
            Initialise the viscosity field based on default
            strain rate.

            Will only initialise the temperature field if diffusivity is defined in the model.

        Parameters:
        -----------
            temperature: defaults to "steady-state", can be an Underworld function
            pressure: defaults to "lithostatic", can be an Underworld function  

        example:
        --------

        >>> from underworld import UWGeodynamics as GEO
        >>> u = GEO.u

        >>> Model = GEO.Model()
        >>> Model.density = 2000. * u.kilogram / u.metre**3
        >>> Model.diffusivity = 9e-7 * u.metre**2 / u.second 
        >>> ign = Model.set_temperatureBCs(top=273.15 * u.degK, bottom=1603.15 * u.degK)
        >>> Model.init_model(temperature="steady-state", pressure="lithostatic")
        ...

        """
        if temperature is None:
            import warnings
            warnings.warn("You have not passed anything to the temperature argument. The temperature field will not be initialised")
        
        if pressure is None:
            import warnings
            warnings.warn("You have not passed anything to the pressure argument. The pressure field will not be initialised")

        # Init Temperature Field only if DiffusivityFn or deffusivity are defined
        if temperature is not None:
            if not self._temperature:
                self._init_temperature_variables()
            if isinstance(temperature, fn.Function):
                self._temperature.data[...] = temperature.evaluate(self.mesh)
            elif isinstance(temperature, str) and temperature.lower() == "steady-state": 
                if not (self.DiffusivityFn or self.diffusivity):
                    import warnings
                    warnings.warn("Skipping the steady state calculation: No diffusivity variable defined on Model")
                else:
                    self.solve_temperature_steady_state()

        # Init pressureField Field
        if pressure is not None:
            if self.pressureField:
                if isinstance(pressure, fn.Function):
                    self.pressureField.data[...] = pressure.evaluate(self.mesh.subMesh)
                elif isinstance(pressure, str) and pressure.lower() == "lithostatic":
                    self.initialize_pressure_to_lithostatic()

        # Init ViscosityField
        if any([material.viscosity for material in self.materials]):
            self.defaultStrainRate = defaultStrainRate
            self.viscosityField

        return

    def run_for(self, duration=None, checkpoint_interval=None, nstep=None,
                checkpoint_times=None, restart_checkpoint=1, dt=None,
                restartStep=None, restartDir=None, output_units=None):
        """ Run the Model

        Parameters
        ----------

        duration :
            Model time in units of time.
        checkpoint_interval :
            Checkpoint interval time.
        nstep :
            Number of steps to run.`
        checkpoint_times :
            Specify a list of additional Checkpoint times ([Time])
        restart_checkpoint :
            This parameter specify how often the swarm and swarm variables
            are checkpointed. A value of 1 means that the swarm and its
            associated variables are saved at every checkpoint.
            A value of 2 results in saving only every second checkpoint.
        dt :
            Specify the time interval (dt) to be used in
            units of time.
        restartStep :
            Restart Model. int (step number)
        restartDir :
            Restart Directory.
        output_units:
            Units used in output. If None, the units of the checkpoint_interval
            are used, if the latter does not have units, defaults to
            rcParams["time.SIunits"]

        """

        self.stepDone = 0
        self.restart(restartStep, restartDir)

        ndduration = self._ndtime + nd(duration) if duration else None

        output_dt_units = _get_output_units(
            output_units, checkpoint_interval, duration)

        checkpointer = _CheckpointFunction(
            self, duration, checkpoint_interval,
            checkpoint_times, restart_checkpoint, output_dt_units)

        if not nstep:
            nstep = self.stepDone

        if dt:
            user_dt = nd(dt)
            self.dt = user_dt
        else:
            user_dt = None

        if self.fssa_factor is not None and not user_dt:
            raise RuntimeError("You have switched FSSA on and have not set a dt.")

        if rank == 0:
            sys.stdout.flush()

        try:
            if self.solver.print_petsc_options():
                print("""Petsc {0}""".format(self.solver.print_petsc_options()))
                sys.stdout.flush()
        except AttributeError:
            pass

        while (ndduration and self._ndtime < ndduration) or self.stepDone < nstep:

            self._pre_solve()

            self.solve()

            self.dt = 2.0 * rcParams["CFL"] * self.swarm_advector.get_max_dt()

            if self.temperatureBCs:
                # Only get a condition if using SUPG
                if rcParams["advection.diffusion.method"] == "SUPG":
                    supg_dt = self._advdiffSystem.get_max_dt()
                    supg_dt *= 2.0 * rcParams["CFL"]
                    self.dt = fn.misc.min(self.dt, supg_dt)

            if duration:
                self.dt = fn.misc.min(self.dt, ndduration - self._ndtime)

            if user_dt:
                self.dt = fn.misc.min(self.dt, user_dt)

            check_dt = checkpointer.get_next_checkpoint_time()
            if check_dt:
                self.dt = fn.misc.min(self.dt, check_dt)

            dte = []
            for material in self.materials:
                if material.elasticity:
                    dte.append(nd(material.elasticity.observation_time))

            if dte:
                dte = np.array(dte).min()
                # Cap dt for observation time, dte / 3.
                if dte and self.dt.value > (dte / 3.):
                    self.dt = dte / 3.

            comm.Barrier()



            self._update()

            self.step += 1
            self.stepDone += 1
            self._ndtime += self.dt.value

            checkpointer.checkpoint()

            if rank == 0:
                string = """Step: {0:5d} Model Time: {1:6.1f} dt: {2:6.1f} ({3})\n""".format(
                    self.stepDone, _adjust_time_units(self.time),
                    _adjust_time_units(self.dt.value),
                    datetime.now().strftime('%Y-%m-%d %H:%M:%S'))
                sys.stdout.write(string)
                sys.stdout.flush()

            self._post_solve()

        return 1

    def _pre_solve(self):
        """ Entry point for functions to be run before attempting a solve """
        for key, val in self.pre_solve_functions.items():
            if not callable(val):
                raise ValueError("""The function {0} must be
                                 callable""".format(key))
            val()

    def _post_solve(self):
        """ Entry point for functions to be run after the solve """
        for key, val in self.post_solve_functions.items():
            if not callable(val):
                raise ValueError("""The function {0} must be
                                 callable""".format(key))
            val()

    def _callback_post_solve(self):
        if rcParams["surface.pressure.normalization"]:
            self._calibrate_pressureField()
        if rcParams["pressure.smoothing"]:
            self.pressSmoother.smooth()
        if self._isostasy:
            self._isostasy.solve()
        for material in self.materials:
            if material.viscosity:
                material.viscosity.firstIter.value = False
        for key, val in self.callback_functions.items():
            if not callable(val):
                raise ValueError("""The function {0} must be
                                 callable""".format(key))
            else:
                val()
        self._solution_exist.value = True
        self.nlstep += 1

    def _update(self):
        """ Update Function

        The following function processes the mesh and swarm variables
        between two solves. It takes care of mesh, swarm advection and
        update the fields according to the Model state.

        """


        dt = self.dt.value

        # Heal plastic strain
        if any([material.healingRate for material in self.materials]):
            healingRates = {}
            for material in self.materials:
                healingRates[material.index] = nd(material.healingRate)
            HealingRateFn = fn.branching.map(fn_key=self.materialField,
                                             mapping=healingRates)

            self.plasticStrain.data[:] -= (dt * HealingRateFn).evaluate(self.swarm)
            self.plasticStrain.data[self.plasticStrain.data < 0.] = 0.

        # Increment plastic strain
        _isYielding = self._viscosity_processor._isYielding
        plasticStrainIncrement = (dt * _isYielding).evaluate(self.swarm)
        self.plasticStrain.data[:] += plasticStrainIncrement

        if any([material.melt for material in self.materials]):
            # Calculate New meltField
            self.update_melt_fraction()

        # Solve for temperature
        if self.temperatureBCs:
            self._advdiffSystem.integrate(dt)

        if self._advector:
            self.swarm_advector.integrate(dt)
            self._advector.advect_mesh(dt)
        elif self._freeSurface:
            self.swarm_advector.integrate(dt, update_owners=False)
            self._freeSurface.solve(dt)
            self.swarm.update_particle_owners()
        else:
            # Integrate Swarms in time
            self.swarm_advector.integrate(dt, update_owners=True)

        # Update stress
        if any([material.elasticity for material in self.materials]):
            self._update_stress_history(dt)

        if self.passive_tracers:
            for key, val in self.passive_tracers.items():
                if val.advector:
                    val.advector.integrate(dt)

        # Do pop control
        self.population_control.repopulate()
        self.swarm.update_particle_owners()

        if self.surfaceProcesses:
            self.surfaceProcesses.solve(dt)

        # Update Time Field
        self.timeField.data[...] += dt

        if self._visugrid:
            self._visugrid.advect(dt)

        self._phaseChangeFn()

    def mesh_advector(self, axis):
        """ Initialize the mesh advector

        Parameters:
        -----------
            axis:
                list of axis (or degree of freedom) along which the
                mesh is allowed to deform
        """
        self._advector = Mesh_advector(self, axis)

    def add_passive_tracers(self, name, vertices,
                            particleEscape=True, centroids=None,
                            advect=True):
        """ Add a swarm of passive tracers to the Model

        Parameters:
        -----------
            name :
                Name of the swarm of tracers.
            vertices :
                nD Numpy array that contains the coordinates of the tracers.
            particleEscape : (bool)
                Allow or prevent tracers from escaping the boundaries of the
                Model (default to True)
            centroids : if a list of centroids is provided, the pattern defined
                by the vertices is reproduced around each centroid.

        example:
        --------

        >>> from underworld import UWGeodynamics as GEO
        >>> import numpy as np

        >>> u = GEO.u

        >>> Model = GEO.Model()
        >>> npoints = 1000
        >>> x = np.linspace(GEO.nd(Model.minCoord[0]), GEO.nd(Model.maxCoord[0]), npoints)
        >>> y = GEO.nd(32. * u.kilometre)
        >>> coords = np.ndarray((npoints, 2))
        >>> coords[:, 0] = x
        >>> coords[:, 1] = y
        >>> Model.add_passive_tracers(name='test', vertices=coords)

        You can pass a list of centroids to the Model.add_passive_tracers method.
        In that case, the coordinates of the passive tracers are relative
        to the position of the centroids. The pattern is repeated around
        each centroid.

        >>> from underworld import UWGeodynamics as GEO
        >>> import numpy as np

        >>> u = GEO.u
        >>> Model = GEO.Model()
        >>> cxpos = np.linspace(GEO.nd(20*u.kilometer), GEO.nd(40*u.kilometer), 5)
        >>> cypos = np.linspace(GEO.nd(20*u.kilometer), GEO.nd(40*u.kilometer), 5)
        >>> cxpos, cypos = np.meshgrid(cxpos, cypos)

        >>> coords = np.ndarray((cxpos.size, 2))
        >>> coords[:, 0] = cxpos.ravel()
        >>> coords[:, 1] = cypos.ravel()
        >>> Model.add_passive_tracers(name='test', vertices=np.array([[0,0]]), centroids=coords)


        We provide a function to create circles on a grid:

        >>> from underworld import UWGeodynamics as GEO

        >>> pts = GEO.circles_grid(radius = 2.0 * u.km,
        ...                 minCoord=[0.*u.km, 0.*u.km],
        ...                 maxCoord=[10.*u.km, 10.*u.km])
        ...

        """

        if centroids is None:
            tracers = PassiveTracers(self.mesh,
                                     name=name,
                                     particleEscape=particleEscape)
            tracers.add_particles_with_coordinates(vertices)

        else:
            x = np.array(vertices[:, 0])[..., np.newaxis] + np.array(centroids[:, 0]).ravel()
            y = np.array(vertices[:, 1])[..., np.newaxis] + np.array(centroids[:, 1]).ravel()
            x = x.ravel()
            y = y.ravel()

            if self.mesh.dim == 2:
                vertices = np.ndarray((x.size, 2))
                vertices[:, 0] = x
                vertices[:, 1] = y
            else:
                z = np.array(vertices[:,2])[..., np.newaxis]  + np.array(centroids[:,2]).ravel()
                z = z.ravel()
                vertices = np.ndarray((x.size, 3))
                vertices[:, 0] = x
                vertices[:, 1] = y
                vertices[:, 2] = z

            tracers = PassiveTracers(self.mesh,
                                     name=name,
                                     particleEscape=particleEscape)
            tracers.add_particles_with_coordinates(vertices)

        if advect:
            tracers.advector = uw.systems.SwarmAdvector(
                self.velocityField, tracers, order=2)

        self.passive_tracers[name] = tracers
        setattr(self, name + "_tracers", tracers)
        return

    def _get_melt_fraction(self):
        """ Melt Fraction function

        Returns:
        -------
            Underworld function that calculates the Melt fraction on the
            particles.

        """
        meltMap = {}
        for material in self.materials:
            if material.melt:
                T_s = material.solidus.temperature(self.pressureField)
                T_l = material.liquidus.temperature(self.pressureField)
                T_ss = (self.temperature - 0.5 * (T_s + T_l)) / (T_l - T_s)
                value = (0.5 + T_ss + (T_ss * T_ss - 0.25) *
                         (0.4256 + 2.988 * T_ss))
                conditions = [((-0.5 < T_ss) & (T_ss < 0.5),
                               fn.misc.min(value, material.meltFractionLimit)),
                              (True, 0.0)]
                meltMap[material.index] = fn.branching.conditional(conditions)

        return fn.branching.map(fn_key=self.materialField,
                                mapping=meltMap, fn_default=0.0)

    def update_melt_fraction(self):
        """ Calculate New meltField """
        meltFraction = self._get_melt_fraction()
        self.meltField.data[:] = meltFraction.evaluate(self.swarm)

    def _get_dynamic_heating(self, material):
        """ Calculate additional heating source due to melt

        Returns:
        --------
            Underworld function

        """

        ratio = material.latentHeatFusion / material.capacity

        if not ratio.dimensionless:
            raise ValueError("""Unit Error in either Latent Heat Fusion or
                             Capacity (Material: """ + material.name)
        ratio = ratio.magnitude

        dF = (self._get_melt_fraction() - self.meltField) / self.dt
        return (ratio * dF) * self.temperature

    @property
    def _lambdaFn(self):
        """ Initialize compressibility """

        materialMap = {}
        if any([material.compressibility for material in self.materials]):
            for material in self.materials:
                if material.compressibility:
                    materialMap[material.index] = nd(material.compressibility)

            return (uw.function.branching.map(fn_key=self.materialField,
                                              mapping=materialMap,
                                              fn_default=0.0))
        return

    @property
    def freeSurface(self):
        return self._freeSurface

    @freeSurface.setter
    def freeSurface(self, value):
        if value:
            self._freeSurface = FreeSurfaceProcessor(self)
    
    @property
    def fssa_factor(self):
        return self._fssa_factor

    @fssa_factor.setter
    def fssa_factor(self, value):
        self._fssa_factor = float(value)

    @property
    def _fssa(self):
        return self.fssa_factor * self._buoyancyFn * self.dt * -1.0

    def add_visugrid(self, elementRes, minCoord=None, maxCoord=None):
        """ Add a tracking grid to the Model

        This is essentially a lagrangian grid that deforms with the materials.

        Parameters:
        -----------
            elementRes:
                Grid resolution in number of elements
                along each axis (x, y, z).
            minCoord:
                Minimum coordinate for each axis.
                Used to define the extent of the grid,
            maxCoord:
                Maximum coordinate for each axis.
                Used to define the extent of the grid,
        """

        if not maxCoord:
            maxCoord = self.maxCoord

        if not minCoord:
            minCoord = self.minCoord

        self._visugrid = Visugrid(self, elementRes, minCoord, maxCoord,
                                  self.velocityField)


_html_global = OrderedDict()
_html_global["Number of Elements"] = "elementRes"
_html_global["length"] = "length"
_html_global["width"] = "width"
_html_global["height"] = "height"


def _model_html_repr(Model):
    header = "<table>"
    footer = "</table>"
    html = ""
    for key, val in _html_global.items():
        value = Model.__dict__.get(val)
        html += "<tr><td>{0}</td><td>{1}</td></tr>".format(key, value)

    return header + html + footer


class _ViscosityFunction():

    def __init__(self, Model):
        """ Build Viscosity Function """

        self.Model = Model
        self.eta_eff = None
        self.plastic_eta = None
        self.viscous_eta = None
        self.elastic_eta = None
        self._averaged_field = None
        self._average_projector = None

    def average(self, f, p):

        if not self._averaged_field:
            # Create Mesh variable and projector for averaging scheme
            self._averaged_field = uw.mesh.MeshVariable(
                self.Model.mesh.subMesh,
                nodeDofCount=1,
                dataType="double")
            self._average_projector = uw.utils.MeshVariable_Projection(
                self._averaged_field,
                fn=1.0)

        def new_function(self):
            func = f**(p)
            projector = self._average_projector
            projector.fn = func
            projector.solve()
            return self._averaged_field**(1.0 / p)

        return new_function(self)

    def _use_initial_viscosity(self, material):
        return (material.initial_viscosity and
                not self.Model._solution_exist and
                self.Model.nlstep == 0)

    def get_effective_eta(self, averaging_scheme):
        """ Effective viscosity"""

        Model = self.Model

        if rcParams["rheologies.combine.method"] == "Harmonic Mean":
            # Harmonic mean
            n = 1.0
            eta_eff = 1.0 / self._getViscousEta()

            if any([material.plasticity for material in Model.materials]):
                eta_eff += 1.0 / self._getPlasticEta()
                n += 1.0

            if any([material.elasticity for material in Model.materials]):
                eta_eff = 1.0 / self._getElasticEta()
                n += 1.0

            eta_eff = n * eta_eff**-1

        if rcParams["rheologies.combine.method"] == "Minimum":
            if any([material.elasticity for material in Model.materials]):
                visc = self._getViscousEta()
                visc = self._getElasticEta()
            else:
                visc = self._getViscousEta()
            if any([material.plasticity for material in Model.materials]):
                eta_eff = fn.misc.min(visc, self._getPlasticEta())
            else:
                eta_eff = visc

        # Melt Modifier
        fac = self._melt_modifierFn()
        if fac:
            eta_eff *= fac

        # Viscosity Limiter
        self.eff_eta = self._viscosity_limiter(eta_eff)

        if averaging_scheme and averaging_scheme != 1:
            return self.average(self.eff_eta,
                                averaging_scheme)

        return self.eff_eta

    @property
    def _isYielding(self):
        Model = self.Model

        if self.elastic_eta is not None:
            yield_condition = [(self.eff_eta < self.elastic_eta,
                                Model.strainRate_2ndInvariant),
                               (True, 0.0)]
        else:
            yield_condition = [(self.eff_eta < self.viscous_eta,
                                Model.strainRate_2ndInvariant),
                               (True, 0.0)]

        return fn.branching.conditional(yield_condition)

    def _getViscousEta(self):

        Model = self.Model

        ViscosityMap = {}

        # Viscous behavior
        for material in Model.materials:
            if self._use_initial_viscosity(material):
                ViscosityMap[material.index] = material._initial_viscosity
                continue
            if material.viscosity:
                ViscosityHandler = material.viscosity
                ViscosityHandler.pressureField = Model.pressureField
                ViscosityHandler.strainRateInvariantField = (
                    Model.strainRate_2ndInvariant)
                ViscosityHandler.temperatureField = Model.temperature
                ViscosityMap[material.index] = ViscosityHandler.muEff

        self.viscous_eta = fn.branching.map(fn_key=Model.materialField,
                                            mapping=ViscosityMap)

        return self.viscous_eta

    def _melt_modifierFn(self):

        Model = self.Model

        melt_modif = {}
        for material in Model.materials:
            if self._use_initial_viscosity(material):
                continue
            if material.viscosity and material.viscosityChange != 1.0:
                X1 = material.viscosityChangeX1
                X2 = material.viscosityChangeX2
                change = (1.0 + (material.viscosityChange - 1.0) /
                          (X2 - X1) * (Model.meltField - X1))
                conditions = [(Model.meltField < X1, 1.0),
                              (Model.meltField > X2, material.viscosityChange),
                              (True, change)]
                melt_modif[material.index] = fn.branching.conditional(conditions)

        if melt_modif:

            return fn.branching.map(fn_key=Model.materialField,
                                    mapping=melt_modif,
                                    fn_default=1.0)

    def _getPlasticEta(self):

        Model = self.Model

        PlasticityMap = {}
        for material in Model.materials:
            if self._use_initial_viscosity(material):
                continue
            if material.plasticity:

                YieldHandler = material.plasticity
                YieldHandler.pressureField = Model.pressureField
                YieldHandler.plasticStrain = Model.plasticStrain

                if Model.mesh.dim == 2:
                    yieldStress = YieldHandler._get_yieldStress2D()

                if Model.mesh.dim == 3:
                    yieldStress = YieldHandler._get_yieldStress3D()

                if material.stressLimiter:
                    yieldStress = Stress_limiter(yieldStress, material.stressLimiter)
                elif Model.stressLimiter:
                    yieldStress = Stress_limiter(yieldStress, Model.stressLimiter)

                if material.elasticity:
                    ElasticityHandler = material.elasticity
                    mu = nd(ElasticityHandler.shear_modulus)
                    dt_e = nd(ElasticityHandler.observation_time)
                    strainRate = fn.tensor.symmetric(
                        Model.velocityField.fn_gradient)
                    D_eff = (strainRate
                             + 0.5
                             * Model._previousStressField
                             / (mu * dt_e))
                    SRInv = fn.tensor.second_invariant(D_eff)
                else:
                    SRInv = Model.strainRate_2ndInvariant

                eij = fn.branching.conditional(
                    [(SRInv < nd(1e-20 / u.second), nd(1e-20 / u.second)),
                     (True, SRInv)])

                muEff = 0.5 * yieldStress / eij
                PlasticityMap[material.index] = muEff

            if Model.frictionalBCs is not None:
                from copy import copy

                # Only affect plastic materials
                if material.plasticity:

                    YieldHandler = copy(material.plasticity)
                    YieldHandler.frictionCoefficient = Model.frictionalBCs.friction
                    YieldHandler.frictionAfterSoftening = Model.frictionalBCs.friction
                    YieldHandler.pressureField = Model.pressureField
                    YieldHandler.plasticStrain = Model.plasticStrain

                    if Model.mesh.dim == 2:
                        yieldStress = YieldHandler._get_yieldStress2D()

                    if Model.mesh.dim == 3:
                        yieldStress = YieldHandler._get_yieldStress3D()

                    eij = fn.branching.conditional(
                        [(Model.strainRate_2ndInvariant <= 1e-20, 1e-20),
                         (True, Model.strainRate_2ndInvariant)])

                    muEff = 0.5 * yieldStress / eij

                    conditions = [(Model.frictionalBCs._mask > 0.0, muEff),
                                  (True, PlasticityMap[material.index])]

                    PlasticityMap[material.index] = (
                        fn.branching.conditional(conditions)
                    )

        self.plastic_eta = fn.branching.map(fn_key=Model.materialField,
                                            mapping=PlasticityMap,
                                            fn_default=self.viscous_eta)
        return self.plastic_eta

    def _getElasticEta(self):

        Model = self.Model

        ElasticEtaMap = {}

        for material in Model.materials:
            if self._use_initial_viscosity(material):
                continue
            if material.elasticity:
                ElasticityHandler = material.elasticity
                ElasticityHandler.viscosity = self.viscous_eta
                ElasticEtaMap[material.index] = ElasticityHandler.muEff

        if ElasticEtaMap:

            self.elastic_eta = fn.branching.map(fn_key=Model.materialField,
                                                mapping=ElasticEtaMap,
                                                fn_default=self.viscous_eta)
        else:
            self.elastic_eta = self.viscous_eta

        return self.elastic_eta

    def _viscosity_limiter(self, eta):

        Model = self.Model

        default = Viscosity_limiter(eta, Model.minViscosity, Model.maxViscosity)
        limiter_map = {}

        for material in Model.materials:
            minViscosity = Model.minViscosity
            maxViscosity = Model.maxViscosity
            if material.minViscosity:
                minViscosity = material.minViscosity
            if material.maxViscosity:
                maxViscosity = material.maxViscosity
            limiter_map[material.index] = Viscosity_limiter(eta, minViscosity, maxViscosity)

        if limiter_map:
            return fn.branching.map(fn_key=Model.materialField,
                                    mapping=limiter_map,
                                    fn_default=default)
        else:
            return default


class _CheckpointFunction(object):
    """This Class is responsible for Checkpointing a Model"""

    def __init__(self, Model, duration=None, checkpoint_interval=None,
                 checkpoint_times=None, restart_checkpoint=None,
                 output_units=None):

        self.Model = Model
        self.output_units = output_units
        self.step_type = None

        if isinstance(checkpoint_interval, u.Quantity):
            self.step_type = "time"
            self.checkpoint_interval = nd(checkpoint_interval)
            self.next_checkpoint = Model._ndtime + self.checkpoint_interval

        elif checkpoint_interval:
            self.step_type = "step"
            self.checkpoint_interval = checkpoint_interval
            self.next_checkpoint = Model.stepDone + checkpoint_interval

        self.checkpoint_times = checkpoint_times
        self.restart_checkpoint = restart_checkpoint
        self.outputDir = Model.outputDir

        if checkpoint_interval or checkpoint_times:
            self.checkpoint_all()

    def checkpoint(self):

        Model = self.Model

        if (((self.step_type == "time") and
             (Model._ndtime == self.next_checkpoint)) or
            ((self.step_type == "step") and
             (Model.stepDone == self.next_checkpoint))):

            Model.checkpointID += 1
            # Save Mesh Variables
            self.checkpoint_fields(checkpointID=Model.checkpointID)
            # Save Tracers
            self.checkpoint_tracers(checkpointID=Model.checkpointID)
            self.next_checkpoint += self.checkpoint_interval

            comm.Barrier()

            # if it's time to checkpoint the swarm, do so.
            if Model.checkpointID % self.restart_checkpoint == 0:
                self.checkpoint_swarm(checkpointID=Model.checkpointID)

            comm.Barrier()

    def get_next_checkpoint_time(self):

        Model = self.Model

        dt1 = None
        dt2 = None

        if self.step_type == "time":
            dt1 = self.next_checkpoint - Model._ndtime

        if self.checkpoint_times:
            tcheck = [val - Model._ndtime for val in self.checkpoint_times]
            tcheck = [val for val in tcheck if val >= 0]
            tcheck.sort()
            dt2 = tcheck[0]

        if dt1 and dt2:
            return min(dt1, dt2)
        elif dt1:
            return dt1
        elif dt2:
            return dt2
        else:
            return

    def create_output_directory(self, outputDir=None):

        Model = self.Model

        if not outputDir:
            outputDir = Model.outputDir

        if not os.path.exists(outputDir):
            if rank == 0:
                os.makedirs(outputDir)
        comm.Barrier()

        return outputDir

    def checkpoint_all(self, checkpointID=None, variables=None,
                       tracers=None, time=None, outputDir=None):
        """ Do a checkpoint (Save fields)

        Parameters:
        -----------
            variables:
                list of fields/variables to save
            checkpointID:
                checkpoint ID.
            outpuDir:
                output directory

        """
        self.checkpoint_fields(variables, checkpointID, time, outputDir)
        self.checkpoint_swarm(variables, checkpointID, time, outputDir)
        self.checkpoint_tracers(tracers, checkpointID, time, outputDir)
        comm.Barrier()

    def checkpoint_fields(self, fields=None, checkpointID=None,
                          time=None, outputDir=None):
        """ Save the mesh and the mesh variables to outputDir

        Parameters
        ----------

        fields : A list of mesh/field variables to be saved.
        checkpointID : Checkpoint ID
        time : Model time at checkpoint
        outputDir : output directory

        """

        Model = self.Model

        if not fields:
            fields = rcParams["default.outputs"]

        if not checkpointID:
            checkpointID = Model.checkpointID

        outputDir = self.create_output_directory(outputDir)

        time = time if time else Model.time
        if isinstance(time, u.Quantity) and self.output_units:
            time = time.to(self.output_units)

        if Model._advector or Model._freeSurface:
            mesh_name = 'mesh-%s' % checkpointID
            mesh_prefix = os.path.join(outputDir, mesh_name)
            mH = Model.mesh.save('%s.h5' % mesh_prefix,
                                 units=u.kilometers,
                                 time=time)
        elif not Model._mesh_saved:
            mesh_name = 'mesh'
            mesh_prefix = os.path.join(outputDir, mesh_name)
            mH = Model.mesh.save('%s.h5' % mesh_prefix,
                                 units=u.kilometers,
                                 time=time)
            Model._mesh_saved = True
        else:
            mesh_name = 'mesh'
            mesh_prefix = os.path.join(outputDir, mesh_name)
            mH = uw.utils.SavedFileData(Model.mesh, '%s.h5' % mesh_prefix)

        if rank == 0:
            filename = "XDMF.fields." + str(checkpointID).zfill(5) + ".xmf"
            filename = os.path.join(outputDir, filename)

            # First write the XDMF header
            string = uw.utils._xdmfheader()
            string += uw.utils._spacetimeschema(mH, mesh_name,
                                                time)

        comm.Barrier()

        for field in fields:
            if field == "temperature" and not Model.temperature:
                continue
            if field in Model.mesh_variables.keys():
                field = str(field)

                try:
                    units = rcParams[field + ".SIunits"]
                except KeyError:
                    units = None

                # Save the h5 file and write the field schema for
                # each one of the field variables
                obj = getattr(Model, field)
                file_prefix = os.path.join(outputDir, field + '-%s' % checkpointID)
                handle = obj.save('%s.h5' % file_prefix, units=units,
                                  time=time)
                if rank == 0:
                    string += uw.utils._fieldschema(handle, field)
            comm.Barrier()

        if rank == 0:
            # Write the footer to the xmf
            string += uw.utils._xdmffooter()

            # Write the string to file - only proc 0
            with open(filename, "w") as xdmfFH:
                xdmfFH.write(string)
        comm.Barrier()

    def checkpoint_swarm(self, fields=None, checkpointID=None, time=None,
                          outputDir=None):
        """ Save the swarm and the swarm variables to outputDir

        Parameters
        ----------

        fields : A list of swarm/field variables to be saved.
        checkpointID : Checkpoint ID
        time : Model time at checkpoint
        outputDir : output directory

        """
        Model = self.Model

        if not fields:
            fields = Model.restart_variables

        if not checkpointID:
            checkpointID = Model.checkpointID

        outputDir = self.create_output_directory(outputDir)

        time = time if time else Model.time
        if isinstance(time, u.Quantity) and self.output_units:
            time = time.to(self.output_units)

        swarm_name = 'swarm-%s.h5' % checkpointID

        sH = Model.swarm.save(os.path.join(outputDir,
                              swarm_name),
                              units=u.kilometers,
                              time=time)

        if rank == 0:
            filename = "XDMF.swarms." + str(checkpointID).zfill(5) + ".xmf"
            filename = os.path.join(outputDir, filename)

            # First write the XDMF header
            string = uw.utils._xdmfheader()
            string += uw.utils._swarmspacetimeschema(sH, swarm_name,
                                                     time)
        comm.Barrier()

        for field in fields:
            if field in Model.swarm_variables.keys():
                field = str(field)
                try:
                    units = rcParams[field + ".SIunits"]
                except KeyError:
                    units = None

                # Save the h5 file and write the field schema for
                # each one of the field variables
                obj = getattr(Model, field)
                file_prefix = os.path.join(outputDir,
                                           field + '-%s' % checkpointID)
                handle = obj.save('%s.h5' % file_prefix,
                                  units=units, time=time)
                if rank == 0:
                    string += _swarmvarschema(handle, field)
                comm.Barrier()

        if rank == 0:
            # Write the footer to the xmf
            string += uw.utils._xdmffooter()

            # Write the string to file - only proc 0
            with open(filename, "w") as xdmfFH:
                xdmfFH.write(string)

        comm.Barrier()

    def checkpoint_tracers(self, tracers=None, checkpointID=None,
                           time=None, outputDir=None):
        """ Checkpoint the tracers

        Parameters
        ----------

        tracers : List of tracers to checkpoint.
        checkpointID : Checkpoint ID.
        time : Model time at checkpoint.
        outputDir : output directory

        """

        Model = self.Model

        if not checkpointID:
            checkpointID = Model.checkpointID

        time = time if time else Model.time
        if isinstance(time, u.Quantity) and self.output_units:
            time = time.to(self.output_units)

        if not outputDir:
            outputDir = Model.outputDir

        if rank == 0 and not os.path.exists(outputDir):
            os.makedirs(outputDir)
        comm.Barrier()

        # Checkpoint passive tracers and associated tracked fields
        if Model.passive_tracers:
            for (dump, item) in Model.passive_tracers.items():
                item.save(outputDir, checkpointID, time)

        comm.Barrier()


class _RestartFunction(object):

    def __init__(self, Model, restartDir):

        self.Model = Model
        self.restartDir = restartDir

        comm.Barrier()

    def restart(self, step):
        """restart

        Parameters
        ----------

        step : int
            Step from which you want to restart the model.
            Must be an int (step number either absolute or relative)
            if step == -1, run the last available step
            if step == -2, run the second last etc.

        Returns
        -------

        This function returns None
        """
        Model = self.Model


        indices = self.find_available_steps()
        step = indices[step] if step < 0 else step
        Model.checkpointID = step

        if step not in indices:
            raise ValueError("Cannot find step in specified folder")

        # Get time from swarm-%.h5 file
        if rank == 0:
            swarm_file = os.path.join(self.restartDir, "swarm-%s.h5" % step)
            with h5py.File(swarm_file, "r") as h5f:
                Model._ndtime = nd(u.Quantity(h5f.attrs.get("time")))
        else:
            Model._ndtime = None

        Model._ndtime = comm.bcast(Model._ndtime, root=0)

        if rank == 0:
            print(80 * "=" + "\n")
            print("Restarting Model from Step {0} at Time = {1}\n".format(step, Model.time))
            print('(' + datetime.now().strftime('%Y-%m-%d %H:%M:%S') + ')')
            print(80 * "=" + "\n")
            sys.stdout.flush()
        comm.Barrier()


        self.reload_mesh(step)
        self.reload_swarm(step)
    
        '''
        ##### Ugly workaround: Passive tracer tracking for restart #####
        If there are passive tracers at reload we need a way to update 
        tracked fields that will not be valid due to restarting.
        This occurs because SwarmVariables are rebuilt at restart and the
        tracked SwarmVariable of passive tracers becomes stale.

        Way around:
        1) Store the "names" of the swarm variables tracked by all tracers.
        Names from the 'swarm_variables' ordered dictionary.
        
        2) Once the _initialize() function has been called then 
        find the new SwarmVariables from the model that belong to
        the names of the original SwarmVariables.
        '''

        # a 2D memory chunk. One axis for passive tracer swarm
        # the 2nd for a dict of the variable 'names' that will
        # be used to looks up the SwarmVariables after Model._initialize()

        var_tracker = {} # 2D: {tracers:variable_dict}
        for name, tracers in Model.passive_tracers.items():

            vdict = {} # tracked var names, to Model attribute name
            tracked_vars = tracers.tracked_fields
            
            # find the name the tracked var exists in Model.swarm_variables.
            for tname, v in tracked_vars.items():
                var = v['value']
                
                if isinstance( var, SwarmVariable ):
                    for vname, svar in Model.swarm_variables.items():
                        if var == svar:
                           vdict[tname] = vname 

            var_tracker[name] = vdict

        Model._initialize()

        self.reload_restart_variables(step)
        self.reload_passive_tracers(step)

        '''
        Overwrite the passive tracer tracked SwarmVariable with
        the new Model.swarm_variables as per point 2) above
        '''
        for name,tracers in Model.passive_tracers.items():
            vdict = var_tracker.get(name)
            for tfield in vdict.keys():
                vname = vdict[tfield]
                # get the current version
                obj = Model.swarm_variables[vname]
                # overwrite the old version
                tracers.tracked_fields[tfield].update({'value':obj})


        if Model._solver:
            solver_options = Model._solver.options
            Model._solver = None
            Model.solver.options = solver_options

        if isinstance(Model.surfaceProcesses,
                      (surfaceProcesses.SedimentationThreshold,
                       surfaceProcesses.ErosionThreshold,
                       surfaceProcesses.ErosionAndSedimentationThreshold)):

            obj = Model.surfaceProcesses
            obj.Model = Model
            obj.timeField = Model.timeField

        # Restart Badlands if we are running a coupled model
        if isinstance(Model.surfaceProcesses, surfaceProcesses.Badlands):
            self.restart_badlands(step)

        return

    def find_available_steps(self):

        # Look for step with swarm available
        indices = [int(os.path.splitext(filename)[0].split("-")[-1])
                   for filename in os.listdir(self.restartDir) if "-" in
                   filename]
        indices.sort()
        return indices

    def reload_mesh(self, step):

        Model = self.Model

        if Model._advector:
            Model.mesh.load(os.path.join(self.restartDir, 'mesh-%s.h5' % step))
        else:
            Model.mesh.load(os.path.join(self.restartDir, "mesh.h5"))

        if rank == 0:
            now = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
            print("Mesh loaded" + '(' + now + ')')
            sys.stdout.flush()

    def reload_swarm(self, step):

        # build a new swarm on the mesh
        Model = self.Model
        Model.swarm = Swarm(mesh=Model.mesh, particleEscape=True)
        Model.swarm.allow_parallel_nn = True # always activate parallel_nn on swarm
        Model.swarm.load(os.path.join(self.restartDir, 'swarm-%s.h5' % step))

        if rank == 0:
            now = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
            print("Swarm loaded" + '(' + now + ')')
            sys.stdout.flush()

    def reload_restart_variables(self, step):

        Model = self.Model

        for field in Model.restart_variables:
            obj = getattr(Model, field)
            path = os.path.join(self.restartDir, field + "-%s.h5" % step)
            obj.load(str(path))
            if rank == 0:
                now = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
                print("{0} loaded".format(field) + '(' + now + ')')
                sys.stdout.flush()

    def reload_passive_tracers(self, step):

        Model = self.Model

        for key, tracer in Model.passive_tracers.items():

            fname = tracer.name + '-%s.h5' % step
            fpath = os.path.join(self.restartDir, fname)
            tracked_fields = tracer.tracked_fields

            obj = PassiveTracers(Model.mesh,
                                 tracer.name,
                                 particleEscape=tracer.particleEscape)
            obj.load(fpath)
            if tracer.advector:
                obj.advector = uw.systems.SwarmAdvector(Model.velocityField, obj, order=2)

            # Reload global indices
            fpath = os.path.join(self.restartDir, tracer.name + '_global_index-%s.h5' % step)
            obj.global_index.load(fpath)

            # Create and Reload all tracked fields
            for name, kwargs in tracked_fields.items():
                field = obj.add_tracked_field(name=name, overwrite=True, **kwargs)
                svar_fname = tracer.name +"_" + name + '-%s.h5' % step
                svar_fpath = os.path.join(self.restartDir, svar_fname)
                field.load(svar_fpath)

            attr_name = tracer.name + "_tracers"
            setattr(Model, attr_name, obj)
            Model.passive_tracers[key] = obj

            if rank == 0:
                now = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
                print("{0} loaded".format(tracer.name) + '(' + now  + ')')
                sys.stdout.flush()

    def restart_badlands(self, step):
        """ Restart Badlands from step
            Note that step is only used if no restartStep has been
            defined on the badlands_model object """

        Model = self.Model

        badlands_model = Model.surfaceProcesses
        restartFolder = badlands_model.restartFolder
        restartStep = badlands_model.restartStep

        if badlands_model.restartFolder:
            restartFolder = badlands_model.restartFolder
        else:
            restartFolder = badlands_model.outputDir

        if badlands_model.restartStep:
            restartStep = badlands_model.restartStep
        else:
            restartStep = step

        # Parse xmf for the last timestep time
        import xml.etree.ElementTree as etree
        xmf = restartFolder + "/xmf/tin.time" + str(restartStep) + ".xmf"
        tree = etree.parse(xmf)
        root = tree.getroot()
        badlands_time = float(root[0][0][0].attrib["Value"])
        uw_time = Model.time.to(u.years).magnitude

        if np.abs(badlands_time - uw_time) > 1:
            raise ValueError("""Time in Underworld and Badlands outputs
                             differs:\n
                             Badlands: {0}\n
                             Underworld: {1}""".format(badlands_time,
                                                       uw_time))

        airIndex = badlands_model.airIndex
        sedimentIndex = badlands_model.sedimentIndex
        XML = badlands_model.XML
        resolution = badlands_model.resolution
        checkpoint_interval = badlands_model.checkpoint_interval

        Model.surfaceProcesses = surfaceProcesses.Badlands(
            airIndex, sedimentIndex,
            XML, resolution,
            checkpoint_interval,
            restartFolder=restartFolder,
            restartStep=restartStep)

        if rank == 0:
            print("Badlands restarted" + '(' + datetime.now().strftime('%Y-%m-%d %H:%M:%S') + ')')
            sys.stdout.flush()


def _get_output_units(*args):
    from pint import UndefinedUnitError
    for arg in args:
        try:
            return u.Unit(arg)
        except (TypeError, UndefinedUnitError):
            pass
        if isinstance(arg, u.Quantity):
            return arg.units

    return rcParams["time.SIunits"]


def _solver_options_dictionary(solver):
    """Return a dictionary of all the solver options"""
    dd = {}
    for key, val in solver.options.__dict__.items():
        if isinstance(val, dict):
            dd2 = {}
            for key2, val2, in val.__dict__.items():
                dd2[key2] = val2
        else:
            dd2 = val
        dd[key] = dd2
    return dd


def _apply_saved_options_on_solver(solver, options):
    """Apply options on a solver

    solver: solver
    options: python dictionary
    """
    for key, val in options.items():
        if isinstance(val, dict):
            for key2, val2 in val.items():
                solver.options.__dict__[key].__dict__[key2] = val2
        else:
            solver.options.__dict__[key] = val
    return solver


def _adjust_time_units(val):
    """ Adjust the units used depending on the value """
    if isinstance(val, u.Quantity):
        mag = val.to(u.years).magnitude
    else:
        val = dimensionalise(val, u.years)
        mag = val.magnitude
    exponent = int("{0:.3E}".format(mag).split("E")[-1])

    if exponent >= 9:
        units = u.gigayear
    elif exponent >= 6:
        units = u.megayear
    elif exponent >= 0:
        units = u.years
    elif exponent > -3:
        units = u.days
    elif exponent > -5:
        units = u.hours
    elif exponent > -7:
        units = u.minutes
    else:
        units = u.seconds
    return val.to(units)
