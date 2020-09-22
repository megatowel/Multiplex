// MTMultiplex.cpp : Defines the entry point for the application.
//

#include "MTMultiplex.h"
#include <msquichelper.h>

using namespace std;

const uint16_t UdpPort = 3000;
const uint64_t IdleTimeoutMs = 15000;
const uint32_t SendBufferLength = 2096;
const QUIC_REGISTRATION_CONFIG RegConfig = { "quicsample", QUIC_EXECUTION_PROFILE_LOW_LATENCY };
const QUIC_BUFFER Alpn = { sizeof("MTMultiplex") - 1, (uint8_t*)"MTMultiplex" };
const QUIC_API_TABLE* MSQuic;

HQUIC Registration;
HQUIC Session;
QUIC_SEC_CONFIG* SecurityConfig;
bool Active;

_IRQL_requires_max_(DISPATCH_LEVEL)
_Function_class_(QUIC_CONNECTION_CALLBACK)
QUIC_STATUS
QUIC_API
ClientConnectionCallback(
    _In_ HQUIC Connection,
    _In_opt_ void* /* Context */,
    _Inout_ QUIC_CONNECTION_EVENT* Event
)
{
    switch (Event->Type) {
    case QUIC_CONNECTION_EVENT_CONNECTED:
        cout << "Connected" << Connection << endl;
        break;
    case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_TRANSPORT:
    case QUIC_CONNECTION_EVENT_SHUTDOWN_INITIATED_BY_PEER:
        cout << "Shutdown" << Connection << endl;
        break;
    case QUIC_CONNECTION_EVENT_SHUTDOWN_COMPLETE:
        cout << "Done" << Connection << endl;
        Active = false;
        if (!Event->SHUTDOWN_COMPLETE.AppCloseInProgress) {
            MSQuic->ConnectionClose(Connection);
        }
        break;
    case QUIC_CONNECTION_EVENT_RESUMPTION_TICKET_RECEIVED:
        printf("[conn][%p] Resumption ticket received (%u bytes):\n", Connection, Event->RESUMPTION_TICKET_RECEIVED.ResumptionTicketLength);
        for (uint32_t i = 0; i < Event->RESUMPTION_TICKET_RECEIVED.ResumptionTicketLength; i++) {
            printf("%.2X", (uint8_t)Event->RESUMPTION_TICKET_RECEIVED.ResumptionTicket[i]);
        }
        printf("\n");
        break;
    default:
        break;
    }
    return QUIC_STATUS_SUCCESS;
}

void Start()
{
	cout << "Starting MSQUIC." << endl;

    QUIC_SETTINGS Settings{ 0 };
    Settings.IdleTimeoutMs = IdleTimeoutMs;
    Settings.IsSet.IdleTimeoutMs = TRUE;
    Settings.ServerResumptionLevel = QUIC_SERVER_RESUME_AND_ZERORTT;
    Settings.IsSet.ServerResumptionLevel = TRUE;
    Settings.PeerBidiStreamCount = 1;
    Settings.IsSet.PeerBidiStreamCount = TRUE;
    QUIC_STATUS Status = QUIC_STATUS_SUCCESS;
    if (QUIC_FAILED(Status = MsQuicOpen(&MSQuic))) {
        cout << "MsQuicOpen failed" << Status << endl;
        return;
    }

    if (QUIC_FAILED(Status = MSQuic->RegistrationOpen(&RegConfig, &Registration))) {
        cout << "RegistrationOpen failed" << Status << endl;
        return;
    }

    if (QUIC_FAILED(Status = MSQuic->SessionOpen(Registration, sizeof(Settings), &Settings, &Alpn, 1, nullptr, &Session))) {
        cout << "SessionOpen failed" << Status << endl;
        return;
    }

    const char* ResumptionTicketString = nullptr;
    HQUIC Connection = nullptr;
    if (QUIC_FAILED(Status = MSQuic->ConnectionOpen(Session, ClientConnectionCallback, nullptr, &Connection))) {
        cout << "ConnectionOpen failed" << Status << endl;
        if (QUIC_FAILED(Status) && Connection != nullptr) {
            MSQuic->ConnectionClose(Connection);
        }
    }

    cout << "Connecting..." << endl;
    Active = true;

    if (QUIC_FAILED(Status = MSQuic->ConnectionStart(Connection, AF_INET, "127.0.0.1", UdpPort))) {
        cout << "ConnectionStart failed" << Status << endl;
        if (QUIC_FAILED(Status) && Connection != nullptr) {
            MSQuic->ConnectionClose(Connection);
        }
    }
    while (Active) {
        cout << Active << endl;
    }
    cout << "So... " << Status << endl;
	return;
}
