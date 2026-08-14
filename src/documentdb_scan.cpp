#include "documentdb/documentdb.hpp"

#include <string>
#include <vector>

namespace documentdb {

ScanPlan ScanPlanner::plan(const std::string& collection_name,
                          const std::string& filter,
                          const std::vector<std::string>& selected_columns,
                          int limit) {
    ScanPlan plan;
    plan.collection_name = collection_name;
    plan.filter = filter;
    plan.projections = selected_columns;
    plan.has_limit = limit > 0;
    plan.limit = limit;
    return plan;
}

}  // namespace documentdb
