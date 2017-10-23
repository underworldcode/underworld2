import os
import types
import underworld as uw
import underworld.function as fn
from itertools import count
import numpy as np
import glucifer
from unsupported import rheology
import unsupported.scaling as sca
from unsupported.scaling import nonDimensionalize as nd
from unsupported.lithopress import lithoPressure
from unsupported.LecodeIsostasy import lecode_tools_isostasy
from unsupported.geodynamics import utils


def smooth_pressure(mesh, pressure):
    # Smooths the pressure field.
    # Assuming that pressure lies on the submesh, do a cell -> nodes -> cell
    # projection.
    NodePressure = uw.mesh.MeshVariable(mesh, nodeDofCount=1)
    Cell2Nodes = uw.utils.MeshVariable_Projection(NodePressure,
                                                  pressure, type=0)
    Nodes2Cell = uw.utils.MeshVariable_Projection(pressure,
                                                  NodePressure, type=0)
    Cell2Nodes.solve()
    Nodes2Cell.solve()


u = UnitRegistry = sca.UnitRegistry


class Plasticity(object):

    def __init__(self, name="User defined", func=None):
        self.name = name
        dictValues = func
        self.coefs = dictValues["coefficients"]

        self.cohesionFn = rheology.linearCohesionWeakening
        self.frictionFn = rheology.linearFrictionWeakening

    def _get_friction(self, plasticStrain):
        self.friction = self.frictionFn(
                plasticStrain,
                FrictionCoef=nd(self.coefs["FrictionCoef"]),
                FrictionCoefSw=nd(self.coefs["FrictionCoefSw"])
            )

    def _get_cohesion(self, plasticStrain):
        self.cohesion = self.cohesionFn(
                plasticStrain,
                Cohesion=nd(self.coefs["Cohesion"]),
                CohesionSw=nd(self.coefs["CohesionSw"])
            )

    def _get_yieldStress(self, plasticStrain, pressure):
        self._get_friction(plasticStrain)
        self._get_cohesion(plasticStrain)
        self.yieldStress = (self.cohesion * fn.math.cos(self.friction) +
                            pressure * fn.math.sin(self.friction))

    def _get_effective_viscosity(self, plasticStrain, pressure, eij, eijdef):
        self._get_yieldStress(plasticStrain, pressure)
        self.effViscosity = 0.5 * self.yieldStress / fn.misc.max(eij, eijdef)
        return self.effViscosity


class Viscosity(object):

    def __init__(self, name="User defined", func=None):
        self.name = name
        if not isinstance(func, dict):
            self.fn = fn.misc.constant(nd(func))
        else:
            dictVals = func
            self.coefs = dictVals["coefficients"]
            self._nd_coefs = {key: nd(val) for key, val in
                              self.coefs.iteritems()}
            self.fn = rheology.Functions[dictVals["Type"]]

    def _get_effective_viscosity(self, pressure, strainRate, temperature,
                                 solutionExist):
        if isinstance(self.fn, fn.misc.constant):
            return self.fn

        # Need to generalize that
        R = nd(8.3144621 * u.joule / u.mole / u.degK)
        return self.fn(R=R, strainRateFn=strainRate,
                       pressureFn=pressure,
                       temperatureFn=temperature,
                       solutionExist=solutionExist,
                       **self._nd_coefs)


class Rheology(object):
    """ This class is deprecated and only remains for 
        backward compatibility. It will be removed soon and
        should not be uses
    """
    def __init__(self):
        self.viscosity = None
        self.plasticity = None
        self.cohesion = None
        self.friction = None
        self.meltModifier = None
        self.stressLimiter = None


