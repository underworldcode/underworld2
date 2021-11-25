cmake_minimum_required(VERSION 3.16)

set(sources
    src/CellLayout.c
    src/ElementCellLayout.c
    src/Finalise.c
    src/GaussBorderParticleLayout.c
    src/GaussParticleLayout.c
    src/GlobalParticleLayout.c
    src/Init.c
    src/IntegrationPoint.c
    src/ParticleCommHandler.c
    src/ParticleLayout.c
    src/ParticleMovementHandler.c
    src/ParticleShadowSync.c
    src/PerCellParticleLayout.c
    src/Random.c
    src/RandomParticleLayout.c
    src/SingleCellLayout.c
    src/SpaceFillerParticleLayout.c
    src/StandardParticle.c
    src/SwarmClass.c
    src/Swarm_Register.c
    src/SwarmVariable.c
    src/SwarmVariable_Register.c
    src/TriGaussParticleLayout.c
    src/TriSingleCellLayout.c)

target_sources(StgDomain PRIVATE ${sources})