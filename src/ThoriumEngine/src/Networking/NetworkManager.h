#pragma once

#include "EngineCore.h"
#include "Object/Object.h"
#include "Game/Player.h"

class CNetworkManager;

enum EHostType
{
	HOST_SERVER, // server and client
	HOST_DEDICATED, // 
	HOST_CLIENT
};

extern ENGINE_API CNetworkManager* gNetworkManager;

class ENGINE_API CNetworkManager
{
public:
	CNetworkManager() = default;

	void Update();

	bool Connect(const FString& ip);
	bool Disconnect();

	bool Host(const FString& port);

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
	void Server_OnUserDissconnect();

	void Client_OnConnect();
	void Client_OnDisconnect();

	// Handles finding/creating entities on the client
	void Client_RegisterEntity(FClass* type, SizeType entId, SizeType netId);

private:
	// Network ID - Object
	TMap<SizeType, TObjectPtr<CObject>> networkedObjects;

	// a list of all players, on the client this will only ever by 1
	TArray<TObjectPtr<CPlayer>> players;
};