################################################################################
# MetaTheme - a common style engine for TQt, GTK+2 and Java/SWING              #
#                                                                              #
# Improvements and feedback are welcome!                                       #
# This software is licensed under the terms of the GNU GPL v3 license.         #
################################################################################

cmake_minimum_required( VERSION 3.1 )

### Project setup ##########################
project( tde-style-metatheme )

### Required modules ###########################################################
include( FindPkgConfig          )
include( CheckFunctionExists    )
include( CheckSymbolExists      )
include( CheckIncludeFile       )
include( CheckLibraryExists     )
include( CheckCSourceCompiles   )
include( CheckCXXSourceCompiles )

### TDE macros #################################################################
include( TDEMacros )
tde_set_project_version( )

include( TDESetupPaths )
tde_setup_paths( )

### Optional support ###########################################################
option( WITH_ALL_OPTIONS    "Enable all optional support"           OFF)
option( WITH_TOOLKIT_TQT    "Enable support for TQt toolkit"        ON)
option( WITH_TOOLKIT_GTK2   "Enable support for Gtk+2 toolkit"      ${WITH_ALL_OPTIONS})
option( WITH_TOOLKIT_JAVA   "Enable support for Java/SWING toolkit" ${WITH_ALL_OPTIONS})
option( WITH_CAIRO          "Enable support for Cairo"              ${WITH_ALL_OPTIONS})
option( WITH_COLORSCHEMES   "Install optional colorschemes"         ${WITH_ALL_OPTIONS})

option( WITH_ALL_ENGINES       "Enable all theme engines"           ${WITH_ALL_OPTIONS})
option( WITH_ENGINE_REDMOND    "Enable Redmond 2000 theme engine"   ${WITH_ALL_ENGINES})
option( WITH_ENGINE_PLASTIK    "Enable Plastik theme engine"        ${WITH_ALL_ENGINES})
option( WITH_ENGINE_HIGHCOLOR  "Enable Highcolor theme engine"      ${WITH_ALL_ENGINES})
option( WITH_ENGINE_INDUSTRIAL "Enable Industrial theme engine"     ${WITH_ALL_ENGINES})
option( WITH_ENGINE_FLAT       "Enable Flat theme engine"           ${WITH_ALL_ENGINES})
option( WITH_ENGINE_MSSTYLE    "Enable MSSTYLE theme engine"        ${WITH_ALL_ENGINES})

### Modules to build ###########################################################
option( BUILD_ALL               "Build all"                   ON )
option( BUILD_DOC               "Build documentation"         ${BUILD_ALL} )
option( BUILD_TRANSLATIONS      "Build translations"          ${BUILD_ALL} )
option( BUILD_CONFIG_TOOL       "Build configuration tool"    ${BUILD_ALL} )

### Configure checks ###########################################################
include( ConfigureChecks.cmake )

### Compiler settings ##########################################################
add_definitions( -DHAVE_CONFIG_H )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TQT_CXX_FLAGS}" )
set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined" )
set( CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--no-undefined" )

set( METATHEMEDIR ${LIB_INSTALL_DIR}/metatheme CACHE INTERNAL "" FORCE)

### Add subdirectories #########################################################
add_subdirectory( metatheme )
add_subdirectory( themes )
add_subdirectory( toolkits )
add_subdirectory( colors )
add_subdirectory( config )
tde_conditional_add_subdirectory( WITH_COLORSCHEMES kde )
tde_conditional_add_project_docs( BUILD_DOC )
tde_conditional_add_project_translations( BUILD_TRANSLATIONS )

### Write configuration ########################################################
configure_file( config.h.cmake config.h @ONLY )