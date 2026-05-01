#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace in_memory_index
{

struct Document
{
    std::uint64_t id = 0;
    std::string name;
    std::vector<std::string> content;

    Document() = default;
    Document(std::uint64_t id, std::string name, std::vector<std::string> content);

    Document(const Document& other) = default;
    Document(Document&& other) noexcept;
    Document& operator=(const Document& other) = default;
    Document& operator=(Document&& other) noexcept;

    std::vector<std::string> text() const noexcept;
};

}