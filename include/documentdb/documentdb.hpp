#pragma once

#include <string>
#include <vector>

namespace documentdb {

struct ConnectionConfig {
    std::string host = "localhost";
    int port = 27017;
    std::string database;
    std::string user;
    std::string password;
    std::string auth_source;
    bool tls = false;
    bool tls_allow_invalid_certificates = false;
    int server_selection_timeout_ms = 5000;
};

struct Document {
    std::string raw_json;
    std::string collection;
};

struct FieldType {
    std::string name;
    std::string duckdb_type;
};

struct ScanPlan {
    std::string collection_name;
    std::string filter;
    std::vector<std::string> projections;
    bool has_limit = false;
    int limit = 0;
};

class Connection {
public:
    explicit Connection(const ConnectionConfig& config);

    std::vector<std::string> list_collections() const;
    std::vector<Document> scan(const std::string& collection_name,
                              const std::string& filter_json = "{}") const;

private:
    ConnectionConfig config_;
};

class SchemaResolver {
public:
    static std::vector<FieldType> infer_from_samples(const std::vector<std::string>& samples);
};

class ScanPlanner {
public:
    static ScanPlan plan(const std::string& collection_name,
                        const std::string& filter,
                        const std::vector<std::string>& selected_columns,
                        int limit);
};

}  // namespace documentdb
