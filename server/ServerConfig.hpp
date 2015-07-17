#pragma once

#include <cstddef>
#include <cstdint>

namespace tell {
namespace commitmanager {

/**
 * @brief The ServerConfig struct containing configuration parameters for the TellStore server
 */
struct ServerConfig {
    /// Port to listen for incoming client connections
    uint16_t port = 7242;
};

} // namespace commitmanager
} // namespace tell
