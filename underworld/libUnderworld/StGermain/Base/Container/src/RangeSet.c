/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <mpi.h>

#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"

#include "types.h"
#include "BTreeNode.h"
#include "BTree.h"
#include "RangeSet.h"


/* Textual name of this class */
const Type RangeSet_Type = "RangeSet";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

RangeSet* RangeSet_New() {
	/* Variables set in this function */
	SizeT                      _sizeOfSelf = sizeof(RangeSet);
	Type                              type = RangeSet_Type;
	Stg_Class_DeleteFunction*      _delete = _RangeSet_Delete;
	Stg_Class_PrintFunction*        _print = _RangeSet_Print;
	Stg_Class_CopyFunction*          _copy = _RangeSet_Copy;

	return _RangeSet_New(  RANGESET_PASSARGS  );
}

RangeSet* _RangeSet_New(  RANGESET_DEFARGS  ) {
	RangeSet* self;

	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(RangeSet) );
	self = (RangeSet*)_Stg_Class_New(  STG_CLASS_PASSARGS  );

	/* Virtual info */

	/* RangeSet info */
	_RangeSet_Init( self );

	return self;
}

void _RangeSet_Init( RangeSet* self ) {
	assert( self );

	self->nInds = 0;
	self->btree = BTree_New( RangeSet_DataCompare, RangeSet_DataCopy, RangeSet_DataDelete, NULL, 
				 BTREE_NO_DUPLICATES );
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _RangeSet_Delete( void* rangeSet ) {
	RangeSet*	self = (RangeSet*)rangeSet;

	assert( self );

	RangeSet_Destruct( self );
	FreeObject( self->btree );

	/* Delete the parent. */
	_Stg_Class_Delete( self );
}

void _RangeSet_Print( void* rangeSet, Stream* stream ) {
	RangeSet*	self = (RangeSet*)rangeSet;
	
	/* Set the Journal for printing informations */
	Stream* rangeSetStream;
	rangeSetStream = Journal_Register( InfoStream_Type, "RangeSetStream" );

	/* Print parent */
	Journal_Printf( stream, "RangeSet (ptr): (%p)\n", self );
	_Stg_Class_Print( self, stream );
}

void* _RangeSet_Copy( void* rangeSet, void* destProc_I, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	RangeSet*	self = (RangeSet*)rangeSet;
	RangeSet*	newRangeSet;
	unsigned	nInds, *inds;

	inds = NULL;
	RangeSet_GetIndices( self, &nInds, &inds );
	newRangeSet = RangeSet_New();
	RangeSet_SetIndices( newRangeSet, nInds, inds );
	FreeArray( inds );

	return (void*)newRangeSet;
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void RangeSet_SetIndices( void* rangeSet, unsigned nInds, unsigned* inds ) {
	RangeSet*	self = (RangeSet*)rangeSet;
	unsigned*	tmpInds;
	unsigned	curInd = 0;

	assert( self );
	assert( !nInds || inds );

	RangeSet_Destruct( self );
	if( !nInds ) return;

	self->nInds = nInds;
	tmpInds = Memory_Alloc_Array_Unnamed( unsigned, nInds );
	memcpy( tmpInds, inds, nInds * sizeof(unsigned) );
	qsort( tmpInds, nInds, sizeof(unsigned), RangeSet_SortCmp );

	while( curInd < nInds ) {
		RangeSet_Range	rng;

		rng.begin = tmpInds[curInd++];
		while( curInd < nInds && tmpInds[curInd] == tmpInds[curInd - 1] ) {
			curInd++;
			self->nInds--;
		}
		if( curInd == nInds ) {
			rng.end = rng.begin + 1;
			rng.step = 1;
			BTree_InsertNode( self->btree, &rng, sizeof(RangeSet_Range) );
			break;
		}

		rng.end = tmpInds[curInd++];
		while( curInd < nInds && tmpInds[curInd] == rng.end ) {
			curInd++;
			self->nInds--;
		}
		rng.step = rng.end - rng.begin;

		while( curInd < nInds && tmpInds[curInd] - rng.end == rng.step ) {
			rng.end = tmpInds[curInd++];
			while( curInd < nInds && tmpInds[curInd] == rng.end ) {
				curInd++;
				self->nInds--;
			}
		}
		rng.end++;

		BTree_InsertNode( self->btree, &rng, sizeof(RangeSet_Range) );
	}

	FreeArray( tmpInds );
}

void RangeSet_AddIndices( void* rangeSet, unsigned nInds, unsigned* inds ) {
	RangeSet*	self = (RangeSet*)rangeSet;
	RangeSet*	tmpSet;

	assert( self );

	tmpSet = RangeSet_New();
	RangeSet_SetIndices( tmpSet, nInds, inds );
	RangeSet_Union( self, tmpSet );
}

void RangeSet_SetRange( void* rangeSet, unsigned begin, unsigned end, unsigned step ) {
	RangeSet*	self = (RangeSet*)rangeSet;
	RangeSet_Range	rng;

	assert( self );

	RangeSet_Destruct( self );
	self->nInds = (end - begin) / step;
	rng.begin = begin;
	rng.end = end;
	rng.step = step;
	BTree_InsertNode( self->btree, &rng, sizeof(RangeSet_Range) );
}

void RangeSet_Clear( void* rangeSet ) {
	RangeSet*	self = (RangeSet*)rangeSet;

	assert( self );

	RangeSet_Destruct( self );
}

void RangeSet_GetIndices( void* rangeSet, unsigned* nInds, unsigned** inds ) {
	RangeSet*		self = (RangeSet*)rangeSet;
	RangeSet_ParseStruct	parse;

	assert( self );

	parse.nInds = 0;
	parse.inds = (*inds) ? *inds : AllocArray( unsigned, self->nInds );
	BTree_ParseTree( self->btree, RangeSet_GetIndicesParse, &parse );

	*nInds = parse.nInds;
	*inds = parse.inds;

	/* Sanity check. */
	assert( *nInds == self->nInds );
}

unsigned RangeSet_GetSize( void* rangeSet ) {
	RangeSet*	self = (RangeSet*)rangeSet;

	assert( self );

	return self->nInds;
}

unsigned RangeSet_GetNumRanges( void* rangeSet ) {
	RangeSet*	self = (RangeSet*)rangeSet;

	assert( self );
	assert( self->btree );

	return self->btree->nodeCount;
}

RangeSet_Range* RangeSet_GetRange( void* rangeSet, unsigned index ) {
	RangeSet*		self = (RangeSet*)rangeSet;
	RangeSet_ParseStruct	parse;

	assert( self );
	assert( self->btree );
	assert( index < self->btree->nodeCount );

	parse.nInds = index;
	parse.curInd = 0;
	BTree_ParseTree( self->btree, RangeSet_GetRangeParse, &parse );

	return parse.range;
}

Bool RangeSet_HasIndex( void* rangeSet, unsigned index ) {
	RangeSet*	self = (RangeSet*)rangeSet;
	RangeSet_Range	rng;
	BTreeNode*	node;

	assert( self );

	rng.begin = index;
	rng.end = index + 1;
	rng.step = 1;
	node = BTree_FindNode( self->btree, &rng );

	if( node )
		return RangeSet_Range_HasIndex( (RangeSet_Range*)node->data, index );
	else
		return False;
}

void RangeSet_Union( void* rangeSet, RangeSet* rSet ) {
	RangeSet*	self = (RangeSet*)rangeSet;
	unsigned	nInds, *inds, *tmpInds;

	assert( self );
	assert( rSet );

	inds = AllocArray( unsigned, rSet->nInds + self->nInds );
	RangeSet_GetIndices( self, &nInds, &inds );
	assert( nInds == self->nInds );
	tmpInds = inds + self->nInds;
	RangeSet_GetIndices( rSet, &nInds, &tmpInds );

	RangeSet_SetIndices( self, rSet->nInds + self->nInds, inds );
	FreeArray( inds );
}

void RangeSet_Intersection( void* rangeSet, RangeSet* rSet ) {
	RangeSet*		self = (RangeSet*)rangeSet;
	RangeSet_ParseStruct	parse;

	assert( self );
	assert( rSet );

	parse.operand = rSet;
	parse.nInds = 0;
	parse.inds = AllocArray( unsigned, (self->nInds > rSet->nInds) ? self->nInds : rSet->nInds );
	BTree_ParseTree( self->btree, RangeSet_IntersectionParse, &parse );

	RangeSet_SetIndices( self, parse.nInds, parse.inds );
	FreeArray( parse.inds );

#if 0
	RangeSet*		self = (RangeSet*)rangeSet;
	RangeSet_ParseStruct	parse;

	assert( self );
	assert( rSet );

	parse.self = self;
	parse.operand = rSet;
	parse.range = NULL;
	parse.newTree = BTree_New( RangeSet_DataCompare, RangeSet_DataCopy, RangeSet_DataDelete, NULL, 
					 BTREE_NO_DUPLICATES );
	parse.nInds = 0;
	BTree_ParseTree( self->btree, RangeSet_IntersectionParse, &parse );

	FreeObject( self->btree );
	self->btree = parse.newTree;
	self->nInds = parse.nInds;
#endif
}

void RangeSet_Subtraction( void* rangeSet, RangeSet* rSet ) {
	RangeSet*		self = (RangeSet*)rangeSet;
	RangeSet_ParseStruct	parse;

	assert( self );
	assert( rSet );

	parse.operand = rSet;
	parse.nInds = 0;
	parse.inds = AllocArray( unsigned, self->nInds );
	BTree_ParseTree( self->btree, RangeSet_SubtractionParse, &parse );

	RangeSet_SetIndices( self, parse.nInds, parse.inds );
	FreeArray( parse.inds );
}

void RangeSet_Pickle( void* rangeSet, unsigned* nBytes, Stg_Byte** bytes ) {
	RangeSet*	self = (RangeSet*)rangeSet;

	assert( self );
	assert( nBytes );
	assert( bytes );

	if( self->nInds ) {
		RangeSet_ParseStruct	parse;

		*nBytes = sizeof(unsigned) + self->btree->nodeCount * sizeof(RangeSet_Range);
		*bytes = AllocArray( Stg_Byte, *nBytes );
		((unsigned*)*bytes)[0] = self->nInds;
		parse.bytes = *bytes;
		parse.curInd = 0;
		BTree_ParseTree( self->btree, RangeSet_PickleParse, &parse );
	}
	else {
		*nBytes = 0;
		*bytes = NULL;
	}
}

void RangeSet_Unpickle( void* rangeSet, unsigned nBytes, Stg_Byte* bytes ) {
	RangeSet*	self = (RangeSet*)rangeSet;

	assert( self );
	assert( (nBytes - sizeof(unsigned)) % sizeof(RangeSet_Range) == 0 );
	assert( !nBytes || bytes );

	RangeSet_Destruct( self );

	if( nBytes ) {
		self->nInds = ((unsigned*)bytes)[0];
		if( self->nInds ) {
			unsigned	nRngs;
			RangeSet_Range*	rng;
			unsigned	r_i;

			nRngs = (nBytes - sizeof(unsigned)) / sizeof(RangeSet_Range);
			for( r_i = 0; r_i < nRngs; r_i++ ) {
				rng = (RangeSet_Range*)(bytes + sizeof(unsigned) + r_i * sizeof(RangeSet_Range));
				BTree_InsertNode( self->btree, rng, sizeof(RangeSet_Range) );
			}
		}
	}
}

unsigned RangeSet_Range_GetNumIndices( RangeSet_Range* self ) {
	unsigned	w;

	assert( self );

	w = self->end - self->begin;
	return w / self->step + ((w % self->step) ? 1 : 0);
}

void RangeSet_Range_GetIndices( RangeSet_Range* self, unsigned* nInds, unsigned** inds ) {
	unsigned	ind_i;

	assert( self );

	if( !(*inds) )
		*inds = AllocArray( unsigned, RangeSet_Range_GetNumIndices( self ) );

	*nInds = 0;
	for( ind_i = self->begin; ind_i < self->end; ind_i += self->step )
		(*inds)[(*nInds)++] = ind_i;
}

Bool RangeSet_Range_HasIndex( RangeSet_Range* self, unsigned index ) {
	assert( self );
	assert( self->step > 0 );

	if( index < self->begin || index >= self->end )
		return False;

	return ((index - self->begin) % self->step) ? False : True;
}

void RangeSet_Range_Intersection( RangeSet_Range* left, RangeSet_Range* right, RangeSet_Range* result ) {
	unsigned	a, b, t;
	unsigned	ind_i;

	assert( left );
	assert( right );
	assert( result );

	/* Find start point. */
	for( ind_i = right->begin; ind_i < right->end; ind_i += right->step ) {
		if( RangeSet_Range_HasIndex( left, ind_i ) )
			break;
	}
	if( ind_i >= right->end ) {
		result->begin = 0;
		result->end = 0;
		result->step = 1;
	}
	else
		result->begin = ind_i;

	/* Calculate step size using Euclid's theorem. */
	a = left->step;
	b = right->step;
	while( b != 0 ) {
		t = b;
		b = a % b;
		a = t;
	}
	result->step = a;

	/* Calculate end point. */
	t = (left->end > right->end) ? left->end : right->end;
	result->end = (t / result->step) * result->step;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

void RangeSet_GetIndicesParse( void* data, void* _parse ) {
	RangeSet_Range*		range = (RangeSet_Range*)data;
	RangeSet_ParseStruct*	parse = (RangeSet_ParseStruct*)_parse;
	unsigned		nInds, *tmpInds;

	assert( range );
	assert( parse );

	tmpInds = parse->inds + parse->nInds;
	RangeSet_Range_GetIndices( range, &nInds, &tmpInds );
	parse->nInds += nInds;
}

void RangeSet_GetRangeParse( void* data, void* _parse ) {
	RangeSet_Range*		range = (RangeSet_Range*)data;
	RangeSet_ParseStruct*	parse = (RangeSet_ParseStruct*)_parse;

	assert( range );
	assert( parse );

	if( parse->curInd++ == parse->nInds )
		parse->range = range;
}

void RangeSet_IntersectionParse( void* data, void* _parse ) {
	RangeSet_Range*		range = (RangeSet_Range*)data;
	RangeSet_ParseStruct*	parse = (RangeSet_ParseStruct*)_parse;
	unsigned		ind_i;

	assert( range );
	assert( parse );

	for( ind_i = range->begin; ind_i < range->end; ind_i += range->step ) {
		if( RangeSet_HasIndex( parse->operand, ind_i ) )
			parse->inds[parse->nInds++] = ind_i;
	}

#if 0
	RangeSet_Range*		range = (RangeSet_Range*)data;
	RangeSet_ParseStruct*	parse = (RangeSet_ParseStruct*)_parse;

	assert( range );
	assert( parse );

	parse->range = data;
	RangeSet_RangeIntersectionParse( parse->operand->btree->root, parse );
#endif
}

void RangeSet_SubtractionParse( void* data, void* _parse ) {
	RangeSet_Range*		range = (RangeSet_Range*)data;
	RangeSet_ParseStruct*	parse = (RangeSet_ParseStruct*)_parse;
	unsigned		ind_i;

	assert( range );
	assert( parse );

	for( ind_i = range->begin; ind_i < range->end; ind_i += range->step ) {
		if( !RangeSet_HasIndex( parse->operand, ind_i ) )
			parse->inds[parse->nInds++] = ind_i;
	}
}

void RangeSet_PickleParse( void* data, void* _parse ) {
	RangeSet_Range*		range = (RangeSet_Range*)data;
	RangeSet_ParseStruct*	parse = (RangeSet_ParseStruct*)_parse;

	assert( range );
	assert( parse );

	memcpy( parse->bytes + sizeof(unsigned) + parse->curInd, range, sizeof(RangeSet_Range) );
	parse->curInd += sizeof(RangeSet_Range);
}

#if 0
void RangeSet_RangeIntersectionParse( BTreeNode* node, RangeSet_ParseStruct* parse ) {
	RangeSet_Range*		range;
	RangeSet_Range		newRng;
	unsigned		nInds;

	assert( node );
	assert( node->data );
	assert( parse );

	range = (RangeSet_Range*)node->data;

	if( range->end > parse->range->begin && range->begin < parse->range->end ) {
		RangeSet_Range_Intersection( range, parse->range, &newRng );
		nInds = RangeSet_Range_GetNumIndices( &newRng );
		if( nInds ) {
			parse->nInds += nInds;
			BTree_InsertNode( parse->newTree, &newRng, sizeof(RangeSet_Range) );
		}
	}

	if( range->begin < parse->range->begin && node->left != NIL )
		RangeSet_RangeIntersectionParse( node->left, parse );
	if( range->end > parse->range->end && node->right != NIL )
		RangeSet_RangeIntersectionParse( node->right, parse );
}
#endif

int RangeSet_SortCmp( const void* itema, const void* itemb ) {
	assert( itema && itemb );
	return *((unsigned*)itema) - *((unsigned*)itemb);
}

int RangeSet_DataCompare( void* left, void* right ) {
	RangeSet_Range*	a = (RangeSet_Range*)left;
	RangeSet_Range*	b = (RangeSet_Range*)right;

	if( a->begin >= b->end )
		return 1;
	else if( a->end <= b->begin )
		return -1;
	else
		return 0;
}

void RangeSet_DataCopy( void** dstData, void* data, SizeT size ) {
	*dstData = AllocArray( RangeSet_Range, 1 );
	memcpy( *dstData, data, sizeof(RangeSet_Range) );
}

void RangeSet_DataDelete( void* data ) {
	FreeArray( data );
}


void RangeSet_Destruct( RangeSet* self ) {
	assert( self );

	self->nInds = 0;
	FreeObject( self->btree );
	self->btree = BTree_New( RangeSet_DataCompare, RangeSet_DataCopy, RangeSet_DataDelete, NULL, 
				 BTREE_NO_DUPLICATES );
}


