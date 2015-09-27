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
#include <commitmanager/CommitManager.hpp>

#include <crossbow/byte_buffer.hpp>
#include <crossbow/logger.hpp>

#include <stdexcept>

namespace tell {
namespace commitmanager {

void CommitManager::serializeSnapshot(crossbow::buffer_writer& writer) const {
    if (!writer.canWrite(3 * sizeof(uint64_t))) {
        throw std::length_error("Output buffer too small for snapshot header");
    }
    writer.write<uint64_t>(mLowestActiveVersion);
    writer.write<uint64_t>(mDescriptor.baseVersion());
    writer.write<uint64_t>(mDescriptor.lastVersion());
    mDescriptor.serialize(writer);
}

bool CommitManager::startTransaction(bool readonly) {
    auto version = mDescriptor.startTransaction(readonly);
    if (version == 0x0u) {
        return false;
    }
    LOG_TRACE("Started transaction %1%", version);

    mReaders.emplace(version, mDescriptor.baseVersion());
    return true;
}

bool CommitManager::commitTransaction(uint64_t version) {
    if (!mDescriptor.commitTransaction(version)) {
        return false;
    }
    LOG_TRACE("Committed transaction %1%", version);

    updateLowestActiveVersion();
    return true;
}

void CommitManager::updateLowestActiveVersion() {
    while (!mReaders.empty() && mDescriptor.isCommitted(mReaders.front().version)) {
        mReaders.pop();
    }

    mLowestActiveVersion = (mReaders.empty() ? mDescriptor.baseVersion() : mReaders.front().baseVersion);
}

} // namespace commitmanager
} // namespace tell
