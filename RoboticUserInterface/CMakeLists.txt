############
## Config ##
############

cmake_minimum_required(VERSION 3.20.0)
project(robotic_user_interface  VERSION 1.0.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_Fortran_COMPILER FALSE CACHE FILEPATH "Disable Fortran")
set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    add_compile_options(-fPIC)
endif()

# 获取当前时间戳
string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%d %H:%M:%S")

# 配置编译信息头文件
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/BuildInfo.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/BuildInfo.h"
)


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
  DataVisualization
  Concurrent
)

###########
## Build ##
###########

# 关闭Eigen测试目标和不必要模块
# 在原有CMakeLists.txt中添加/修改以下选项

# 禁用测试
option(BUILD_TESTING "Enable creation of Eigen tests." OFF)
option(EIGEN_BUILD_DOC "Enable creation of Eigen documentation" OFF)
option(EIGEN_BUILD_BTL "Build benchmark suite" OFF)
option(EIGEN_BUILD_PKGCONFIG "Build pkg-config .pc file for Eigen" OFF)
option(EIGEN_TEST_CXX11 "Enable testing with C++11 and C++11 features (e.g. Tensor module)." OFF)
option(EIGEN_TEST_NO_EXPLICIT_VECTORIZATION "Disable explicit vectorization in tests/examples" ON)
option(EIGEN_TEST_NO_EXPLICIT_ALIGNMENT "Disable explicit alignment (hence vectorization) in tests/examples" ON)
option(EIGEN_TEST_SYCL "Add Sycl support." OFF)
option(EIGEN_SPLIT_LARGE_TESTS "Split large tests into smaller executables" OFF)

add_subdirectory(libraries/eigen)
add_subdirectory(libraries/ellipsoid-fit)
# add_subdirectory(libraries/QSsh)
add_subdirectory(libraries/qwool)
add_subdirectory(libraries/qt_material_widgets)
add_subdirectory(libraries/qt_gcw)
add_subdirectory(libraries/qcustomplot)
add_subdirectory(libraries/FluentUI)
add_subdirectory(libraries/taskflow)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include/robotic_user_interface
  ${CMAKE_CURRENT_SOURCE_DIR}/libraries
  ${CMAKE_CURRENT_SOURCE_DIR}/libraries/FluentUI
  ${CMAKE_CURRENT_SOURCE_DIR}/libraries/eigen
  ${CMAKE_CURRENT_SOURCE_DIR}/libraries/ellipsoid-fit/include/ellipsoid_fit
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

# 添加翻译文件列表
set(TS_FILES
    translations/zh_CN.ts

)

# 查找 Qt 的工具（包含 lrelease）
find_package(Qt6 REQUIRED COMPONENTS Core LinguistTools)

# 自动从 .ts 生成 .qm
qt6_add_translation(QM_FILES ${TS_FILES})

link_directories(
)

# 构建可执行
add_executable(${PROJECT_NAME}
  ${PROJECT_SOURCES}
  ${PROJECT_HEADERS}
  ${UI_HEADERS}
  ${QRC_SOURCES}
  ${QM_FILES}
)

if (MSVC)
  if (CMAKE_BUILD_TYPE STREQUAL "Release")
    # Release 模式：隐藏控制台（GUI 程序）
    set_target_properties(${PROJECT_NAME} PROPERTIES
      LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
    )
  else()
    # Debug 模式：保留控制台（方便调试）
    set_target_properties(${PROJECT_NAME} PROPERTIES
      LINK_FLAGS "/SUBSYSTEM:CONSOLE"
    )
  endif()
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME}
  qwool
  qt_material_widgets
  qt_gcw
  qcustomplot
  FluControls
  FluUtils
  ellipsoid-fit
  Taskflow

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
  Qt6::DataVisualization
  Qt6::Concurrent
)

if (WIN32)
  target_link_libraries(${PROJECT_NAME} ws2_32 advapi32)
endif()

#############
## Install ##
#############

