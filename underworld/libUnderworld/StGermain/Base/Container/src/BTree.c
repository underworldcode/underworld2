/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <Base/Foundation/Foundation.h>
#include <Base/IO/IO.h>

#include "types.h"
#include "BTreeNode.h"
#include "BTree.h"
#include "ChunkArray.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define  POOL_SIZE 1000
#define  POOL_DELTA 1000

const Type BTree_Type = "BTree";

BTree *_BTree_New(
		SizeT					_sizeOfSelf,
		Type					type,
		Stg_Class_DeleteFunction*	_delete,
		Stg_Class_PrintFunction*	_print,
		Stg_Class_CopyFunction*		_copy
		)
{
	BTree *self = NULL;

	/** BTree has to be malloced instead of using Class_New, because Class_New uses Memory_Alloc, but the Memory module will not have been
	 *  initialized at this stage */
	
	self = (BTree*)malloc( _sizeOfSelf );
	memset( self, 0, _sizeOfSelf );
	
	self->_sizeOfSelf = _sizeOfSelf;
	self->_deleteSelf = True;
	self->type = type;
	self->_delete = _delete;
	self->_print = _print;
	self->_copy = _copy;
	
	return self;
}

BTree *BTree_New( 
		BTree_compareFunction*		compareFunction,
		BTree_dataCopyFunction*		dataCopyFunction,
		BTree_dataDeleteFunction*	dataDeleteFunction,
		BTree_dataPrintFunction*	dataPrintFunction,
		BTreeProperty				property )
{
	
	BTree* self;
	
	/* Allocate memory */
	assert ( compareFunction );
	
	self = _BTree_New(
			sizeof(BTree),
			BTree_Type,
			_BTree_DeleteFunc,
			_BTree_PrintFunc,
			NULL
			);
	/* General info */
	
	/* Virtual functions */
	
	self->compareFunction = compareFunction;
	self->dataCopyFunction = dataCopyFunction;
	self->dataDeleteFunction = dataDeleteFunction;
	self->dataPrintFunction = dataPrintFunction;
	self->property = property;

	self->pool = NULL;

	_BTree_Init( self );	
	return self;
}

void _BTree_Init( BTree* self )
{

	assert(self);
	self->root = NIL;

	self->pool = ChunkArray_New( BTreeNode, POOL_SIZE );
}

void BTree_Init( BTree *self )
{
	_BTree_Init( self );
}

void BTree_LeftRotate( BTree *tree, BTreeNode *x ) 
{
 
	BTreeNode *root;
	BTreeNode *y;
	 
	assert( tree );
	assert( x );
	
	y = x->right;
	root = tree->root;
 
 
	if (x == NIL){
	}
	
	/* establish x->right link */
	x->right = y->left;
	if ( y->left != NIL ) y->left->parent = x;
 
	/* establish y->parent link */
	if (y != NIL) {
		y->parent = x->parent;
	}
	
	if ( x->parent ) {
		if (x == x->parent->left)
		{
             x->parent->left = y;
		}
		else{
             x->parent->right = y;
		}
	} 
	else{
		tree->root = y;
	}
 
	/* link x and y */
	y->left = x;
	if (x != NIL){
		x->parent = y;
	}
 }
 
void BTree_RightRotate( BTree *tree, BTreeNode *x ) {
 
	BTreeNode *root;
	BTreeNode *y;
	
	assert( tree );
	assert( x );
	
	y = x->left;
	root = tree->root;
 
 
	if( x == NIL ){
	}

	/* establish x->left link */
	x->left = y->right;
	if (y->right != NIL){
		y->right->parent = x;
	}
		
	/* establish y->parent link */
	if (y != NIL) {
		y->parent = x->parent;
	}
	
	if( x->parent ){
		if (x == x->parent->right){
			x->parent->right = y;
		}
        else{
             x->parent->left = y;
		}
	}
	else{
		tree->root = y;
	}
 
	/* link x and y */
	y->right = x;
	if (x != NIL){
		x->parent = y;
	}
}

