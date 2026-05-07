#pragma once

#include <expected>
#include <string>
#include <utility>

namespace index_transaction
{
template <typename T> class Result
{
  public:
    static Result<T> ok(const T& value)
    {
        return Result(value);
    }
    static Result<T> ok(T&& value)
    {
        return Result(std::move(value));
    }
    static Result<T> err(const std::string& error)
    {
        return Result(std::unexpected(error));
    }
    static Result<T> err(std::string&& error)
    {
        return Result(std::unexpected(std::move(error)));
    }

    bool has_value() const noexcept
    {
        return data_.has_value();
    }
    bool has_error() const noexcept
    {
        return !has_value();
    }

    T& value()
    {
        return data_.value();
    }
    const T& value() const
    {
        return data_.value();
    }

    std::string error() const
    {
        return data_.error();
    }

    explicit operator bool() const noexcept
    {
        return has_value();
    }

  private:
    std::expected<T, std::string> data_;
    Result(const T& value) : data_(value) {}
    Result(T&& value) : data_(std::move(value)) {}
    Result(std::unexpected<std::string> error) : data_(std::move(error)) {}
};
} // namespace index_transaction