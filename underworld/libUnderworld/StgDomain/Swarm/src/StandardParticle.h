/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Swarm_StandardParticle_h__
#define __StgDomain_Swarm_StandardParticle_h__

	extern const Type StandardParticle_Type;
	extern const Type LocalParticle_Type;
	extern const Type GlobalParticle_Type;

	/** Standard base class for particle */
	#define __BaseParticle \
		Cell_DomainIndex	owningCell;	/**< The cell in a mesh which this particle currently lives in */

	struct BaseParticle { __BaseParticle };

	/* For backward compatibility */
	#if 0
	#define __StandardParticle \
		__BaseParticle \
		Coord                   coord;
		
	struct StandardParticle { __StandardParticle };
	#endif

	struct StandardParticle { __BaseParticle };

	/** A particle with a local coordinate system */
	#define __LocalParticle \
		__BaseParticle \
		Coord                   xi;             /**< Local coordinates within element/cell */

	struct LocalParticle { __LocalParticle };

	/** A particle with a global coordinate system */
	#define __GlobalParticle \
		__BaseParticle \
		Coord                   coord;          /**< The global coordinates of this particle */ 

	struct GlobalParticle { __GlobalParticle };

#endif
