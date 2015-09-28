/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include <PICellerator/PICellerator.h>
#include <Underworld/Underworld.h>

#include "types.h"
//#include "Rheology_Register.h"
#include "Underworld/Rheology/ConstitutiveMatrix.h"
#include "Matrix_NaiNbj.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>


/* Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */

const Type Matrix_NaiNbj_Type = "Matrix_NaiNbj";

Matrix_NaiNbj* Matrix_NaiNbj_New( 
		Name                                                name,
		StiffnessMatrix*                                    stiffnessMatrix,
		Swarm*                                              swarm,
		Dimension_Index                                     dim,
		FiniteElementContext*                               context,
		double                                              incompressibility_Penalty,
		Bool 											    viscosityWeighting )
{
	Matrix_NaiNbj* self = (Matrix_NaiNbj*) _Matrix_NaiNbj_DefaultNew( name );

   _StiffnessMatrixTerm_Init( self, context, stiffnessMatrix, swarm, NULL );
   _ConstitutiveMatrix_Init( (ConstitutiveMatrix*)self, dim, False, True );
   _Matrix_NaiNbj_Init( self, incompressibility_Penalty, viscosityWeighting );
   self->isConstructed = True;

	return self;
}

/* Private Constructor: This will accept all the virtual functions for this class as arguments. */

Matrix_NaiNbj* _Matrix_NaiNbj_New(  MATRIX_NAINBJ_DEFARGS  )
{
	Matrix_NaiNbj* self;
	
	/* Call private constructor of parent - this will set virtual functions of parent and continue up the hierarchy tree. At the beginning of the tree it will allocate memory of the size of object and initialise all the memory to zero. */
	assert( _sizeOfSelf >= sizeof(Matrix_NaiNbj) );
	self = (Matrix_NaiNbj*) _ConstitutiveMatrix_New(  CONSTITUTIVEMATRIX_PASSARGS  );
	
	/* Function pointers for this class that are not on the parent class should be set here */
	
	return self;
}

void _Matrix_NaiNbj_Init( 
		Matrix_NaiNbj*                   self,
		double                                         incompressibility_Penalty,
		Bool 	                                       viscosityWeighting )
{
	self->rowSize = self->columnSize = StGermain_nSymmetricTensorVectorComponents( self->dim );
	self->Dtilda_B = Memory_Alloc_2DArray( double, self->rowSize, self->dim, (Name)"D~ times B matrix" );

	if( self->dim == 2 ) {
		self->_setValue = _Matrix_NaiNbj2D_SetValueInAllEntries;
		self->_setSecondViscosity = _Matrix_NaiNbj2D_SetSecondViscosity;
		self->_getViscosity = _Matrix_NaiNbj2D_GetIsotropicViscosity;
		self->_isotropicCorrection = _Matrix_NaiNbj2D_IsotropicCorrection;
		self->_assemble_D_B = _Matrix_NaiNbj2D_Assemble_D_B;
		self->_calculateStress = _Matrix_NaiNbj2D_CalculateStress;
	} else {
		self->_setValue = _Matrix_NaiNbj3D_SetValueInAllEntries;
		self->_setSecondViscosity = _Matrix_NaiNbj3D_SetSecondViscosity;
		self->_getViscosity = _Matrix_NaiNbj3D_GetIsotropicViscosity;
		self->_isotropicCorrection = _Matrix_NaiNbj3D_IsotropicCorrection;
		self->_assemble_D_B = _Matrix_NaiNbj3D_Assemble_D_B;
		self->_calculateStress = _Matrix_NaiNbj3D_CalculateStress;
	}
	
	self->incompressibility_Penalty = incompressibility_Penalty;
	self->viscosityWeighting = viscosityWeighting;

  /* store each particle's constitutiveMatrix */
  if( self->storeConstitutiveMatrix )
        assert(0);
		//Matrix_NaiNbj_SetupParticleStorage( self );

}

void _Matrix_NaiNbj_Delete( void* constitutiveMatrix ) {
	Matrix_NaiNbj* self = (Matrix_NaiNbj*)constitutiveMatrix;

	_ConstitutiveMatrix_Delete( self  );
}

void _Matrix_NaiNbj_Print( void* constitutiveMatrix, Stream* stream ) {
	Matrix_NaiNbj* self = (Matrix_NaiNbj*)constitutiveMatrix;
	
	_ConstitutiveMatrix_Print( self, stream );

	/* General info */
	
	Journal_PrintValue( stream, self->incompressibility_Penalty );
	Journal_PrintValue( stream, self->viscosityWeighting );
	
}

