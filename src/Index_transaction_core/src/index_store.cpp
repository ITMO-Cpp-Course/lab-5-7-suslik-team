#pragma once

#include "index_transaction/index_store.hpp"
#include "in_memory_index/document_builder.hpp"

namespace in_memory_index
{

using index_transaction::Result;

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

Result<std::vector<Document>> IndexStore::search(const std::string& word) const
{
    std::vector<std::uint64_t> ids = index_.search(word);
    std::vector<Document> docs;
    docs.reserve(ids.size());
    for (std::uint64_t id : ids)
    {
        auto docRes = index_.getDocument(id);
        if (!docRes.has_value())
        {
            return Result<std::vector<Document>>::err("Document id " + std::to_string(id) + " not found in storage");
        }
        docs.push_back(std::move(docRes.value()));
    }
    return Result<std::vector<Document>>::ok(std::move(docs));
}

Result<std::unordered_map<std::uint64_t, int>> IndexStore::getWordOccurrences(const std::string& word) const
{
    return Result<std::unordered_map<std::uint64_t, int>>::ok(index_.getWordOccurrences(word));
}

size_t IndexStore::documentCount() const noexcept
{
    return index_.documentCount();
}

void IndexStore::clear() noexcept
{
    index_.clear();
}
} // namespace in_memory_index