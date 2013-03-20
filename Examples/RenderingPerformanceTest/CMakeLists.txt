MITK_CHECK_MODULE(MISSING QmitkExt)
message(STATUS "Build RenderingPerformanceTest")
if(MISSING)
  message("RenderingPerformanceTest won't be built because QmitkExt is missing.")
else(MISSING)

  MITK_USE_MODULE(QmitkExt)
  MITK_USE_MODULE(MitkExt)

  include_directories(${ALL_INCLUDE_DIRECTORIES})

  #QT4_WRAP_CPP(STEP8_MOC_CPP Step8.h)

  add_executable(RenderingPerformanceTest RenderingPerformanceTest.cpp)
  target_link_libraries(RenderingPerformanceTest ${ALL_LIBRARIES})

  set_property(TARGET RenderingPerformanceTest PROPERTY LABELS ${MITK_DEFAULT_SUBPROJECTS})

endif(MISSING)
