#pragma once

#include <crossbow/non_copyable.hpp>

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <new>

namespace crossbow {
namespace infinio {
class BufferReader;
class BufferWriter;
} // namespace infinio
} // namespace crossbow

namespace tell {
namespace commitmanager {

class Descriptor;

/**
 * @brief Descriptor containing information about the versions a transaction is allowed to read
 */
class SnapshotDescriptor final : crossbow::non_copyable, crossbow::non_movable {
public: // Construction
    static std::unique_ptr<SnapshotDescriptor> create(uint64_t lowestActiveVersion, const Descriptor& descriptor);

    static std::unique_ptr<SnapshotDescriptor> create(uint64_t lowestActiveVersion, uint64_t baseVersion,
            uint64_t version, const char* descriptor);

    void* operator new(size_t size, size_t descLen);

    void* operator new[](size_t size) = delete;

    void operator delete(void* ptr);

    void operator delete[](void* ptr) = delete;

public: // Serialization
    static std::unique_ptr<SnapshotDescriptor> deserialize(crossbow::infinio::BufferReader& reader);

    static size_t descriptorLength(uint64_t baseVersion, uint64_t lastVersion) {
        if (baseVersion == lastVersion) {
            return 0x0u;
        }
        return (((lastVersion - 1) / BITS_PER_BLOCK) - (baseVersion / BITS_PER_BLOCK) + 1) * sizeof(BlockType);
    }

    size_t serializedLength() const {
        return (3 * sizeof(uint64_t)) + descriptorLength(mBaseVersion, mVersion);
    }

    void serialize(crossbow::infinio::BufferWriter& writer) const;

public: // Version
    using BlockType = uint8_t;

    static constexpr size_t BITS_PER_BLOCK = sizeof(BlockType) * 8u;

    uint64_t lowestActiveVersion() const {
        return mLowestActiveVersion;
    }

    uint64_t baseVersion() const {
        return mBaseVersion;
    }

    uint64_t version() const {
        return mVersion;
    }

    const char* data() const {
        return reinterpret_cast<const char*>(this) + sizeof(SnapshotDescriptor);
    }

    bool inReadSet(uint64_t version) const {
        if (version <= mBaseVersion) {
            return true;
        }
        if (version > mVersion) {
            return false;
        }

        auto block = reinterpret_cast<const BlockType*>(data())[(version - (mBaseVersion + 1)) / BITS_PER_BLOCK];
        auto mask = (0x1u << ((version - 1) % BITS_PER_BLOCK));
        return (block & mask) != 0x0u;
    }

private:
    friend std::ostream& operator<<(std::ostream& out, const SnapshotDescriptor& rhs);

    SnapshotDescriptor(uint64_t lowestActiveVersion, uint64_t baseVersion, uint64_t version)
            : mLowestActiveVersion(lowestActiveVersion),
              mBaseVersion(baseVersion),
              mVersion(version) {
    }

    char* data() {
        return const_cast<char*>(const_cast<const SnapshotDescriptor*>(this)->data());
    }

    uint64_t mLowestActiveVersion;
    uint64_t mBaseVersion;
    uint64_t mVersion;
};

std::ostream& operator<<(std::ostream& out, const SnapshotDescriptor& rhs);

} // namespace commitmanager
} // namespace tell
