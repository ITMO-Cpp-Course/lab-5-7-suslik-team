#include "in_memory_index/inverted_index.hpp"
#include "in_memory_index/document_builder.hpp"
#include "index_transaction/result.hpp"   

namespace in_memory_index
{
using index_transaction::Result;
using index_transaction::ErrorCode;

Result<void> InvertedIndex::addDocument(Document doc)
{
    if (documents_.find(doc.id) != documents_.end())
    {
        return std::unexpected(ErrorCode::DuplicateDocument);
    }
    documents_[doc.id] = std::move(doc);
    for (const auto& word : documents_[doc.id].text())
    {
        std::string norm = DocumentBuilder::normalizeWord(word);
        invertedIndex_[norm].insert(doc.id);
        occurrences_[norm][doc.id]++;
    }
    return {};
}

Result<void> InvertedIndex::addDocument(std::uint64_t id, const std::string& name, const std::string& text)
{
    Document doc = DocumentBuilder::buildDocument(id, name, text);
    return addDocument(std::move(doc));
}

Result<void> InvertedIndex::removeDocument(std::uint64_t id)
{
    auto it = documents_.find(id);
    if (it == documents_.end())
    {
        return std::unexpected(ErrorCode::DocumentNotFound);
    }
    for (const auto& word : it->second.text())
    {
        std::string norm = DocumentBuilder::normalizeWord(word);
        auto wordIt = invertedIndex_.find(norm);
        if (wordIt != invertedIndex_.end())
        {
            wordIt->second.erase(id);
            if (wordIt->second.empty())
                invertedIndex_.erase(wordIt);
        }
        auto occIt = occurrences_.find(norm);
        if (occIt != occurrences_.end())
        {
            occIt->second.erase(id);
            if (occIt->second.empty())
                occurrences_.erase(occIt);
        }
    }
    documents_.erase(it);
    return {};
}

std::vector<std::uint64_t> InvertedIndex::search(const std::string& word) const
{
    std::string normalized = DocumentBuilder::normalizeWord(word);
    auto it = invertedIndex_.find(normalized);
    if (it == invertedIndex_.end())
        return {};
    return std::vector<std::uint64_t>(it->second.begin(), it->second.end());
}

std::unordered_map<std::uint64_t, int> InvertedIndex::getWordOccurrences(const std::string& word) const
{
    std::string normalized = DocumentBuilder::normalizeWord(word);
    auto it = occurrences_.find(normalized);
    if (it == occurrences_.end())
        return {};
    return it->second;
}

size_t InvertedIndex::documentCount() const
{
    return documents_.size();
}

bool InvertedIndex::hasDocument(std::uint64_t id) const noexcept
{
    return documents_.find(id) != documents_.end();
}

Result<Document> InvertedIndex::getDocument(std::uint64_t id) const
{
    auto it = documents_.find(id);
    if (it == documents_.end())
    {
        return std::unexpected(ErrorCode::DocumentNotFound);
    }
    return Result<Document>(it->second);
}

void InvertedIndex::clear() noexcept
{
    occurrences_.clear();
    invertedIndex_.clear();
    documents_.clear();
}

} // namespace in_memory_index