#include "commitmanager/Descriptor.hpp"

#include <crossbow/byte_buffer.hpp>
#include <crossbow/logger.hpp>

#include <algorithm>
#include <stdexcept>

namespace tell {
namespace commitmanager {

void Descriptor::serialize(crossbow::buffer_writer& writer) const {
    if (mBaseVersion >= mLastVersion) {
        return;
    }

    auto startIndex = blockIndex(mBaseVersion + 1);
    auto endIndex = blockIndex(mLastVersion);
    auto descLen = ((endIndex < startIndex ? CAPACITY - startIndex + endIndex : endIndex - startIndex) + 1)
            * sizeof(BlockType);
    LOG_ASSERT(descLen == serializedLength(), "Sizes do not match");
    if (!writer.canWrite(descLen)) {
        throw std::length_error("Output buffer too small for descriptor");
    }

    if (endIndex < startIndex) {
        writer.write(&mDescriptor[startIndex], (CAPACITY - startIndex) * sizeof(BlockType));
        startIndex = 0;
    }
    writer.write(&mDescriptor[startIndex], (endIndex - startIndex) * sizeof(BlockType));

    auto mask = (0x1u << ((mLastVersion -  1) % BITS_PER_BLOCK));
    auto endBlock = mDescriptor[endIndex] | mask;
    writer.write<BlockType>(endBlock);
}

uint64_t Descriptor::startTransaction(bool readOnly) {
    if (mBaseVersion + (CAPACITY * BITS_PER_BLOCK) == mLastVersion) {
        return 0x0u;
    }

    auto version = ++mLastVersion;

    if (readOnly) {
        commitVersion(version);
    }

    return version;
}

bool Descriptor::commitTransaction(uint64_t version) {
    if (version > mLastVersion) {
        LOG_ERROR("Trying to commit invalid version %1%", version);
        return false;
    }

    if (version > mBaseVersion) {
        commitVersion(version);
    }
    return true;
}

bool Descriptor::isCommitted(uint64_t version) const {
    if (version <= mBaseVersion) {
        return true;
    }
    if (version > mLastVersion) {
        return false;
    }

    auto index = blockIndex(version);
    auto mask = (0x1u << ((version -  1) % BITS_PER_BLOCK));
    return (mDescriptor[index] & mask) != 0x0u;
}

void Descriptor::commitVersion(uint64_t version) {
    auto index = blockIndex(version);
    auto mask = (0x1u << ((version -  1) % BITS_PER_BLOCK));
    mDescriptor[index] |= mask;

    if (version == mBaseVersion + 1) {
        updateBaseVersion();
    }
}

void Descriptor::updateBaseVersion() {
    auto index = blockIndex(mBaseVersion + 1);

    // Process version blocks where all versions are marked as committed
    // Release the block and increase base version so that it is aligned to the next block
    for (; mDescriptor[index] == std::numeric_limits<BlockType>::max(); index = ((index + 1) % CAPACITY)) {
        mBaseVersion += ((mBaseVersion % BITS_PER_BLOCK != 0)
                ? (BITS_PER_BLOCK - (mBaseVersion % BITS_PER_BLOCK))
                : BITS_PER_BLOCK);
        mDescriptor[index] = 0x0u;
    }

    // Process the version block where the versions are only partially marked as committed
    // Check the block bit by bit and increase base version until the first uncommitted version is encountered
    for (; (mDescriptor[index] & (0x1u << (mBaseVersion % BITS_PER_BLOCK))) != 0x0u; ++mBaseVersion) {
    }

    LOG_ASSERT(blockIndex(mBaseVersion + 1) == index, "Base version and block index do not match");
}

} // namespace commitmanager
} // namespace tell
