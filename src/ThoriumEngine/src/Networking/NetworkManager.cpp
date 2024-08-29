
#include "NetworkManager.h"
#include "Engine.h"
#include "EngineCore.h"

#include "Steam/isteamnetworkingsockets.h"

class CNetClient
{
public:
	HSteamNetConnection conn;
};

class P_CNetworkManager
{
public:
	static void ConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info);
};

static HSteamListenSocket listenSocket;
static HSteamNetPollGroup pollGroup;

void P_CNetworkManager::ConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* info)
{

}

void CNetworkManager::Update()
{

}

void CNetworkManager::Init()
{
	if (api)
		return;

	api = SteamNetworkingSockets();
}

bool CNetworkManager::Connect(const FString& ip)
{

	return false;
}

void CNetworkManager::Disconnect()
{
	if (gIsServer)
	{
		for (auto* client : clients)
			api->CloseConnection(client->conn, 0, "Server Shutting down", true);

		clients.Clear();
		api->CloseListenSocket(listenSocket);
		api->DestroyPollGroup(pollGroup);
		listenSocket = k_HSteamListenSocket_Invalid;
		pollGroup = k_HSteamNetPollGroup_Invalid;
	}
}

bool CNetworkManager::Host(uint16 port)
{
	Init();

	SteamNetworkingIPAddr addr;
	addr.Clear();
	addr.m_port = port;

	SteamNetworkingConfigValue_t opt;
	opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)P_CNetworkManager::ConnectionStatusChanged);

	listenSocket = api->CreateListenSocketIP(addr, 1, &opt);
	THORIUM_ASSERT(listenSocket != k_HSteamListenSocket_Invalid, "[0] Failed to listen on port " + FString::ToString(port));

	pollGroup = api->CreatePollGroup();
	THORIUM_ASSERT(pollGroup != k_HSteamNetPollGroup_Invalid, "[1] Failed to listen on port " + FString::ToString(port));

	CONSOLE_LogInfo("CNetworkManager", "Hosting server on port " + FString::ToString(port));

	gIsServer = true;
	return true;
}

bool CNetworkManager::IsConnected() const
{
	return false;
}

bool CNetworkManager::IsHost() const
{
	return false;
}

void CNetworkManager::RegisterObject(CObject* obj)
{

}

bool CNetworkManager::DeleteObject(CObject* obj)
{
	return false;
}

void CNetworkManager::CallRPC_Client(CObject* obj, FFunction* function, FStack& stack)
{

}

void CNetworkManager::CallRPC_Server(CObject* obj, FFunction* function, FStack& stack)
{

}

void CNetworkManager::CallRPC_Mutlicast(CObject* obj, FFunction* function, FStack& stack)
{

}

void CNetworkManager::ObjectPropertyChanged(CObject* obj, FProperty* property)
{

}

void CNetworkManager::Server_OnUserConnect()
{

}

void CNetworkManager::Server_OnUserDisconnect()
{

}

void CNetworkManager::Client_OnConnect()
{

}

void CNetworkManager::Client_OnDisconnect()
{

}

void CNetworkManager::Client_RegisterEntity(FClass* type, SizeType entId, SizeType netId)
{

}
