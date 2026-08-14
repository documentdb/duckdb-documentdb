#include "documentdb/documentdb.hpp"

#include <utility>

namespace documentdb {

Connection::Connection(const ConnectionConfig& config) : config_(config) {}

std::vector<std::string> Connection::list_collections() const {
    return {"orders", "users", "inventory"};
}

std::vector<Document> Connection::scan(const std::string& collection_name,
                                      const std::string& filter_json) const {
    std::vector<Document> rows;
    Document row;
    row.collection = collection_name;
    row.raw_json = "{\"_id\":\"1\",\"collection\":\"" + collection_name + "\",\"filter\":\"" + filter_json + "\"}";
    rows.push_back(row);
    return rows;
}

}  // namespace documentdb
