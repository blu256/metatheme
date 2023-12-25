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

set( themedir ${METATHEMEDIR}/themes/highcolor )
set( themedir_b3 ${METATHEMEDIR}/themes/b3 )

tde_add_library(mt_highcolor SHARED
  SOURCES highcolor.cpp
  LINK metatheme-shared
  DESTINATION ${themedir}
)

install( FILES themerc DESTINATION ${themedir} )
install( FILES b3rc DESTINATION ${themedir_b3} RENAME themerc )