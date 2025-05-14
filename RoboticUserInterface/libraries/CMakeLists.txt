add_subdirectory(qwool)
add_subdirectory(qt_material_widgets)
add_subdirectory(qt_gcw)
add_subdirectory(qcustomplot)
add_subdirectory(FluControls)
add_subdirectory(FluUtils)

target_link_libraries(${PROJECT_NAME}
  qwool
  qt_material_widgets
  qt_gcw
  qcustomplot
  FluControls
  FluUtils
)
