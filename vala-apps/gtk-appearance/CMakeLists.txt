cmake_minimum_required(VERSION 3.10)
project (gtk-appearance)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(ValaPrecompile)

find_package(PkgConfig)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0>=3.10.0)

include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})

set(GETTEXT_PACKAGE "gtk-appearance")
add_definitions(-DGETTEXT_PACKAGE=\"gtk-appearance\")

vala_precompile(VALA_C 
    src/settings.vala
    src/main.vala
PACKAGES
    gtk+-3.0
OPTIONS)
add_executable(gtk-appearance ${VALA_C})
target_link_libraries(gtk-appearance ${GTK3_LIBRARIES})

set(CMAKE_INSTALL_PREFIX /usr)

install(TARGETS gtk-appearance DESTINATION bin)
install(PROGRAMS data/gtk-appearance-openbox DESTINATION bin)
install(FILES data/gtk-appearance.desktop DESTINATION share/applications)
