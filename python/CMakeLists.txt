# add_subdirectory(pybind11)

find_package(pybind11 CONFIG REQUIRED)



pybind11_add_module(pyhodlrlib
                    wrapper/pyhodlrlib.cpp
                   )
target_link_libraries(pyhodlrlib PRIVATE HODLR)

if (WIN32)
  set (INSTALL_PYTHONDIR Lib/site-packages CACHE STRING "site packages dir")
else ()
  set (INSTALL_PYTHONDIR ${CMAKE_INSTALL_LIBDIR}/python${Python_VERSION_MAJOR}.${Python_VERSION_MINOR}/site-packages CACHE STRING "site packages dir")
endif ()

install(TARGETS pyhodlrlib DESTINATION ${INSTALL_PYTHONDIR})
