cmake_minimum_required(VERSION 3.16)

set(sources
    ./src/BTree.c
    ./src/BTreeIterator.c
    ./src/BTreeNode.c
    ./src/ChunkArray.c
    ./src/Finalise.c
    ./src/HashTable.c
    ./src/IArray.c
    ./src/IMap.c
    ./src/IMapIter.c
    ./src/IndexMap.c
    ./src/IndexSet.c
    ./src/Init.c
    ./src/ISet.c
    ./src/ISetIter.c
    ./src/Iter.c
    ./src/LinkedList.c
    ./src/LinkedListIterator.c
    ./src/LinkedListNode.c
    ./src/List.c
    ./src/MemoryPool.c
    ./src/MPIRoutines.c
    ./src/PtrMap.c
    ./src/PtrSet.c
    ./src/RangeSet.c
    ./src/Set.c
    ./src/STree.c
    ./src/STreeMap.c
    ./src/UIntMap.c)

target_sources(StGermain PRIVATE ${sources})
