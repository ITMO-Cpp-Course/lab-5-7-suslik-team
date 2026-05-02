#pragma once

#include "in_memory_index/document.hpp"
#include "in_memory_index/document_builder.hpp"
#include <string>
#include <unordered_map>
#include <vector>

namespace in_memory_index
{
using DocId = int;

class InvertedIndex
{
public:
    void addDocument(Document doc);

    void addDocument(std::uint64_t id, const std::string& name, const std::string& text);

    void removeDocument(DocId id);
    std::vector<DocId> search(const std::string& word) const;
    std::unordered_map<DocId, int> getWordOccurrences(const std::string& word) const;
    size_t documentCount() const;
    void clear();

private:
    std::unordered_map<std::string, std::unordered_map<DocId, int>> index_;
    std::unordered_map<DocId, Document> documents_;
};
} // namespace in_memory_index