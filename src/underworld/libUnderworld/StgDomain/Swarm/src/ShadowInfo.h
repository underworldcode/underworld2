/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Swarm_ShadowInfo_h__
#define __StgDomain_Swarm_ShadowInfo_h__

struct ProcNbrInfo {
	/** Count of neighbouring processors */
	PartitionIndex	procNbrCnt;
	/** The rank of each neighbour processor */
	PartitionIndex*	procNbrTbl;
};

/** Stores the information necessary to transfer shadowed values between
processors. */
struct ShadowInfo {
	/** Info on neigbouring processor indices */
	ProcNbrInfo*	procNbrInfo;
	/** For each nbr processor, count of how many of its shadow nodes
	live on the current processor */
	Index*		procShadowedCnt;
	/** For each nbr processor, for each of its shadow nodes
	that live on the current processor, gives the local index of that
	node */
	LocalIndex**	procShadowedTbl;
	/** For each nbr processor, count of how many of my shadow nodes
	live on that processor */
	Index*		procShadowCnt;
	/** For each nbr processor, for each of my shadow nodes that
	live on that processor, gives its domain index on my proc. */
	DomainIndex**	procShadowTbl;
};

#endif /* __StgDomain_Swarm_ShadowInfo_h__ */
