cmake_minimum_required(VERSION 3.5)

set(TARGET_NAME documentdb)
set(EXTENSION_NAME ${TARGET_NAME}_extension)
set(LOADABLE_EXTENSION_NAME ${TARGET_NAME}_loadable_extension)

project(${TARGET_NAME})

set(CMAKE_CXX_STANDARD "17" CACHE STRING "C++ standard to enforce")
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(mongoc CONFIG REQUIRED)

include_directories(src/include)

set(EXTENSION_SOURCES
    src/documentdb_extension.cpp
    src/documentdb_connection.cpp
    src/documentdb_schema.cpp
    src/documentdb_scan.cpp
)

build_static_extension(${TARGET_NAME} ${EXTENSION_SOURCES})
build_loadable_extension(${TARGET_NAME} " " ${EXTENSION_SOURCES})

target_link_libraries(${EXTENSION_NAME} duckdb_yyjson mongoc::static)
target_link_libraries(${LOADABLE_EXTENSION_NAME} duckdb_yyjson mongoc::static)

target_include_directories(${EXTENSION_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(${LOADABLE_EXTENSION_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
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
