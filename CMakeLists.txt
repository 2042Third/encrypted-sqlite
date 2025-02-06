cmake_minimum_required(VERSION 3.20)
project(encrypted_sqlite VERSION 0.1 LANGUAGES C CXX)

# Include GNU install directory variables
include(GNUInstallDirs)

# Optionally force an install prefix (you can also set this on the command line)
if(NOT CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install" CACHE PATH "Installation Directory" FORCE)
endif()

# Use C++20
set(CMAKE_CXX_STANDARD 20)

# Force shared builds for dependencies
set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries" FORCE)

# Define your project files
set(PROJECT_FILES
    src/db/pdm_database.h
    src/db/pdm_database.cpp
    src/crypto/pdmCryptoDB.hpp
)

# Build dependencies (forcing them static here if desired)
set(BUILD_SHARED_LIBS_TEMP ${BUILD_SHARED_LIBS})
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(lib/pdm-crypt-module)  # pdm encryption module
add_subdirectory(lib/cryptoSQLite)        # encrypted database
set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS_TEMP})

# Create the shared library for encrypted_sqlite
add_library(encrypted_sqlite SHARED ${PROJECT_FILES})

set_target_properties(encrypted_sqlite PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    # List public headers (they will be installed)
    PUBLIC_HEADER "src/db/pdm_database.h;src/crypto/pdmCryptoDB.hpp"
)

# Define include directories so that during the build and for consumers, they are available.
target_include_directories(encrypted_sqlite PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# macOS-specific settings
if(APPLE)
  set_target_properties(encrypted_sqlite PROPERTIES
      LINK_FLAGS "-Wl,-all_load"
      OSX_ARCHITECTURES "arm64;x86_64"
      # This tells the installed dylib where to look for dependent libraries.
      INSTALL_RPATH "@loader_path/../${CMAKE_INSTALL_LIBDIR}"
  )
  # Allow undefined symbols in shared libs (commonly needed on macOS)
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -undefined dynamic_lookup")
endif()

# Export symbols for the shared library (if needed)
include(GenerateExportHeader)
generate_export_header(encrypted_sqlite)
target_compile_definitions(encrypted_sqlite PRIVATE ENCRYPTED_SQLITE_EXPORTS)

# Link against dependencies (assuming these targets are defined by the subdirectories)
target_link_libraries(encrypted_sqlite PRIVATE
    cc20
    cryptosqlite
)

# Install the library and headers in a central location.
install(TARGETS encrypted_sqlite
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}     # for shared libraries (.dylib on macOS)
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}     # for static libraries
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}      # for executables
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/encrypted_sqlite
)

install(
    FILES "${CMAKE_SOURCE_DIR}/build/lib/cryptoSQLite/libcryptosqlite.dylib"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)