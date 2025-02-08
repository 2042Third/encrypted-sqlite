cmake_minimum_required(VERSION 3.20)
project(encrypted_sqlite VERSION 0.1 LANGUAGES C CXX)

# Include GNU install directory variables
include(GNUInstallDirs)

# Optionally force an install prefix
if(NOT CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/install" CACHE PATH "Installation Directory" FORCE)
endif()

# Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform-specific settings
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# Define your project files
set(PROJECT_FILES
        src/db/pdm_database.h
        src/db/pdm_database.cpp
        src/crypto/pdmCryptoDB.hpp
)

# Build dependencies
add_subdirectory(lib/pdm-crypt-module)  # pdm encryption module
add_subdirectory(lib/cryptoSQLite)      # encrypted database

# Create the shared library for encrypted_sqlite
add_library(encrypted_sqlite SHARED ${PROJECT_FILES})

set_target_properties(encrypted_sqlite PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        PUBLIC_HEADER "src/db/pdm_database.h;src/crypto/pdmCryptoDB.hpp"
        # Set output directories for all configurations
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# Define include directories
target_include_directories(encrypted_sqlite PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/cryptoSQLite/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_include_directories(encrypted_sqlite PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/cryptoSQLite/external/securememory/include
)

# Platform-specific library settings
if(APPLE)
    set_target_properties(encrypted_sqlite PROPERTIES
            OSX_ARCHITECTURES "arm64;x86_64"
            INSTALL_RPATH "@loader_path/../${CMAKE_INSTALL_LIBDIR}"
            BUILD_WITH_INSTALL_RPATH TRUE
    )
elseif(WIN32)
    # Windows-specific settings
    set_target_properties(encrypted_sqlite PROPERTIES
            WINDOWS_EXPORT_ALL_SYMBOLS ON
    )
endif()

# Link dependencies
target_link_libraries(encrypted_sqlite PRIVATE
        cc20
        cryptosqlite
)

# Generate export header
include(GenerateExportHeader)
generate_export_header(encrypted_sqlite)
target_compile_definitions(encrypted_sqlite PRIVATE ENCRYPTED_SQLITE_EXPORTS)

# Install rules
install(TARGETS encrypted_sqlite cryptosqlite
        EXPORT encrypted_sqlite-targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/encrypted_sqlite
)

# Wrapper library
add_library(wrapper SHARED wrapper.cpp wrapper.h)

target_include_directories(wrapper PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/db
        ${CMAKE_CURRENT_SOURCE_DIR}/src/crypto
        ${CMAKE_CURRENT_SOURCE_DIR}/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/cryptoSQLite
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/cryptoSQLite/external/securememory/include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/pdm-crypt-module/src/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/cryptoSQLite/include
)

target_link_libraries(wrapper PRIVATE
        encrypted_sqlite
        cryptosqlite
)

set_target_properties(wrapper PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

if(APPLE)
    set_target_properties(wrapper PROPERTIES
            OSX_ARCHITECTURES "arm64;x86_64"
            INSTALL_RPATH "@loader_path/../${CMAKE_INSTALL_LIBDIR}"
            BUILD_WITH_INSTALL_RPATH TRUE
    )
elseif(WIN32)
    set_target_properties(wrapper PROPERTIES
            WINDOWS_EXPORT_ALL_SYMBOLS ON
    )
endif()

# Install wrapper
install(TARGETS wrapper
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES "wrapper.h"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Create and install CMake config files
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/encrypted_sqlite-config-version.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)

install(EXPORT encrypted_sqlite-targets
        FILE encrypted_sqlite-targets.cmake
        NAMESPACE encrypted_sqlite::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/encrypted_sqlite
)
