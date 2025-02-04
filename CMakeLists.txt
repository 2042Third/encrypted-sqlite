cmake_minimum_required(VERSION 3.20)

project(encrypted_sqlite VERSION 0.1 LANGUAGES C CXX)
set (CMAKE_CXX_STANDARD 20)

# Force shared builds for all dependencies ----------------------
set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries" FORCE)
# --------------------------------------------------------------

set(PROJECT_FILES
    src/db/pdm_database.h
    src/db/pdm_database.cpp
    src/crypto/pdmCryptoDB.hpp
)

# Force static builds for dependencies
set(BUILD_SHARED_LIBS_TEMP ${BUILD_SHARED_LIBS})
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(lib/pdm-crypt-module) # pdm encryption
add_subdirectory(lib/cryptoSQLite) # encrypted database
set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS_TEMP})

# Always build shared library
add_library(encrypted_sqlite SHARED ${PROJECT_FILES})

set_target_properties(encrypted_sqlite PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "src/db/pdm_database.h;src/crypto/pdmCryptoDB.hpp"  # Fixed header name
)

target_include_directories(encrypted_sqlite PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib>
    $<INSTALL_INTERFACE:include>
)

if(APPLE)
  set_target_properties(encrypted_sqlite PROPERTIES
      LINK_FLAGS "-Wl,-all_load"
      OSX_ARCHITECTURES "arm64;x86_64"
  )
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -undefined dynamic_lookup")
endif()

# Export symbols for shared library
include(GenerateExportHeader)
generate_export_header(encrypted_sqlite)
target_compile_definitions(encrypted_sqlite PRIVATE ENCRYPTED_SQLITE_EXPORTS)

target_link_libraries(encrypted_sqlite PRIVATE
    cc20
    cryptosqlite  # Now properly links to shared lib chain
)

# Simplified local installation
install(TARGETS encrypted_sqlite
    LIBRARY  DESTINATION lib
    ARCHIVE  DESTINATION lib
    RUNTIME  DESTINATION bin
    PUBLIC_HEADER DESTINATION include/encrypted_sqlite
)