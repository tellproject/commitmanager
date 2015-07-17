#include "ServerSocket.hpp"

#include <commitmanager/ErrorCode.hpp>
#include <commitmanager/MessageTypes.hpp>

#include <crossbow/enum_underlying.hpp>
#include <crossbow/logger.hpp>

#include <chrono>

namespace tell {
namespace commitmanager {

void ServerSocket::onRequest(crossbow::infinio::MessageId messageId, uint32_t messageType,
        crossbow::infinio::BufferReader& message) {
    manager().onMessage(this, messageId, messageType, message);
}

ServerManager::ServerManager(crossbow::infinio::InfinibandService& service, const ServerConfig& config)
        : Base(service, config.port),
          mProcessor(service.createProcessor()),
          mLowestActiveVersion(0x1u) {
}

ServerSocket* ServerManager::createConnection(crossbow::infinio::InfinibandSocket socket,
        const crossbow::string& data) {
    return new ServerSocket(*this, *mProcessor, std::move(socket));
}

void ServerManager::onMessage(ServerSocket* con, crossbow::infinio::MessageId messageId, uint32_t messageType,
        crossbow::infinio::BufferReader& message) {
    LOG_TRACE("MID %1%] Handling request of type %2%", messageId.userId(), messageType);
    auto startTime = std::chrono::steady_clock::now();

    switch (messageType) {

    case crossbow::to_underlying(RequestType::START): {
        handleStartTransaction(con, messageId, message);
    } break;

    case crossbow::to_underlying(RequestType::COMMIT): {
        handleCommitTransaction(con, messageId, message);
    } break;

    default: {
        con->writeErrorResponse(messageId, error::unkown_request);
    } break;
    }

    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime);
    LOG_TRACE("MID %1%] Handling request took %2%ns", messageId.userId(), duration.count());
}

void ServerManager::handleStartTransaction(ServerSocket* con, crossbow::infinio::MessageId messageId,
        crossbow::infinio::BufferReader& /* message */) {
    auto version = mDescriptor.startTransaction();
    if (version == 0x0u) {
        con->writeErrorResponse(messageId, error::transaction_limit_reached);
        return;
    }
    mReaders.emplace(version, mDescriptor.baseVersion());

    auto descLen = mDescriptor.serializedLength();
    uint32_t messageLength = (4 * sizeof(uint64_t)) + descLen;
    con->writeResponse(messageId, ResponseType::START, messageLength, [this, version]
            (crossbow::infinio::BufferWriter& message, std::error_code& /* ec */) {
        message.write<uint64_t>(mLowestActiveVersion);
        message.write<uint64_t>(mDescriptor.baseVersion());
        message.write<uint64_t>(version);
        mDescriptor.serialize(message);
    });
}

void ServerManager::handleCommitTransaction(ServerSocket* con, crossbow::infinio::MessageId messageId,
        crossbow::infinio::BufferReader& message) {
    auto version = message.read<uint64_t>();

    auto succeeded = mDescriptor.commitTransaction(version);

    updateLowestActiveVersion();

    uint32_t messageLength = sizeof(uint8_t);
    con->writeResponse(messageId, ResponseType::COMMIT, messageLength, [succeeded]
            (crossbow::infinio::BufferWriter& message, std::error_code& /* ec */) {
        message.write<uint8_t>(succeeded ? 0x1u : 0x0u);
    });
}

void ServerManager::updateLowestActiveVersion() {
    while (!mReaders.empty() && mDescriptor.isCommitted(mReaders.front().version)) {
        mReaders.pop();
    }

    mLowestActiveVersion = (mReaders.empty() ? mDescriptor.baseVersion() : mReaders.front().baseVersion);
}

} // namespace commitmanager
} // namespace tell
