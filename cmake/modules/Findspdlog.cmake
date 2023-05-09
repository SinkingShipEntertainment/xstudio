
# FindSpdlog
# -------
# Finds the Spdlog library
#
# This will define the following variables:
#
# SPDLOG_FOUND - system has Spdlog
# spdlog_INCLUDE_DIRS - the Spdlog include directory
# SPDLOG_LIBRARIES - the Spdlog libraries
# SPDLOG_DEFINITIONS - the Spdlog compile definitions
#
# and the following imported targets:
#
#   spdlog::spdlog   - The Spdlog library


find_package(spdlog CONFIG REQUIRED
    PATHS $ENV{REZ_SPDLOG_ROOT}/lib64/cmake/spdlog)

find_path(spdlog_INCLUDE_DIR
    NAMES spdlog/spdlog.h
    PATHS $ENV{REZ_SPDLOG_ROOT}/include)

find_library(spdlog_LIBRARY
    NAMES spdlog
    PATHS $ENV{REZ_SPDLOG_ROOT}/lib64)

include(SelectLibraryConfigurations)
select_library_configurations(SPDLOG)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Spdlog REQUIRED_VARS
    spdlog_LIBRARY
    spdlog_INCLUDE_DIR)

if(spdlog_FOUND)
  set(spdlog_LIBRARIES ${spdlog_LIBRARY})
  set(spdlog_INCLUDE_DIRS ${spdlog_INCLUDE_DIR})
  set(spdlog_DEFINITIONS -DSPDLOG_FMT_EXTERNAL
                         -DSPDLOG_DEBUG_ON
                         -DSPDLOG_NO_ATOMIC_LEVELS
                         -DSPDLOG_ENABLE_PATTERN_PADDING
                         -DSPDLOG_COMPILED_LIB
                         ${SPDLOG_CFLAGS})

  if(NOT TARGET spdlog::spdlog)
    add_library(spdlog::spdlog UNKNOWN IMPORTED)
    if(spdlog_LIBRARY)
      set_target_properties(spdlog::spdlog PROPERTIES
                                           IMPORTED_CONFIGURATIONS RELEASE
                                           IMPORTED_LOCATION "${spdlog_LIBRARY}")
    endif()
    set_target_properties(spdlog::spdlog PROPERTIES
                                         INTERFACE_INCLUDE_DIRECTORIES "${spdlog_INCLUDE_DIR}"
                                         INTERFACE_COMPILE_DEFINITIONS "${SPDLOG_DEFINITIONS}")
  endif()
  if(TARGET spdlog)
    add_dependencies(spdlog::spdlog spdlog)
  endif()
  set_property(GLOBAL APPEND PROPERTY INTERNAL_DEPS_PROP spdlog::spdlog)
endif()

mark_as_advanced(spdlog_INCLUDE_DIR spdlog_LIBRARY)