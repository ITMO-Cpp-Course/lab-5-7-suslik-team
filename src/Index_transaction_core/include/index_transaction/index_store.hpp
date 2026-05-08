#pragma once

#include "in_memory_index/inverted_index.hpp"
#include "index_transaction/result.hpp"
#include "index_transaction/update_transaction.hpp"
#include <string>
#include <vector>

namespace in_memory_index
{

class IndexStore
{
  public:
    IndexStore() = default;

    index_transaction::Result<void> addDocument(const Document& doc);
    index_transaction::Result<void> addDocument(std::uint64_t id, const std::string& name, const std::string& text);
    index_transaction::Result<void> removeDocument(std::uint64_t id);

    index_transaction::Result<std::vector<Document>> search(const std::string& word) const;
    index_transaction::Result<std::unordered_map<std::uint64_t, int>> getWordOccurrences(const std::string& word) const;

    size_t documentCount() const noexcept;
    void clear() noexcept;

  private:
    InvertedIndex index_;
};

} // namespace in_memory_index