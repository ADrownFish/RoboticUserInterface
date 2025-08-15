############
## Config ##
############

cmake_minimum_required(VERSION 3.20.0)
project(Realsense VERSION 1.0.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_Fortran_COMPILER FALSE CACHE FILEPATH "Disable Fortran")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    add_compile_options(-fPIC)
endif()

find_package(Qt6 REQUIRED COMPONENTS
  Core
  Gui
  Network
  Widgets
)

###########
## Build ##
###########

add_subdirectory(libraries/qwool)
add_subdirectory(libraries/qt_material_widgets)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/libraries
)

# UI 文件
file(GLOB_RECURSE UI_FILES CONFIGURE_DEPENDS
  form/*.ui
)
qt6_wrap_ui(UI_HEADERS
  ${UI_FILES}
)

# 资源文件
qt6_add_resources(QRC_SOURCES

)

# 所有源文件（.cpp）
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
  src/*.cpp
)

# 头文件（.h）
file(GLOB_RECURSE PROJECT_HEADERS CONFIGURE_DEPENDS
  include/*.h
)

link_directories(
)

# 构建可执行
add_library(${PROJECT_NAME} MODULE
  ${PROJECT_SOURCES}
  ${PROJECT_HEADERS}
  ${UI_HEADERS}
  ${QRC_SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  qwool
  qt_material_widgets

  Qt6::Core
  Qt6::Gui
  Qt6::Network
  Qt6::Widgets
)

if (WIN32)
  target_link_libraries(${PROJECT_NAME} ws2_32 advapi32)
endif()

# 确保导出所有符号
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_VISIBILITY_PRESET default
    VISIBILITY_INLINES_HIDDEN OFF
)

#############
## Install ##
#############

