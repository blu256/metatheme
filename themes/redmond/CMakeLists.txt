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
)

set( themedir ${METATHEMEDIR}/themes/redmond )

tde_add_library(mt_redmond SHARED
  SOURCES redmond.c
  DESTINATION ${themedir}
)

install( FILES themerc radio.xpm DESTINATION ${themedir} )