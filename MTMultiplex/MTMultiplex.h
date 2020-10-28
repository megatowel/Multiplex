// MTMultiplex.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#pragma region Export defines
#ifndef MTMULTIPLEX_EXPORT_H
#define MTMULTIPLEX_EXPORT_H

#if defined(_MSC_VER)
	//  Microsoft 
#define MTMULTIPLEX_EXPORT __declspec(dllexport)
#define MTMULTIPLEX_IMPORT __declspec(dllimport)
#elif defined(__GNUC__)
	//  GCC
#define MTMULTIPLEX_EXPORT __attribute__((visibility("default")))
#define MTMULTIPLEX_IMPORT
#else
	//  do nothing and hope for the best?
#define MTMULTIPLEX_EXPORT
#define MTMULTIPLEX_IMPORT
#pragma warning Unknown dynamic link import/export semantics.
#endif


#endif /* MTMULTIPLEX_EXPORT_H */
#pragma endregion

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <cstring>

#define MAX_MULTIPLEX_CHANNELS 32 /* The amount of instance channels. */
#define MAX_MULTIPLEX_SERVER_CONNECTIONS 1024

#define PACK_FIELD_RESPONSE_TYPE 0
#define PACK_FIELD_FROM_USERID 1
#define PACK_FIELD_DATA 2
#define PACK_FIELD_INFO 3
#define PACK_FIELD_USERIDS 4
#define PACK_FIELD_INSTANCEID 5
#define PACK_FIELD_ACTION 6
#define PACK_FIELD_CHANNELID 7


namespace Megatowel {
	namespace Multiplex {

		enum class MultiplexActions {
			EditChannel = 0,
			ServerMessage
		};

		enum class MultiplexSystemResponses {
			Message = 0,
			UserSetup,
			InstanceConnected,
			InstanceUserJoin,
			InstanceUserLeave
		};

		enum class MultiplexEventType {
			Error = -1,
			UserMessage,
			UserSetup,
			Connected,
			InstanceConnected,
			Disconnected,
			InstanceUserUpdate,
			ServerCustom
		};

		enum class MultiplexErrors {
			None = 0,
			ENet,
			NoEvent,
			FailedRelay,
			MissingActionArgs,
			BadPacking
		};

		// Used for sending.
		enum MultiplexSendFlags {
			MT_SEND_RELIABLE = 1 << 0,
			MT_NO_FLUSH = 1 << 1
		};

		struct MultiplexInstanceUser {
			unsigned int channel;
			void* peer;
		};

		struct MultiplexInstance {
			unsigned long long id = 0;
			char* info = nullptr;
			std::map<unsigned long long, MultiplexInstanceUser> users;
		};

		struct MultiplexEvent {
			MultiplexEventType eventType = MultiplexEventType::Error;
			unsigned long long fromUserId = 0;
			unsigned int channelId = 0;
			unsigned long long instanceId = 0;
			char* data = nullptr;
			unsigned int dataSize = 0;
			char* info = nullptr;
			unsigned int infoSize = 0;
			unsigned long long* userIds = nullptr;
			unsigned int userIdsSize = 0;
			MultiplexErrors Error = MultiplexErrors::None;
			int ENetError = 0;
		};

		struct MultiplexUser {
			unsigned long long userId;
			unsigned long long channelInstances[MAX_MULTIPLEX_CHANNELS];
			void* peer;
		};

		// This needs to be called before Multiplex can work.
		extern "C" { MTMULTIPLEX_EXPORT int init_enet(); }

	}
}
