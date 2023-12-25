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

set( themedir ${METATHEMEDIR}/themes/plastik )
set( themedir_xp ${METATHEMEDIR}/themes/plastikxp )

tde_add_library(mt_plastik SHARED
  SOURCES plastik.c
  DESTINATION ${themedir}
)

install( FILES themerc DESTINATION ${themedir} )
install( FILES plastikxp DESTINATION ${themedir_xp} RENAME themerc )