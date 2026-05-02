#include "in_memory_index/inverted_index.hpp"
#include "in_memory_index/document_builder.hpp"

namespace in_memory_index
{

void InvertedIndex::addDocument(DocId id, const std::vector<std::string>& words)
{
    if (docWords_.find(id) != docWords_.end())
    {
        removeDocument(id);
    }
    docWords_[id] = words;
    for (const auto& word : words)
    {
        std::string lowerWord = DocumentBuilder::normalizeWord(word);
        index_[lowerWord][id]++;
    }
}

void InvertedIndex::addDocument(const Document& doc)
{
    addDocument(doc.id, doc.content);
}

void InvertedIndex::addDocument(DocId id, const std::string& name, const std::string& text)
{
    auto doc = DocumentBuilder::buildDocument(id, name, text);
    addDocument(doc);
}

void InvertedIndex::removeDocument(DocId id)
{
    if (docWords_.find(id) != docWords_.end())
    {
        auto it = docWords_.find(id);
        const auto& words = it->second;
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
        docWords_.erase(it);
    }
}

std::vector<DocId> InvertedIndex::search(const std::string& word) const
{
    std::string normalized = DocumentBuilder::normalizeWord(word);
    auto it = index_.find(normalized);
    if (it == index_.end())
    {
        return std::vector<DocId>();
    }
    std::vector<DocId> result;
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
    {
        return std::unordered_map<DocId, int>();
    }
    return it->second;
}

size_t InvertedIndex::documentCount() const
{
    return docWords_.size();
}

void InvertedIndex::clear()
{
    index_.clear();
    docWords_.clear();
}

} // namespace in_memory_index