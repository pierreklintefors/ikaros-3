set(CMAKE_INCLUDE_CURRENT_DIR ON)
add_subdirectory(Logger)
add_subdirectory(Idle)
add_subdirectory(KernelTestModule)

add_subdirectory(Constant)
add_subdirectory(Randomizer)

add_subdirectory(CombineFunctions)
add_subdirectory(ReduceFunctions)

add_subdirectory(Scale)
add_subdirectory(Oscillator)
add_subdirectory(Protocol)
add_subdirectory(Print)
add_subdirectory(OutputFile)

set(SOURCE ${SOURCE} PARENT_SCOPE)
set(MODULES_INCLUDS ${MODULES_INCLUDS} PARENT_SCOPE)
set(MODULES_LIBS ${MODULES_LIBS} PARENT_SCOPE)

