
enable_language(Fortran)

# Provide the input file that you want to solve for:
# This can also be passed as a command line argument:
if(NOT INPUT_FILE)
    set (INPUT_FILE "example/example_lowrank.f90")
endif()
# Here set the name for the output executable:
# This can also be passed as a command line argument:
if(NOT OUTPUT_EXECUTABLE)
    set (OUTPUT_EXECUTABLE "example_fortran")
endif()

if("${CMAKE_BUILD_TYPE}" STREQUAL "TESTING")
    set (INPUT_FILE "test/test_HODLR.f90")
    set (OUTPUT_EXECUTABLE "test_HODLR")
endif()

set(CMAKE_Fortran_FLAGS "-lstdc++")

################ ADDING OF THE NECESSARY HEADER FOLDERS ####################
# Adding folders which contain necessary header files:
include_directories(wrapper example)

add_executable(${OUTPUT_EXECUTABLE}
               ${INPUT_FILE}
               wrapper/fortran_wrapper.cpp
               wrapper/hodlr_mod.f90
              )
target_link_libraries(${OUTPUT_EXECUTABLE} HODLR)

set_property(TARGET ${OUTPUT_EXECUTABLE} PROPERTY LINKER_LANGUAGE Fortran)

add_test(NAME ${OUTPUT_EXECUTABLE} COMMAND ${OUTPUT_EXECUTABLE})