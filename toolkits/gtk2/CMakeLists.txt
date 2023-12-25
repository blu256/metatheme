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
)

link_directories(
  ${CMAKE_BINARY_DIR}/metatheme
  ${GTK2_LIBRARY_DIRS}
)

tde_add_library(mt_gtk2 SHARED
  SOURCES
    mt_gtk2.c
    mt_gtk2_style.c
    mt_gtk2_draw.c
    mt_gtk2_patches.c

  LINK
    ${GTK2_LIBRARIES}
    ${CAIRO_LIBRARIES}
    metatheme-shared

  DESTINATION ${METATHEMEDIR}/toolkits
)

configure_file(gtkrc.in gtkrc @ONLY)
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/gtkrc
  DESTINATION ${INSTALL_PATH_GTK2_THEMES}/MetaTheme/gtk-2.0
)