void* _Matrix_NaiNbj_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                                  _sizeOfSelf = sizeof(Matrix_NaiNbj);
	Type                                                          type = Matrix_NaiNbj_Type;
	Stg_Class_DeleteFunction*                                  _delete = _Matrix_NaiNbj_Delete;
	Stg_Class_PrintFunction*                                    _print = _Matrix_NaiNbj_Print;
	Stg_Class_CopyFunction*                                      _copy = NULL;
	Stg_Component_DefaultConstructorFunction*      _defaultConstructor = _Matrix_NaiNbj_DefaultNew;
	Stg_Component_ConstructFunction*                        _construct = _Matrix_NaiNbj_AssembleFromXML;
	Stg_Component_BuildFunction*                                _build = _Matrix_NaiNbj_Build;
	Stg_Component_InitialiseFunction*                      _initialise = _Matrix_NaiNbj_Initialise;
	Stg_Component_ExecuteFunction*                            _execute = _Matrix_NaiNbj_Execute;
	Stg_Component_DestroyFunction*                            _destroy = _Matrix_NaiNbj_Destroy;
	StiffnessMatrixTerm_AssembleElementFunction*      _assembleElement = _Matrix_NaiNbj_AssembleElement;
	ConstitutiveMatrix_SetValueFunc*                         _setValue = NULL;
	ConstitutiveMatrix_GetValueFunc*                     _getViscosity = NULL;
	ConstitutiveMatrix_SetValueFunc*              _isotropicCorrection = NULL;
	ConstitutiveMatrix_SetSecondViscosityFunc*     _setSecondViscosity = NULL;
	ConstitutiveMatrix_Assemble_D_B_Func*                _assemble_D_B = NULL;
	ConstitutiveMatrix_CalculateStressFunc*           _calculateStress = NULL;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

	return (void*)_Matrix_NaiNbj_New(  MATRIX_NAINBJ_PASSARGS  );
}

void _Matrix_NaiNbj_AssembleFromXML( void* constitutiveMatrix, Stg_ComponentFactory* cf, void* data ) {
	Matrix_NaiNbj*            self = (Matrix_NaiNbj*)constitutiveMatrix;
	double                                         incompressibility_Penalty;
	Bool										   viscosityWeighting;
	

	/* Construct Parent */
	_ConstitutiveMatrix_AssignFromXML( self, cf, data );
	
	incompressibility_Penalty   = Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"incompressibility_Penalty", 0.0  );
	viscosityWeighting  = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"viscosity_weighting", True );

	_Matrix_NaiNbj_Init( self, incompressibility_Penalty, viscosityWeighting );
}

void _Matrix_NaiNbj_Build( void* constitutiveMatrix, void* data ) {
	Matrix_NaiNbj*             self             = (Matrix_NaiNbj*)constitutiveMatrix;

	_ConstitutiveMatrix_Build( self, data );
}

void _Matrix_NaiNbj_Initialise( void* constitutiveMatrix, void* data ) {
	Matrix_NaiNbj*             self             = (Matrix_NaiNbj*)constitutiveMatrix;

	_ConstitutiveMatrix_Initialise( self, data );
}

void _Matrix_NaiNbj_Execute( void* constitutiveMatrix, void* data ) {
	_ConstitutiveMatrix_Execute( constitutiveMatrix, data );
}

void _Matrix_NaiNbj_Destroy( void* constitutiveMatrix, void* data ) {
	Matrix_NaiNbj* self = (Matrix_NaiNbj*)constitutiveMatrix;

	_ConstitutiveMatrix_Destroy( constitutiveMatrix, data );

	Memory_Free( self->Dtilda_B );
	Memory_Free( self->Ni );
}

