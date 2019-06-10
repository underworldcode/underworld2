/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Container_types_h__
#define __StGermain_Base_Container_types_h__

typedef struct Iter Iter;
typedef struct IArray IArray;
typedef struct STree STree;
typedef struct STreeMap STreeMap;
typedef struct ISet ISet;
typedef struct ISetItem ISetItem;
typedef struct ISetIter ISetIter;
typedef struct IMap IMap;
typedef struct IMapItem IMapItem;
typedef struct IMapIter IMapIter;

typedef struct STreeNode STreeNode;
struct STreeNode {
      void* data;
      STreeNode* left;
      STreeNode* right;
};

typedef int (STree_CompareCB)( const void* left, const void* right );
typedef void (STree_DeleteCB)( void* itm );

struct ISetItem {
  int key;
  ISetItem* next;
};

struct IMapItem {
  int key;
  int val;
  IMapItem* next;
};
	
	/* IndexSet types */
	typedef Index					IndexSet_Index;
	
	/* classes */
	typedef struct IndexSet				IndexSet;
	typedef struct PtrMap				PtrMap;
	typedef struct IndexMap				IndexMap;
	typedef struct List				List;
	typedef struct Hasher				Hasher;
	typedef struct NumberHasher			NumberHasher;
	typedef struct Mapping				Mapping;
	typedef struct LinkedListNode			LinkedListNode;
	typedef struct LinkedList			LinkedList;
	typedef struct LinkedListIterator	LinkedListIterator;
	typedef struct HashTable_Entry		HashTable_Entry;
	typedef struct HashTable_Index		HashTable_Index;
	typedef struct HashTable			HashTable;
	typedef struct Set				Set;
	typedef struct PtrSet				PtrSet;
	typedef struct UIntMap			UIntMap;
	typedef struct MemoryPool		MemoryPool;
	typedef struct RangeSet			RangeSet;
	typedef unsigned char		Stg_Byte;
	typedef char					BitField;
	
	typedef struct BTreeNode			BTreeNode;
	typedef struct BTree				BTree;
	typedef struct BTreeIterator			BTreeIterator;
	typedef struct Chunk				Chunk;
	typedef struct ChunkArray			ChunkArray;

	typedef struct MapTuple				MapTuple;

	typedef enum Color_t{
		BTREE_NODE_RED,
		BTREE_NODE_BLACK
	}
	Color;

	typedef enum BTreeProperty_t{
		BTREE_ALLOW_DUPLICATES,
		BTREE_NO_DUPLICATES
	}
	BTreeProperty;

	typedef enum Order_t{
		LINKEDLIST_SORTED,
		LINKEDLIST_UNSORTED
	}
	Order;

	typedef enum KeyType_t{
		HASHTABLE_STRING_KEY,
		HASHTABLE_INTEGER_KEY,
		HASHTABLE_POINTER_KEY
	}
	KeyType;


	typedef struct {
		unsigned	begin;
		unsigned	end;
		unsigned	step;
	} RangeSet_Range;

	typedef struct {
		RangeSet*	self;
		RangeSet*	operand;
		RangeSet_Range*	range;
		BTree*		newTree;
		unsigned	nInds;
		unsigned*	inds;
		unsigned	curInd;
		Stg_Byte*	bytes;
	} RangeSet_ParseStruct;


#endif /* __StGermain_Base_Container_types_h__ */