class Material(object):
    _ids = count(0)

    def __init__(self, name="Undefined", vertices=None, density=None,
                 diffusivity=None, capacity=None, thermalExpansivity=None,
                 radiogenicHeatProd=None, top=None, bottom=None, shape=None,
                 minX=None, maxX=None):

        self.name = name
        self.top = top
        self.bottom = bottom

        if vertices:
            self.shape = uw.function.shape.Polygon(np.array(vertices))
        elif shape == "layer":
            if (top is not None and
                bottom is not None and
                minX is not None  and 
                maxX is not None):
                self.shape = utils.layer(top, bottom, minX, maxX)
        else:
            self.shape = None
        self.index = self._ids.next()

        self._density = density
        self._diffusivity = diffusivity
        self._capacity = capacity
        self._thermalExpansivity = thermalExpansivity
        self._radiogenicHeatProd = radiogenicHeatProd
        self.meltFraction = 0.
        self.meltFractionLimit = 0.
        self.solidus = None
        self.liquidus = None
        self.latentHeatFusion = 0
        self.latentSurfaceProcesses = None

        self.rheology = None  # For backward compatibility

        self._viscosity = None
        self._plasticity = None

    def __repr__(self):
        return self.name

    @property
    def density(self):
        return self._density

    @density.setter
    def density(self, value):
        self._density = value

    @property
    def diffusivity(self):
        return self._diffusivity

    @diffusivity.setter
    def diffusivity(self, value):
        self._diffusivity = value

    @property
    def capacity(self):
        return self._capacity

    @capacity.setter
    def capacity(self, value):
        self._capacity = value

    @property
    def radiogenicHeatProd(self):
        return self._radiogenicHeatProd

    @radiogenicHeatProd.setter
    def radiogenicHeatProd(self, value):
        self._radiogenicHeatProd = value
    
    @property
    def thermalExpansivity(self):
        return self._thermalExpansivity

    @thermalExpansivity.setter
    def thermalExpansivity(self, value):
        self._thermalExpansivity = value
    
    @property
    def viscosity(self):
        return self._viscosity

    @viscosity.setter
    def viscosity(self, value):
        self._viscosity = Viscosity(func=value)  # For backward compatibility

    @property
    def plasticity(self):
        return self._plasticity

    @plasticity.setter
    def plasticity(self, value):
        self._plasticity = Plasticity(func=value)  # For backward compatibility