void BTree_InsertFix (BTree *tree, BTreeNode *x){
	
	BTreeNode *y = NIL, *root;

	assert ( tree );
	assert ( x );

	root = tree->root;
	
	while ( ( x != root ) && ( x->parent->color == BTREE_NODE_RED ) ){
    	if ( x->parent == x->parent->parent->left ) {
			/* If x's parent is a left, y is x's right 'uncle' */
			y = x->parent->parent->right;
			
			if ( y->color == BTREE_NODE_RED ) {
				/* case 1 - change the colours */
				x->parent->color = BTREE_NODE_BLACK;
				y->color = BTREE_NODE_BLACK;
				x->parent->parent->color = BTREE_NODE_RED;
				/* Move x up the tree */
				x = x->parent->parent;
			}
			else{
				/* y is a BTREE_NODE_BLACK node */
				if ( x == x->parent->right ) {
					/* and x is to the right  */
					/* case 2 - move x up and rotate */
					x = x->parent;
					BTree_LeftRotate( tree, x );
				}
				/* case 3 */
				x->parent->color = BTREE_NODE_BLACK;
				x->parent->parent->color = BTREE_NODE_RED;
				BTree_RightRotate( tree, x->parent->parent );
			}
		}
		else if ( x->parent == x->parent->parent->right ){
			
			/* If x's parent is a right, y is x's right 'uncle' */ 
			y = x->parent->parent->left;
			
				if ( y->color == BTREE_NODE_RED ) {
					/* case 1 - change the colours */
					x->parent->color = BTREE_NODE_BLACK;
					y->color = BTREE_NODE_BLACK;
					x->parent->parent->color = BTREE_NODE_RED;
					/* Move x up the tree */
					x = x->parent->parent;
				}
				else{
					/* y is a BTREE_NODE_BLACK node */
					if ( x == x->parent->left ) {
						/* and x is to the right  */
						/* case 2 - move x up and rotate */ 
						x = x->parent;
						BTree_RightRotate( tree, x );
					}
					/* case 3 */
					x->parent->color = BTREE_NODE_BLACK;
					x->parent->parent->color = BTREE_NODE_RED;
					BTree_LeftRotate( tree, x->parent->parent );
				}
		}
		else{

		}
	}

	/* Colour the root BTREE_NODE_BLACK */
	tree->root->color = BTREE_NODE_BLACK;
}
/* The BTree_Insert function assumes that the compare function accepts two void pointers to NodeData as arguments */
int BTree_InsertNode ( BTree *tree, void *newNodeData, SizeT sizeOfData ){
	
	BTreeNode *curr, *spot, *newNode;

	assert ( tree );
	assert ( newNodeData );
	/*assert ( sizeOfData > 0 ); removed for PtrSet - Luke */
	
	curr = tree->root;
	
	newNode = BTreeNode_New( tree->pool );
	newNode->sizeOfData = sizeOfData;
	
	if ( tree->dataCopyFunction ){
		tree->dataCopyFunction ( &newNode->data, newNodeData, newNode->sizeOfData);
	}
	else{
		newNode->data = newNodeData;
	}
	
	spot = NULL;
	while ( curr != NIL ){
		spot = curr;
		if ( tree->compareFunction( newNode->data, curr->data ) > 0 ){
			curr = curr->right;
		}
		else if( tree->compareFunction( newNode->data, curr->data ) == 0 ){
			if( tree->property == BTREE_ALLOW_DUPLICATES ){
				curr = curr->right;
			}
			else{
				if( tree->dataDeleteFunction )
					tree->dataDeleteFunction( newNode->data );
				
				if( tree->pool ){
					ChunkArray_DeleteObjectID( tree->pool, newNode->id );
				}
				else{
					free ( newNode );
				}

				return 0;
			}
		}
		else
		{
			curr = curr->left;
		}
	}

	if (spot){
		
		newNode->parent = spot;
		if ( tree->compareFunction( newNode->data, spot->data ) < 0 ){
			spot->left = newNode;
		}
		else{
			spot->right = newNode;
		}
	}
	else{
		tree->root = newNode;
	}

	BTree_InsertFix ( tree, newNode );
	++tree->nodeCount;
#if	0
	printf ("nodeCount from insert%d\n", tree->nodeCount);
#endif
	return 1;
}

void BTree_SetCompareFunction ( BTree *tree, BTree_compareFunction *compareFunction )
{
	assert( tree );
	assert( compareFunction );
	
	tree->compareFunction = compareFunction;
}

BTreeNode *BTree_FindNode( BTree *tree, void *data ){
	BTreeNode *curr = NIL;
	int result;

	assert ( tree );

	if ( data == NULL ){
		return NULL;
	}
	
	curr = tree->root;

#if 0
	printf ( "[Finding Node---------\n" );
#endif
	
	while (curr != NIL){
		result = tree->compareFunction ( (void*) curr->data, (void*) data );
		if ( result == 0 ){
			
#if 0
			printf ( "\tFound node after %d hops\n\n", hops );
#endif			

#if 0
	printf ( "Finding Node---------]\n" );
#endif

			return curr;
		}
		else{
			if ( result > 0 ){
				curr = curr->left;
			}
			else{
				curr = curr->right;
			}
		}
	}
#if 0
	printf ( "Finding Node---------]\n" );
#endif
	return NULL;
}

