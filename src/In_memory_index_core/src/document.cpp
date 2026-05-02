#include "in_memory_index/document.hpp"

namespace in_memory_index
{

Document::Document(std::uint64_t doc_id, std::string doc_name, std::vector<std::string> doc_content)
    : id(doc_id), name(std::move(doc_name)), content(std::move(doc_content))
{
}

Document::Document(Document&& other) noexcept
    : id(other.id), name(std::move(other.name)), content(std::move(other.content))
{
}

Document& Document::operator=(Document&& other) noexcept
{
    if (this != &other)
    {
        id = other.id;
        name = std::move(other.name);
        content = std::move(other.content);
    }
    return *this;
}

std::vector<std::string> Document::text() const noexcept
{
    return content;
}

} // namespace in_memory_index