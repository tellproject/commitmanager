#pragma once

#include <commitmanager/SnapshotDescriptor.hpp>

#include <array>
#include <cstdint>

namespace crossbow {
namespace infinio {
class BufferWriter;
} // namespace infinio
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
    void serialize(crossbow::infinio::BufferWriter& writer) const;

public: // Version
    uint64_t baseVersion() const {
        return mBaseVersion;
    }

    uint64_t lastVersion() const {
        return mLastVersion;
    }

    uint64_t startTransaction();

    bool commitTransaction(uint64_t version);

    bool isCommitted(uint64_t version) const;

private:
    using BlockType = SnapshotDescriptor::BlockType;

    static constexpr size_t BITS_PER_BLOCK = SnapshotDescriptor::BITS_PER_BLOCK;

    static constexpr size_t CAPACITY = 32768ull;

    static size_t blockIndex(uint64_t version) {
        return (((version - 1) / BITS_PER_BLOCK) % CAPACITY);
    }

    void updateBaseVersion();

    uint64_t mBaseVersion;
    uint64_t mLastVersion;
    std::array<BlockType, CAPACITY> mDescriptor;
};

} // namespace commitmanager
} // namespace tell
