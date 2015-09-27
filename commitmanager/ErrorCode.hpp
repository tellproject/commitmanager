/*
 * (C) Copyright 2015 ETH Zurich Systems Group (http://www.systems.ethz.ch/) and others.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Contributors:
 *     Markus Pilman <mpilman@inf.ethz.ch>
 *     Simon Loesing <sloesing@inf.ethz.ch>
 *     Thomas Etter <etterth@gmail.com>
 *     Kevin Bocksrocker <kevin.bocksrocker@gmail.com>
 *     Lucas Braun <braunl@inf.ethz.ch>
 */
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
