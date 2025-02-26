cmake_minimum_required(VERSION 3.16)

set(sources
    src/checks.c
    src/filename.c
    src/pcuassert.c
    src/runner.c
    src/source.c
    src/suite.c
    src/test.c
    src/textoutput.c
    src/utils.c
    src/xmloutput.c
    )

target_sources(pcu PRIVATE ${sources})
