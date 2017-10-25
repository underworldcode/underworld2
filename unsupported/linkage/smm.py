import os
import pandas
import xml.etree.ElementTree as ET

from linkage import LinkageModel
from pyBadlands.model import Model as BadlandsModel
import underworld as uw
from underworld import function as fn


class SimpleMaterialModel(object):
    """
    A SimpleMaterialModel streamlines model development by making some
    simplifying assumptions:
    * There will be an Underworld mesh/swarm. You can choose the dimensions and
      resolution.
    * The only change you make to the Underworld model is the material types.
      You can define any material types you like
    * Standard solver
    * All output in the same directory, automatically incrementing on each checkpoint
    * Tracking velocity, pressure and material type from Underworld
    """
    def __init__(self, badlands_config, underworld_resolution, materials, min_coords=None, max_coords=None, badlands_resolution=None, surface_elevation=None, elev_range=None):
        self.linkage = LinkageModel()

        # Set up the Badlands model
        badlands_model = BadlandsModel()
        badlands_model.load_xml(badlands_config)
        self.linkage.badlands_model = badlands_model

        # Load the Badlands config to find out if we're using a DEM file
        tree = ET.parse(badlands_config)
        root = tree.getroot()

        # Is a demfile loaded already through the badlands config?
        try:
            # This will throw an exception if not defined
            demfile = root.find('grid').find('demfile').text

            # The DEM sets the bounds of the model
            dem = pandas.read_csv(demfile, sep=' ', header=None, na_filter=False, dtype=numpy.float, low_memory=False)
            min_coords = [dem[0].min(), dem[1].min(), elev_range[0]]
            max_coords = [dem[0].max(), dem[1].max(), elev_range[1]]
        except (ValueError, AttributeError):
            assert min_coords is not None and max_coords is not None and badlands_resolution is not None and surface_elevation is not None, "If no DEM file is loaded in the Badlands XML config, you must supply the min_coords, max_coords, badlands_resolution and surface_elevation parameters"
            dem = self.linkage.generate_flat_dem(minCoord=min_coords, maxCoord=max_coords, resolution=badlands_resolution, elevation=surface_elevation)
            self.linkage.load_badlands_dem_array(dem)

        # Configure the linkage material map
        # TODO this needs to be updated when we have multiple erodibility layers in Badlands
        mm = [[], []]  # we assume air and sediment layers
        for item in materials:
            mm[item['bl_layer']].append(item['uw_index'])
        self.linkage.material_map = mm

        ### SET UP THE UNDERWORLD MODEL
        mesh = uw.mesh.FeMesh_Cartesian(elementType=("Q1/dQ0"),
                                        elementRes =underworld_resolution,
                                        minCoord   =min_coords,
                                        maxCoord   =max_coords)
        self.linkage.mesh = mesh

        # We want to track velocity and pressure.
        velocityField = uw.mesh.MeshVariable(mesh=mesh, nodeDofCount=mesh.dim)
        self.linkage.velocity_field = velocityField
        pressureField = uw.mesh.MeshVariable(mesh=mesh.subMesh, nodeDofCount=1)
        self.pressure_field = pressureField

        # Set initial states
        velocityField.data[:] = [0., 0., 0.]
        pressureField.data[:] = 0.

        # Create the swarm, material index variable and swarm advector
        swarm = uw.swarm.Swarm(mesh=mesh)
        self.linkage.swarm = swarm
        materialIndex = swarm.add_variable(dataType="int", count=1)
        self.linkage.material_index = materialIndex

        swarmLayout = uw.swarm.layouts.GlobalSpaceFillerLayout(swarm=swarm, particlesPerCell=20)
        swarm.populate_using_layout(layout=swarmLayout)

        self.advector = uw.systems.SwarmAdvector(swarm=swarm, velocityField=velocityField, order=2)

        # Set viscosities and densities of the model.
        viscosityMapFn = 1e19

        density_map = {}
        for item in materials:
            density_map[item['uw_index']] = item['density']
        densityFn = fn.branching.map(fn_key=materialIndex, mapping=density_map)

        # And the final buoyancy force function.
        buoyancyFn = densityFn * 9.8 * [0.0, 0.0, -1.0]

        # wall velocity boundary conditions - free slip on all walls
        iWalls = mesh.specialSets["MinI_VertexSet"] + mesh.specialSets["MaxI_VertexSet"]
        jWalls = mesh.specialSets["MinJ_VertexSet"] + mesh.specialSets["MaxJ_VertexSet"]
        kWalls = mesh.specialSets["MinK_VertexSet"] + mesh.specialSets["MaxK_VertexSet"]
        velocityBC = uw.conditions.DirichletCondition(variable=velocityField,
                                                      indexSetsPerDof=(iWalls, jWalls, kWalls))

        # combine all the above into Stokes system and get solver
        stokesPIC = uw.systems.Stokes(velocityField=velocityField,
                                      pressureField=pressureField,
                                      voronoi_swarm=swarm,
                                      conditions   =[velocityBC, ],
                                      fn_viscosity =viscosityMapFn,
                                      fn_bodyforce =buoyancyFn)
        self.solver = uw.systems.Solver(stokesPIC)

        # FINISH SETTING UP LINKAGE
        self.linkage.update_function = self.update_function
        self.linkage.checkpoint_function = self.checkpoint_function

        # stuff that gets exported to user
        self.swarm = swarm
        self.particle_coordinates = swarm.particleCoordinates
        self.material_index = materialIndex

        ### SET UP THE OUTPUT DIRECTORY
        self.out_dir = badlands_model.input.outDir

        # Store the SimpleMaterialModel instance for later
        self.linkage.smm = self

    @staticmethod
    def update_function(linkage, max_seconds):
        smm = linkage.smm

        # Get solution for initial configuration.
        smm.solver.solve()

        # Determine the maximum possible timestep for the advection system.
        dtmax_seconds = smm.advector.get_max_dt()
        dt_seconds = min(max_seconds, dtmax_seconds)

        smm.advector.integrate(dt_seconds)

        return dt_seconds

    @staticmethod
    def checkpoint_function(linkage, checkpoint_number, time_years):
        smm = linkage.smm

        mH = linkage.mesh.save(os.path.join(smm.out_dir, "mesh.h5"))

        file_prefix = os.path.join(smm.out_dir, 'velocity-%s' % checkpoint_number)
        handle = linkage.velocity_field.save('%s.h5' % file_prefix)
        linkage.velocity_field.xdmf('%s.xdmf' % file_prefix, handle, 'velocity', mH, 'mesh', modeltime=time_years)

        file_prefix = os.path.join(smm.out_dir, 'pressure-%s' % checkpoint_number)
        handle = smm.pressure_field.save('%s.h5' % file_prefix)
        smm.pressure_field.xdmf('%s.xdmf' % file_prefix, handle, 'pressure', mH, 'mesh', modeltime=time_years)

        sH = linkage.swarm.save(os.path.join(smm.out_dir, 'swarm-%s.h5' % checkpoint_number))

        file_prefix = os.path.join(smm.out_dir, 'material-%s' % checkpoint_number)
        handle = smm.material_index.save('%s.h5' % file_prefix)
        smm.material_index.xdmf('%s.xdmf' % file_prefix, handle, 'material', sH, 'swarm', modeltime=time_years)

    def run_for_years(self, years):
        self.linkage.run_for_years(years)
