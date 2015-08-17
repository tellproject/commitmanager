#include <commitmanager/SnapshotDescriptor.hpp>

#include <commitmanager/Descriptor.hpp>

#include <crossbow/byte_buffer.hpp>
#include <crossbow/logger.hpp>

#include <cstdlib>
#include <cstring>
#include <iostream>

namespace tell {
namespace commitmanager {

std::unique_ptr<SnapshotDescriptor> SnapshotDescriptor::create(uint64_t lowestActiveVersion,
        const Descriptor& descriptor) {
    auto descLen = descriptorLength(descriptor.baseVersion(), descriptor.lastVersion());

    std::unique_ptr<SnapshotDescriptor> snapshot(new (descLen) SnapshotDescriptor(lowestActiveVersion,
            descriptor.baseVersion(), descriptor.lastVersion()));
    if (snapshot) {
        crossbow::buffer_writer writer(snapshot->data(), descLen);
        descriptor.serialize(writer);
    }
    return snapshot;
}

std::unique_ptr<SnapshotDescriptor> SnapshotDescriptor::create(uint64_t lowestActiveVersion, uint64_t baseVersion,
        uint64_t version, const char* descriptor) {
    auto descLen = descriptorLength(baseVersion, version);

    std::unique_ptr<SnapshotDescriptor> snapshot(new (descLen) SnapshotDescriptor(lowestActiveVersion, baseVersion,
            version));
    if (snapshot) {
        memcpy(snapshot->data(), descriptor, descLen);
    }
    return snapshot;
}

void* SnapshotDescriptor::operator new(size_t size, size_t descLen) {
    LOG_ASSERT(size == sizeof(SnapshotDescriptor), "Requested size does not match SnapshotDescriptor size");
    return ::malloc(size + descLen);
}

void SnapshotDescriptor::operator delete(void* ptr) {
    ::free(ptr);
}

std::unique_ptr<SnapshotDescriptor> SnapshotDescriptor::deserialize(crossbow::buffer_reader& reader) {
    auto lowestActiveVersion = reader.read<uint64_t>();
    auto baseVersion = reader.read<uint64_t>();
    auto version = reader.read<uint64_t>();
    auto descLen = descriptorLength(baseVersion, version);

    std::unique_ptr<SnapshotDescriptor> snapshot(new (descLen) SnapshotDescriptor(lowestActiveVersion, baseVersion,
            version));
    if (snapshot) {
        memcpy(snapshot->data(), reader.read(descLen), descLen);
    }
    return snapshot;
}

void SnapshotDescriptor::serialize(crossbow::buffer_writer& writer) const {
    writer.write<uint64_t>(mLowestActiveVersion);
    writer.write<uint64_t>(mBaseVersion);
    writer.write<uint64_t>(mVersion);
    writer.write(data(), descriptorLength(mBaseVersion, mVersion));
}

std::ostream& operator<<(std::ostream& out, const SnapshotDescriptor& rhs) {
    out << "SnapshotDescriptor ["
           "Lowest Active Version = " << rhs.mLowestActiveVersion << ", "
           "Base Version = " << rhs.mBaseVersion << ", "
           "Version = " << rhs.mVersion << ", "
           "Descriptor = [";

    auto first = true;
    for (auto i = rhs.mBaseVersion + 1u; i <= rhs.mVersion; ++i) {
        if (i % 8u == 1u || first) {
            out << (first ? "[" : " [") << i << "] ";
            first = false;
        }
        out << (rhs.inReadSet(i) ? "x" : "-");
    }
    out << "]]";

    return out;
}

} // namespace commitmanager
} // namespace tell
