cmake_minimum_required(VERSION 3.20.0)
project(FluUtils)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS
	  Core
		Gui
		Widgets
		StateMachine
		Svg
)

add_library(${PROJECT_NAME} INTERFACE)

set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)

if (WIN32)
	target_link_libraries(${PROJECT_NAME} INTERFACE
		  $<$<CONFIG:Debug>:${LIB_DIR}/${PROJECT_NAME}d.lib>
			$<$<CONFIG:Release>:${LIB_DIR}/${PROJECT_NAME}.lib>
		)
elseif (UNIX)
	target_link_libraries(${PROJECT_NAME} INTERFACE
		  $<$<CONFIG:Debug>:${LIB_DIR}/lib${PROJECT_NAME}d.a>
			$<$<CONFIG:Release>:${LIB_DIR}/lib${PROJECT_NAME}.a>
		)
endif()

target_link_libraries(${PROJECT_NAME} INTERFACE
	Qt6::Core
	Qt6::Gui
	Qt6::Widgets
	Qt6::StateMachine
	Qt6::Svg
)
