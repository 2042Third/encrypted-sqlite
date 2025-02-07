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



if(MSVC)
    set(BUILD_SHARED_LIBS_TEMP ${BUILD_SHARED_LIBS})
    set(BUILD_SHARED_LIBS OFF)
    add_subdirectory(lib/cryptoSQLite/external/securememory)
    set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS_TEMP})

    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

if(NOT APPLE)
    # Remove the -undefined dynamic_lookup flag for non-Apple platforms
    string(REPLACE "-undefined dynamic_lookup" "" CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")
endif()

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

# macOS-specific settings for encrypted_sqlite
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

if(WIN32)
    set_target_properties(encrypted_sqlite PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    )
endif()

# Link against dependencies (assuming these targets are defined by the subdirectories)
target_link_libraries(encrypted_sqlite PRIVATE
    cc20
    cryptosqlite
    "${CMAKE_BINARY_DIR}/lib/libSecureMemory.dll.a"  # Use exact path and filename
)
# Also make sure the build order is correct by adding:
add_dependencies(encrypted_sqlite SecureMemory)
# Also add this to help find the DLL at runtime
target_link_directories(encrypted_sqlite PRIVATE
    "${CMAKE_BINARY_DIR}/lib"
)
# And ensure the runtime output directories are set properly for Windows:
if(WIN32)
    set_target_properties(SecureMemory PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    )
    
    set_target_properties(encrypted_sqlite PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    )
endif()

target_include_directories(encrypted_sqlite PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/cryptoSQLite/external/securememory/include
)

# Install the encrypted_sqlite library and headers
install(TARGETS encrypted_sqlite
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}     # for shared libraries (.dylib on macOS)
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}       # for static libraries
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}        # for executables
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/encrypted_sqlite
)

# (Retain any extra install rules you already have)
install(
    FILES "${CMAKE_SOURCE_DIR}/build/lib/cryptoSQLite/libcryptosqlite.dylib"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

########################################################################
# Build the wrapper shared library from wrapper.cpp (replacing manual clang++)
########################################################################

# Assuming wrapper.cpp sits in the top-level source directory.
add_library(wrapper SHARED wrapper.cpp wrapper.h)

# Add the include directories used in your manual command:
target_include_directories(wrapper PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}                                  # ./
    ${CMAKE_CURRENT_SOURCE_DIR}/src                              # ./src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/db                           # ./src/db
    ${CMAKE_CURRENT_SOURCE_DIR}/src/crypto                         # ./src/crypto
    ${CMAKE_CURRENT_SOURCE_DIR}/lib                              # ./lib
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/cryptoSQLite                   # ./lib/cryptoSQLite
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/cryptoSQLite/external/securememory/include  # ./lib/cryptoSQLite/external/securememory/include
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/pdm-crypt-module/src/lib       # ./lib/pdm-crypt-module/src/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/cryptoSQLite/include           # ./lib/cryptoSQLite/include
)

# Link the wrapper against the libraries you used (-lencrypted_sqlite -lcryptosqlite)
target_link_libraries(wrapper PRIVATE
    encrypted_sqlite
    cryptosqlite
)

# On macOS, set the rpath and install_name options to match your manual flags.
if(APPLE)
  target_link_options(wrapper PRIVATE
      "-Wl,-rpath,@loader_path/../Frameworks"
      "-Wl,-install_name,@rpath/libwrapper.dylib"
  )
  set_target_properties(wrapper PROPERTIES OSX_ARCHITECTURES "arm64;x86_64")
endif()

# Install the wrapper library into the same install directories.
install(TARGETS wrapper
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    FILES "${CMAKE_SOURCE_DIR}/wrapper.h"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
