#include "documentdb/documentdb.hpp"

#include <cctype>
#include <string>
#include <unordered_map>
#include <vector>

namespace documentdb {
namespace {

std::string trim(const std::string& value) {
    size_t start = 0;
    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start])) != 0) {
        ++start;
    }
    size_t end = value.size();
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1])) != 0) {
        --end;
    }
    return value.substr(start, end - start);
}

std::string infer_value_type(const std::string& value) {
    std::string trimmed = trim(value);
    if (trimmed.empty()) {
        return "VARCHAR";
    }
    if (trimmed.front() == '"') {
        return "VARCHAR";
    }
    if (trimmed == "true" || trimmed == "false") {
        return "BOOLEAN";
    }
    if (trimmed == "null") {
        return "VARCHAR";
    }
    bool has_dot = trimmed.find('.') != std::string::npos;
    bool is_number = true;
    for (char ch : trimmed) {
        if ((ch < '0' || ch > '9') && ch != '-' && ch != '+' && ch != '.') {
            is_number = false;
            break;
        }
    }
    if (is_number) {
        return has_dot ? "DOUBLE" : "BIGINT";
    }
    return "VARCHAR";
}

}  // namespace

std::vector<FieldType> SchemaResolver::infer_from_samples(const std::vector<std::string>& samples) {
    std::vector<FieldType> fields;
    std::unordered_map<std::string, std::string> inferred;

    for (const std::string& sample : samples) {
        std::string text = sample;
        size_t cursor = 0;
        while (cursor < text.size()) {
            size_t key_start = text.find('"', cursor);
            if (key_start == std::string::npos) {
                break;
            }
            size_t key_end = text.find('"', key_start + 1);
            if (key_end == std::string::npos) {
                break;
            }
            std::string key = text.substr(key_start + 1, key_end - key_start - 1);
            size_t colon = text.find(':', key_end + 1);
            if (colon == std::string::npos) {
                break;
            }
            size_t value_start = text.find_first_not_of(" \t\r\n", colon + 1);
            if (value_start == std::string::npos) {
                break;
            }
            size_t value_end = value_start;
            while (value_end < text.size()) {
                char ch = text[value_end];
                if (ch == ',' || ch == '}') {
                    break;
                }
                ++value_end;
            }
            std::string value = text.substr(value_start, value_end - value_start);
            std::string normalized = trim(value);
            if (inferred.find(key) == inferred.end()) {
                inferred[key] = infer_value_type(normalized);
            }
            cursor = value_end;
        }
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
