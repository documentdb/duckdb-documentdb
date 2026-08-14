#include "documentdb/documentdb.hpp"

#include <algorithm>
#include <cassert>
#include <string>
#include <vector>

int main() {
    documentdb::ConnectionConfig cfg;
    cfg.host = "localhost";
    cfg.port = 27017;
    cfg.database = "app";

    documentdb::Connection conn(cfg);
    auto collections = conn.list_collections();
    assert(!collections.empty());
    assert(std::find(collections.begin(), collections.end(), "orders") != collections.end());

    auto rows = conn.scan("orders", "{\"status\": \"active\"}");
    assert(!rows.empty());
    assert(rows.front().collection == "orders");

    const std::vector<std::string> sample_docs = {
        R"({"_id": 1, "status": "active", "total": 99.5})",
        R"({"_id": 2, "status": "pending", "total": 49.0})"
    };

    auto schema = documentdb::SchemaResolver::infer_from_samples(sample_docs);
    assert(schema.size() >= 3);
    assert(std::find_if(schema.begin(), schema.end(), [](const documentdb::FieldType& field) {
        return field.name == "status" && field.duckdb_type == "VARCHAR";
    }) != schema.end());

    auto plan = documentdb::ScanPlanner::plan("orders", "{\"status\": \"active\"}", {"_id", "status"}, 10);
    assert(plan.collection_name == "orders");
    assert(plan.limit == 10);
    assert(plan.projections.size() == 2);

    return 0;
}
