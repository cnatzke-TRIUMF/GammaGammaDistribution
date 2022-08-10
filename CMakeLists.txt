#------------------------------------------------------------------
#Setup the project
#------------------------------------------------------------------
cmake_minimum_required(VERSION 3.2)

project(GenerateDistribution VERSION 0.1)

#Locate the ROOT package and defines a number of variables (e.g. ROOT_INCLUDE_DIRS)
find_package(ROOT CONFIG REQUIRED)
include(${ROOT_USE_FILE})

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

# pick up c++ standard from ROOT install
string(APPEND CMAKE_CXX_FLAGS "${ROOT_CXX_FLAGS} -pedantic -Wno-long-long -g")

# Connect ROOT to project
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})

# Connect GRSISORT headers
set(GRSI_INCLUDE_DIRS $ENV{GRSISYS}/include $ENV{GRSISYS}/GRSIData/include)

# Grsisort libraries for linking
execute_process(COMMAND grsi-config --all-libs --GRSIData-libs --root 
   OUTPUT_VARIABLE GRSI_CONFIG
   OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Add fitting libraries - very specific order otherwise it fails
#set(GRSI_CONFIG "-L$ENV{GRSISYS} -lTPeakFitting -lTGRSIFit -lGROOT ${GRSI_CONFIG}")

# Add parsing library
set(PARSE_LINKER "-lconfig++")

# Add parsing library
set(JROOT_LINKER "-lJanalysistools -lX11")

# Message for debugging
#message(STATUS "Found Grsisort libraries: ${GRSI_CONFIG}")

set(CMAKE_CXX_FLAGS "-Wall -O3 ${CMAKE_CXX_FLAGS} ${GRSI_CONFIG}")

# Adding src files
file(GLOB SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)

# Naming main executable
add_executable(GenerateDistribution ${SOURCES})

# linking libraries
target_link_libraries(GenerateDistribution PUBLIC
   ${GRSI_CONFIG}
   ${PARSE_LINKER}
   ${JROOT_LINKER}
)

# add the binary tree to the search path for include files so that we will find AnalysisTreeConverter.h
target_include_directories(GenerateDistribution PUBLIC
    "${PROJECT_BINARY_DIR}"
    "${GRSI_INCLUDE_DIRS}"
    include
    )

# add install targets
install(TARGETS GenerateDistribution DESTINATION "${PROJECT_BINARY_DIR}/bin")
install(FILES "${PROJECT_BINARY_DIR}/GenerateDistribution.h"
   DESTINATION "${PROJECT_BINARY_DIR}/include"
)
