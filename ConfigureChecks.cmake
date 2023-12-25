################################################################################
# MetaTheme - a common style engine for TQt, GTK+2 and Java/SWING              #
#                                                                              #
# Improvements and feedback are welcome!                                       #
# This software is licensed under the terms of the GNU GPL v3 license.         #
################################################################################

# TQt toolkit: tqt, tde
if( WITH_TOOLKIT_TQT )
  find_package( TQt )
  find_package( TDE )

  tde_setup_architecture_flags( )

  include( TestBigEndian )
  test_big_endian( WORDS_BIGENDIAN )

  tde_setup_largefiles( )

  if( WITH_GCC_VISIBILITY )
    tde_setup_gcc_visibility( )
  endif( WITH_GCC_VISIBILITY )
endif( )

# Configuration tool: needs gtk+2 support
if( BUILD_CONFIG_TOOL AND NOT WITH_TOOLKIT_GTK2 )
  tde_message_fatal( "The configuration tool requires GTK+2 support." )
endif( )

# MSStyle engine: needs gtk+2 support
if( WITH_ENGINE_MSSTYLE AND NOT WITH_TOOLKIT_GTK2 )
  tde_message_fatal( "The MSSTYLE theme engine requires GTK+2 support." )
endif( )

# Optional Cairo support
if( WITH_CAIRO )
  pkg_check_modules( CAIRO cairo>=0.5.1 )
  if( NOT CAIRO_FOUND )
    tde_message_fatal( "Cairo support requested, but it was not found on your system." )
  endif( )

  set( HAVE_CAIRO 1 CACHE INTERNAL "" FORCE )
endif( )

# GTK+2 toolkit: gtk+2
if( WITH_TOOLKIT_GTK2 )
  pkg_check_modules( GTK2 gtk+-2.0>=2.2.0 )
  if( NOT GTK2_FOUND )
    tde_message_fatal( "GTK+2 support requested, but it was not found on your system." )
  endif( )

  execute_process(
    COMMAND ${PKG_CONFIG_EXECUTABLE} gtk+-2.0 --variable=gtk_binary_version
    OUTPUT_VARIABLE GTK2_BIN_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  set( INSTALL_PATH_GTK2_ENGINES "${GTK2_LIBDIR}/gtk-2.0/${GTK2_BIN_VERSION}/engines"
       CACHE PATH "The directory in which to install the theme engine library" )
  message( STATUS "  theme engine lib directory: ${INSTALL_PATH_GTK2_ENGINES}" )
  set( INSTALL_PATH_GTK2_THEMES "${GTK2_PREFIX}/share/themes"
       CACHE PATH "The directory in which to install the theme data" )
  MESSAGE( STATUS "  theme engine data directory: ${INSTALL_PATH_GTK2_THEMES}" )
endif( )

# MSStyle engine: glib2
if( WITH_ENGINE_MSSTYLE )
  pkg_search_module( GLIB2 glib-2.0 )
  if( NOT GLIB2_FOUND )
    tde_message_fatal( "glib-2.0 are required, but not found on your system" )
  endif( )
endif( )

if( WITH_TOOLKIT_JAVA )
  # TODO
  message( "Java toolkit build is not supported yet, sorry! ")
endif( )

message("")
message("Build configuration:")
message("--------------------")
message("  Toolkits")
message("  ==================")
message("  TQt:           ${WITH_TOOLKIT_TQT}")
message("  GTK+2:         ${WITH_TOOLKIT_GTK2}")
message("  Java/SWING:    ${WITH_TOOLKIT_JAVA}")
message("")
message("  Themes")
message("  ==================")
message("  Redmond 2000:  ${WITH_ENGINE_REDMOND}")
message("  Plastik:       ${WITH_ENGINE_PLASTIK}")
message("  Highcolor:     ${WITH_ENGINE_HIGHCOLOR}")
message("  Industrial:    ${WITH_ENGINE_INDUSTRIAL}")
message("  Flat:          ${WITH_ENGINE_FLAT}")
message("  MSSTYLE:       ${WITH_ENGINE_MSSTYLE}")
message("")
message("  Optional modules")
message("  ==================")
message("  Config tool:   ${BUILD_CONFIG_TOOL}")
message("  Colorschemes:  ${WITH_COLORSCHEMES}")
message("")
message("  Optional support")
message("  ==================")
message("  Cairo:         ${WITH_CAIRO}")
message("")