void _Matrix_NaiNbj_AssembleElement( 
		void*                                              constitutiveMatrix,
		StiffnessMatrix*                                   stiffnessMatrix, 
		Element_LocalIndex                                 lElement_I, 
		SystemLinearEquations*                             sle,
		FiniteElementContext*                              context,
		double**                                           elStiffMat ) 
{
      Matrix_NaiNbj*     self       = (Matrix_NaiNbj*) constitutiveMatrix;
      Swarm*                  swarm               = self->integrationSwarm;
      FeVariable*             variable1           = stiffnessMatrix->rowVariable;
      Dimension_Index         dim                 = stiffnessMatrix->dim;
      IntegrationPoint*       particle;
      Particle_InCellIndex	cParticle_I;
      Particle_InCellIndex	cellParticleCount;
      Element_NodeIndex       elementNodeCount;
      Node_ElementLocalIndex  rowNode_I;
      Node_ElementLocalIndex  colNode_I;
      double**                GNx;
      double                  detJac;
      Cell_Index              cell_I;
      ElementType*            elementType;
      double                  B0i_x; 
      double                  B0i_y;
      double                  B0i_z;
      double                  B0j_x; 
      double                  B0j_y;
      double                  B0j_z;
      Dof_Index               rowNodeDof_I;
      Dof_Index               colNodeDof_I;
      Dof_Index               nodeDofCount;
      double**                Dtilda_B;
      double                  *Ni;
      double                  eta, averaged_eta;
      double 	              total_weight;
      double 	              origin[3] = {0.0, 0.0, 0.0};
      double** 		      GN0x; 

      self->sle = sle;

      /* Set the element type */
      elementType       = FeMesh_GetElementType( variable1->feMesh, lElement_I );
      elementNodeCount  = elementType->nodeCount;
      nodeDofCount      = dim;

      /* allocate */
#ifndef PDE
      if( elementNodeCount > self->max_nElNodes ) {
	    self->max_nElNodes = elementNodeCount;
	    self->GNx = ReallocArray2D( self->GNx, double, dim, elementNodeCount );
	    self->Ni = ReallocArray( self->Ni, double, elementNodeCount );
      }
	
	
      GNx = self->GNx;
      Ni = self->Ni;
#endif
      Dtilda_B = self->Dtilda_B;

      /* Get number of particles per element */
      cell_I            = CellLayout_MapElementIdToCellId( swarm->cellLayout, lElement_I );
      cellParticleCount = swarm->cellParticleCountTbl[ cell_I ];

      /* Determine whether this is the first solve for not */
      Journal_Firewall( sle != NULL, Journal_Register( Error_Type, (Name)ConstitutiveMatrix_Type  ), 
			"In func %s: SLE is NULL.\n", __func__ );

      /* Note: we may have deliberately set the previousSolutionExists flag to true in the
	 parent ConstitutiveMatrix constructor if in restart mode, even if the SLE hasn't executed yet
	 in this run - so only update to the sle's value when SLE is confirming it has
	 executed */
		
      if ( True == sle->hasExecuted ) {
	    self->previousSolutionExists = sle->hasExecuted;
      }
	
      self->sleNonLinearIteration_I = sle->nonLinearIteration_I;
	
      /* Loop over points to build Stiffness Matrix */
	
      averaged_eta = 0.0;
      total_weight = 0.0;
	
      for ( cParticle_I = 0 ; cParticle_I < cellParticleCount ; cParticle_I++ ) {

	    particle = (void*) Swarm_ParticleInCellAt( swarm, cell_I, cParticle_I );

	    /* Assemble Constitutive Matrix */
	    // TODO : pass in the context here?
	    ConstitutiveMatrix_Assemble( constitutiveMatrix, lElement_I, swarm->cellParticleTbl[cell_I][cParticle_I], particle );

	    eta = self->matrixData[2][2];
	    averaged_eta += particle->weight * log(eta);
	    total_weight += particle->weight;
      }
	
	
      /* 	
	 Use element average quantities and add in the penalty term after
	 the rest of the stiffness matrix has been constructed.
	 This means we can use different methods for averaging viscosity ...
      */
	
      if(lElement_I == 1) {
	    PetscPrintf( PETSC_COMM_WORLD, "Incompressibility Penalty is %g\n",self->incompressibility_Penalty);
	    PetscPrintf( PETSC_COMM_WORLD, "Weighting by element viscosity is %s\n",self->viscosityWeighting ? "True" : "False");	
      }
    
      if (self->incompressibility_Penalty != 0.0) {
	
	    GN0x = Memory_Alloc_2DArray( double, dim, elementNodeCount, (Name)"GN0x"  );

	    /* Shape functions at the element centre for "underintegration" of the penalty term */	
	    ElementType_ShapeFunctionsGlobalDerivs( 
		  elementType,
		  variable1->feMesh, lElement_I,
		  origin, dim, &detJac, GN0x );
	
		
	    /* The average of the log viscosity ... now get the average viscosity */
	    if(self->viscosityWeighting)
		  averaged_eta = total_weight * exp(averaged_eta / total_weight);
	    else
		  averaged_eta = 1.0;
			
	    // fprintf(stderr,"Average viscosity in element %d is %g/%g, weight = %g\n",lElement_I,averaged_eta,eta,total_weight);

	
	    for( rowNode_I = 0 ; rowNode_I < elementNodeCount ; rowNode_I++ ) {
		  rowNodeDof_I = rowNode_I*nodeDofCount;
		  B0j_x = GN0x[ I_AXIS ][rowNode_I];
		  B0j_y = GN0x[ J_AXIS ][rowNode_I];
	
		  for( colNode_I = 0 ; colNode_I < elementNodeCount ; colNode_I++ ) {
			colNodeDof_I = colNode_I*nodeDofCount;
			B0i_x = GN0x[ I_AXIS ][colNode_I];
			B0i_y = GN0x[ J_AXIS ][colNode_I];
	
			if ( 2 == dim ) {
			      elStiffMat[ colNodeDof_I     ][ rowNodeDof_I     ] += self->incompressibility_Penalty * detJac * averaged_eta * B0i_x * B0j_x ;
			      elStiffMat[ colNodeDof_I     ][ rowNodeDof_I + 1 ] += self->incompressibility_Penalty * detJac * averaged_eta * B0i_x * B0j_y ;
			      elStiffMat[ colNodeDof_I + 1 ][ rowNodeDof_I     ] += self->incompressibility_Penalty * detJac * averaged_eta * B0i_y * B0j_x ;
			      elStiffMat[ colNodeDof_I + 1 ][ rowNodeDof_I + 1 ] += self->incompressibility_Penalty * detJac * averaged_eta * B0i_y * B0j_y ;
		        }                                                                                          
			else {
				   
			      B0i_z = GN0x[ K_AXIS ][colNode_I];
			      B0j_z = GN0x[ K_AXIS ][rowNode_I];
									                                                                                     
			      elStiffMat[ colNodeDof_I     ][ rowNodeDof_I     ] += self->incompressibility_Penalty * detJac * averaged_eta * B0j_x * B0i_x ;
			      elStiffMat[ colNodeDof_I     ][ rowNodeDof_I + 1 ] += self->incompressibility_Penalty * detJac * averaged_eta * B0j_y * B0i_x ;
			      elStiffMat[ colNodeDof_I     ][ rowNodeDof_I + 2 ] += self->incompressibility_Penalty * detJac * averaged_eta * B0j_z * B0i_x ;
			      elStiffMat[ colNodeDof_I + 1 ][ rowNodeDof_I     ] += self->incompressibility_Penalty * detJac * averaged_eta * B0j_x * B0i_y ;
			      elStiffMat[ colNodeDof_I + 1 ][ rowNodeDof_I + 1 ] += self->incompressibility_Penalty * detJac * averaged_eta * B0j_y * B0i_y ;
			      elStiffMat[ colNodeDof_I + 1 ][ rowNodeDof_I + 2 ] += self->incompressibility_Penalty * detJac * averaged_eta * B0j_z * B0i_y ;
			      elStiffMat[ colNodeDof_I + 2 ][ rowNodeDof_I     ] += self->incompressibility_Penalty * detJac * averaged_eta * B0j_x * B0i_z ;
			      elStiffMat[ colNodeDof_I + 2 ][ rowNodeDof_I + 1 ] += self->incompressibility_Penalty * detJac * averaged_eta * B0j_y * B0i_z ;
			      elStiffMat[ colNodeDof_I + 2 ][ rowNodeDof_I + 2 ] += self->incompressibility_Penalty * detJac * averaged_eta * B0j_z * B0i_z ;
			}
		  }
	    }	
			
	
	    Memory_Free( GN0x );
      }	
	
}

