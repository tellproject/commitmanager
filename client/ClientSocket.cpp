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

#include <commitmanager/ClientSocket.hpp>

#include <crossbow/logger.hpp>

namespace tell {
namespace commitmanager {

void StartResponse::processResponse(crossbow::buffer_reader& message) {
    setResult(SnapshotDescriptor::deserialize(message));
}

void CommitResponse::processResponse(crossbow::buffer_reader& message) {
    setResult(message.read<uint8_t>() != 0x0u);
}

void ClientSocket::connect(const crossbow::infinio::Endpoint& host) {
      
          LOG_INFO("Connecting to CommitManager server %1%", host);

    crossbow::infinio::RpcClientSocket::connect(host, handshakeString());
}

void ClientSocket::shutdown() {
    LOG_INFO("Shutting down CommitManager connection");

    crossbow::infinio::RpcClientSocket::shutdown();
}

std::shared_ptr<StartResponse> ClientSocket::startTransaction(crossbow::infinio::Fiber& fiber, bool readonly) {
    auto response = std::make_shared<StartResponse>(fiber);

    uint32_t messageLength = sizeof(uint8_t);
    sendRequest(response, RequestType::START, messageLength, [readonly] (crossbow::buffer_writer& message,
            std::error_code& /* ec */) {
        message.write<uint8_t>(readonly ? 0x1u : 0x0u);
    });

    return response;
}

std::shared_ptr<CommitResponse> ClientSocket::commitTransaction(crossbow::infinio::Fiber& fiber, uint64_t version) {
    auto response = std::make_shared<CommitResponse>(fiber);

    uint32_t messageLength = sizeof(uint64_t);
    sendRequest(response, RequestType::COMMIT, messageLength, [version] (crossbow::buffer_writer& message,
            std::error_code& /* ec */) {
        message.write<uint64_t>(version);
    });

    return response;
}

} // namespace commitmanager
} // namespace tell
