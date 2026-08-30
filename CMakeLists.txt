cmake_minimum_required(VERSION 3.5)

set(TARGET_NAME documentdb)
set(EXTENSION_NAME ${TARGET_NAME}_extension)
set(LOADABLE_EXTENSION_NAME ${TARGET_NAME}_loadable_extension)

project(${TARGET_NAME})

set(CMAKE_CXX_STANDARD "17" CACHE STRING "C++ standard to enforce")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(COMMAND set_extension_version_sources)
  set_extension_version_sources(${TARGET_NAME} src/documentdb_extension.cpp)
endif()

find_package(mongoc CONFIG QUIET)
if(NOT mongoc_FOUND)
  find_package(mongoc-1.0 CONFIG QUIET)
endif()
if(NOT mongoc_FOUND AND NOT mongoc-1.0_FOUND)
  find_package(mongo-c-driver CONFIG REQUIRED)
endif()

set(MONGOC_TARGET "")
if(TARGET mongoc::mongoc_static)
  set(MONGOC_TARGET mongoc::mongoc_static)
elseif(TARGET mongoc::mongoc)
  set(MONGOC_TARGET mongoc::mongoc)
elseif(TARGET mongo::mongoc_static)
  set(MONGOC_TARGET mongo::mongoc_static)
elseif(TARGET mongo::mongoc_shared)
  set(MONGOC_TARGET mongo::mongoc_shared)
elseif(TARGET mongo::mongoc)
  set(MONGOC_TARGET mongo::mongoc)
else()
  message(FATAL_ERROR "MongoDB C driver package was found, but no usable mongoc target could be resolved.")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src/include)

set(EXTENSION_SOURCES
    src/documentdb_extension.cpp
    src/documentdb_connection.cpp
    src/documentdb_schema.cpp
    src/documentdb_scan.cpp
)

build_static_extension(${TARGET_NAME} ${EXTENSION_SOURCES})
build_loadable_extension(${TARGET_NAME} " " ${EXTENSION_SOURCES})

target_link_libraries(${EXTENSION_NAME} duckdb_yyjson ${MONGOC_TARGET})
target_link_libraries(${LOADABLE_EXTENSION_NAME} duckdb_yyjson ${MONGOC_TARGET})

target_include_directories(${EXTENSION_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src/include
)

target_include_directories(${LOADABLE_EXTENSION_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src/include
)

include(CTest)
if(BUILD_TESTING)
  add_executable(documentdb_smoke_test tests/documentdb_smoke_test.cpp)
  target_link_libraries(documentdb_smoke_test PRIVATE ${EXTENSION_NAME})
  add_test(NAME documentdb_smoke_test COMMAND documentdb_smoke_test)
endif()

install(
  TARGETS ${EXTENSION_NAME}
  EXPORT "${DUCKDB_EXPORT_SET}"
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
  ARCHIVE DESTINATION "${INSTALL_LIB_DIR}"
)

install(
  TARGETS ${LOADABLE_EXTENSION_NAME}
  EXPORT "${DUCKDB_EXPORT_SET}"
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
  ARCHIVE DESTINATION "${INSTALL_LIB_DIR}"
)
