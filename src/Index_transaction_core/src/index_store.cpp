#include "index_transaction/index_store.hpp"
#include "in_memory_index/document_builder.hpp"
#include "index_transaction/update_transaction.hpp"
#include <expected>

namespace in_memory_index
{
using index_transaction::ErrorCode;
using index_transaction::Result;
using index_transaction::UpdateTransaction;

Result<void> IndexStore::addDocument(const Document& doc)
{
    return index_.addDocument(doc);
}

Result<void> IndexStore::addDocument(std::uint64_t id, const std::string& name, const std::string& text)
{
    return index_.addDocument(id, name, text);
}

Result<void> IndexStore::removeDocument(std::uint64_t id)
{
    return index_.removeDocument(id);
}

Result<Document> IndexStore::getDocument(std::uint64_t id) const
{
    return index_.getDocument(id);
}

Result<std::vector<Document>> IndexStore::search(const std::string& word) const
{
    std::vector<std::uint64_t> ids = index_.search(word);
    std::vector<Document> docs;
    docs.reserve(ids.size());
    for (std::uint64_t id : ids)
    {
        auto docRes = index_.getDocument(id);
        if (!docRes)
        {
            return std::unexpected(ErrorCode::InternalError);
        }
        docs.push_back(std::move(docRes.value()));
    }
    return docs;
}

Result<std::unordered_map<std::uint64_t, int>> IndexStore::getWordOccurrences(const std::string& word) const
{
    return index_.getWordOccurrences(word);
}

size_t IndexStore::documentCount() const noexcept
{
    return index_.documentCount();
}

void IndexStore::clear() noexcept
{
    index_.clear();
}

Result<UpdateTransaction> IndexStore::beginTransaction()
{
    return UpdateTransaction(*this);
}

} // namespace in_memory_index