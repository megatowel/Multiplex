#include "multiplex/types.hpp"
#include "multiplex/base.hpp"
#include "enet/enet.h"
#include "error.hpp"

using namespace Megatowel::Multiplex;
using namespace std;

// ----- MultiplexInstance

MultiplexInstance::MultiplexInstance(MultiplexBase *owner)
{
    if (!owner)
    {
        MULTIPLEX_ERROR("Instance can't be constructed without an owner.");
    }
    owner = owner;
    owner->instances.push_back(this);
}

MultiplexInstance::~MultiplexInstance()
{
    // TODO: For each user, tell them that the instance is closing.
    owner->instances.erase(owner->instances.begin() + this->get_id());
}

void MultiplexInstance::send(const MultiplexUser *destination, const MultiplexUser *sender, const MultiplexResponse type, const char *data, const size_t dataSize) const
{
    owner->send(destination, this, sender, type, data, dataSize);
}

void MultiplexInstance::bind(MultiplexUser *user, const unsigned int channel)
{
    owner->bind_channel(user, this, channel);
}

ptrdiff_t MultiplexInstance::get_user_index(const MultiplexUser *user) const
{
    if (user)
    {
        auto index = std::find(this->users.begin(), this->users.end(), user) - this->users.begin();
        if (index == (this->users.end() - this->users.begin()))
            MULTIPLEX_ERROR("User was not found in instance.");
        return index;
    }
    else
        MULTIPLEX_ERROR("User parameter cannot be null.");
};

ptrdiff_t MultiplexInstance::get_id() const
{
    auto index = std::find(owner->instances.begin(), owner->instances.end(), this) - owner->instances.begin();
    return index;
};

unsigned int MultiplexInstance::find_channel(const MultiplexUser *user) const
{
    if (user)
    {
        return this->channels[get_user_index(user)];
    }
    else
        return 0;
};

// ----- MultiplexUser

MultiplexUser::MultiplexUser(MultiplexBase *owner)
{
    if (!owner)
    {
        MULTIPLEX_ERROR("User can't be constructed without an owner.");
    }
    this->owner = owner;
    owner->users.push_back(this);
}

MultiplexUser::~MultiplexUser()
{
    // For each instance, tell them that the user is leaving, then erase it.
    for (MultiplexInstance *instance : this->channels) {
        ptrdiff_t index = this->get_user_index(instance);
        instance->send(nullptr, this, MultiplexResponse::RemoteInstanceLeave);
        instance->users.erase(instance->users.begin() + index);
    }
    owner->users.erase(owner->users.begin() + this->get_id());
}

void MultiplexUser::send(const MultiplexInstance *instance, const MultiplexUser *sender, const MultiplexResponse type, const char *data, const size_t dataSize) const
{
    owner->send(this, instance, sender, type, data, dataSize);
}

void MultiplexUser::bind(MultiplexInstance *instance, const unsigned int channel)
{
    owner->bind_channel(this, instance, channel);
}

unsigned int MultiplexUser::find_channel(const MultiplexInstance *instance) const
{
    if (instance != nullptr)
    {
        auto index = std::find(instance->users.begin(), instance->users.end(), this) - instance->users.begin();
        if (index == (instance->users.end() - instance->users.begin()))
            MULTIPLEX_ERROR("User was not found in instance.");
        return instance->channels[index];
    }
    else
        return 0;
};

ptrdiff_t MultiplexUser::get_user_index(const MultiplexInstance *instance) const
{
    if (instance != nullptr)
    {
        auto index = std::find(instance->users.begin(), instance->users.end(), this) - instance->users.begin();
        if (index == (instance->users.end() - instance->users.begin()))
            MULTIPLEX_ERROR("User was not found in instance.");
        return index;
    }
    else
        MULTIPLEX_ERROR("Instance parameter cannot be null.");
};

ptrdiff_t MultiplexUser::get_id() const
{
    auto index = std::find(owner->users.begin(), owner->users.end(), this) - owner->users.begin();
    return index;
};

// ----- MultiplexPacket

MultiplexPacket::MultiplexPacket(MultiplexResponse type, const MultiplexUser *sender, const MultiplexInstance *instance, const char *data, const size_t size)
{
    size_t pos = 0;

    memcpy((void *)(this->data + pos), &type, sizeof(MultiplexResponse));
    pos += sizeof(MultiplexResponse);

    if (type == MultiplexResponse::Message && sender && instance) {
        memcpy((void *)(this->data + pos), (char *)instance->get_user_index(sender), sizeof(ptrdiff_t));
        pos += sizeof(ptrdiff_t);
    }

    if (data) {
        memcpy((void *)(this->data + pos), (char *)data, size);
        pos += size;
    }
    
    this->size = pos;
}

void *MultiplexPacket::to_native_packet() const
{
    return enet_packet_create(this->data, this->size, 0);
}