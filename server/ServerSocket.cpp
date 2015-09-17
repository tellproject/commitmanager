#include "ServerSocket.hpp"

#include <commitmanager/ErrorCode.hpp>
#include <commitmanager/MessageTypes.hpp>

#include <crossbow/enum_underlying.hpp>
#include <crossbow/logger.hpp>

#include <chrono>

namespace tell {
namespace commitmanager {

void ServerSocket::onRequest(crossbow::infinio::MessageId messageId, uint32_t messageType,
        crossbow::buffer_reader& message) {
    manager().onMessage(this, messageId, messageType, message);
}

ServerManager::ServerManager(crossbow::infinio::InfinibandService& service, const ServerConfig& config)
        : Base(service, config.port),
          mProcessor(service.createProcessor()) {
}

ServerSocket* ServerManager::createConnection(crossbow::infinio::InfinibandSocket socket,
        const crossbow::string& data) {
    return new ServerSocket(*this, *mProcessor, std::move(socket));
}

void ServerManager::onMessage(ServerSocket* con, crossbow::infinio::MessageId messageId, uint32_t messageType,
        crossbow::buffer_reader& message) {
#ifdef NDEBUG
#else
    LOG_TRACE("MID %1%] Handling request of type %2%", messageId.userId(), messageType);
    auto startTime = std::chrono::steady_clock::now();
#endif

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

#ifdef NDEBUG
#else
    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime);
    LOG_TRACE("MID %1%] Handling request took %2%ns", messageId.userId(), duration.count());
#endif
}

void ServerManager::handleStartTransaction(ServerSocket* con, crossbow::infinio::MessageId messageId,
        crossbow::buffer_reader& /* message */) {
    if (!mCommitManager.startTransaction()) {
        con->writeErrorResponse(messageId, error::transaction_limit_reached);
        return;
    }

    auto messageLength = mCommitManager.serializedLength();
    con->writeResponse(messageId, ResponseType::START, messageLength, [this]
            (crossbow::buffer_writer& message, std::error_code& /* ec */) {
        mCommitManager.serializeSnapshot(message);
    });
}

void ServerManager::handleCommitTransaction(ServerSocket* con, crossbow::infinio::MessageId messageId,
        crossbow::buffer_reader& message) {
    auto version = message.read<uint64_t>();

    auto succeeded = mCommitManager.commitTransaction(version);

    uint32_t messageLength = sizeof(uint8_t);
    con->writeResponse(messageId, ResponseType::COMMIT, messageLength, [succeeded]
            (crossbow::buffer_writer& message, std::error_code& /* ec */) {
        message.write<uint8_t>(succeeded ? 0x1u : 0x0u);
    });
}

} // namespace commitmanager
} // namespace tell
