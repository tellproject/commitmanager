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

#include <commitmanager/Descriptor.hpp>
#include <commitmanager/SnapshotDescriptor.hpp>

#include <cstdint>
#include <memory>
#include <queue>

namespace crossbow {
class buffer_writer;
} // namespace crossbow

namespace tell {
namespace commitmanager {

/**
 * @brief Class providing commit manager functionality
 */
class CommitManager {
public:
    CommitManager()
            : mLowestActiveVersion(0x1u) {
    }

    std::unique_ptr<SnapshotDescriptor> createSnapshot() const {
        return SnapshotDescriptor::create(mLowestActiveVersion, mDescriptor);
    }

    uint32_t serializedLength() const {
        return (3 * sizeof(uint64_t)) + mDescriptor.serializedLength();
    }

    void serializeSnapshot(crossbow::buffer_writer& writer) const;

    uint64_t lowestActiveVersion() const {
        return mLowestActiveVersion;
    }

    bool startTransaction(bool readonly);

    bool commitTransaction(uint64_t version);

private:
    struct Reader {
        Reader(uint64_t v, uint64_t b)
                : version(v),
                  baseVersion(b) {
        }

        uint64_t version;
        uint64_t baseVersion;
    };

    void updateLowestActiveVersion();

    uint64_t mLowestActiveVersion;
    std::queue<Reader> mReaders;
    Descriptor mDescriptor;
};

} // namespace commitmanager
} // namespace tell
