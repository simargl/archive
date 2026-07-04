cmake_minimum_required(VERSION 3.10)
project (wpset)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(ValaPrecompile)

find_package(PkgConfig)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0>=3.10.0)
pkg_check_modules(IMLIB2 REQUIRED imlib2)

include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})

include_directories(${IMLIB2_INCLUDE_DIRS})
link_directories(${IMLIB2_LIBRARY_DIRS})

set(GETTEXT_PACKAGE "wpset")
add_definitions(-DGETTEXT_PACKAGE=\"wpset\")

vala_precompile(VALA_C 
    src/wpset.vala
PACKAGES
    gtk+-3.0
OPTIONS)
add_executable(wpset ${VALA_C})
add_executable(imlibsetroot src/imlibsetroot.c)

target_link_libraries(wpset ${GTK3_LIBRARIES})
target_link_libraries(imlibsetroot ${IMLIB2_LIBRARIES} -lX11 -lXinerama)

set(CMAKE_INSTALL_PREFIX /usr)

install(TARGETS wpset DESTINATION bin)
install(TARGETS imlibsetroot DESTINATION bin)
install(PROGRAMS data/wpset-shell DESTINATION bin)
install(FILES data/wpset.desktop DESTINATION share/applications)
install(FILES data/org.vala-apps.wpset.gschema.xml DESTINATION share/glib-2.0/schemas)
