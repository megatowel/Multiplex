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

#define MAX_MULTIPLEX_CHANNELS 32 /* The amount of instance channels. */
#define MAX_MULTIPLEX_SERVER_CONNECTIONS 1024

namespace Megatowel {
	namespace Multiplex {

		typedef enum class MultiplexActions {
			EditChannel = 0,
			Server
		};

		typedef enum class MultiplexSystemResponses {
			Message = 0,
			UserSetup,
			InstanceConnected,
			InstanceUserJoin,
			InstanceUserLeave
		};

		typedef enum class MultiplexEventType {
			Error = -1,
			UserMessage,
			UserSetup,
			Connected,
			InstanceConnected,
			Disconnected,
			InstanceUserUpdate,
			ServerCustom
		};

		typedef enum class MultiplexErrors {
			None = 0,
			ENet,
			NoEvent,
			FailedRelay
		};

		// Used for sending.
		typedef enum MultiplexSendFlags {
			MT_SEND_RELIABLE = 1 << 0
		};

		typedef struct MultiplexInstanceUser {
			unsigned int channel;
			void* peer;
		};

		typedef struct MultiplexInstance {
			unsigned long long id = 0;
			char* info = nullptr;
			std::map<int, MultiplexInstanceUser> users;
		};

		typedef struct MultiplexEvent {
			MultiplexEventType eventType = MultiplexEventType::Error;
			unsigned long long fromUserId = 0;
			unsigned int channelId = 0;
			unsigned long long instanceId = 0;
			char* data = nullptr;
			unsigned int dataSize = 0;
			MultiplexErrors Error = MultiplexErrors::None;
			int ENetError = 0;
		};

		typedef struct MultiplexUser {
			unsigned long long userId;
			int channelInstances[32];
		};

		// This needs to be called before Multiplex can work.
		extern "C" { MTMULTIPLEX_EXPORT int init_enet(); }

	}
}
