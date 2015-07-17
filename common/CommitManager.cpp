#include <commitmanager/CommitManager.hpp>

#include <crossbow/infinio/ByteBuffer.hpp>

namespace tell {
namespace commitmanager {

void CommitManager::serializeSnapshot(crossbow::infinio::BufferWriter& writer) const {
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
    mReaders.emplace(version, mDescriptor.baseVersion());
    return true;
}

bool CommitManager::commitTransaction(uint64_t version) {
    auto succeeded = mDescriptor.commitTransaction(version);
    updateLowestActiveVersion();
    return succeeded;
}

void CommitManager::updateLowestActiveVersion() {
    while (!mReaders.empty() && mDescriptor.isCommitted(mReaders.front().version)) {
        mReaders.pop();
    }

    mLowestActiveVersion = (mReaders.empty() ? mDescriptor.baseVersion() : mReaders.front().baseVersion);
}

} // namespace commitmanager
} // namespace tell
