#ifndef MULTIPLEX_TYPES_HPP
#define MULTIPLEX_TYPES_HPP

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <cstring>
#include <memory>
#include <any>
#include "packing.hpp"

namespace Megatowel
{
    namespace Multiplex
    {
        class MultiplexBase;
        class MultiplexUser;
        class MultiplexInstance;
        struct MultiplexEvent;
        struct MultiplexPacket;

        /// @enum MultiplexResponse
        /// @brief Describes the type of a received message.
        enum class MultiplexResponse : char
        {
            Error = -1,
            Message,
            Connect,
            Disconnect,
            InstanceJoin,
            InstanceLeave,
            InstanceModify,
            RemoteInstanceJoin,
            RemoteInstanceLeave
        };

        /// @class MultiplexInstance
        /// @brief A 'room' that can contain users.
        class MultiplexInstance
        {
            friend class MultiplexUser;
            friend class MultiplexServer;
            friend class MultiplexClient;

        public:
            MultiplexInstance(MultiplexBase *owner);
            ~MultiplexInstance();
            unsigned int find_channel(const MultiplexUser *user) const;
            ptrdiff_t get_user_index(const MultiplexUser *user) const;
            ptrdiff_t get_id() const;
            void send(const MultiplexUser *destination, const MultiplexUser *sender, const MultiplexResponse type, const char *data = nullptr, const size_t dataSize = 0) const;
            void bind(MultiplexUser *user, const unsigned int channel);
            std::any meta;

        private:
            std::vector<MultiplexUser *> users;
            std::vector<unsigned int> channels;
            MultiplexBase *owner;
            MultiplexInstance(const MultiplexInstance &) = delete;
        };

        /// @class MultiplexUser
        /// @brief A user currently connected to the server.
        class MultiplexUser
        {
            friend class MultiplexInstance;
            friend class MultiplexServer;
            friend class MultiplexClient;

        public:
            MultiplexUser(MultiplexBase *owner);
            ~MultiplexUser();
            unsigned int find_channel(const MultiplexInstance *instance) const;
            ptrdiff_t get_user_index(const MultiplexInstance *instance) const;
            ptrdiff_t get_id() const;
            void send(const MultiplexInstance *instance, const MultiplexUser *sender, const MultiplexResponse type, const char *data = nullptr, const size_t dataSize = 0) const;
            void bind(MultiplexInstance *instance, const unsigned int channel);
            std::any meta;

        private:
            MultiplexBase *owner;
            std::vector<MultiplexInstance *> channels;
            void *peer;
            MultiplexUser(const MultiplexUser &) = delete;
        };

        /// @struct MultiplexEvent
        /// @brief A received message.
        struct MultiplexEvent
        {
            MultiplexResponse type;
            MultiplexUser *sender;
            MultiplexInstance *instance;
            char *data;
            size_t size;
        };

        /// @struct MultiplexPacket
        /// @brief A sending message.
        struct MultiplexPacket
        {
            MultiplexPacket(MultiplexResponse type, const MultiplexUser *sender, const MultiplexInstance *instance, const char *data = nullptr, const size_t size = 0);
            void *to_native_packet() const;
            MultiplexResponse responseType;
            unsigned short id;
            char data[50000];
            size_t size;
        };
    }
}
#endif