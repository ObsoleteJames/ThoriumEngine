
#include "NetworkManager.h"
#include "Engine.h"

void CNetworkManager::Update()
{

}

bool CNetworkManager::Connect(const FString& ip)
{

}

bool CNetworkManager::Disconnect()
{

}

bool CNetworkManager::Host(const FString& port)
{

}

bool CNetworkManager::IsConnected() const
{

}

bool CNetworkManager::IsHost() const
{

}

void CNetworkManager::RegisterObject(CObject* obj)
{

}

bool CNetworkManager::DeleteObject(CObject* obj)
{

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

void CNetworkManager::Server_OnUserDissconnect()
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
