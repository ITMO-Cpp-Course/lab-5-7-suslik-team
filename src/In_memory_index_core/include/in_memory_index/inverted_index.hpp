#pragma once
#include <string>
#include <unordered_map>
#include <vector>

namespace in_memory_index
{
using DocId = int;
class InvertedIndex
{
  public:
    void addDocument(DocId id, const std::vector<std::string>& words);
    void removeDocument(DocId id);
    std::vector<DocId> search(const std::string& word) const;
    std::unordered_map<DocId, int> getWordOccurrences(const std::string& word) const;
    size_t documentCount() const;
    void clear();

  private:
    std::unordered_map<std::string, std::unordered_map<DocId, int>> index_;
    std::unordered_map<DocId, std::vector<std::string>> docWords_;
};
} // namespace in_memory_index