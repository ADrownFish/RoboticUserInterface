cmake_minimum_required(VERSION 3.20.0)
project(qcustomplot)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    StateMachine
    Svg
    OpenGL
    OpenGLWidgets
    PrintSupport
)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_link_libraries(${PROJECT_NAME}
  PRIVATE
      Qt6::Core
      Qt6::Gui
      Qt6::Widgets
      Qt6::StateMachine
      Qt6::Svg
      Qt6::OpenGL
      Qt6::OpenGLWidgets
      Qt6::PrintSupport
)
