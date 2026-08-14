# Usage

This project serves as a repository skeleton for a DocumentDB extension patterned after duckdb-mongo.

## Build

```bash
make build
```

## Run tests

```bash
make test
```

## Example API

```cpp
#include <documentdb/documentdb.hpp>

int main() {
    documentdb::ConnectionConfig cfg;
    cfg.host = "localhost";
    cfg.port = 27017;
    cfg.database = "app";

    documentdb::Connection conn(cfg);
    auto collections = conn.list_collections();
    auto rows = conn.scan("orders", "{\"status\": \"active\"}");
    return rows.empty() ? 0 : 1;
}
```

## Roadmap

1. Implement DocumentDB connection protocol support
2. Add SQL attach and scan entry points
3. Build pushdown translation for filters and aggregates
4. Add integration tests against a DocumentDB instance
