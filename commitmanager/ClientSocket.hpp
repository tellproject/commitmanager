#pragma once

#include <commitmanager/ErrorCode.hpp>
#include <commitmanager/MessageTypes.hpp>
#include <commitmanager/SnapshotDescriptor.hpp>

#include <crossbow/infinio/RpcClient.hpp>
#include <crossbow/string.hpp>

#include <cstdint>
#include <system_error>

namespace tell {
namespace commitmanager {

/**
 * @brief Response for a Start-Transaction request
 */
class StartResponse final
        : public crossbow::infinio::RpcResponseResult<StartResponse, std::unique_ptr<SnapshotDescriptor>> {
    using Base = crossbow::infinio::RpcResponseResult<StartResponse, std::unique_ptr<SnapshotDescriptor>>;

public:
    using Base::Base;

private:
    friend Base;

    static constexpr ResponseType MessageType = ResponseType::START;

    static const std::error_category& errorCategory() {
        return error::get_error_category();
    }

    void processResponse(crossbow::buffer_reader& message);
};

/**
 * @brief Response for a Commit-Transaction request
 */
class CommitResponse final : public crossbow::infinio::RpcResponseResult<CommitResponse, bool> {
    using Base = crossbow::infinio::RpcResponseResult<CommitResponse, bool>;

public:
    using Base::Base;

private:
    friend Base;

    static constexpr ResponseType MessageType = ResponseType::COMMIT;

    static const std::error_category& errorCategory() {
        return error::get_error_category();
    }

    void processResponse(crossbow::buffer_reader& message);
};

/**
 * @brief Handles communication with one CommitManager server
 *
 * Sends RPC requests and returns the pending response.
 */
class ClientSocket final : public crossbow::infinio::RpcClientSocket {
    using Base = crossbow::infinio::RpcClientSocket;

public:
    using Base::Base;

    void connect(const crossbow::infinio::Endpoint& host);

    void shutdown();

    std::shared_ptr<StartResponse> startTransaction(crossbow::infinio::Fiber& fiber, bool readonly);

    std::shared_ptr<CommitResponse> commitTransaction(crossbow::infinio::Fiber& fiber, uint64_t version);
};

} // namespace commitmanager
} // namespace tell