BTreeNode *BTree_FindNodeAndHops( BTree *tree, void *data, int *hops ){
	BTreeNode *curr = NIL;
	int result;

	assert ( tree );
	assert( hops );

	if ( data == NULL ){
		return NULL;
	}
	
	curr = tree->root;
	*hops = 0;

#if 0
	printf ( "[Finding Node---------\n" );
#endif
	
	while (curr != NIL){
		result = tree->compareFunction ( (void*) curr->data, (void*) data );
		if ( result == 0 ){
			
#if 0
			printf ( "\tFound node after %d hops\n\n", hops );
#endif			

#if 0
	printf ( "Finding Node---------]\n" );
#endif

			return curr;
		}
		else{
			(*hops)++;
			
			if ( result > 0 ){
				curr = curr->left;
			}
			else{
				curr = curr->right;
			}
		}
	}
#if 0
	printf ( "\tFailed to find node after %d hops\n\n", hops );
#endif
#if 0
	printf ( "Finding Node---------]\n" );
#endif
	return NULL;
}

void BTree_DeleteFixup( BTree *tree, BTreeNode *x ) {
 
	BTreeNode *root, *w;

	assert( tree );
	assert ( x );
	
	root = tree->root;
	while ( x != root && x->color == BTREE_NODE_BLACK ) {
		if ( x == x->parent->left ) {
			w = x->parent->right;
			if ( w->color == BTREE_NODE_RED ) {
				w->color = BTREE_NODE_BLACK;
				x->parent->color = BTREE_NODE_RED;
				BTree_LeftRotate ( tree, x->parent );
				w = x->parent->right;
			}
			if ( w->left->color == BTREE_NODE_BLACK && w->right->color == BTREE_NODE_BLACK ) {
				w->color = BTREE_NODE_RED;
				x = x->parent;
			} else {
				if ( w->right->color == BTREE_NODE_BLACK ) {
					w->left->color = BTREE_NODE_BLACK;
					w->color = BTREE_NODE_RED;
					BTree_RightRotate ( tree, w );
					w = x->parent->right;
				}
				w->color = x->parent->color;
				x->parent->color = BTREE_NODE_BLACK;
				w->right->color = BTREE_NODE_BLACK;
				BTree_LeftRotate ( tree, x->parent );
				x = root;
			}
		} else {
			w = x->parent->left;
			if ( w->color == BTREE_NODE_RED ) {
				w->color = BTREE_NODE_BLACK;
				x->parent->color = BTREE_NODE_RED;
				BTree_RightRotate ( tree, x->parent );
				w = x->parent->left;
			}
			if ( w->right->color == BTREE_NODE_BLACK && w->left->color == BTREE_NODE_BLACK ) {
				w->color = BTREE_NODE_RED;
				x = x->parent;
			} else {
				if ( w->left->color == BTREE_NODE_BLACK ) {
					w->right->color = BTREE_NODE_BLACK;
					w->color = BTREE_NODE_RED;
					BTree_LeftRotate ( tree, w );
					w = x->parent->left;
				}
				w->color = x->parent->color;
				x->parent->color = BTREE_NODE_BLACK;
				w->left->color = BTREE_NODE_BLACK;
				BTree_RightRotate ( tree, x->parent );
				x = root;
			}
		}
	}
	x->color = BTREE_NODE_BLACK;
}
 
void BTree_DeleteNode( BTree *tree, BTreeNode *z ) {
	BTreeNode *x, *y;
 
	assert( tree );
	
	if ( !z || z == NIL ) return;
 
	assert ( z );
	
	if ( z->left == NIL || z->right == NIL ) {
		/* y has a NIL node as a child */
		y = z;
	} else {
		/* find tree successor with a NIL node as a child */
		y = z->right;
		while ( y->left != NIL ) y = y->left;
	}
 
	/* x is y's only child */
	if ( y->left != NIL )
		x = y->left;
	else
		x = y->right;
 
	/* remove y from the parent chain */
	x->parent = y->parent;
	
	if ( y->parent ){
		if ( y == y->parent->left ){
			y->parent->left = x;
		}
		else{
			y->parent->right = x;
		}
	}
	else{
		tree->root = x;
	}
 
	if ( y != z ){
		if ( tree->dataCopyFunction ){
		   	tree->dataCopyFunction ( &z->data, y->data, y->sizeOfData );
		}
		else{
			z->data = y->data;
		}
	}
 
 
	if ( y->color == BTREE_NODE_BLACK ){
		BTree_DeleteFixup ( tree, x );
	}
 
	if ( tree->dataDeleteFunction ){
		tree->dataDeleteFunction( (void*) y->data );
	}

	if( tree->pool ){
		ChunkArray_DeleteObjectID( tree->pool, y->id );
	}
	else{
		free ( y );
	}
	--tree->nodeCount;
#if	0
	printf ("nodeCount from delete %d\n", tree->nodeCount);
#endif
}

