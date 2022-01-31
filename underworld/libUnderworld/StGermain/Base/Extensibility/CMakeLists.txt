cmake_minimum_required(VERSION 3.16)

set(sources
    src/ClassHook.c
    src/ClassPtrExtensionInfo.c
    src/EntryPoint.c
    src/EntryPoint_Register.c
    src/ExtensionInfo.c
    src/ExtensionManager.c
    src/ExtensionManager_Register.c
    src/Finalise.c
    src/Hook.c
    src/Init.c
    src/Module.c
    src/ModulesManager.c
    src/Plugin.c
    src/PluginsManager.c
    src/SimpleExtensionInfo.c
    src/Toolbox.c
    src/ToolboxesManager.c
    )

target_sources(StGermain PRIVATE ${sources})
