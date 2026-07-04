cmake_minimum_required(VERSION 3.10)
project (pangea)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(ValaPrecompile)

find_package(PkgConfig)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0>=3.10.0)
pkg_check_modules(GUDEV REQUIRED gudev-1.0)

include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})

include_directories(${GUDEV_INCLUDE_DIRS})
link_directories(${GUDEV_LIBRARY_DIRS})

set(GETTEXT_PACKAGE "pangea")
add_definitions(-DGETTEXT_PACKAGE=\"pangea\")

vala_precompile(VALA_C 
    src/iconview.vala
    src/main.vala
    src/menu.vala
    src/monitor.vala
    src/operations.vala
    src/settings.vala
    src/thumbnails.vala
    src/vars.vala
    src/window.vala
PACKAGES
    gtk+-3.0
    gudev-1.0
OPTIONS
)

add_executable(pangea ${VALA_C})
target_link_libraries(pangea ${GTK3_LIBRARIES} ${GUDEV_LIBRARIES})

set(CMAKE_INSTALL_PREFIX /usr)

install(TARGETS pangea DESTINATION bin)
install(FILES data/pangea.desktop DESTINATION share/applications)
install(FILES data/org.vala-apps.pangea.gschema.xml DESTINATION share/glib-2.0/schemas)


