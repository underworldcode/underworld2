cmake_minimum_required(VERSION 3.16)

set(sources
    src/BinaryStream.c
    src/CFile.c
    src/CmdLineArgs.c
    src/CStream.c
    src/Dictionary.c
    src/Dictionary_Entry.c
    src/Dictionary_Entry_Value.c
    src/DictionaryUtils.c
    src/File.c
    src/Finalise.c
    src/IndentFormatter.c
    src/Init.c
    src/IO_Handler.c
    src/LineFormatter.c
    src/MPIFile.c
    src/MPIStream.c
    src/PathUtils.c
    src/RankFormatter.c
    src/shortcuts.c
    src/stgmessaging.c
    src/Stream.c
    src/StreamFormatter.c
    src/XML_IO_Handler.c
    src/Journal.cpp
    )

target_sources(StGermain PRIVATE ${sources})
