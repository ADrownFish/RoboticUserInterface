############
## Config ##
############

cmake_minimum_required(VERSION 3.0.2)
project(filter)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
  add_compile_options(-fPIC)
endif()

###########
## Build ##
###########

include_directories(
  include
)

link_directories(
)

## Declare a cpp library
add_library(${PROJECT_NAME}  STATIC 
  src/LiteKalmanFilter.cpp
  src/MovingAverageFilter.cpp
  src/IIRFilter.cpp
)

target_link_libraries(${PROJECT_NAME} 

)

#############
## Install ##
#############
