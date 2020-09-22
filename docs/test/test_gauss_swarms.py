import underworld as uw
res = 4
mesh = uw.mesh.FeMesh_Cartesian(elementRes=(res,res))
v = mesh.add_variable(2)
p = mesh.subMesh.add_variable(1)
t = mesh.add_variable(1)
diffusivity = 1.
counted_diffusivity = uw.function.view.count(diffusivity)


for i in range(1,5):
    counted_diffusivity.reset()
    gaussSwarm = uw.swarm.GaussIntegrationSwarm(mesh,i)
    sys = uw.systems.Stokes(v,p, fn_viscosity=counted_diffusivity, fn_bodyforce=(0.,0.),gauss_swarm=gaussSwarm)
    solver = uw.systems.Solver(sys).solve()
    count_encountered  = counted_diffusivity.count()
    count_expected = i*i*res*res*2 # factor of 2 for preconditioner
    if (count_encountered != count_expected):
        raise RuntimeError("Function call count ({}) not as expected ({}) for Stokes. Gauss swarm possibly not being used correctly.".format(count_encountered,count_expected))

for i in range(1,5):
    counted_diffusivity.reset()
    gaussSwarm = uw.swarm.GaussIntegrationSwarm(mesh,i)
    sys = uw.systems.SteadyStateHeat(t,counted_diffusivity, gauss_swarm=gaussSwarm)
    solver = uw.systems.Solver(sys).solve()
    count_encountered  = counted_diffusivity.count()
    count_expected = i*i*res*res
    if (count_encountered != count_expected):
        raise RuntimeError("Function call count ({}) not as expected ({}) for SteadyStateHeat. Gauss swarm possibly not being used correctly.".format(count_encountered,count_expected))


v.data[:] = (1.,1.)
for i in range(1,5):
    gaussSwarm = uw.swarm.GaussIntegrationSwarm(mesh,i)
    sys = uw.systems.AdvectionDiffusion(t,v,counted_diffusivity,phiDotField=t,gauss_swarm=gaussSwarm)
    dt = sys.get_max_dt()
    counted_diffusivity.reset()
    sys.integrate()
    count_encountered  = counted_diffusivity.count()
    count_expected = i*i*res*res*4
    if (count_encountered != count_expected):
        raise RuntimeError("Function call count ({}) not as expected ({}) for AdvectionDiffusion. Gauss swarm possibly not being used correctly.".format(count_encountered,count_expected))

v.data[:] = (1.,1.)
for i in range(1,5):
    gaussSwarm = uw.swarm.GaussIntegrationSwarm(mesh,i)
    sys = uw.systems.AdvectionDiffusion(t,v,counted_diffusivity,method="SLCN",gauss_swarm=gaussSwarm)
    dt = sys.get_max_dt()
    counted_diffusivity.reset()
    sys.integrate()
    count_encountered  = counted_diffusivity.count()
    count_expected = i*i*res*res*2
    if (count_encountered != count_expected):
        raise RuntimeError("Function call count ({}) not as expected ({}) for AdvectionDiffusion. Gauss swarm possibly not being used correctly.".format(count_encountered,count_expected))