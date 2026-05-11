#pragma once

#include "in_memory_index/document.hpp"
#include "index_transaction/result.hpp"

namespace in_memory_index {
    class IndexStore;
}

namespace index_transaction {

class UpdateTransaction {
public:
    explicit UpdateTransaction(in_memory_index::IndexStore& store);
    ~UpdateTransaction();
    
    UpdateTransaction(const UpdateTransaction&) = delete;
    UpdateTransaction& operator=(const UpdateTransaction&) = delete;
    
    Result<void> addDocument(const in_memory_index::Document& doc);
    Result<void> addDocument(std::uint64_t id, const std::string& name, const std::string& text);
    Result<void> removeDocument(std::uint64_t id);
    
    Result<void> commit();
    
private:
    in_memory_index::IndexStore& store_;
    bool committed_ = false;

    struct RollbackOperation {
        enum class Type { ADD, REMOVE } type;
        std::uint64_t id;
        in_memory_index::Document document;
        bool isPlaceholder = false;
    };
    
    std::vector<RollbackOperation> rollbackLog_;
    
    void rollback();
};

} // namespace index_transaction