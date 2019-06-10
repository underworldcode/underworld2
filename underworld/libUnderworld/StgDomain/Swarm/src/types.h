/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Swarm_types_h__
#define __StgDomain_Swarm_types_h__
	
	/* Types user needs to define */
	typedef struct _Particle			Particle;

	typedef struct ProcNbrInfo	ProcNbrInfo;
	typedef struct ShadowInfo	ShadowInfo;

	/** Types of coordinate system a particle/swarm is using */
	typedef enum {
		LocalCoordSystem = 0,
		GlobalCoordSystem
	} CoordSystem;

	/* Cell types ... for readability */
	typedef Index					Cell_Index;
	typedef Index					Cell_LocalIndex;
	typedef Index					Cell_DomainIndex;
	typedef Index					Cell_ShadowIndex;
	typedef Index					Cell_ShadowTransferIndex;
	typedef double*	    			Cell_Point;
	typedef Cell_Point*				Cell_Points;
	typedef Cell_Points*				Cell_PointsList;
	typedef Index					Cell_PointIndex;
	typedef Cell_PointIndex*			Cell_PointIndexList;
	
	/* Particle types ... for readability */
	typedef Index					Particle_Index;
	typedef Index					Particle_InCellIndex;
	typedef Particle_Index*				Cell_Particles;
	typedef Cell_Particles*				Cell_ParticlesList;
	typedef Particle_InCellIndex*			Cell_ParticlesIndexList;
	typedef char                                    Particle_Bool;

	typedef Particle*				Particle_List;

	/* Domain_Swarm types/classes */
	typedef struct CellLayout			CellLayout;
	typedef struct SingleCellLayout			SingleCellLayout;
	typedef struct TriSingleCellLayout		TriSingleCellLayout;
	typedef struct ElementCellLayout		ElementCellLayout;
	typedef struct ParticleLayout			ParticleLayout;
	typedef struct PerCellParticleLayout		PerCellParticleLayout;
	typedef struct RandomParticleLayout		RandomParticleLayout;
	typedef struct GaussParticleLayout		GaussParticleLayout;
	typedef struct TriGaussParticleLayout		TriGaussParticleLayout;
	typedef struct GaussBorderParticleLayout	GaussBorderParticleLayout;
	typedef struct GlobalParticleLayout		GlobalParticleLayout;
	typedef struct SpaceFillerParticleLayout        SpaceFillerParticleLayout;
	typedef struct Swarm				Swarm;
	typedef struct Swarm_Register			Swarm_Register;
	typedef struct ParticleCommHandler		ParticleCommHandler;
	typedef struct ParticleMovementHandler		ParticleMovementHandler;
	typedef struct ParticleShadowSync               ParticleShadowSync;
	typedef struct SwarmVariable_Register           SwarmVariable_Register;
	typedef struct SwarmVariable			SwarmVariable;

	typedef struct BaseParticle                     BaseParticle;
	typedef struct StandardParticle			StandardParticle;
	typedef struct LocalParticle			LocalParticle;
	typedef struct GlobalParticle			GlobalParticle;
	typedef struct IntegrationPoint			IntegrationPoint;
	
	/* output streams: initialised in Swarm_Init() */
	extern Stream* Swarm_VerboseConfig;
	extern Stream* Swarm_Debug;
	extern Stream* Swarm_Warning;
	extern Stream* Swarm_Error;
	
#endif /* __StgDomain_Swarm_types_h__ */
