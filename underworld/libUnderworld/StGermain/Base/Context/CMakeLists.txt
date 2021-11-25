cmake_minimum_required(VERSION 3.16)

set(sources
    src/AbstractContext.c
    src/Codelet.c
    src/ConditionFunction.c
    src/ConditionFunction_Register.c
    src/ContextEntryPoint.c
    src/DictionaryCheck.c
    src/Finalise.c
    src/Init.c
    src/PythonVC.c
    src/Variable.c
    src/VariableCondition.c
    src/VariableCondition_Register.c
    src/Variable_Register.c
    )

target_sources(StGermain PRIVATE ${sources})
