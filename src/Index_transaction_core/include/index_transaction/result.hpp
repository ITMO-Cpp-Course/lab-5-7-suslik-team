#pragma once

#include <expected>
#include <string>

namespace index_transaction {

enum class ErrorCode {
    None = 0,
    InvalidDocument,
    DuplicateDocument,
    DocumentNotFound,
    InvalidWord,
    InternalError,
};

inline std::string errorCodeToString(ErrorCode code) {
    switch (code) {
        case ErrorCode::InvalidDocument: return "Invalid document";
        case ErrorCode::DuplicateDocument: return "Document already exists";
        case ErrorCode::DocumentNotFound: return "Document not found";
        case ErrorCode::InvalidWord: return "Invalid word in document";
        case ErrorCode::InternalError: return "Internal error";
        default: return "Unknown error";
    }
}

template<typename T>
struct ResultImpl {
    using type = std::expected<T, ErrorCode>;
};

template<>
struct ResultImpl<void> {
    using type = std::expected<void, ErrorCode>;
};

template<typename T>
using Result = typename ResultImpl<T>::type;

} // namespace index_transaction