void BTree_ParseTreeNode( BTreeNode *root, BTree_parseFunction *parseFunction, void *args ){

	assert ( root );
	assert ( parseFunction );
	
	if ( root == NIL ){
		return;
	}
	
	BTree_ParseTreeNode( root->left, parseFunction, args );
	
	assert ( root->data );
	parseFunction ( root->data, args );
	
	BTree_ParseTreeNode( root->right, parseFunction, args );
}

void BTree_ParseTree( BTree *tree, BTree_parseFunction *parseFunction, void *args ){

	assert ( tree );
	assert ( parseFunction );
	
	if ( tree->root == NIL ){
		return;
	}
	
	BTree_ParseTreeNode(tree->root, parseFunction, args);
}

void* BTree_GetData( BTreeNode *node )
{
	if ( node == NULL ){
		return NULL;
	}
	else{
		return (void*) node->data;
	}
}

void _BTree_DeleteFunc_Helper( BTreeNode *node, BTree_dataDeleteFunction *nodeDataDeleteFunc, ChunkArray *pool )
{
	BTreeNode *left = NULL;
	BTreeNode *right = NULL;
	
	if( node != NULL ){
		if ( node == NIL ){
			return;
		}
	
		left = node->left;
		right = node->right;
		
		_BTree_DeleteFunc_Helper( left, nodeDataDeleteFunc, pool );
		
		if( node->data != NULL ){
			if( nodeDataDeleteFunc != NULL ){
				nodeDataDeleteFunc( node->data );
			}
			/** The BTree_Delete function will not attempt to delete any NodeData unless a nodeDataDeleteFunction is explicitly specified by the user */
#if 0
			else{
				free( node->data );
			}
#endif
		}

		if( pool ){
			ChunkArray_DeleteObjectID( pool, node->id );
		}
		else{
			free( node );
		}

		_BTree_DeleteFunc_Helper( right, nodeDataDeleteFunc, pool );
	}
}

void _BTree_DeleteFunc( void *self )
{
	BTree *tree = NULL;
	
	tree = (BTree*) self;
	assert( tree );

	_BTree_DeleteFunc_Helper( tree->root, tree->dataDeleteFunction, tree->pool );
	tree->nodeCount = 0;
	/* freeing the tree instead of using class_delete, because it was initially malloced */

	if( tree->pool ){
		Stg_Class_Delete( tree->pool );
	}

	free( tree );
}

void _BTree_PrintFunc_Helper( BTreeNode *node, BTree_dataPrintFunction *nodeDataPrintFunc, Stream *myStream )
{
	if( node != NULL ){
		if ( node == NIL ){
			return;
		}
		assert( myStream );

		_BTree_PrintFunc_Helper( node->left, nodeDataPrintFunc, myStream );
		if( node->data != NULL ){
			if( nodeDataPrintFunc != NULL ){
				nodeDataPrintFunc( node->data, myStream );
			}
			else{
				Journal_Printf( myStream, "\tData - (ptr): (%p)\n", node->data );
			}
		}
		_BTree_PrintFunc_Helper( node->right, nodeDataPrintFunc, myStream );
	}
}

void _BTree_PrintFunc( void *self, Stream *myStream )
{
	BTree *tree = NULL;

	tree = (BTree*) self;
	assert( tree );

	/* print parent */
	_Stg_Class_Print( (void*) tree, myStream );

	/* general info */
	Journal_Printf( myStream, "BTree (ptr): (%p)\n", tree );
	
	_BTree_PrintFunc_Helper( tree->root, tree->dataPrintFunction, myStream );
	/* Virtual Info */

	/* BTree Info */
	
}

void BTree_Delete( void* tree ) {
	Stg_Class_Delete( tree );
}


