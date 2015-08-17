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

bool CommitManager::startTransaction() {
    auto version = mDescriptor.startTransaction();
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
