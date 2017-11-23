import os
import types
import underworld as uw
import underworld.function as fn
import numpy as np
import scaling as sca
from scaling import nonDimensionalize as nd
from lithopress import LithostaticPressure
from LecodeIsostasy import LecodeIsostasy
from utils import PressureSmoother, ViscosityLimiter, PassiveTracers
from Material import Material
from Plots import Plots
from visugrid import Visugrid
from kinematicBCs import VelocityBCs
from temperatureBCs import TemperatureBCs
import surfaceProcesses
import shapes
import sys

u = UnitRegistry = sca.UnitRegistry

class Model(Material):
    def __init__(self, elementRes, minCoord, maxCoord,
                 gravity=(0.0, -9.81 * u.meter / u.second**2),
                 periodic=None, elementType="Q1/dQ0",
                 swarmLayout=None, Tref=273.15 * u.degK, name="undefined",
                 outputDir="outputs", populationControl=True, scaling=None,
                 minViscosity=1e19*u.pascal*u.second,
                 maxViscosity=1e25*u.pascal*u.second,
                 strainRate_default=1e-15 / u.second):

        super(Model, self).__init__()

        self.name = name
        self.top = maxCoord[-1]
        self.bottom = minCoord[-1]

        self.outputDir = outputDir
        self.checkpointID = 0
        self._checkpoint = None
        self.checkpoint = None

        self.viscosityLimiter = ViscosityLimiter(minViscosity, maxViscosity)

        self.gravity = tuple([nd(val) for val in gravity])
        self.Tref = Tref
        self.elementType = elementType
        self.elementRes = elementRes
        self.minCoord = minCoord
        self.maxCoord = maxCoord

        if not periodic:
            self.periodic = tuple([False for val in range(self.mesh.dim)])
        else:
            self.periodic = periodic

        minCoord = tuple([nd(val) for val in self.minCoord])
        maxCoord = tuple([nd(val) for val in self.maxCoord])

        self.mesh = uw.mesh.FeMesh_Cartesian(elementType=self.elementType,
                                             elementRes=self.elementRes,
                                             minCoord=minCoord,
                                             maxCoord=maxCoord,
                                             periodic=self.periodic)
        
        # Add common fields
        self.temperature = None
        self.pressureField = uw.mesh.MeshVariable(mesh=self.mesh.subMesh, nodeDofCount=1)
        self.velocityField = uw.mesh.MeshVariable(mesh=self.mesh, nodeDofCount=self.mesh.dim)
        self.tractionField = uw.mesh.MeshVariable(mesh=self.mesh.subMesh, nodeDofCount=1)
        self.strainRateField = uw.mesh.MeshVariable(mesh=self.mesh, nodeDofCount=1)
        self.pressureField.data[...] = 0.
        self.velocityField.data[...] = 0.
       
        # symmetric component of the gradient of the flow velocityField.
        self.strainRate_default = strainRate_default
        self.solutionExist = False
        self.strainRate = fn.tensor.symmetric(self.velocityField.fn_gradient)
        self.strainRate_2ndInvariant = fn.tensor.second_invariant(self.strainRate)
 
        # Create the material swarm
        self.swarm = uw.swarm.Swarm(mesh=self.mesh, particleEscape=True)
        self.swarmLayout = swarmLayout
        self.population_control = populationControl
        self.swarm_advector = uw.systems.SwarmAdvector(swarm=self.swarm,
                                  velocityField=self.velocityField,
                                  order=2)

        self.materials = []
        self._defaultMaterial = 0

        if self.mesh.dim == 2:
            # Create a series of aliases for the boundary sets
            self.leftWall   = self.mesh.specialSets["MinI_VertexSet"]
            self.topWall    = self.mesh.specialSets["MaxJ_VertexSet"]
            self.bottomWall = self.mesh.specialSets["MinJ_VertexSet"]
            self.rightWall  = self.mesh.specialSets["MaxI_VertexSet"]
        
        if self.mesh.dim == 3:
            self.leftWall   = self.mesh.specialSets["MinI_VertexSet"]
            self.rightWall  = self.mesh.specialSets["MaxI_VertexSet"]
            self.frontWall = self.mesh.specialSets["MinJ_VertexSet"]
            self.backWall  = self.mesh.specialSets["MaxJ_VertexSet"]
            self.topWall    = self.mesh.specialSets["MaxK_VertexSet"]
            self.bottomWall = self.mesh.specialSets["MinK_VertexSet"]

        # Boundary Conditions
        self.velocityBCs = None
        self.temperatureBCs = None

        self.time = 0.0 * u.megayears
        self.step = 0
        self.Isostasy = None

        self.pressSmoother = PressureSmoother(self.mesh, self.pressureField)
        self.surfaceProcesses = None
        
        self.nonLinearTolerance = 1.0e-2

        # Passive Tracers
        self.passiveTracers = []

        # Plots
        self.plot = Plots(self)

        # Visugrid
        self._visugrid = None
        
        self._initialize()

    def _initialize(self):

        # Add Common Swarm Variables
        self.materialField = self.swarm.add_variable(dataType="int", count=1)
        self.plasticStrain = self.swarm.add_variable(dataType="double", count=1)
        self.viscosityField = self.swarm.add_variable(dataType="double", count=1)
        self.densityField = self.swarm.add_variable(dataType="double", count=1)
        self.plasticStrain.data[...] = 0.0
        self.viscosityField.data[...] = 0.
        self.densityField.data[...] = 0.
 

        # Create a bunch of tools to project swarmVariable onto the mesh
        self._projMaterialField  = uw.mesh.MeshVariable( mesh=self.mesh,nodeDofCount=1)
        self._materialFieldProjector = uw.utils.MeshVariable_Projection(self._projMaterialField, self.materialField, type=0)

        self._projViscosityField = uw.mesh.MeshVariable(mesh=self.mesh, nodeDofCount=1)
        self._viscosityFieldProjector = uw.utils.MeshVariable_Projection(self._projViscosityField, self.viscosityField, type=0)

        self._projPlasticStrain = uw.mesh.MeshVariable(mesh=self.mesh, nodeDofCount=1)
        self._plasticStrainProjector = uw.utils.MeshVariable_Projection(self._projPlasticStrain,
                                                                     self.plasticStrain, type=0)
        
        self._projStrainRate = uw.mesh.MeshVariable(mesh=self.mesh, nodeDofCount=1)
        self._strainRateProjector = uw.utils.MeshVariable_Projection(self._projStrainRate,
                                                     self.strainRate_2ndInvariant, type=0)
        
        self._projDensityField = uw.mesh.MeshVariable(mesh=self.mesh, nodeDofCount=1)
        self._densityFieldProjector = uw.utils.MeshVariable_Projection(self._projDensityField, self.densityField, type=0)

    @property
    def outputDir(self):
        return self._outputDir

    def restart(self, restartDir=None, step=None):
        if not restartDir:
            restartDir = self._outputDir
        if not step:
            step = max([int(os.path.splitext(file)[0].split("-")[-1]) 
                        for file in os.listdir(restartDir) if "-" in file])


        self.checkpointID = step
        self.mesh.load(os.path.join(restartDir, "mesh.h5"))
        self.swarm = uw.swarm.Swarm(mesh=self.mesh, particleEscape=True)
        self.swarm.load(os.path.join(restartDir, 'swarm-%s.h5' % step))
        self._initialize()
        self.materialField.load(os.path.join(restartDir, "material-%s.h5" % step))
        self.temperature.load(os.path.join(restartDir, 'temperature-%s.h5' % step))
        self.pressureField.load(os.path.join(restartDir, 'pressureField-%s.h5' % step))
        self.plasticStrain.load(os.path.join(restartDir, 'pstrain-%s.h5' % step))
        self.velocityField.load(os.path.join(restartDir, 'velocityField-%s.h5' % step))

    @property
    def surfaceProcesses(self):
        return self._surfaceProcesses
    
    @property
    def projMaterialField(self):
        self._materialFieldProjector.solve()
        return self._projMaterialField
    
    @property
    def projPlasticStrain(self):
        self._plasticStrainProjector.solve()
        return self._projPlasticStrain
    
    @property
    def projStrainRate(self):
        self._strainRateProjector.solve()
        return self._projStrainRate
    
    @property
    def projViscosityField(self):
        self.viscosityField.data[...] = self.viscosityFn.evaluate(self.swarm)
        self._viscosityFieldProjector.solve()
        return self._projViscosityField
    
    @property
    def projDensityField(self):
        self.densityField.data[...] = self.densityFn.evaluate(self.swarm)
        self._densityFieldProjector.solve()
        return self._projDensityField

    @surfaceProcesses.setter
    def surfaceProcesses(self, value):
        self._surfaceProcesses = value
        if isinstance(value, surfaceProcesses.Badlands):
            self._surfaceProcesses.Model = self

    @outputDir.setter
    def outputDir(self, value):
        if uw.rank() == 0:
            if not os.path.exists(value):
                os.makedirs(value)
        self._outputDir = value

    @property
    def swarmLayout(self):
        return self._swarmLayout

    @swarmLayout.setter
    def swarmLayout(self, value):
        
        if value is not None:
            self._swarmLayout = value
        else:
            self._swarmLayout = uw.swarm.layouts.GlobalSpaceFillerLayout(
                                                 swarm=self.swarm,
                                                 particlesPerCell=50)

        self.swarm.populate_using_layout(layout=self._swarmLayout)

    @property
    def population_control(self):
        return self._population_control

    @population_control.setter
    def population_control(self, value):
        self._population_control = uw.swarm.PopulationControl(
                 self.swarm,
                 aggressive=True, splitThreshold=0.15,
                 maxSplits=10,
                 particlesPerCell=50
            )

    def set_temperatureBCs(self, left=None, right=None, top=None, bottom=None,
                           front=None, back=None,
                           indexSets=[], materials=[(None,None)]):
        
        if not self.temperature:
            self.temperature = uw.mesh.MeshVariable(mesh=self.mesh,
                                                    nodeDofCount=1)
            self._temperatureDot = uw.mesh.MeshVariable(mesh=self.mesh,
                                                    nodeDofCount=1)
            self.temperature.data[...] = nd(self.Tref)
            self._temperatureDot.data[...] = 0.
        
        self.temperatureBCs = TemperatureBCs(self, left=left, right=right,
                                             top=top, bottom=bottom, 
                                             indexSets=indexSets,
                                             materials=materials)

    @property
    def _temperatureBCs(self):
        if not self.temperatureBCs:
            raise ValueError("Set Boundary Conditions")
        return self.temperatureBCs.get_conditions()

    def init_advection_diffusion(self):
        DiffusivityMap = {}
        for material in self.materials:
            DiffusivityMap[material.index] = nd(material.diffusivity)
        
        self.DiffusivityFn = fn.branching.map(fn_key=self.materialField,
                                              mapping=DiffusivityMap)
       
        HeatProdMap = {}
        for material in self.materials:
            HeatProdMap[material.index] = (nd(material.radiogenicHeatProd) /
                                           (nd(material.density) *
                                            nd(material.capacity)))
        
        self.HeatProdFn = fn.branching.map(fn_key=self.materialField,
                                           mapping=HeatProdMap)
        
        self.advdiffSystem = uw.systems.AdvectionDiffusion(
                self.temperature,
                self._temperatureDot,
                velocityField=self.velocityField,
                fn_diffusivity=self.DiffusivityFn,
                fn_sourceTerm=self.HeatProdFn,
                conditions=[self._temperatureBCs]
            )

    def init_stokes_system(self):
        self.buoyancyFn = self.densityFn * self.gravity

        if any([material.viscosity for material in self.materials]): 
            
            stokes = uw.systems.Stokes(velocityField=self.velocityField,
                                       pressureField=self.pressureField,
                                       conditions=self._velocityBCs,
                                       fn_viscosity=self.viscosityFn,
                                       fn_bodyforce=self.buoyancyFn,
                                       fn_one_on_lambda = None)

            self.solver = uw.systems.Solver(stokes)
    
    def set_velocityBCs(self, left=None, right=None, top=None, bottom=None,
                        front=None, back=None, indexSets=[]):
       
        self.velocityBCs = VelocityBCs(self, left=left,
                                       right=right, top=top,
                                       bottom=bottom, front=front,
                                       back=back, indexSets=indexSets) 
    
    @property
    def _velocityBCs(self):
        if not self.velocityBCs:
            raise ValueError("Set Boundary Conditions")
        return self.velocityBCs.get_conditions()

    def add_material(self, vertices=None, reset=False, name="unknown",
                     shape=None):

        if reset:
            self.materials = []

        if vertices:
            vertices = [(nd(x), nd(y)) for x, y in vertices]
        
        mat = Material()
                
        mat.name = name
        mat.vertices           = vertices
        mat.diffusivity        = self.diffusivity
        mat.capacity           = self.capacity
        mat.thermalExpansivity = self.thermalExpansivity
        mat.radiogenicHeatProd = self.radiogenicHeatProd
        
        if isinstance(shape, shapes.Layer):
            shape.minX = self.minCoord[0]
            shape.maxX = self.maxCoord[0]
            mat.top = shape.top
            mat.bottom = shape.bottom

            if self.mesh.dim == 3:
                shape.minY = self.minCoord[1]
                shape.maxY = self.maxCoord[1]

        mat.shape              = shape
        mat.indices = self._get_material_indices(mat)
        self.materials.reverse()
        self.materials.append(mat)
        self.materials.reverse()
        self._fill_model()

        return mat

    def _fill_model(self):

        conditions = [(obj.shape.fn, obj.index)
                      for obj in self.materials if obj.shape is not None]

        conditions.append((True, self._defaultMaterial))
        self.materialField.data[:] = fn.branching.conditional(conditions).evaluate(self.swarm)

    @property
    def material_drawing_order(self):
        return self.material_drawing_order()

    @material_drawing_order.setter
    def material_drawing_order(self, value):
        self._material_drawing_order = value
        self.materials = value
        self._fill_model()

    @property
    def densityFn(self):
        return self._densityFn()

    def _densityFn(self):
        densityMap = {}
        for material in self.materials:
            if not material.density:
                raise ValueError("Please set %s density" % material.name)

            if self.temperature:
                densityMap[material.index] = nd(material.density) * (1.0 -
                                             nd(material.thermalExpansivity)
                                             * (self.temperature - nd(self.Tref)))
            else:
                densityMap[material.index] = nd(material.density)

        return fn.branching.map(fn_key=self.materialField, mapping=densityMap)

    def _frictionFn(self):

        friction = {}

        for material in self.materials:
            if material.plasticity:
                friction[material.index] = material.plasticity._friction
            else:
                friction[material.index] = 0.

        return fn.branching.map(self.materialField, mapping=friction)

    @property
    def viscosityFn(self):
        return self._viscosityFn()

    def _viscosityFn(self):

        ViscosityMap = {}
        BGViscosityMap = {}

        # Viscous behavior
        for material in self.materials:
            if material.viscosity:
                ViscosityHandler = material.viscosity
                ViscosityHandler.pressureField = self.pressureField
                ViscosityHandler.strainRateInvariantField = self.strainRate_2ndInvariant
                ViscosityHandler.temperatureField = self.temperature
                ViscosityHandler.viscosityLimiter = self.viscosityLimiter
                ViscosityMap[material.index] = ViscosityHandler.muEff

        # Plasticity
        PlasticityMap = {}
        for material in self.materials:
            if material.plasticity:
                YieldHandler = material.plasticity
                YieldHandler.pressureField = self.pressureField
                YieldHandler.plasticStrain = self.plasticStrain
                if self.mesh.dim == 2:
                    yieldStress = YieldHandler._get_yieldStress2D()
                if self.mesh.dim == 3:
                    yieldStress = YieldHandler._get_yieldStress3D()
                eijdef =  nd(self.strainRate_default)
                eij = fn.branching.conditional([(self.strainRate_2ndInvariant < sys.float_info.epsilon, eijdef),
                                              (True, self.strainRate_2ndInvariant)])
                muEff = 0.5 * yieldStress / eij
                muEff = self.viscosityLimiter.apply(muEff)
                PlasticityMap[material.index] = muEff

        # Combine rheologies
        EffViscosityMap = {}
        PlasticMap = {}
        for material in self.materials:
            idx = material.index
            if material.viscosity and material.plasticity:
                EffViscosityMap[idx] = fn.misc.min(PlasticityMap[idx], ViscosityMap[idx])
                BGViscosityMap[idx] = ViscosityMap[idx]
                PlasticMap[idx] = 0.
            elif material.viscosity:
                EffViscosityMap[idx] = ViscosityMap[idx]
                BGViscosityMap[idx] = ViscosityMap[idx]
                PlasticMap[idx] = 0.
            elif material.plasticity:
                EffViscosityMap[idx] = PlasticityMap[idx]
                BGViscosityMap[idx] = PlasticityMap[idx]
                PlasticMap[idx] = 1.0

        viscosityFn = fn.branching.map(fn_key=self.materialField, mapping=EffViscosityMap)
        backgroundViscosityFn = fn.branching.map(fn_key=self.materialField,mapping=BGViscosityMap)

        isPlastic = fn.branching.map(fn_key=self.materialField, mapping=PlasticMap)
        yieldConditions = [(viscosityFn < backgroundViscosityFn, 1.0),
                           (isPlastic > 0.5, 1.0),
                           (True, 0.0)]

        self.isYielding = fn.branching.conditional(yieldConditions) * self.strainRate_2ndInvariant
        
        return viscosityFn

    @property
    def yieldStressFn(self):
        return self._yieldStressFn()

    def _yieldStressFn(self):
        eij = self.strainRate_2ndInvariant
        eijdef =  nd(self.strainRate_default)
        return 2.0 * self.viscosityFn * fn.misc.max(eij, eijdef)

    def solve_temperature_steady_state(self):
        
        if self.materials:
            DiffusivityMap = {}
            for material in self.materials:
                DiffusivityMap[material.index] = nd(material.diffusivity)
            
            self.DiffusivityFn = fn.branching.map(fn_key = self.materialField, mapping = DiffusivityMap)
       
            HeatProdMap = {}
            for material in self.materials:
                HeatProdMap[material.index] = nd(material.radiogenicHeatProd)/(nd(material.density)*nd(material.capacity))
            
            self.HeatProdFn = fn.branching.map(fn_key = self.materialField, mapping = HeatProdMap)
        else:
            self.DiffusivityFn = fn.misc.constant(nd(self.diffusivity))
            self.HeatProdFn = fn.misc.constant(nd(self.radiogenicHeatProd))

        conditions = self._temperatureBCs
        heatequation = uw.systems.SteadyStateHeat(temperatureField=self.temperature,
                                                  fn_diffusivity=self.DiffusivityFn,
                                                  fn_heating=self.HeatProdFn,
                                                  conditions=conditions)
        heatsolver = uw.systems.Solver(heatequation)
        heatsolver.solve(nonLinearIterate=True)

    def solve_lithostatic_pressureField(self):
        gravity = np.abs(self.gravity[-1])  # Ugly!!!!!
        lithoPress = LithostaticPressure(self.mesh, self.densityFn, gravity)
        self.pressureField.data[:], LPresBot = lithoPress.solve()
        self.pressSmoother.smooth()

    def _calibrate_pressureField(self):
        surfaceArea = uw.utils.Integral(fn=1.0, mesh=self.mesh,
                                        integrationType='surface',
                                        surfaceIndexSet=self.topWall)
        surfacepressureFieldIntegral = uw.utils.Integral(
                fn=self.pressureField,
                mesh=self.mesh,
                integrationType='surface',
                surfaceIndexSet=self.topWall
            )
        (area,) = surfaceArea.evaluate()
        (p0,) = surfacepressureFieldIntegral.evaluate()
        offset = p0/area
        self.pressureField.data[:] -= offset
        self.pressSmoother.smooth()
        
        for material in self.materials:
            if material.viscosity:
                material.viscosity.firstIter = False

    def _get_material_indices(self, material):
        nodes = np.arange(0,self.mesh.nodesLocal)[self.projMaterialField.evaluate(self.mesh.data[0:self.mesh.nodesLocal])[:,0] == material.index]
        return uw.mesh.FeMesh_IndexSet(self.mesh, topologicalIndex=0, size=self.mesh.nodesGlobal, fromObject=nodes)

    def solve(self):
        self.solver.solve(nonLinearIterate=True,
                          callback_post_solve=self._calibrate_pressureField,
                          nonLinearTolerance=self.nonLinearTolerance)

    def init_model(self, temperature=True, pressureField=True):
        
        # Init Temperature Field
        if self.temperature and temperature:
            self.solve_temperature_steady_state()
            self.init_advection_diffusion()

        # Init pressureField Field
        if self.pressureField and pressureField:
            self.solve_lithostatic_pressureField()
        
        self.init_stokes_system()
    
    def run_for(self, endTime=None, checkpoint=None, timeCheckpoints=[]):
        step = self.step
        time = nd(self.time)
        units = endTime.units
        endTime = time + nd(endTime)

        next_checkpoint = None

        if timeCheckpoints:
            timeCheckpoints = [nd(val) for val in timeCheckpoints]

        if checkpoint:
            next_checkpoint = time + nd(checkpoint)
       
        if self.temperature:
            self.init_advection_diffusion()
        
        self.init_stokes_system()
       
        while time < endTime:
            self.solve()

            # Whats the longest we can run before reaching the end of the model
            # or a checkpoint?
            # Need to generalize that
            dt = self.swarm_advector.get_max_dt()

            if self.temperature:
                dt = min(dt, self.advdiffSystem.get_max_dt())

            if checkpoint:
                dt = min(dt, next_checkpoint - time)
            
            self._dt = min(dt, endTime - time)

            uw.barrier()
            
            self.update()

            step += 1
            self.time += sca.Dimensionalize(self._dt, units)
            time += self._dt
            
            if time == next_checkpoint:
                self.checkpointID += 1
                self.checkpoint()
                self.output_glucifer_figures(self.checkpointID)
                next_checkpoint += nd(checkpoint)

            if checkpoint or step % 1 == 0:
                if uw.rank() == 0:
                    print "Time: ", str(self.time.to(units))

    def update(self):

        dt = self._dt
        # Increment plastic strain
        plasticStrainIncrement = dt * self.isYielding.evaluate(self.swarm)
        self.plasticStrain.data[:] += plasticStrainIncrement
        
        # Solve for temperature
        if self.temperature:
            self.advdiffSystem.integrate(dt)

        # Integrate Swarms in time
        self.swarm_advector.integrate(dt, update_owners=True)
        
        if self.passiveTracers:
            for tracers in self.passiveTracers:
                tracers.integrate(dt)
        
        # Do pop control
        self.population_control.repopulate()

        if self.surfaceProcesses:
            self.surfaceProcesses.solve(dt)

        if self.Isostasy:
            self.Isostasy.solve()

        if self._visugrid:
            self._visugrid.advect(dt)

    def _default_checkpoint_function(self):
        self.save_velocityField(self.checkpointID)
        self.save_pressureField(self.checkpointID)
        if self.temperature:
            self.save_temperature(self.checkpointID)
        self.save_material(self.checkpointID)
        self.save_plasticStrain(self.checkpointID)

    def add_passive_tracers(self, name=None, vertices=None, particleEscape=True):
        self.passiveTracers.append(PassiveTracers(self.mesh,
                                                  self.velocityField,
                                                  name=name,
                                                  vertices=vertices,
                                                  particleEscape=particleEscape))

    @property
    def lambdaFn(self):
        materialMap = {}
        if any([material.compressibility for material in self.materials]): 
            for material in self.materials:
                if material.compressibility:
                    materialMap[material.index] = material.compressibility

            return uw.function.branching.map(fn_key=self.materialField,
                   mapping=materialMap, fn_default=0.0)
        return None

    @property
    def checkpoint(self):
        return self._checkpoint

    @checkpoint.setter
    def checkpoint(self, func=None):
        if func:
            self._checkpoint = types.MethodType(func, self)
        else:
            self._checkpoint = self._default_checkpoint_function

    def output_glucifer_figures(self, step):
        import glucifer
        GluciferStore = glucifer.Store(os.path.join(self.outputDir, "glucifer"))
        GluciferStore.step = step
   
        pressure = self.plot.pressureField(store=GluciferStore, show=False)
        pressure.save()
        
        temperature = self.plot.temperature(store=GluciferStore, show=False)
        temperature.save()
        
        velocity = self.plot.velocityField(store=GluciferStore, show=False)
        velocity.save()
        
        strainrate = self.plot.strainRate(store=GluciferStore, show=False)
        strainrate.save()
        
        material = self.plot.material(projected=True, store=GluciferStore, show=False)
        material.save()
        
        strain = self.plot.strain(projected=True, store=GluciferStore, show=False)
        strain.save()
        
        density = self.plot.density(projected=True, store=GluciferStore, show=False)
        density.save()

        viscosity = self.plot.viscosity(projected=True, store=GluciferStore, show=False)
        viscosity.save()

    def add_visugrid(self, elementRes, minCoord=None, maxCoord=None):
        if not maxCoord:
            maxCoord = self.maxCoord

        if not minCoord:
            minCoord = self.minCoord

        self._visugrid = Visugrid(self, elementRes, minCoord, maxCoord, self.velocityField)

    def save_velocityField(self, checkpointID, units=u.centimeter/u.year):
        mH = self.mesh.save(os.path.join(self.outputDir, "mesh.h5"), units=u.kilometers)
        file_prefix = os.path.join(self.outputDir, 'velocityField-%s' % checkpointID)
        handle = self.velocityField.save('%s.h5' % file_prefix, units=units)
        self.velocityField.xdmf('%s.xdmf' % file_prefix, handle,'velocityField', mH, 'mesh', modeltime=self.time.magnitude)
    
    def save_pressureField(self, checkpointID, units=u.pascal):
        mH = self.mesh.save(os.path.join(self.outputDir, "mesh.h5"), units=u.kilometers)
        file_prefix = os.path.join(self.outputDir, 'pressureField-%s' % checkpointID)
        handle = self.pressureField.save('%s.h5' % file_prefix, units=units)
        self.pressureField.xdmf('%s.xdmf' % file_prefix, handle, 'pressureField', mH, 'mesh', modeltime=self.time.magnitude)
   
    def save_temperature(self, checkpointID, units=u.degK):
        mH = self.mesh.save(os.path.join(self.outputDir, "mesh.h5"), units=u.kilometers)
        file_prefix = os.path.join(self.outputDir, 'temperature-%s' % checkpointID)
        handle = self.temperature.save('%s.h5' % file_prefix, units=units)
        self.temperature.xdmf('%s.xdmf' % file_prefix, handle, 'temperature', mH, 'mesh', modeltime=self.time.magnitude)
    
    def save_strainRate(self, checkpointID, units=1.0/u.seconds):
        mH = self.mesh.save(os.path.join(self.outputDir, "mesh.h5"), units=u.kilometers)
        self.strainRateField.data[:] = self.strainRate_2ndInvariant.evaluate(self.mesh)
        file_prefix = os.path.join(self.outputDir, 'strainRate-%s' % checkpointID)
        handle = self.strainRateField.save('%s.h5' % file_prefix, units=units)            
        self.strainRateField.xdmf('%s.xdmf' % file_prefix, handle,
                                          'strainRate', mH, 'mesh',
                                          modeltime=self.time.magnitude)

    def save_material(self, checkpointID, onMesh=False):
        if onMesh:
            mH = self.mesh.save(os.path.join(self.outputDir, "mesh.h5"), units=u.kilometers)
            file_prefix = os.path.join(self.outputDir, 'material2-%s' % checkpointID)
            handle = self.projMaterialField.save('%s.h5' % file_prefix)
            self.projMaterialField.xdmf('%s.xdmf' % file_prefix, handle, 'material2', mH,
                                        'mesh', modeltime=self.time.magnitude)
        else:
            sH = self.swarm.save(os.path.join(self.outputDir, 'swarm-%s.h5' % checkpointID), units=u.kilometers)
            file_prefix = os.path.join(self.outputDir, 'material-%s' % checkpointID)
            handle = self.materialField.save('%s.h5' % file_prefix)
            self.materialField.xdmf('%s.xdmf' % file_prefix, handle, 'material', sH,
                               'swarm', modeltime=self.time.magnitude)

    def save_plasticStrain(self, checkpointID, onMesh=False):
        if onMesh:
            pass
        else:
            sH = self.swarm.save(os.path.join(self.outputDir, 'swarm-%s.h5' % checkpointID), units=u.kilometers)
            file_prefix = os.path.join(self.outputDir, 'pstrain-%s' % checkpointID)
            handle = self.plasticStrain.save('%s.h5' % file_prefix)
            self.plasticStrain.xdmf('%s.xdmf' % file_prefix, handle, 'pstrain', sH, 'swarm', modeltime=self.time.magnitude)

    def save_viscosityField(self, checkpointID, onMesh=False):
        if onMesh:
            pass
        else:
            sH = self.swarm.save(os.path.join(self.outputDir, 'swarm-%s.h5' % checkpointID), units=u.kilometers)
            file_prefix = os.path.join(self.outputDir, 'viscosity-%s' % checkpointID)
            self.viscosityField.data[:] = self.viscosityFn.evaluate(self.swarm)
            handle = self.viscosityField.save('%s.h5' % file_prefix)
            self.viscosityField.xdmf('%s.xdmf' % file_prefix, handle, 'viscosity', sH, 'swarm', modeltime=self.time.magnitude)
    
    def save_densityField(self, checkpointID, onMesh=False):
        if onMesh:
            pass
        else:
            sH = self.swarm.save(os.path.join(self.outputDir, 'swarm-%s.h5' % checkpointID), units=u.kilometers)
            file_prefix = os.path.join(self.outputDir, 'density-%s' % checkpointID)
            self.densityField.data[:] = self.densityFn.evaluate(self.swarm)
            handle = self.densityField.save('%s.h5' % file_prefix)
            self.densityField.xdmf('%s.xdmf' % file_prefix, handle, 'density', sH, 'swarm', modeltime=self.time.magnitude)
