
#ifndef RELEASE_FRAME_H
#define RELEASE_FRAME_H

#include "packet.h"

#include <cereal/types/polymorphic.hpp>
#include <cstdint>

struct release_frame : packet {
    uint8_t framebuffer_id;

    template <class Archive>
    void serialize(Archive& archive)
    {
        archive(framebuffer_id);
    }

    void poly() override {};
};

#include <cereal/archives/binary.hpp>

CEREAL_REGISTER_TYPE(release_frame);

CEREAL_REGISTER_POLYMORPHIC_RELATION(packet, release_frame);

#endif // RELEASE_FRAME_H