void _Matrix_NaiNbj2D_SetValueInAllEntries( void* constitutiveMatrix, double value ) {
	ConstitutiveMatrix* self   = (ConstitutiveMatrix*) constitutiveMatrix;

	if ( fabs( value ) < 1.0e-20 ) 
		ConstitutiveMatrix_ZeroMatrix( self );
	else {
		double**            D      = self->matrixData;

		D[0][0] = D[0][1] = D[0][2] = value;
		D[1][0] = D[1][1] = D[1][2] = value;
		D[2][0] = D[2][1] = D[2][2] = value;
	
		self->isDiagonal = False;
	}
}

void _Matrix_NaiNbj3D_SetValueInAllEntries( void* _constitutiveMatrix, double value ) {
	ConstitutiveMatrix* self   = (ConstitutiveMatrix*)_constitutiveMatrix;

	if ( fabs( value ) < 1.0e-20 ) 
		ConstitutiveMatrix_ZeroMatrix( self );
	else {
		double**            D      = self->matrixData;

		D[0][0] = D[0][1] = D[0][2] = D[0][3] = D[0][4] = D[0][5] = value;
		D[1][0] = D[1][1] = D[1][2] = D[1][3] = D[1][4] = D[1][5] = value;
		D[2][0] = D[2][1] = D[2][2] = D[2][3] = D[2][4] = D[2][5] = value;
		D[3][0] = D[3][1] = D[3][2] = D[3][3] = D[3][4] = D[3][5] = value;
		D[4][0] = D[4][1] = D[4][2] = D[4][3] = D[4][4] = D[4][5] = value;
		D[5][0] = D[5][1] = D[5][2] = D[5][3] = D[5][4] = D[5][5] = value;
	
		self->isDiagonal = False;
	}
}

