#pragma once

#include <cstdint>
#include <system_error>
#include <type_traits>

namespace tell {
namespace commitmanager {
namespace error {

/**
 * @brief CommitManager errors triggered while executing an operation
 */
enum errors {
    /// Server received an unknown request type.
    unkown_request = 1,

    /// Server reached the maximum limit of transactions.
    transaction_limit_reached,
};

/**
 * @brief Category for server errors
 */
class error_category : public std::error_category {
public:
    const char* name() const noexcept {
        return "tell.commitmanager";
    }

    std::string message(int value) const {
        switch (value) {
        case error::unkown_request:
            return "Server received an unknown request type";

        case error::transaction_limit_reached:
            return "Server reached the maximum limit of transactions";

        default:
            return "tell.commitmanager error";
        }
    }
};

inline const std::error_category& get_error_category() {
    static error_category instance;
    return instance;
}

inline std::error_code make_error_code(error::errors e) {
    return std::error_code(static_cast<int>(e), get_error_category());
}

} // namespace error
} // namespace commitmanager
} // namespace tell

namespace std {

template<>
struct is_error_code_enum<tell::commitmanager::error::errors> : public std::true_type {
};

} // namespace std
