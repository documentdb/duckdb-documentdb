#include "documentdb/documentdb.hpp"
#include <mongoc/mongoc.h>

#include <memory>
#include <mutex>
#include <stdexcept>
#include <utility>

namespace documentdb {
namespace {

template <typename T, void (*Destroy)(T*)>
using MongoHandle = std::unique_ptr<T, decltype(Destroy)>;

void initialize_driver() {
    static std::once_flag initialized;
    std::call_once(initialized, mongoc_init);
}

MongoHandle<mongoc_uri_t, mongoc_uri_destroy> create_uri(const ConnectionConfig& config) {
    initialize_driver();
    MongoHandle<mongoc_uri_t, mongoc_uri_destroy> uri(
        mongoc_uri_new_for_host_port(config.host.c_str(), static_cast<uint16_t>(config.port)),
        mongoc_uri_destroy);
    if (!uri) {
        throw std::invalid_argument("invalid DocumentDB host or port");
    }

    if (!config.database.empty() && !mongoc_uri_set_database(uri.get(), config.database.c_str())) {
        throw std::invalid_argument("invalid DocumentDB database name");
    }
    if (!config.user.empty() && !mongoc_uri_set_username(uri.get(), config.user.c_str())) {
        throw std::invalid_argument("invalid DocumentDB username");
    }
    if (!config.password.empty() && !mongoc_uri_set_password(uri.get(), config.password.c_str())) {
        throw std::invalid_argument("invalid DocumentDB password");
    }
    if (!config.auth_source.empty() && !mongoc_uri_set_auth_source(uri.get(), config.auth_source.c_str())) {
        throw std::invalid_argument("invalid DocumentDB authentication source");
    }
    if (!mongoc_uri_set_option_as_bool(uri.get(), MONGOC_URI_TLS, config.tls) ||
        !mongoc_uri_set_option_as_bool(uri.get(), MONGOC_URI_TLSALLOWINVALIDCERTIFICATES,
                                       config.tls_allow_invalid_certificates) ||
        !mongoc_uri_set_option_as_int32(uri.get(), MONGOC_URI_SERVERSELECTIONTIMEOUTMS,
                                        config.server_selection_timeout_ms)) {
        throw std::invalid_argument("invalid DocumentDB connection option");
    }
    return uri;
}

MongoHandle<mongoc_client_t, mongoc_client_destroy> create_client(const ConnectionConfig& config) {
    auto uri = create_uri(config);
    bson_error_t error;
    MongoHandle<mongoc_client_t, mongoc_client_destroy> client(
        mongoc_client_new_from_uri_with_error(uri.get(), &error), mongoc_client_destroy);
    if (!client) {
        throw std::runtime_error("failed to create DocumentDB client: " + std::string(error.message));
    }
    mongoc_client_set_error_api(client.get(), MONGOC_ERROR_API_VERSION_2);
    return client;
}

void throw_cursor_error(mongoc_cursor_t* cursor, const std::string& operation) {
    bson_error_t error;
    if (mongoc_cursor_error(cursor, &error)) {
        throw std::runtime_error(operation + ": " + error.message);
    }
}

}  // namespace

Connection::Connection(const ConnectionConfig& config) : config_(config) {}

std::vector<std::string> Connection::list_collections() const {
    auto client = create_client(config_);
    MongoHandle<mongoc_database_t, mongoc_database_destroy> database(
        mongoc_client_get_database(client.get(), config_.database.c_str()), mongoc_database_destroy);
    MongoHandle<mongoc_cursor_t, mongoc_cursor_destroy> cursor(
        mongoc_database_find_collections_with_opts(database.get(), nullptr), mongoc_cursor_destroy);

    std::vector<std::string> collections;
    const bson_t* document;
    while (mongoc_cursor_next(cursor.get(), &document)) {
        bson_iter_t name;
        if (bson_iter_init_find(&name, document, "name") && BSON_ITER_HOLDS_UTF8(&name)) {
            uint32_t length = 0;
            const char* value = bson_iter_utf8(&name, &length);
            collections.emplace_back(value, length);
        }
    }
    throw_cursor_error(cursor.get(), "failed to list DocumentDB collections");
    return collections;
}

std::vector<Document> Connection::scan(const std::string& collection_name,
                                      const std::string& filter_json) const {
    bson_error_t error;
    MongoHandle<bson_t, bson_destroy> filter(
        bson_new_from_json(reinterpret_cast<const uint8_t*>(filter_json.data()),
                           static_cast<ssize_t>(filter_json.size()), &error),
        bson_destroy);
    if (!filter) {
        throw std::invalid_argument("filter_json must contain a valid JSON object: " + std::string(error.message));
    }

    auto client = create_client(config_);
    MongoHandle<mongoc_collection_t, mongoc_collection_destroy> collection(
        mongoc_client_get_collection(client.get(), config_.database.c_str(), collection_name.c_str()),
        mongoc_collection_destroy);
    MongoHandle<mongoc_cursor_t, mongoc_cursor_destroy> cursor(
        mongoc_collection_find_with_opts(collection.get(), filter.get(), nullptr, nullptr), mongoc_cursor_destroy);

    std::vector<Document> rows;
    const bson_t* result;
    while (mongoc_cursor_next(cursor.get(), &result)) {
        size_t json_length = 0;
        char* json = bson_as_relaxed_extended_json(result, &json_length);
        if (json == nullptr) {
            throw std::runtime_error("failed to serialize a DocumentDB result");
        }
        rows.push_back({std::string(json, json_length), collection_name});
        bson_free(json);
    }
    throw_cursor_error(cursor.get(), "failed to scan DocumentDB collection");
    return rows;
}

}  // namespace documentdb