double _Matrix_NaiNbj2D_GetIsotropicViscosity( void* constitutiveMatrix ) {
	ConstitutiveMatrix* self = (ConstitutiveMatrix*) constitutiveMatrix;

	return self->matrixData[2][2];
}

double _Matrix_NaiNbj3D_GetIsotropicViscosity( void* constitutiveMatrix ) {
	ConstitutiveMatrix* self = (ConstitutiveMatrix*) constitutiveMatrix;

	return self->matrixData[3][3];
}

void _Matrix_NaiNbj2D_IsotropicCorrection( void* constitutiveMatrix, double isotropicCorrection ) {
	ConstitutiveMatrix* self   = (ConstitutiveMatrix*) constitutiveMatrix;
	double**            D      = self->matrixData;
		
	D[0][0] += 2.0 * isotropicCorrection;
	D[1][1] += 2.0 * isotropicCorrection;
	D[2][2] += isotropicCorrection;
}

void _Matrix_NaiNbj3D_IsotropicCorrection( void* constitutiveMatrix, double isotropicCorrection ) {
	ConstitutiveMatrix* self   = (ConstitutiveMatrix*) constitutiveMatrix;
	double**            D      = self->matrixData;

	D[0][0] += 2.0 * isotropicCorrection;
	D[1][1] += 2.0 * isotropicCorrection;
	D[2][2] += 2.0 * isotropicCorrection;
	
	D[3][3] += isotropicCorrection;
	D[4][4] += isotropicCorrection;
	D[5][5] += isotropicCorrection;
}

void _Matrix_NaiNbj2D_SetSecondViscosity( void* constitutiveMatrix, double deltaViscosity, XYZ director ) {
	ConstitutiveMatrix* self      = (ConstitutiveMatrix*) constitutiveMatrix;
	double**            D         = self->matrixData;
	double              n1        = director[ I_AXIS ];
	double              n2        = director[ J_AXIS ];
	double              a0;
	double              a1;

	a0 = 4.0 * deltaViscosity * n1 * n1 * n2 * n2;
	a1 = 2.0 * deltaViscosity * n1 * n2 * (n2*n2 - n1*n1);

	D[0][0] += -a0 ;	D[0][1] +=  a0 ;	D[0][2] += -a1 ;
	D[1][0] +=  a0 ;	D[1][1] += -a0 ;	D[1][2] +=  a1 ;
	D[2][0] += -a1 ;	D[2][1] +=  a1 ;	D[2][2] +=  a0 - deltaViscosity ;

	self->isDiagonal = False;
}

