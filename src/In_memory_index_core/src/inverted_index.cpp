#include "in_memory_index/inverted_index.hpp"
#include "in_memory_index/document_builder.hpp"
#include <utility>

namespace in_memory_index
{

void InvertedIndex::addDocument(Document doc)
{
    DocId id = static_cast<DocId>(doc.id);

    if (documents_.find(id) != documents_.end())
    {
        removeDocument(id);
    }

    documents_.emplace(id, std::move(doc));
    const auto& words = documents_.at(id).text();
    for (const auto& word : words)
    {
        std::string lowerWord = DocumentBuilder::normalizeWord(word);
        index_[lowerWord][id]++;
    }
}

void InvertedIndex::addDocument(std::uint64_t id, const std::string& name, const std::string& text)
{
    Document doc = DocumentBuilder::buildDocument(id, name, text);
    addDocument(std::move(doc));
}

void InvertedIndex::removeDocument(DocId id)
{
    auto it = documents_.find(id);
    if (it == documents_.end())
        return;

    const auto& words = it->second.text();
    for (const auto& word : words)
    {
        std::string lowerWord = DocumentBuilder::normalizeWord(word);
        auto wordIt = index_.find(lowerWord);
        if (wordIt != index_.end())
        {
            wordIt->second.erase(id);
            if (wordIt->second.empty())
            {
                index_.erase(wordIt);
            }
        }
    }

    documents_.erase(it);
}

std::vector<DocId> InvertedIndex::search(const std::string& word) const
{
    std::string normalized = DocumentBuilder::normalizeWord(word);
    auto it = index_.find(normalized);
    if (it == index_.end())
        return {};

    std::vector<DocId> result;
    result.reserve(it->second.size());
    for (const auto& pair : it->second)
    {
        result.push_back(pair.first);
    }
    return result;
}

std::unordered_map<DocId, int> InvertedIndex::getWordOccurrences(const std::string& word) const
{
    std::string normalized = DocumentBuilder::normalizeWord(word);
    auto it = index_.find(normalized);
    if (it == index_.end())
        return {};

    return it->second;
}

size_t InvertedIndex::documentCount() const
{
    return documents_.size();
}

void InvertedIndex::clear()
{
    index_.clear();
    documents_.clear();
}

} // namespace in_memory_index