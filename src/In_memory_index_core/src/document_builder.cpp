#pragma once
#include "in_memory_index/document_builder.hpp"
#include "in_memory_index/document.hpp"
#include <cctype>
#include <string>
#include <vector>

namespace in_memory_index
{

Document DocumentBuilder::buildDocument(std::uint64_t id, const std::string& name, const std::string& words)
{
    return Document(id, name, splitWords(words));
}
std::vector<std::string> DocumentBuilder::splitWords(const std::string& text)
{
    std::vector<std::string> words;
    std::string word;
    for (char ch : text)
    {
        if (std::isalnum(static_cast<unsigned char>(ch)) || ch == '_')
        {
            word.push_back(ch);
        }
        else
        {
            if (!word.empty())
            {
                words.push_back(word);
                word.clear();
            }
        }
    }
    if (!word.empty())
    {
        words.push_back(normalizeWord(word));
    }
    return words;
}
std::string DocumentBuilder::normalizeWord(const std::string& word)
{
    std::string result;
    for (char ch : word)
    {
        result.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
    }
    return result;
}
} // namespace in_memory_index