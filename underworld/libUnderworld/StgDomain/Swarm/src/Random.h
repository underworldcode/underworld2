/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Swarm_Random_h__
#define __StgDomain_Swarm_Random_h__
	
	/* Maximum random number */
	extern const long int Swarm_Random_RandMax;
	
	/* Seed */
	void Swarm_Random_Seed( long seed );
	
	/* Random */
	double Swarm_Random_Random();
	double Swarm_Random_Random_WithMinMax( double min, double max ) ;
	
#endif /* __StgDomain_Swarm_Random_h__ */