class Model(object):
    def __init__(self, elementRes, minCoord, maxCoord, gravity,
                 periodic=(False, False), elementType="Q1/dQ0",
                 swarmLayout=None, Tref=273.15 * u.degK, name="undefined",
                 outputDir="outputs"):

        self.name = name
        self.outputDir = outputDir
        self.checkpointID = 0
        self._checkpoint = None
        self.checkpoint = None

        self.minViscosity = 1e19 * u.pascal * u.second
        self.maxViscosity = 1e25 * u.pascal * u.second

        self.gravity = tuple([nd(val) for val in gravity])
        self.Tref = Tref
        self.elementType = elementType
        self.elementRes = elementRes
        self.minCoord = minCoord
        self.maxCoord = maxCoord
        self.periodic = periodic

        self._set_mesh()
        
        # Add common fields
        self.temperature = None
        self.pressure = uw.mesh.MeshVariable(mesh=self.mesh.subMesh,
                                             nodeDofCount=1)
        self.velocity = uw.mesh.MeshVariable(mesh=self.mesh,
                                             nodeDofCount=self.mesh.dim)

        self.swarm = uw.swarm.Swarm(mesh=self.mesh, particleEscape=True)

        if swarmLayout is not None:
            self._swarmLayout = swarmLayout
        else:
            self._swarmLayout = uw.swarm.layouts.GlobalSpaceFillerLayout(
                                                 swarm=self.swarm,
                                                 particlesPerCell=25)

        self.swarm.populate_using_layout(layout=self._swarmLayout)
        self.swarm_population_control = uw.swarm.PopulationControl(
                 self.swarm,
                 aggressive=True, splitThreshold=0.15,
                 maxDeletions=2, maxSplits=10,
                 particlesPerCell=20
            )

        self.swarm_advector = uw.systems.SwarmAdvector(
                swarm=self.swarm,
                velocityField=self.velocity,
                order=self.mesh.dim
            )

        # symmetric component of the gradient of the flow velocity.
        self.strainRate_default = 1.0e-15 / u.second
        self.solutionExist = fn.misc.constant(False)
        self.strainRate = fn.tensor.symmetric(self.velocity.fn_gradient)
        self.strainRate_2ndInvariant = fn.tensor.second_invariant(
                self.strainRate
            )

        # Force initialisation to zero
        self.pressure.data[...] = 0.
        self.velocity.data[...] = 0.

        # Add Common Swarm Variables
        self.material = self.swarm.add_variable(dataType="int", count=1)
        self.strainRateVar = self.swarm.add_variable(dataType="double", count=1)
        self.plasticStrain = self.swarm.add_variable(dataType="double",
                                                     count=1)
        self.plasticStrain.data[...] = 0.0
	self.strainRateVar.data[:] = 0.0
 
        self.materials = []
        self._defaultMaterial = 0

        self.leftWall = self.mesh.specialSets["MinI_VertexSet"]
        self.topWall = self.mesh.specialSets["MaxJ_VertexSet"]
        self.bottomWall = self.mesh.specialSets["MinJ_VertexSet"]
        self.rightWall = self.mesh.specialSets["MaxI_VertexSet"]
        
        if self.mesh.dim > 2:
            self.frontWall = self.mesh.specialSets["MinK_VertexSet"]
            self.backWall = self.mesh.specialSets["MaxK_VertexSet"]

        self.time = 0.0

        # global variables
        self.global_thermal_diffusivity = None
        self.global_thermal_capacity = None
        self.global_thermal_expansivity = None
        self.global_radiogenic_heat_production = None

        self._lecodeRefMaterial = None

    @property
    def outputDir(self):
        return self._outputDir

    @outputDir.setter
    def outputDir(self, value):
        if uw.rank() == 0:
            if not os.path.exists(value):
                os.makedirs(value)
        self._outputDir = value


    def _set_mesh(self):

        minCoord = tuple([nd(val) for val in self.minCoord])
        maxCoord = tuple([nd(val) for val in self.maxCoord])

        self.mesh = uw.mesh.FeMesh_Cartesian(elementType=self.elementType,
                                             elementRes=self.elementRes,
                                             minCoord=minCoord,
                                             maxCoord=maxCoord,
                                             periodic=self.periodic)

    def set_temperatureBCs(self, left=None, right=None, top=None, bottom=None,
                           indexSets=[]):
        self.temperature = uw.mesh.MeshVariable(mesh=self.mesh,
                                                nodeDofCount=1)
        self._temperatureDot = uw.mesh.MeshVariable(mesh=self.mesh,
                                                    nodeDofCount=1)
        self.temperature.data[...] = nd(self.Tref)
        self._temperatureDot.data[...] = 0.
        
        indices = [self.mesh.specialSets["Empty"]]
        if left is not None:
            self.temperature.data[self.leftWall.data] = nd(left)
            indices[0] += self.leftWall
        if right is not None:
            self.temperature.data[self.rightWall.data] = nd(right)
            indices[0] += self.rightWall
        if top is not None:
            self.temperature.data[self.topWall.data] = nd(top)
            indices[0] += self.topWall
        if bottom is not None:
            self.temperature.data[self.bottomWall.data] = nd(bottom)
            indices[0] += self.bottomWall

        for indexSet, temp in indexSets:
            self.temperature.data[indexSet.data] = nd(temp)
            indices[0] += indexSet

        self._temperatureBCs = uw.conditions.DirichletCondition(
                variable=self.temperature,
                indexSetsPerDof=indices
            )

    def init_advection_diffusion(self):
        DiffusivityMap = {}
        for material in self.materials:
            DiffusivityMap[material.index] = nd(material.diffusivity)
        
        self.DiffusivityFn = fn.branching.map(fn_key=self.material,
                                              mapping=DiffusivityMap)
       
        HeatProdMap = {}
        for material in self.materials:
            HeatProdMap[material.index] = (nd(material.radiogenicHeatProd) /
                                           (nd(material.density) *
                                            nd(material.capacity)))
        
        self.HeatProdFn = fn.branching.map(fn_key=self.material,
                                           mapping=HeatProdMap)
        
        self.advdiffSystem = uw.systems.AdvectionDiffusion(
                self.temperature,
                self._temperatureDot,
                velocityField=self.velocity,
                fn_diffusivity=self.DiffusivityFn,
                fn_sourceTerm=self.HeatProdFn,
                conditions=[self._temperatureBCs]
            )

    def init_stokes_system(self):
        conditions = self._velocityBCs
        self._set_density()
        self.buoyancyFn = self.densityFn * self.gravity

        if any([material.viscosity for material in self.materials]): 
            self._set_viscosity()
            
            stokes = uw.systems.Stokes(velocityField=self.velocity,
                                       pressureField=self.pressure,
                                       conditions=conditions,
                                       fn_viscosity=self.viscosityFn,
                                       fn_bodyforce=self.buoyancyFn)

            self.solver = uw.systems.Solver(stokes)
            if uw.nProcs()>1:
                self.solver.set_inner_method("mumps")
            else:
                self.solver.set_inner_method("lu")
            self.solver.set_penalty(1e6)

    def set_velocityBCs(self, left=None, right=None, top=None, bottom=None,
                        indexSets=[]):
        
        indices = [self.mesh.specialSets["Empty"]] * self.mesh.dim 
        if left is not None:
            for dim in range(self.mesh.dim):
                if left[dim] is not None:
                    self.velocity.data[self.leftWall.data, dim] = nd(left[dim])
                    indices[dim] += self.leftWall
        if right is not None:
            for dim in range(self.mesh.dim):
                if right[dim] is not None:
                    self.velocity.data[self.rightWall.data, dim] = nd(right[dim])
                    indices[dim] += self.rightWall
        if top is not None:
            for dim in range(self.mesh.dim):
                if top[dim] is not None:
                    self.velocity.data[self.topWall.data, dim] = nd(top[dim])
                    indices[dim] += self.topWall
        if bottom is not None:
            for dim in range(self.mesh.dim):
                if bottom[dim] is not None:
                    self.velocity.data[self.bottomWall.data, dim] = nd(bottom[dim])
                    indices[dim] += self.bottomWall

        for indexSet, temp in indexSets:
            for dim in range(self.mesh.dim):
                if indexSet[dim] is not None:
                    self.velocity.data[indexSet.data, dim] = nd(indexSet[dim])
                    indices[dim] += indexSet

        self._velocityBCs = uw.conditions.DirichletCondition(
                                      variable=self.velocity,
                                      indexSetsPerDof=indices)

    def use_lecode_isostasy(self, material_ref):
        self._lecodeRefMaterial = material_ref


    def add_material(self, vertices=None, reset=False, name="unknown",
                     shape=None, top=None, bottom=None):

        if reset:
            self.materials = []

        if vertices:
            vertices = [(nd(x), nd(y)) for x, y in vertices]
        mat = Material(name=name, vertices=vertices, 
                       diffusivity=self.global_thermal_diffusivity,
                       capacity=self.global_thermal_capacity,
                       thermalExpansivity=self.global_thermal_expansivity,
                       radiogenicHeatProd=self.global_radiogenic_heat_production,
                       shape=shape,
                       minX=nd(self.minCoord[0]),
                       maxX=nd(self.maxCoord[0]),
                       top=nd(top),
                       bottom=nd(bottom))
        mat.indices = self._get_material_indices(mat)
        self.materials.reverse()
        self.materials.append(mat)
        self.materials.reverse()
        self._fill_model()

        return mat

    def _fill_model(self):

        conditions = [(obj.shape, obj.index)
                      for obj in self.materials if obj.shape is not None]

        conditions.append((True, self._defaultMaterial))
        self.material.data[:] = fn.branching.conditional(conditions).evaluate(self.swarm)

    @property
    def material_drawing_order(self):
        return self.material_drawing_order()

    @material_drawing_order.setter
    def material_drawing_order(self, value):
        self._material_drawing_order = value
        self.materials = value
        self._fill_model()

    def _get_material_indices(self, mat):
        indices = []
        for index, coords in enumerate(self.mesh.data):
            coords = np.expand_dims(coords, axis=0)
            if mat.shape.evaluate(coords):
                indices.append(index)
        return uw.mesh.FeMesh_IndexSet(self.mesh, topologicalIndex=0,
                                       size=self.mesh.nodesGlobal,
                                       fromObject=indices)

    def _set_density(self):
        densityMap = {}
        for material in self.materials:
            if self.temperature:
                densityMap[material.index] = nd(material.density) * (1.0 -
                                             nd(material.thermalExpansivity)
                                             * (self.temperature - nd(self.Tref)))
            else:
                densityMap[material.index] = nd(material.density)

        self.densityFn = fn.branching.map(fn_key=self.material, mapping=densityMap)

    def _viscosity_limiter(self, viscosityFn):
        maxViscosity = fn.misc.constant(nd(self.maxViscosity))
        minViscosity = fn.misc.constant(nd(self.minViscosity))
        maxBound = fn.misc.min(viscosityFn, maxViscosity)
        minMaxBound = fn.misc.max(maxBound, minViscosity)
        return minMaxBound

    def _set_viscosity(self):

        ViscosityMap = {}
        BGViscosityMap = {}
        for material in self.materials:
            backgroundViscosity = material.viscosity._get_effective_viscosity(
                    self.pressure,
                    self.strainRate,
                    self.temperature,
                    self.solutionExist
                )
            
            ViscosityMap[material.index] = self._viscosity_limiter(
                    backgroundViscosity
                )

            BGViscosityMap[material.index] = ViscosityMap[material.index]

            if material.plasticity:
                eff_viscosity = material.plasticity._get_effective_viscosity(
                        self.plasticStrain,
                        self.pressure,
                        self.strainRate_2ndInvariant,
                        nd(self.strainRate_default)
                    )
                ViscosityMap[material.index] = self._viscosity_limiter(
                        fn.misc.min(eff_viscosity, backgroundViscosity)
                    )

        self.viscosityFn = fn.branching.map(fn_key=self.material,
                                            mapping=ViscosityMap)

        # Yielding
        backgroundViscosityFn = fn.branching.map(fn_key=self.material,
                                                 mapping=BGViscosityMap)
        SYconditions = [(self.viscosityFn < backgroundViscosityFn,
                         self.strainRate_2ndInvariant),
                        (True, 0.0)]
        self.isYielding = fn.branching.conditional(SYconditions)
        
        return self.viscosityFn, self.isYielding

    def solve_temperature_steady_state(self):
        DiffusivityMap = {}
        for material in self.materials:
            DiffusivityMap[material.index] = nd(material.diffusivity)
        
        self.DiffusivityFn = fn.branching.map(fn_key = self.material, mapping = DiffusivityMap)
       
        HeatProdMap = {}
        for material in self.materials:
            HeatProdMap[material.index] = nd(material.radiogenicHeatProd)/(nd(material.density)*nd(material.capacity))
        
        self.HeatProdFn = fn.branching.map(fn_key = self.material, mapping = HeatProdMap)
        
        heatequation = uw.systems.SteadyStateHeat(temperatureField=self.temperature,
                                                  fn_diffusivity=self.DiffusivityFn,
                                                  fn_heating=self.HeatProdFn,
                                                  conditions=self._temperatureBCs)
        heatsolver = uw.systems.Solver(heatequation)
        heatsolver.solve(nonLinearIterate=True)

    def solve_lithostatic_pressure(self):
        self._set_density()
        gravity = np.abs(self.gravity[-1])  # Ugly!!!!!
        self.pressure.data[:], LPresBot = lithoPressure(self.mesh,
                                                        self.densityFn, 
                                                        gravity)
        smooth_pressure(self.mesh, self.pressure)

    def _calibrate_pressure(self):
        surfaceArea = uw.utils.Integral(fn=1.0, mesh=self.mesh,
                                        integrationType='surface',
                                        surfaceIndexSet=self.topWall)
        surfacePressureIntegral = uw.utils.Integral(
                fn=self.pressure,
                mesh=self.mesh,
                integrationType='surface',
                surfaceIndexSet=self.topWall
            )
        (area,) = surfaceArea.evaluate()
        (p0,) = surfacePressureIntegral.evaluate()
        self.pressure.data[:] = self.pressure.data[:] - (p0 / area)

    def solve(self):
        self.solver.solve(nonLinearIterate=True,
                          callback_post_solve=self._calibrate_pressure)
        self.solutionExist.value = True

    def init_model(self, temperature=True, pressure=True):
        
        # Init Temperature Field
        if self.temperature and temperature:
            self.solve_temperature_steady_state()
            self.init_advection_diffusion()

        # Init Pressure Field
        if self.pressure and pressure:
            self.solve_lithostatic_pressure()
        
        self.init_stokes_system()

    def run_for(self, endTime=None, checkpoint=None):
        self.time = 0.
        units = endTime.units
        endTime = self.time + nd(endTime)
        step = 0

        next_checkpoint = None
        if checkpoint:
            next_checkpoint = self.time + nd(checkpoint)

        while self.time < endTime:
            self.solve()

            if self.time == next_checkpoint:
                self.checkpointID += 1
                self.checkpoint()
                next_checkpoint += nd(checkpoint)

            # Whats the longest we can run before reaching the end of the model
            # or a checkpoint?
            # Need to generalize that
            dt = self.swarm_advector.get_max_dt()

            if self.temperature:
                dt = min(dt, self.advdiffSystem.get_max_dt())

            if checkpoint:
                dt = min(dt, next_checkpoint - self.time)
            
            self._dt = min(dt, endTime - self.time)
            uw.barrier()
            
            self.update()

            step += 1
            if checkpoint or step % 1 == 0:
                print "Time: ", str(sca.Dimensionalize(self.time, units))

    def update(self):

        dt = self._dt
	self.strainRateVar.data[:] = self.strainRate_2ndInvariant.evaluate(self.swarm)
        # Increment plastic strain
        plasticStrainIncrement = dt * self.isYielding.evaluate(self.swarm)
        #weight = boundary(swarm.particleCoordinates.data[:,0], minX, maxX, 20, 4)
        weight = 0.0
        plasticStrainIncrement[:, 0] *= weight
        self.plasticStrain.data[:] += plasticStrainIncrement
        
        # Solve for temperature
        if self.temperature:
            self.advdiffSystem.integrate(dt)

        # Integrate Swarms in time
        self.swarm_advector.integrate(dt, update_owners=True) 
        
        # Do pop control
        self.swarm_population_control.repopulate() 
    
        # Apply change in boundary condition
        if self._lecodeRefMaterial:
            lecode_tools_isostasy(self.mesh, 
                                  self.swarm,
                                  self.velocity,
                                  self.densityFn,
                                  self.material, 
                                  self._lecodeRefMaterial.index,
                                  average=False)

        self.time += dt

    def _default_checkpoint_function(self):
        self.save_velocity(self.checkpointID)
        self.save_pressure(self.checkpointID)
        if self.temperature:
            self.save_temperature(self.checkpointID)
        self.save_material(self.checkpointID)
	
    @property
    def checkpoint(self):
        return self._checkpoint

    @checkpoint.setter
    def checkpoint(self, func=None):
        if func:
            self._checkpoint = types.MethodType(func, self)
        else:
            self._checkpoint = self._default_checkpoint_function

    def plot_material(self, figsize=(1200, 400), **args):
        Fig = glucifer.Figure(figsize=(1200, 400), title="Materials")
        Fig.append(glucifer.objects.Points(self.swarm,
                                           fn_colour=self.material,
                                           fn_size=2.0))
        Fig.show()
        return Fig

    def save_velocity(self, checkpointID, units=u.centimeter/u.year):
        mH = self.mesh.save(os.path.join(self.outputDir, "mesh.h5"),
                             units=u.kilometers)
        file_prefix = os.path.join(self.outputDir,
                                   'velocity-%s' % checkpointID)
        handle = self.velocity.save('%s.h5' % file_prefix, 
                                    units=units)
        self.velocity.xdmf('%s.xdmf' % file_prefix, handle, 'velocity', mH,
                           'mesh', modeltime=self.time)
    
    def save_pressure(self, checkpointID, units=u.pascal):
        mH = self.mesh.save(os.path.join(self.outputDir, "mesh.h5"),
                             units=u.kilometers)
        file_prefix = os.path.join(self.outputDir,
                                   'pressure-%s' % checkpointID)
        handle = self.pressure.save('%s.h5' % file_prefix, 
                                    units=units)
        self.pressure.xdmf('%s.xdmf' % file_prefix, handle, 'pressure', mH,
                           'mesh', modeltime=self.time)
    
    def save_temperature(self, checkpointID, units=u.degK):
        mH = self.mesh.save(os.path.join(self.outputDir, "mesh.h5"),
                             units=u.kilometers)
        file_prefix = os.path.join(self.outputDir,
                                   'temperature-%s' % checkpointID)
        handle = self.temperature.save('%s.h5' % file_prefix, 
                                       units=units)
        self.temperature.xdmf('%s.xdmf' % file_prefix, handle,
                              'temperature', mH,
                              'mesh', modeltime=self.time)

    def save_material(self, checkpointID):
        sH = self.swarm.save(os.path.join(self.outputDir,
                                          'swarm-%s.h5' % checkpointID),
                             units=u.kilometers)

        file_prefix = os.path.join(self.outputDir, 'material-%s' % checkpointID)
        handle = self.material.save('%s.h5' % file_prefix)
        self.material.xdmf('%s.xdmf' % file_prefix, handle, 'material', sH,
                           'swarm', modeltime=self.time)


    def plot_viscosity(self, figsize=(1200, 400), units=u.pascal*u.second, **args):
        Fig = glucifer.Figure(figsize=(1200, 400), title="Viscosity Field")
        Fig.append(glucifer.objects.Points(self.swarm,
                                           fn_colour=self.viscosityFn,
                                           fn_size=2.0,
                                           logScale=True))
        Fig.show()
        return Fig

    def plot_density(self, figsize=(1200, 400), **args):
        Fig = glucifer.Figure(figsize=(1200, 400))
        Fig.append(glucifer.objects.Points(self.swarm,
                                           fn_colour=self.densityFn,
                                           fn_size=2.0))
        Fig.show()
        return Fig

    def plot_temperature(self, figsize=(1200, 400), units=u.degK, **args):
        Fig = glucifer.Figure(figsize=(1200, 400), title="Temperature Field")
        Fig.append(glucifer.objects.Surface(self.mesh, 
                   sca.Dimensionalize(self.temperature, units)))
        Fig.show()
        return Fig

    def plot_velocity(self, figsize=(1200, 400), **args):
        Fig = glucifer.Figure(figsize=(1200, 400))
        Fig.append(glucifer.objects.Surface(self.mesh, self.velocity[0]))
        Fig.append(glucifer.objects.VectorArrows(self.mesh, self.velocity,
                                                 scaling=0.03, arrowHead=10.,
                                                 resolutionI=25, 
                                                 resolutionJ=10)) 
        Fig.show()
        return Fig

    def plot_pressure(self, figsize=(1200, 400), units=u.pascal, **kwargs):
        Fig = glucifer.Figure(figsize=(1200, 400), title="Pressure Field")
        Fig.append(glucifer.objects.Surface(self.mesh,
                   sca.Dimensionalize(self.pressure, units), **kwargs))
        Fig.show()
        return Fig
   def plot_strainRate(self, figsize=(1200, 400), **kwargs):
        Fig = glucifer.Figure(figsize=(1200, 400), title="Strain Rate")
        Fig.append(glucifer.objects.Points(self.swarm,
                                           fn_colour=self.strainRateVar,
                                           fn_size=2.0,
                                           logScale=True))
        Fig.show()
        return Fig
