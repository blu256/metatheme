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
)

link_directories(
  ${GTK2_LIBRARY_DIRS}
  ${CAIRO_LIBRARY_DIRS}
  ${CMAKE_BINARY_DIR}/metatheme
)

tde_add_executable(mt-config
  SOURCES
    about.c
    color_scheme.c
    colors.c
    fonts.c
    util.c
    main.c
    preview.c
    themes.c
    settings.c

  LINK
    ${GTK2_LIBRARIES}
    ${CAIRO_LIBRARIES}
    metatheme-shared

  DESTINATION ${BIN_INSTALL_DIR}
)