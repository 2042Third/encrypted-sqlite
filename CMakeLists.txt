cmake_minimum_required(VERSION 3.20)

project(encrypted_sqlite VERSION 0.1 LANGUAGES C CXX)

# Option to choose between static or shared library
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

set(PROJECT_FILES
    src/db/pdm_database.h
    src/db/pdm_database.cpp
    src/crypto/pdmCryptoDB.hpp
)

# Create the library
add_library(encrypted_sqlite ${PROJECT_FILES})

# Set properties for the library
set_target_properties(encrypted_sqlite PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER "src/db/pdm_db.h;src/crypto/pdmCryptoDB.hpp"
)

target_include_directories(encrypted_sqlite PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/pdm-crypt-module>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/pdm-crypt-module/src/lib>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/cryptoSQLite>
    $<INSTALL_INTERFACE:include>
)

add_subdirectory(lib/pdm-crypt-module) # pdm encryption
add_subdirectory(lib/cryptoSQLite) # encrypted database

target_link_libraries(encrypted_sqlite PRIVATE
    cc20 # pdm encryption
    cryptosqlite # encrypted database interface
)

# Installation rules
include(GNUInstallDirs)
install(TARGETS encrypted_sqlite
    EXPORT encrypted_sqliteTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/encrypted_sqlite
)

# Generate and install export file
install(EXPORT encrypted_sqliteTargets
    FILE encrypted_sqliteTargets.cmake
    NAMESPACE encrypted_sqlite::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/encrypted_sqlite
)
