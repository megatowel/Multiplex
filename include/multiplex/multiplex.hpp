/// @file MTMultiplex.h
/// @brief This file contains general library exports.
#ifndef MULTIPLEX_H
#define MULTIPLEX_H

// Library export macro
#if defined(_MSC_VER)
//  Microsoft
#define MULTIPLEX_EXPORT __declspec(dllexport)
#define MULTIPLEX_IMPORT __declspec(dllimport)
#elif defined(__GNUC__)
//  GCC
#define MULTIPLEX_EXPORT __attribute__((visibility("default")))
#define MULTIPLEX_IMPORT
#else
//  do nothing and hope for the best?
#define MULTIPLEX_EXPORT
#define MULTIPLEX_IMPORT
#pragma warning Unknown dynamic link import / export semantics.
#endif

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <cstring>
#include <memory>

#define MULTIPLEX_MAX_CHANNELS 32 /* The amount of instance channels. */
#define MULTIPLEX_MAX_SERVER_CONNECTIONS 1024
#define MULTIPLEX_MAX_DATA_SIZE 65535

#define PACK_FIELD_RESPONSE_TYPE 0
#define PACK_FIELD_FROM_USERID 1
#define PACK_FIELD_DATA 2
#define PACK_FIELD_INFO 3
#define PACK_FIELD_USERIDS 4
#define PACK_FIELD_INSTANCEID 5
#define PACK_FIELD_ACTION 6
#define PACK_FIELD_CHANNELID 7

namespace Megatowel
{
	namespace Multiplex
	{
		/// @enum MultiplexActions
		/// @brief Actions that the MultiplexServer can accept internally.
		enum class MultiplexActions
		{
			EditChannel = 0,
			ServerMessage
		};

		/// @enum MultiplexSystemResponses
		/// @brief Responses that the MultiplexClient can accept internally.
		enum class MultiplexSystemResponses
		{
			Message = 0,
			UserSetup,
			InstanceConnected,
			InstanceUserJoin,
			InstanceUserLeave
		};

		/// @enum MultiplexEventType
		/// @brief Describes what type of event an event was.
		enum class MultiplexEventType
		{
			Error = -1,
			UserMessage,
			UserSetup,
			Connected,
			InstanceConnected,
			Disconnected,
			InstanceUserUpdate,
			ServerCustom
		};

		/// @enum MultiplexErrors
		/// @brief Describes what went wrong when something goes wrong.
		enum class MultiplexErrors
		{
			None = 0,
			ENet,
			NoEvent,
			FailedRelay,
			MissingActionArgs,
			BadPacking
		};

		/// @enum MultiplexSendFlags
		/// @brief Modifiers for sending
		enum MultiplexSendFlags
		{
			MT_SEND_RELIABLE = 1 << 0,
			MT_NO_FLUSH = 1 << 1
		};

		/// @struct MultiplexInstanceUser
		/// @brief Describes a user in a MultiplexInstance
		struct MultiplexInstanceUser
		{
			unsigned int channel;
			void *peer;
		};

		/// @struct MultiplexInstance
		/// @brief Describes an instance
		struct MultiplexInstance
		{
			unsigned long long id = 0;
			const char *info = nullptr;
			std::map<unsigned long long, MultiplexInstanceUser> users;
		};

		/// @struct MultiplexEvent
		/// @brief A message.
		struct MultiplexEvent
		{
			MultiplexEventType eventType = MultiplexEventType::Error;
			unsigned long long fromUserId = 0;
			unsigned int channelId = 0;
			unsigned long long instanceId = 0;
			char *data = nullptr;
			unsigned int dataSize = 0;
			char *info = nullptr;
			unsigned int infoSize = 0;
			unsigned long long *userIds = nullptr;
			unsigned int userIdsSize = 0;
			MultiplexErrors Error = MultiplexErrors::None;
			int ENetError = 0;
		};

		/// @struct MultiplexUser
		/// @brief Describes a user, instance agnostic
		struct MultiplexUser
		{
			unsigned long long userId;
			unsigned long long channelInstances[MULTIPLEX_MAX_CHANNELS];
			void *peer;
		};

		struct PackingField
		{
			uint16_t size;
			char *data;
		};
	}
}
#endif
