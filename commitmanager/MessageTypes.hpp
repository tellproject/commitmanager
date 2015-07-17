#pragma once

#include <cstdint>

namespace tell {
namespace commitmanager {

/**
 * @brief The possible messages types of a request
 */
enum class RequestType : uint32_t {
    START = 0x1u,
    COMMIT,
};

/**
 * @brief The possible messages types of a response
 */
enum class ResponseType : uint32_t {
    START = 0x1u,
    COMMIT,
};

} // namespace commitmanager
} // namespace tell
