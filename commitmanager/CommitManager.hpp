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
