from underworld import UWGeodynamics as GEO
import underworld as uw

u = GEO.u

Model = GEO.Model()
Model3D = GEO.Model(elementRes=(10, 10, 10),
                    gravity=(0.,0.,-9.81*u.m/u.s**2),
                    minCoord=(0. * u.kilometer,
                              0. * u.kilometer,
                              0. * u.kilometer),
                    maxCoord=(10. * u.kilometer,
                              10. * u.kilometer,
                              10. * u.kilometer))

def test_scaling():

    velocity = 1.0 * u.centimeter / u.hour
    model_height = 1. * u.meter
    bodyforce = 200 * u.kilogram / u.metre**3 * 9.81 * u.meter / u.second**2
    temperature = 1200 * u.degK

    KL = model_height
    Kt = KL / velocity
    KM = bodyforce * KL**2 * Kt**2
    KT = temperature

    GEO.scaling_coefficients["[length]"] = KL
    GEO.scaling_coefficients["[time]"] = Kt
    GEO.scaling_coefficients["[mass]"] = KM
    GEO.scaling_coefficients["[temperature]"] = KT

def test_adding_materials():
    air = Model.add_material(
        name="Air",
        shape=GEO.shapes.Layer(top=Model.top,
                               bottom=Model.bottom)
    )
    assert(isinstance(air, GEO.Material))

def test_material_attributes():
    Material = GEO.Material(name="Material")
    Material.density = 3000. * u.kilogram / u.metre**3
    Material.viscosity = 1e19 * u.pascal * u.second
    Material.diffusivity = 1e-6 * u.metre**2 / u.second
    Material.capacity = 1000. * u.joule / (u.kelvin * u.kilogram)
    Material.radiogenicHeatProd = 0.7 * u.microwatt / u.metre**3

def test_viscous_registry():
    rh = GEO.ViscousCreepRegistry()
    Material = GEO.Material(name="Material")
    for name, rheology in rh.__dict__["_dir"].items():
        Material.viscosity = rheology

def test_shapes():

    layer = GEO.shapes.Layer2D(top=30.*u.kilometer, bottom=0.*u.kilometer)
    polygon = GEO.shapes.Polygon(vertices=[(10.* u.kilometer, 10.*u.kilometer),
                                           (20.* u.kilometer, 35.*u.kilometer),
                                           (35.* u.kilometer, 5.*u.kilometer)])
    box = GEO.shapes.Box(top=10.* u.kilometer, bottom=5*u.kilometer,
                         minX=10.*u.kilometer, maxX=15*u.kilometer)
    disk = GEO.shapes.Disk(center=(32. * u.kilometer, 32. * u.kilometer), radius=10.*u.kilometer)

    annulus = GEO.shapes.Annulus(center=(35.*u.kilometer, 50.*u.kilometer),
                                 r1=5.*u.kilometer,
                                 r2=10.*u.kilometer)

    disk1 = GEO.shapes.Disk(center=(10. * u.kilometer, 10. * u.kilometer), radius=10.*u.kilometer)
    disk2 = GEO.shapes.Disk(center=(20. * u.kilometer, 20. * u.kilometer), radius=5.*u.kilometer)

    shape = disk1 | disk2
    shape2 = layer | polygon | box | disk | annulus

    material = Model.add_material(name="Material", shape=layer)
    material = Model.add_material(name="Material", shape=polygon)
    material = Model.add_material(name="Material", shape=box)
    material = Model.add_material(name="Material", shape=disk)
    material = Model.add_material(name="Material", shape=annulus)
    material = Model.add_material(name="Material", shape=shape)
    material = Model.add_material(name="Material", shape=shape2)

def test_plastic_registry():
    pl = GEO.PlasticityRegistry()
    Material = GEO.Material(name="Material")
    for name, rheology in pl.__dict__["_dir"].items():
        Material.plasticity = rheology

def test_set_velocity_boundary_conditions():
    velocityBCs = Model.set_velocityBCs(
        left=[1.0 * u.centimetre / u.year, None],
        right=[-1.0 * u.centimetre / u.year, None],
        bottom=[None, 0.],
        top=[None, 0.])
    assert(isinstance(velocityBCs, uw.conditions.DirichletCondition))

def test_user_defined_viscous_creep():
    viscosity = GEO.ViscousCreep(preExponentialFactor=1.0,
                                 stressExponent=1.0,
                                 activationVolume=0.,
                                 activationEnergy=200 * u.kilojoules,
                                 waterFugacity=0.0,
                                 grainSize=0.0,
                                 meltFraction=0.,
                                 grainSizeExponent=0.,
                                 waterFugacityExponent=0.,
                                 meltFractionFactor=0.0,
                                 f=1.0)
    assert(isinstance(viscosity, GEO.ViscousCreep))

def test_user_defined_drucker_prager():
    plasticity = GEO.DruckerPrager(cohesion=10. * u.megapascal,
                                   cohesionAfterSoftening=10. * u.megapascal,
                                   frictionCoefficient = 0.3,
                                   frictionAfterSoftening = 0.2,
                                   epsilon1=0.5,
                                   epsilon2=1.5)
    assert(isinstance(plasticity, GEO.DruckerPrager))

def test_temperature_boundary_condition():
    Model.set_temperatureBCs(top=500. * u.degK,
                             bottom=1000. * u.degK)
    Model.capacity = 1000. * u.joule / (u.kelvin * u.kilogram)
    Model.density = 1000. * u.kilogram / u.metre**3
    Model.set_temperatureBCs(top=500. * u.degK,
                             bottom=1200. * u.degK)

def test_passive_tracers():
    import numpy as np
    npoints = 1000
    coords = np.ndarray((1000, 2))
    coords[:, 0] = np.linspace(GEO.nd(Model.minCoord[0]), GEO.nd(Model.maxCoord[0]), npoints)
    coords[:, 1] = GEO.nd(32. * u.kilometer)
    Model.add_passive_tracers(name="Tracers", vertices=coords)

def test_set_velocity_boundary_conditions_in_3D():
    velocityBCs = Model3D.set_velocityBCs(
        left=[1.0 * u.centimetre / u.year, None, 0.],
        right=[-1.0 * u.centimetre / u.year, None, 0.],
        bottom=[None, None, 0.],
        top=[None, None, 0.],
        front=[None, 0., None],
        back=[None, 0., None])
    assert(isinstance(velocityBCs, uw.conditions.DirichletCondition))
