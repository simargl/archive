cmake_minimum_required(VERSION 3.10)
project (emendo)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(ValaPrecompile)

find_package(PkgConfig)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0>=3.10.0)
pkg_check_modules(GTKSOURCEVIEW3 REQUIRED gtksourceview-3.0)

include_directories(${GTKSOURCEVIEW3_INCLUDE_DIRS})
link_directories(${GTKSOURCEVIEW3_LIBRARY_DIRS})

set(GETTEXT_PACKAGE emendo)
add_definitions(-DGETTEXT_PACKAGE=\"${GETTEXT_PACKAGE}\")

vala_precompile(VALA_C 
    src/operations.vala
    src/tabs.vala
    src/prefdialog.vala
    src/settings.vala
    src/apply.vala
    src/replace.vala
    src/find.vala
    src/application.vala
    src/color.vala
    src/nbook.vala
    src/dialogs.vala
    src/mainwin.vala
PACKAGES
    gtk+-3.0
    gtksourceview-3.0
OPTIONS)
add_executable(emendo ${VALA_C})
target_link_libraries(emendo ${GTKSOURCEVIEW3_LIBRARIES} -lm)

set(CMAKE_INSTALL_PREFIX /usr)

install(TARGETS emendo DESTINATION bin)
install(FILES data/emendo.desktop DESTINATION share/applications)
install(FILES data/org.vala-apps.emendo.gschema.xml DESTINATION share/glib-2.0/schemas)
