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
  ${CAIRO_INCLUDE_DIRS}
)

link_directories(
  ${CAIRO_LIBRARY_DIRS}
)

tde_add_library(metatheme SHARED
  SOURCES
    metatheme.c
    refresh.c

  LINK
    ${CAIRO_LIBRARIES}
    X11

  DESTINATION ${LIB_INSTALL_DIR}
)

configure_file( metatheme-install.in metatheme-install @ONLY )
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/metatheme-install
  DESTINATION ${BIN_INSTALL_DIR}
  PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE
)

install(
  FILES metathemerc-sample
  DESTINATION ${METATHEMEDIR}
)