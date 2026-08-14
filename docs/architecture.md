# Architecture

This repository is deliberately structured as a reusable starting point for a DuckDB integration layer for DocumentDB.

## Goals

- Expose MongoDB-compatible collections through SQL
- Keep data in DocumentDB while querying through DuckDB
- Push down filters, projections, and aggregates when supported
- Keep the extension design compatible with the duckdb-mongo architecture

## Core components

### 1. Connection layer

The connection layer handles endpoint configuration, authentication, and database selection.

### 2. Schema inference

Schema inference samples documents and converts BSON-like fields into DuckDB-friendly logical types. This follows the same pattern used in the duckdb-mongo repo and is extended to DocumentDB semantics.

### 3. Scan layer

The scan layer builds a query plan and carries the DocumentDB collection read path. It is responsible for generating a logical scan, mapping document fields, and sending pushdown operations.

### 4. Pushdown planner

This planner converts SQL predicates into DocumentDB-native operations such as filter, projection, and aggregate stages. It is intentionally modeled after the duckdb-mongo pushdown strategy.

## Planned SQL surface

```sql
ATTACH 'host=localhost port=27017 dbname=app' AS documentdb (TYPE DOCUMENTDB);
SELECT * FROM documentdb.app.orders LIMIT 10;
SELECT status, COUNT(*) FROM documentdb.app.orders GROUP BY status;
```

## Reference base

This repo borrows architecture and design ideas from the duckdb-mongo extension and re-targets them for DocumentDB.