void _Matrix_NaiNbj3D_SetSecondViscosity( void* constitutiveMatrix, double deltaViscosity, XYZ director ) {
	ConstitutiveMatrix* self      = (ConstitutiveMatrix*) constitutiveMatrix;
	double**            D         = self->matrixData;
	double              n1        = director[ I_AXIS ];
	double              n2        = director[ J_AXIS ];
	double              n3        = director[ K_AXIS ];
	double              a00,a01,a02,a03,a04,a05;
	double                  a11,a12,a13,a14,a15;
	double                      a22,a23,a24,a25;
	double                          a33,a34,a35;
	double                              a44,a45;
	double                                  a55;	

	a00 = -4 * n1*n1 * ( 1 - n1*n1 ) * deltaViscosity; 
	a01 =  4 * n1*n1 * n2*n2 * deltaViscosity; 
	a02 =  4 * n1*n1 * n3*n3 * deltaViscosity; 
	a03 =  2 * n1*n2 * (2*n1*n1-1) * deltaViscosity;
	a04 =  2 * n1*n3 * (2*n1*n1-1) * deltaViscosity; 
	a05 =  4 * n1*n1 * n2*n3 * deltaViscosity;
		 
	a11= 4 * n2*n2 * (n2*n2-1) * deltaViscosity; 
	a12= 4 * n2*n2 * n3*n3 * deltaViscosity; 
	a13= 2 * n1*n2 * (2*n2*n2-1) * deltaViscosity; 
	a14= 4 * n1*n2 * n2*n3 * deltaViscosity;  
	a15= 2 * n2*n3 * (2*n2*n2-1) * deltaViscosity;
		
	a22 = 4 * n3*n3 * (n3*n3-1) * deltaViscosity; 
	a23 = 4 * n1*n2 * n3*n3 * deltaViscosity; 
	a24 = 2 * n1*n3 * (2*n3*n3-1) * deltaViscosity;
	a25 = 2 * n2*n3 * (2*n3*n3-1) * deltaViscosity;
	 
	a33 = (4 * n1*n1 * n2*n2 - n1*n1 - n2*n2) * deltaViscosity; 
	a34 = (4 * n1*n1 * n2*n3 - n2*n3) * deltaViscosity; 
	a35 = (4 * n1*n2 * n2*n3 - n1*n3) * deltaViscosity;
	
	a44 = (4 * n1*n1 * n3*n3 - n1*n1 -n3*n3) * deltaViscosity; 
	a45 = (4 * n1*n2 * n3*n3 - n1*n2) * deltaViscosity;

	a55 = (4 * n3*n3 * n2*n2 - n3*n3 - n2*n2) * deltaViscosity;
		
	/* D_{anisotropic} to D */
	D[0][0] += a00 ; D[0][1] += a01 ; D[0][2] += a02 ; D[0][3] += a03 ; D[0][4] += a04 ; D[0][5] += a05 ;
	D[1][0] += a01 ; D[1][1] += a11 ; D[1][2] += a12 ; D[1][3] += a13 ; D[1][4] += a14 ; D[1][5] += a15 ;
	D[2][0] += a02 ; D[2][1] += a12 ; D[2][2] += a22 ; D[2][3] += a23 ; D[2][4] += a24 ; D[2][5] += a25 ;
	D[3][0] += a03 ; D[3][1] += a13 ; D[3][2] += a23 ; D[3][3] += a33 ; D[3][4] += a34 ; D[3][5] += a35 ;
	D[4][0] += a04 ; D[4][1] += a14 ; D[4][2] += a24 ; D[4][3] += a34 ; D[4][4] += a44 ; D[4][5] += a45 ;
	D[5][0] += a05 ; D[5][1] += a15 ; D[5][2] += a25 ; D[5][3] += a35 ; D[5][4] += a45 ; D[5][5] += a55 ;

	self->isDiagonal = False;
}

/*
[B] = [ d/dx,     0  ]
      [    0,  d/dy  ]
      [ d/dy,  d/dx  ]  */
void _Matrix_NaiNbj2D_Assemble_D_B( void* constitutiveMatrix, double** GNx, Node_Index node_I, double** D_B ){
	ConstitutiveMatrix* self = (ConstitutiveMatrix*) constitutiveMatrix;
	double**            D    = self->matrixData;
	double              d_dx = GNx[ I_AXIS ][ node_I ];
	double              d_dy = GNx[ J_AXIS ][ node_I ];

	if (self->isDiagonal) {
		D_B[0][0] = D[0][0] * d_dx;
		D_B[0][1] = 0.0;
				
		D_B[1][0] = 0.0;
		D_B[1][1] = D[1][1] * d_dy;
				
		D_B[2][0] = D[2][2] * d_dy;
		D_B[2][1] = D[2][2] * d_dx;		
	}
	else {
		D_B[0][0] = D[0][0] * d_dx + D[0][2] * d_dy;
		D_B[0][1] = D[0][1] * d_dy + D[0][2] * d_dx;
				
		D_B[1][0] = D[1][0] * d_dx + D[1][2] * d_dy;
		D_B[1][1] = D[1][1] * d_dy + D[1][2] * d_dx;
				
		D_B[2][0] = D[2][0] * d_dx + D[2][2] * d_dy;
		D_B[2][1] = D[2][1] * d_dy + D[2][2] * d_dx;
	}
}


/*
[B] = [ d/dx,     0,      0  ]
      [    0,  d/dy,      0  ]
      [    0,     0,   d/dx  ]
      [ d/dy,  d/dx,      0  ] 
      [ d/dz,     0,   d/dx  ]
      [    0,  d/dz,   d/dy  ] */

