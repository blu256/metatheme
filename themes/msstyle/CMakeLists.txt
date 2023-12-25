################################################################################
# MetaTheme - a common style engine for TQt, GTK+2 and Java/SWING              #
#                                                                              #
# Improvements and feedback are welcome!                                       #
# This software is licensed under the terms of the GNU GPL v3 license.         #
################################################################################

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/metatheme
  ${GTK2_INCLUDE_DIRS}
  ${CAIRO_INCLUDE_DIRS}
  ${GLIB2_INCLUDE_DIRS}
)

link_directories(
  ${GTK2_LIBRARY_DIRS}
  ${CAIRO_LIBRARY_DIRS}
  ${GLIB2_LIBRARY_DIRS}
  ${CMAKE_BINARY_DIR}/metatheme
)

set( themedir ${METATHEMEDIR}/themes/msstyle )

tde_add_library(mt_msstyle SHARED
  SOURCES msstyle-loader.c msstyle.c msstyle-ini.c
  LINK ${GLIB2_LIBRARIES}
  DESTINATION ${themedir}
)

tde_add_executable(msstyle-config
  SOURCES msstyle-config.c
  LINK ${GTK2_LIBRARIES} ${CAIRO_LIBRARIES} metatheme-shared
  DESTINATION ${themedir}
)

install( FILES themerc DESTINATION ${themedir} )