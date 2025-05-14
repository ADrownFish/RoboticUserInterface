############
## Config ##
############

cmake_minimum_required(VERSION 3.20.0)
project(robotic_user_interface)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
add_compile_options(-fPIC)

set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS
  Core
  Gui
  Network
  Widgets
  Bluetooth
  OpenGL
  OpenGLWidgets
  PrintSupport
  SerialPort
  StateMachine
  Svg
)

###########
## Build ##
###########

add_subdirectory(libraries/qwool)
add_subdirectory(libraries/qt_material_widgets)
add_subdirectory(libraries/qt_gcw)
add_subdirectory(libraries/qcustomplot)
add_subdirectory(libraries/FluControls)
add_subdirectory(libraries/FluUtils)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include/robotic_user_interface
  ${CMAKE_CURRENT_SOURCE_DIR}/libraries
)

# UI 文件
file(GLOB_RECURSE UI_FILES CONFIGURE_DEPENDS
  include/form/*.ui
)
qt6_wrap_ui(UI_HEADERS
  ${UI_FILES}
)

# 资源文件
qt6_add_resources(QRC_SOURCES
  resources.qrc
)

# 所有源文件（.cpp）
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
  src/*.cpp
)

# 头文件（.h）
file(GLOB_RECURSE PROJECT_HEADERS CONFIGURE_DEPENDS
  include/robotic_user_interface/*.h
)

link_directories(
)

# 构建可执行
add_executable(${PROJECT_NAME}
  ${PROJECT_SOURCES}
  ${PROJECT_HEADERS}
  ${UI_HEADERS}
  ${QRC_SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}         # ✅ 这里面有 ui_xxx.h
)

target_link_libraries(${PROJECT_NAME}

  qwool
  qt_material_widgets
  qt_gcw
  qcustomplot
  FluControls
  FluUtils

  Qt6::Core
  Qt6::Gui
  Qt6::Network
  Qt6::Widgets
  Qt6::Bluetooth
  Qt6::OpenGL
  Qt6::OpenGLWidgets
  Qt6::PrintSupport
  Qt6::SerialPort
  Qt6::StateMachine
  Qt6::Svg
)

#############
## Install ##
#############

