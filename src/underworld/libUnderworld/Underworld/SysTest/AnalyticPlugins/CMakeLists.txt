cmake_minimum_required(VERSION 3.16)

set(sources
    ./Velic_solDA/solDA.c
    ./Velic_solIA/solIA.c
    ./Velic_solF/solF.c
    ./Velic_solC/solC.c
    ./VelicIC/VelicIC.c
    ./Velic_solB/solB.c
    ./Velic_solKx/solKx.c
    ./Velic_solHy/solHy.c
    ./Velic_solH/solH.c
    ./Velic_solJ/solJ.c
    ./Velic_solE/solE.c
    ./Velic_solHAy/solHAy.c
    ./Velic_solG/solG.c
    ./Velic_solL/solL.c
    ./Velic_solHA/solHA.c
    ./Velic_solI/solI.c
    ./Velic_solCA/solCA.c
    ./Velic_solJA/solJA.c
    ./Velic_solA/solA.c
    ./Velic_solKz/solKz.c
    ./Velic_solCx/solCx.c
    ./Velic_solS/solS.c
    )

target_sources(Underworld PRIVATE ${sources})