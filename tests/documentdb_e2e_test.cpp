#include "documentdb/documentdb.hpp"
#include "yyjson.hpp"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <stdexcept>
#include <string>

using namespace duckdb_yyjson;

namespace {

std::string required_environment_variable(const char* name) {
    const char* value = std::getenv(name);
    if (value == nullptr || *value == '\0') {
        throw std::runtime_error(std::string("missing environment variable: ") + name);
    }
    return value;
}

}  // namespace

int main() {
    documentdb::ConnectionConfig config;
    config.host = required_environment_variable("DOCUMENTDB_HOST");
    config.port = 10260;
    config.database = "duckdb_e2e";
    config.user = required_environment_variable("DOCUMENTDB_USER");
    config.password = required_environment_variable("DOCUMENTDB_PASSWORD");
    config.auth_source = "admin";
    config.tls = true;
    config.tls_allow_invalid_certificates = true;
    config.server_selection_timeout_ms = 15000;

    documentdb::Connection connection(config);
    const std::string collection_name = "orders\"archive";
    auto collections = connection.list_collections();
    assert(std::find(collections.begin(), collections.end(), collection_name) != collections.end());

    auto rows = connection.scan(collection_name, R"({"status":"active"})");
    assert(rows.size() == 1);
    assert(rows.front().collection == collection_name);

    yyjson_doc* document = yyjson_read(rows.front().raw_json.data(), rows.front().raw_json.size(), 0);
    assert(document != nullptr);
    yyjson_val* root = yyjson_doc_get_root(document);
    assert(std::string(yyjson_get_str(yyjson_obj_get(root, "status"))) == "active");
    assert(yyjson_get_int(yyjson_obj_get(root, "total")) == 99);
    yyjson_doc_free(document);
    return 0;
}