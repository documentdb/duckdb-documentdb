#define DUCKDB_EXTENSION_MAIN

#include "documentdb_extension.hpp"

#include "duckdb.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/function/scalar_function.hpp"
#include <duckdb/parser/parsed_data/create_scalar_function_info.hpp>

namespace duckdb {

inline void DocumentDBVersionScalarFun(DataChunk &args, ExpressionState &state, Vector &result) {
    auto &name_vector = args.data[0];
    UnaryExecutor::Execute<string_t, string_t>(name_vector, result, args.size(), [&](string_t name) {
        auto db_name = name.GetString();
        // TODO: Replace the scaffold version with the extension build version.
        return StringVector::AddString(result, "documentdb:" + db_name + " v0.1.0");
    });
}

inline void DocumentDBCollectionsScalarFun(DataChunk &args, ExpressionState &state, Vector &result) {
    auto &name_vector = args.data[0];
    UnaryExecutor::Execute<string_t, string_t>(name_vector, result, args.size(), [&](string_t name) {
        (void)name;
        // TODO: Replace the scaffold collection list with live DocumentDB discovery.
        return StringVector::AddString(result, R"(["orders","users","inventory"])"
        );
    });
}

static void LoadInternal(ExtensionLoader &loader) {
    auto version_function = ScalarFunction(
        "documentdb_version",
        {LogicalType::VARCHAR},
        LogicalType::VARCHAR,
        DocumentDBVersionScalarFun);
    loader.RegisterFunction(version_function);

    auto collections_function = ScalarFunction(
        "documentdb_collections",
        {LogicalType::VARCHAR},
        LogicalType::VARCHAR,
        DocumentDBCollectionsScalarFun);
    loader.RegisterFunction(collections_function);
}

void DocumentdbExtension::Load(ExtensionLoader &loader) {
    LoadInternal(loader);
}

std::string DocumentdbExtension::Name() {
    return "documentdb";
}

std::string DocumentdbExtension::Version() const {
#ifdef EXT_VERSION_DOCUMENTDB
    return EXT_VERSION_DOCUMENTDB;
#else
    return "0.1.0";
#endif
}

} // namespace duckdb

extern "C" {

DUCKDB_CPP_EXTENSION_ENTRY(documentdb, loader) {
    duckdb::LoadInternal(loader);
}

}
