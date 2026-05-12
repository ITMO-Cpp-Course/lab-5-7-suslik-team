#include "index_transaction/update_transaction.hpp"
#include "in_memory_index/document_builder.hpp"
#include "index_transaction/index_store.hpp"
#include <algorithm>

namespace index_transaction
{

UpdateTransaction::UpdateTransaction(in_memory_index::IndexStore& store) : store_(store), committed_(false) {}

UpdateTransaction::~UpdateTransaction()
{
    if (!committed_)
    {
        rollback();
    }
}

Result<void> UpdateTransaction::addDocument(const in_memory_index::Document& doc)
{
    auto result = store_.addDocument(doc);
    if (result.has_value())
    {
        rollbackLog_.push_back({RollbackOperation::Type::REMOVE, doc.id, doc, false});
    }
    return result;
}

Result<void> UpdateTransaction::addDocument(std::uint64_t id, const std::string& name, const std::string& text)
{
    auto doc = in_memory_index::DocumentBuilder::buildDocument(id, name, text);
    auto result = store_.addDocument(doc);
    if (result.has_value())
    {
        rollbackLog_.push_back({RollbackOperation::Type::REMOVE, id, doc, false});
    }
    return result;
}

Result<void> UpdateTransaction::removeDocument(std::uint64_t id)
{
    auto result = store_.removeDocument(id);
    if (result.has_value())
    {
        in_memory_index::Document placeholderDoc;
        rollbackLog_.push_back({RollbackOperation::Type::ADD, id, placeholderDoc, true});
    }
    return result;
}

Result<void> UpdateTransaction::commit()
{
    committed_ = true;
    rollbackLog_.clear();
    return Result<void>::ok();
}

void UpdateTransaction::rollback()
{
    for (auto it = rollbackLog_.rbegin(); it != rollbackLog_.rend(); ++it)
    {
        if (it->type == RollbackOperation::Type::ADD)
        {
            store_.addDocument(it->document);
        }
        else if (it->type == RollbackOperation::Type::REMOVE)
        {
            store_.removeDocument(it->id);
        }
    }
    rollbackLog_.clear();
}

} // namespace index_transaction