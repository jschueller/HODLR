cmake_minimum_required (VERSION 3.13)

set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type")

project (HODLRlib)

if (POLICY CMP0148)
  cmake_policy(SET CMP0148 NEW)
endif ()

include (GNUInstallDirs)

option(BUILD_SHARED_LIBS "shared/static libs" ON)

# set RPATH, useful for python bindings
set (CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})

set (CMAKE_CXX_STANDARD 11)

# Provide the input file that you want to solve for:
# This can also be passed as a command line argument:
if(NOT INPUT_FILE)
    set (INPUT_FILE "")
endif()
# Here set the name for the output executable:
# This can also be passed as a command line argument:
if(NOT OUTPUT_EXECUTABLE)
    set (OUTPUT_EXECUTABLE "")
endif()

################ FINDING EIGEN ####################
find_package(Eigen3 CONFIG REQUIRED)

find_package(OpenMP)

################ SETTING DATATYPE TO USE:FLOAT, DOUBLE AND COMPLEX ####################
# Set this to: "float", "double", "complex32" and "complex64"
set (DTYPE "double" CACHE STRING "scalar type")

if(DTYPE STREQUAL "float")
    set(DTYPE_FLAG "USE_FLOAT")
elseif(DTYPE STREQUAL "double")
    set(DTYPE_FLAG "USE_DOUBLE")
elseif(DTYPE STREQUAL "complex32")
    set(DTYPE_FLAG "USE_COMPLEX32")
elseif(DTYPE STREQUAL "complex64")
    set(DTYPE_FLAG "USE_COMPLEX64")
else()
    message(FATAL_ERROR "Invalid Choice for Datatype. Exiting...")
endif()

# Flags for testing coverage information
# Need by coveralls:
set(CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS} --coverage")
set(CMAKE_EXE_LINKER_FLAGS_COVERAGE "${CMAKE_EXE_LINKER_FLAGS} --coverage")
set(CMAKE_SHARED_LINKER_FLAGS_COVERAGE "${CMAKE_SHARED_LINKER_FLAGS} --coverage")

################ ADDING OF THE NECESSARY HEADER FOLDERS ####################
# Adding folders which contain necessary header files:
include_directories("include")

################ MENTIONING C++ FILES TO LINK WITH HEADERS + FILE TO BUILD ####################

set(HODLRlib_SRCS
    ${PROJECT_SOURCE_DIR}/src/HODLR_Matrix.cpp
    ${PROJECT_SOURCE_DIR}/src/LowRank.cpp
    ${PROJECT_SOURCE_DIR}/src/HODLR_Node.cpp
    ${PROJECT_SOURCE_DIR}/src/HODLR.cpp
    ${PROJECT_SOURCE_DIR}/src/HODLR_NonSPD.cpp
    ${PROJECT_SOURCE_DIR}/src/HODLR_SPD.cpp
    ${PROJECT_SOURCE_DIR}/src/KDTree.cpp
   )

if (MSVC)
  set_source_files_properties (${PROJECT_SOURCE_DIR}/src/LowRank.cpp PROPERTIES COMPILE_FLAGS "/bigobj")
endif ()
   
add_library(HODLR ${HODLRlib_SRCS})
set_target_properties(HODLR PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(HODLR PUBLIC ${DTYPE_FLAG})
target_link_libraries(HODLR PUBLIC Eigen3::Eigen)
if (OPENMP_FOUND)
  target_compile_definitions(HODLR PUBLIC HODLR_HAVE_OPENMP)
  target_link_libraries(HODLR PUBLIC OpenMP::OpenMP_CXX)
endif ()

install(TARGETS HODLR DESTINATION ${CMAKE_INSTALL_LIBDIR})

file(GLOB HODLRlib_HDRS ${PROJECT_SOURCE_DIR}/include/*.hpp)
install(FILES ${HODLRlib_HDRS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

enable_testing()

if(NOT("${CMAKE_BUILD_TYPE}" STREQUAL "COVERAGE"))
    add_subdirectory(examples)
    add_subdirectory(test)
endif()

if(INPUT_FILE)
  add_executable(${OUTPUT_EXECUTABLE}
                 ${INPUT_FILE}
                )
  target_link_libraries(${OUTPUT_EXECUTABLE} HODLR)
endif()

option(HODLR_PYTHON "Enable Python bindings" ON)
if (HODLR_PYTHON)
  add_subdirectory(python)
endif ()

option(HODLR_FORTRAN "Enable Fortran bindings" OFF)
if (HODLR_FORTRAN)
  add_subdirectory(fortran)
endif ()

################ SUMMARY OF BUILD INFORMATION ################

message("")
message("#################")
message("# Build options #")
message("#################")
message("")
message("Input File       : " ${INPUT_FILE})
message("Data Type        : " ${DTYPE})
message("CXX Compiler     : " ${CMAKE_CXX_COMPILER})
message("CXX_FLAGS        : " ${CMAKE_CXX_FLAGS})
message("Eigen dir        : " ${EIGEN3_INCLUDE_DIR})
message("Build dir        : " ${CMAKE_BINARY_DIR})
message("")
