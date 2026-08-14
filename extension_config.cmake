# This file is included by DuckDB's extension build system.
# It tells the build which extension to compile for the repo.

duckdb_extension_load(documentdb
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}
)
