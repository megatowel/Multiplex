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

#define MAX_MULTIPLEX_CHANNELS 32 /* The amount of channels to use. 0 for system, rest for instances. */
#define MAX_MULTIPLEX_SERVER_CONNECTIONS 1024

namespace Megatowel {
	namespace Multiplex {

		MTMULTIPLEX_EXPORT enum class MultiplexActions {
			EditChannel = 0,
			Server
		};

		MTMULTIPLEX_EXPORT enum class MultiplexEventType {
			UserMessage = 0,
			Connected,
			Disconnected,
			UserSetup,
			InstanceUserUpdate,
			Error,
			ServerCustom
		};

		MTMULTIPLEX_EXPORT enum class MultiplexErrors {
			None = 0,
			ENet,
			NoEvent
		};

		// Used for sending.
		MTMULTIPLEX_EXPORT typedef enum MultiplexSendFlags {
			MT_SEND_RELIABLE = 1 << 0
		};

		MTMULTIPLEX_EXPORT typedef struct MultiplexInstanceUser {
			unsigned int channel;
			void* peer;
		};

		MTMULTIPLEX_EXPORT typedef struct MultiplexInstance {
			unsigned int id = 0;
			char* info = NULL;
			std::map<int, MultiplexInstanceUser> users;
		};

		MTMULTIPLEX_EXPORT typedef struct MultiplexEvent {
			MultiplexEventType eventType;
			unsigned int fromUserId = -1;
			unsigned int channelId = 0;
			unsigned int instanceId = 0;
			std::vector<uint8_t> data;
			MultiplexErrors Error = MultiplexErrors::None;
			int ENet_Error;
		};

		typedef struct MultiplexUser {
			unsigned int userId;
			int channelInstances[32];
		};

		// This needs to be called before Multiplex can work.
		MTMULTIPLEX_EXPORT int init_enet();

	}
}
