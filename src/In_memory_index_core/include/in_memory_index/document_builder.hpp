#pragma once
#include "document.hpp"
#include <string>
#include <vector>

namespace in_memory_index
{
class DocumentBuilder
{
  public:
    static Document buildDocument(std::uint64_t id, const std::string& name, const std::string& words);
    static std::vector<std::string> splitWords(const std::string& text);
    static std::string normalizeWord(const std::string& word);
};
} // namespace in_memory_index
