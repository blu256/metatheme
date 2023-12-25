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
  ${TDE_INCLUDE_DIR}
  ${TQT_INCLUDE_DIRS}
  ${CAIRO_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
  ${CMAKE_BINARY_DIR}/metatheme
  ${CAIRO_LIBRARY_DIRS}
)

tde_add_library(mt_tqt SHARED AUTOMOC
  SOURCES mt_tqt.cpp

  LINK
    tqt-mt
    tdecore-shared
    tdeui-shared
    tdefx-shared
    ${CAIRO_LIBRARIES}
    metatheme-shared

  DESTINATION ${METATHEMEDIR}/toolkits
)

install(
  FILES metatheme.themerc
  DESTINATION ${DATA_INSTALL_DIR}/tdestyle/themes
)

# Create a symlink so that TDE sees the library
add_custom_command(
    TARGET mt_tqt-shared POST_BUILD
    COMMAND ln -sf ${METATHEMEDIR}/toolkits/libmt_tqt.so ${PLUGIN_INSTALL_DIR}/plugins/styles/metatheme.so
)