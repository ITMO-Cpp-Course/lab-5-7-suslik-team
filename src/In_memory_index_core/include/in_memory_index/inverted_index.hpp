#pragma once

#include "../../Index_transaction_core/include/index_transaction/result.hpp"
#include "in_memory_index/document.hpp"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace in_memory_index
{

class InvertedIndex
{
  public:
    index_transaction::Result<void> addDocument(Document doc);
    index_transaction::Result<void> addDocument(std::uint64_t id, const std::string& name, const std::string& text);
    index_transaction::Result<void> removeDocument(std::uint64_t id);
    std::vector<std::uint64_t> search(const std::string& word) const;
    std::unordered_map<std::uint64_t, int> getWordOccurrences(const std::string& word) const;
    bool hasDocument(std::uint64_t id) const noexcept;
    index_transaction::Result<Document> getDocument(std::uint64_t id) const;
    size_t documentCount() const;
    void clear() noexcept;

  private:
    std::unordered_map<std::uint64_t, Document> documents_;
    std::unordered_map<std::string, std::unordered_set<std::uint64_t>> invertedIndex_;
    std::unordered_map<std::string, std::unordered_map<std::uint64_t, int>> occurrences_;
};
} // namespace in_memory_index