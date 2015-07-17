#include <commitmanager/ClientSocket.hpp>

#include <crossbow/logger.hpp>

namespace tell {
namespace commitmanager {

void StartResponse::processResponse(crossbow::infinio::BufferReader& message) {
    setResult(SnapshotDescriptor::deserialize(message));
}

void CommitResponse::processResponse(crossbow::infinio::BufferReader& message) {
    setResult(message.read<uint8_t>() != 0x0u);
}

void ClientSocket::connect(const crossbow::string& host, uint16_t port) {
    LOG_INFO("Connecting to CommitManager server %1%:%2%", host, port);

    crossbow::infinio::RpcClientSocket::connect(host, port, crossbow::string{});
}

void ClientSocket::shutdown() {
    LOG_INFO("Shutting down CommitManager connection");

    crossbow::infinio::RpcClientSocket::shutdown();
}

std::shared_ptr<StartResponse> ClientSocket::startTransaction(crossbow::infinio::Fiber& fiber) {
    auto response = std::make_shared<StartResponse>(fiber);

    uint32_t messageLength = 0x0u;
    sendRequest(response, RequestType::START, messageLength, [] (crossbow::infinio::BufferWriter& message,
            std::error_code& /* ec */) {
    });

    return response;
}

std::shared_ptr<CommitResponse> ClientSocket::commitTransaction(crossbow::infinio::Fiber& fiber, uint64_t version) {
    auto response = std::make_shared<CommitResponse>(fiber);

    uint32_t messageLength = sizeof(uint64_t);
    sendRequest(response, RequestType::COMMIT, messageLength, [version] (crossbow::infinio::BufferWriter& message,
            std::error_code& /* ec */) {
        message.write<uint64_t>(version);
    });

    return response;
}

} // namespace commitmanager
} // namespace tell
