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

#include <commitmanager/SnapshotDescriptor.hpp>

#include <array>
#include <cstdint>

namespace crossbow {
class buffer_writer;
} // namespace crossbow

namespace tell {
namespace commitmanager {

/**
 * @brief Descriptor containing information about the committed versions
 *
 * The committed versions are stored as bits in a ring buffer. Versions begin at version 1 and are stored in each block
 * from LSB to MSB.
 *
 * | 8 | 7 | ... | 2 | 1 || 16 | 15 | ... | 10 | 9 |
 */
class Descriptor {
public: // Construction
    Descriptor()
            : mBaseVersion(0x0u),
              mLastVersion(0x0u),
              mDescriptor({}) {
    }

public: // Serialization
    size_t serializedLength() const {
        return SnapshotDescriptor::descriptorLength(mBaseVersion, mLastVersion);
    }

    /**
     * @brief Serialize the descriptor to the buffer
     *
     * The transaction which was started last will be marked as committed in the written descriptor.
     *
     * @param writer Writer to serialize the descriptor to
     */
    void serialize(crossbow::buffer_writer& writer) const;

public: // Version
    uint64_t baseVersion() const {
        return mBaseVersion;
    }

    uint64_t lastVersion() const {
        return mLastVersion;
    }

    uint64_t startTransaction(bool readOnly);

    bool commitTransaction(uint64_t version);

    bool isCommitted(uint64_t version) const;

private:
    using BlockType = SnapshotDescriptor::BlockType;

    static constexpr size_t BITS_PER_BLOCK = SnapshotDescriptor::BITS_PER_BLOCK;

    static constexpr size_t CAPACITY = 64 * 1024ull;

    static size_t blockIndex(uint64_t version) {
        return (((version - 1) / BITS_PER_BLOCK) % CAPACITY);
    }

    void commitVersion(uint64_t version);

    void updateBaseVersion();

    uint64_t mBaseVersion;
    uint64_t mLastVersion;
    std::array<BlockType, CAPACITY> mDescriptor;
};

} // namespace commitmanager
} // namespace tell
