#pragma once

#include "EngineCore.h"
#include "Object/Object.h"
#include "Game/Player.h"
#include "Console.h"

class CNetworkManager;
class ISteamNetworkingSockets;
class CNetClient;

enum EHostType
{
	HOST_SERVER, // server and client
	HOST_DEDICATED,
	HOST_CLIENT
};

extern CConCmd cvNetConnect;
extern CConCmd cvNetHost;
extern CConCmd cvNetDisconnect;

extern ENGINE_API CNetworkManager* gNetworkManager;

class ENGINE_API CNetworkManager
{
	friend class P_CNetworkManager;

public:
	CNetworkManager() = default;

	void Update();

	bool Connect(const FString& ip);
	void Disconnect();

	bool Host(uint16 port);

	bool IsConnected() const;
	bool IsHost() const;

	// Register Object for networking
	void RegisterObject(CObject* obj);

	// Returns true if we're the server
	bool DeleteObject(CObject* obj);

public:
	// Object Network Functions
	void CallRPC_Client(CObject* obj, FFunction* function, FStack& stack); // Calls this function on the owning client
	void CallRPC_Server(CObject* obj, FFunction* function, FStack& stack); // Calls this function on the server
	void CallRPC_Mutlicast(CObject* obj, FFunction* function, FStack& stack); // Calls this function on all connected clients, including the server.

	void ObjectPropertyChanged(CObject* obj, FProperty* property);

private:
	void Server_OnUserConnect();
	void Server_OnUserDisconnect();

	void Client_OnConnect();
	void Client_OnDisconnect();

	// Handles finding/creating entities on the client
	void Client_RegisterEntity(FClass* type, SizeType entId, SizeType netId);

	void Init();

private:
	// Network ID - Object
	TMap<SizeType, TObjectPtr<CObject>> networkedObjects;

	// a list of all players, on the client this will only ever have one entry
	TArray<TObjectPtr<CPlayer>> players;

	TArray<CNetClient*> clients;

	ISteamNetworkingSockets* api = nullptr;
};
