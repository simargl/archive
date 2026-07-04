cmake_minimum_required(VERSION 3.10)
project (agatha)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(ValaPrecompile)

find_package(PkgConfig)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0>=3.8.0)
pkg_check_modules(POPPLER-GLIB REQUIRED poppler-glib)

include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})

include_directories(${POPPLER-GLIB_INCLUDE_DIRS})
link_directories(${POPPLER-GLIB_LIBRARY_DIRS})

set(GETTEXT_PACKAGE "agatha")
add_definitions(-DGETTEXT_PACKAGE=\"agatha\")

vala_precompile(VALA_C 
    src/settings.vala
    src/pagezoom.vala
    src/viewer.vala
    src/main.vala
    src/dialogs.vala
PACKAGES
    gtk+-3.0
    poppler-glib
OPTIONS
    ${GLOBAL_VALAC_OPTIONS})
add_executable(agatha ${VALA_C})
target_link_libraries(agatha ${GTK3_LIBRARIES} ${POPPLER-GLIB_LIBRARIES})

set(CMAKE_INSTALL_PREFIX /usr)

install(TARGETS agatha DESTINATION bin)
install(FILES data/agatha.desktop DESTINATION share/applications)
install(FILES data/org.vala-apps.agatha.gschema.xml DESTINATION share/glib-2.0/schemas)