void _Matrix_NaiNbj3D_Assemble_D_B( void* constitutiveMatrix, double** GNx, Node_Index node_I, double** D_B ){
	ConstitutiveMatrix* self = (ConstitutiveMatrix*) constitutiveMatrix;
	double**            D    = self->matrixData;
	double              d_dx = GNx[ I_AXIS ][ node_I ];
	double              d_dy = GNx[ J_AXIS ][ node_I ];
	double              d_dz = GNx[ K_AXIS ][ node_I ];

	if (self->isDiagonal) {
		D_B[0][0] = D[0][0] * d_dx;
		D_B[0][1] = 0.0;
		D_B[0][2] = 0.0;
		
		D_B[1][0] = 0.0;
		D_B[1][1] = D[1][1] * d_dy;
		D_B[1][2] = 0.0;

		D_B[2][0] = 0.0;
		D_B[2][1] = 0.0;
		D_B[2][2] = D[2][2] * d_dz;

		D_B[3][0] = D[3][3] * d_dy;
		D_B[3][1] = D[3][3] * d_dx;
		D_B[3][2] = 0.0;

		D_B[4][0] = D[4][4] * d_dz;
		D_B[4][1] = 0.0;
		D_B[4][2] = D[4][4] * d_dx;
			
		D_B[5][0] = 0.0;
		D_B[5][1] = D[5][5] * d_dz;
		D_B[5][2] = D[5][5] * d_dy;
	}
	else {
		D_B[0][0] = D[0][0] * d_dx + D[0][3] * d_dy + D[0][4] * d_dz;
		D_B[0][1] = D[0][1] * d_dy + D[0][3] * d_dx + D[0][5] * d_dz;
		D_B[0][2] = D[0][2] * d_dz + D[0][4] * d_dx + D[0][5] * d_dy;
		
		D_B[1][0] = D[1][0] * d_dx + D[1][3] * d_dy + D[1][4] * d_dz;
		D_B[1][1] = D[1][1] * d_dy + D[1][3] * d_dx + D[1][5] * d_dz;
		D_B[1][2] = D[1][2] * d_dz + D[1][4] * d_dx + D[1][5] * d_dy;

		D_B[2][0] = D[2][0] * d_dx + D[2][3] * d_dy + D[2][4] * d_dz;
		D_B[2][1] = D[2][1] * d_dy + D[2][3] * d_dx + D[2][5] * d_dz;
		D_B[2][2] = D[2][2] * d_dz + D[2][4] * d_dx + D[2][5] * d_dy;

		D_B[3][0] = D[3][0] * d_dx + D[3][3] * d_dy + D[3][4] * d_dz;
		D_B[3][1] = D[3][1] * d_dy + D[3][3] * d_dx + D[3][5] * d_dz;
		D_B[3][2] = D[3][2] * d_dz + D[3][4] * d_dx + D[3][5] * d_dy;

		D_B[4][0] = D[4][0] * d_dx + D[4][3] * d_dy + D[4][4] * d_dz;
		D_B[4][1] = D[4][1] * d_dy + D[4][3] * d_dx + D[4][5] * d_dz;
		D_B[4][2] = D[4][2] * d_dz + D[4][4] * d_dx + D[4][5] * d_dy;
			
		D_B[5][0] = D[5][0] * d_dx + D[5][3] * d_dy + D[5][4] * d_dz;
		D_B[5][1] = D[5][1] * d_dy + D[5][3] * d_dx + D[5][5] * d_dz;
		D_B[5][2] = D[5][2] * d_dz + D[5][4] * d_dx + D[5][5] * d_dy;
	}
}

void _Matrix_NaiNbj2D_CalculateStress( void* constitutiveMatrix, SymmetricTensor strainRate, SymmetricTensor stress ) {
	ConstitutiveMatrix* self = (ConstitutiveMatrix*) constitutiveMatrix;
	double**            D    = self->matrixData;

	if (self->isDiagonal) {
		stress[0] = D[0][0] * strainRate[0];
		stress[1] = D[1][1] * strainRate[1];
		stress[2] = D[2][2] * 2.0 * strainRate[2];
	}
	else {
		stress[0] = D[0][0] * strainRate[0] + D[0][1] * strainRate[1] + D[0][2] * 2.0 * strainRate[2];
		stress[1] = D[1][0] * strainRate[0] + D[1][1] * strainRate[1] + D[1][2] * 2.0 * strainRate[2];
		stress[2] = D[2][0] * strainRate[0] + D[2][1] * strainRate[1] + D[2][2] * 2.0 * strainRate[2];
	}
}



