#include "documentdb/documentdb.hpp"
#include "yyjson.hpp"

#include <string>
#include <unordered_map>
#include <vector>

using namespace duckdb_yyjson;

namespace documentdb {
namespace {

std::string infer_value_type(yyjson_val* value) {
    if (yyjson_is_bool(value)) {
        return "BOOLEAN";
    }
    if (yyjson_is_int(value) || yyjson_is_uint(value)) {
        return "BIGINT";
    }
    if (yyjson_is_real(value)) {
        return "DOUBLE";
    }
    if (yyjson_is_str(value) || yyjson_is_null(value)) {
        return "VARCHAR";
    }
    return "VARCHAR";
}

}  // namespace

std::vector<FieldType> SchemaResolver::infer_from_samples(const std::vector<std::string>& samples) {
    std::vector<FieldType> fields;
    std::unordered_map<std::string, std::string> inferred;

    for (const std::string& sample : samples) {
        yyjson_doc* document = yyjson_read(sample.data(), sample.size(), 0);
        if (document == nullptr || !yyjson_is_obj(yyjson_doc_get_root(document))) {
            yyjson_doc_free(document);
            continue;
        }

        size_t index, max;
        yyjson_val* key;
        yyjson_val* value;
        yyjson_obj_foreach(yyjson_doc_get_root(document), index, max, key, value) {
            std::string field_name(yyjson_get_str(key), yyjson_get_len(key));
            if (inferred.find(field_name) == inferred.end()) {
                inferred[field_name] = infer_value_type(value);
            }
        }
        yyjson_doc_free(document);
    }

    for (const auto& entry : inferred) {
        fields.push_back({entry.first, entry.second});
    }

    if (fields.empty()) {
        fields.push_back({"_id", "VARCHAR"});
    }

    return fields;
}

}  // namespace documentdb
