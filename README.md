# duckdb-documentdb

DocumentDB integration for DuckDB, modeled after the MongoDB-style extension architecture used in duckdb-mongo. The repository is being evolved into a proper DuckDB extension with real extension registration, loadable binary output, and a documented DocumentDB-facing API surface.

## What this repo is

This repository is a practical starting point for a DuckDB extension that targets the DocumentDB ecosystem. The implementation currently includes:

- a public connection and schema API
- a scan planner inspired by MongoDB-style pushdown design
- a real extension entry point for loading functions into DuckDB
- an extension build configuration aligned with DuckDB’s extension model

The initial extension surface is read-only. It supports collection discovery, schema inference, scans, and query pushdown, but does not provide insert, update, or delete operations.

## Key design goals

- expose DocumentDB collections through DuckDB SQL
- keep document access live and pushdown-friendly
- infer schema from sample documents for DuckDB columns
- provide extension functions that load cleanly into the DuckDB runtime
- preserve a MongoDB-like design while adapting to DocumentDB semantics

## Repository structure

- [docs/architecture.md](docs/architecture.md): architecture notes and extension layout
- [docs/usage.md](docs/usage.md): build and usage guidance
- [include/documentdb/documentdb.hpp](include/documentdb/documentdb.hpp): public connection and planner API
- [src/documentdb_extension.cpp](src/documentdb_extension.cpp): DuckDB extension entry point and function registration
- [src/documentdb_connection.cpp](src/documentdb_connection.cpp): connection handling
- [src/documentdb_schema.cpp](src/documentdb_schema.cpp): schema inference and resolution
- [src/documentdb_scan.cpp](src/documentdb_scan.cpp): scan and pushdown planning
- [tests/documentdb_smoke_test.cpp](tests/documentdb_smoke_test.cpp): smoke validation for the API layer

## Example usage

```sql
SELECT documentdb_version('documentdb') AS version;
SELECT documentdb_collections('app') AS collections;
```

## Build

```bash
git submodule update --init --recursive
make build
```

## Docker end-to-end test

The end-to-end test starts the official DocumentDB Local image, creates test data through `mongosh`, and verifies that the C++ connection layer discovers and filters the real collection through the MongoDB wire protocol:

```bash
./tests/run_documentdb_e2e.sh
```

The script generates an ephemeral password for each run and removes the test container and network when it finishes. Self-signed TLS certificates are accepted only by this local test configuration.

## Status

This repo now includes the core extension plumbing needed for a real DuckDB extension and keeps the working C++ API layer and smoke tests. The next step is to connect it to a live DocumentDB backend and expand the SQL attach and scan semantics beyond the current scaffold.

## Relationship to the base project

This repo uses the duckdb-mongo extension as the architectural reference and adapts the same core ideas to DocumentDB:

- SQL attach semantics
- document-to-column mapping
- schema inference
- pushdown-oriented scan planning
- live collection access through DuckDB

The adaptation is targeted to DocumentDB rather than MongoDB while preserving the same developer experience where possible.