void _Matrix_NaiNbj3D_CalculateStress( void* constitutiveMatrix, SymmetricTensor strainRate, SymmetricTensor stress ) {
	ConstitutiveMatrix* self = (ConstitutiveMatrix*) constitutiveMatrix;
	double**            D    = self->matrixData;
	          	
	if (self->isDiagonal) {
		stress[0] = D[0][0] * strainRate[0];
		stress[1] = D[1][1] * strainRate[1];
		stress[2] = D[2][2] * strainRate[2];
		stress[3] = D[3][3] * 2.0 * strainRate[3];
		stress[4] = D[4][4] * 2.0 * strainRate[4];
		stress[5] = D[5][5] * 2.0 * strainRate[5];
	}
	else {
		stress[0] = D[0][0] * strainRate[0] + D[0][1] * strainRate[1] + D[0][2] * strainRate[2] 
			+ 2.0 * (D[0][3] * strainRate[3] + D[0][4] * strainRate[4] + D[0][5] * strainRate[5]);

		stress[1] = D[1][0] * strainRate[0] + D[1][1] * strainRate[1] + D[1][2] * strainRate[2] 
			+ 2.0 * (D[1][3] * strainRate[3] + D[1][4] * strainRate[4] + D[1][5] * strainRate[5]);

		stress[2] = D[2][0] * strainRate[0] + D[2][1] * strainRate[1] + D[2][2] * strainRate[2] 
			+ 2.0 * (D[2][3] * strainRate[3] + D[2][4] * strainRate[4] + D[2][5] * strainRate[5]);

		stress[3] = D[3][0] * strainRate[0] + D[3][1] * strainRate[1] + D[3][2] * strainRate[2] 
			+ 2.0 * (D[3][3] * strainRate[3] + D[3][4] * strainRate[4] + D[3][5] * strainRate[5]);

		stress[4] = D[4][0] * strainRate[0] + D[4][1] * strainRate[1] + D[4][2] * strainRate[2] 
			+ 2.0 * (D[4][3] * strainRate[3] + D[4][4] * strainRate[4] + D[4][5] * strainRate[5]);

		stress[5] = D[5][0] * strainRate[0] + D[5][1] * strainRate[1] + D[5][2] * strainRate[2] 
			+ 2.0 * (D[5][3] * strainRate[3] + D[5][4] * strainRate[4] + D[5][5] * strainRate[5]);
	}
}

//void Matrix_NaiNbj_SetupParticleStorage( Matrix_NaiNbj* self ) {
//	/* a function which defines the storage of each particle's constitutive information on the particle, 
//	 * should be called before the "Build" phase */
//
//	static int beenHere = 0; /* don't want to do this routine twice */
//
//	IntegrationPointsSwarm* swarm = (IntegrationPointsSwarm*)self->integrationSwarm;
//	MaterialPointsSwarm **materialSwarms, *materialSwarm;
//	MaterialPoint particle;
//	Index materialSwarmCount;
//	double *cMatrix = NULL;
//
//	if( beenHere ) return; 
//
//	beenHere = 1;
//
//	/* get material swram mapped to the integration points,
//	*      * currently only one material point is mapped 26 FEB 09 */
//	materialSwarms = IntegrationPointMapper_GetMaterialPointsSwarms( swarm->mapper, &materialSwarmCount );
//	assert( materialSwarmCount < 2 );
//	materialSwarm = materialSwarms[0];
//
//	/* add extension to material swarm */
//	self->storedConstHandle = ExtensionManager_Add( materialSwarm->particleExtensionMgr, (Name)self->type, self->rowSize * self->columnSize * sizeof(double)  );
//
//	cMatrix = ExtensionManager_Get( materialSwarm->particleExtensionMgr, &particle, self->storedConstHandle );
//
//	if( self->dim == 2 ) {
//		/* TODO: clean up this vector logic. The only reson there's an if is because
//		*        * of the list of names the must be given as the final arguments to this function.  */ 
//		self->storedConstSwarmVar = Swarm_NewVectorVariable( materialSwarm, (Name)"ConstitutiveMatrix", (ArithPointer)cMatrix - (ArithPointer)&particle,
//		Variable_DataType_Double, self->rowSize * self->columnSize,
//		"c00", "c01", "c02", "c10", "c11", "c12", "c20", "c21", "c22" );
//	} else {
//		self->storedConstSwarmVar = Swarm_NewVectorVariable( materialSwarm, (Name)"ConstitutiveMatrix", (ArithPointer)cMatrix - (ArithPointer)&particle,
//		Variable_DataType_Double, self->rowSize * self->columnSize,
//		"c00", "c01", "c02", "c03", "c04", "c05",
//		"c10", "c11", "c12", "c13", "c14", "c15",
//		"c20", "c21", "c22", "c23", "c24", "c25",
//		"c30", "c31", "c32", "c33", "c34", "c35",
//		"c40", "c41", "c42", "c43", "c44", "c45",
//		"c50", "c51", "c52", "c53", "c54", "c55" );
//	}
//
//	/* set the storedConstitutive matrix NOT to be checkpointed */
//	self->storedConstSwarmVar->isCheckpointedAndReloaded = False;
//}
//
//
