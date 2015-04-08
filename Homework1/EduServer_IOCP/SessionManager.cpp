#include "stdafx.h"
#include "ClientSession.h"
#include "SessionManager.h"

SessionManager* GSessionManager = nullptr;

SessionManager::SessionManager()
    : mCurrentConnectionCount(0)
{
    mLock = CreateMutex(NULL, FALSE, NULL);
}

ClientSession* SessionManager::CreateClientSession(SOCKET sock)
{
	ClientSession* client = new ClientSession(sock);

	//TODO: lock으로 보호할 것
    WaitForSingleObject(mLock, INFINITE);
	{
		mClientList.insert(ClientList::value_type(sock, client));
	}
    ReleaseMutex(mLock);

	return client;
}


void SessionManager::DeleteClientSession(ClientSession* client)
{
	//TODO: lock으로 보호할 것
    WaitForSingleObject(mLock, INFINITE);
	{
		mClientList.erase(client->mSocket);
	}
    ReleaseMutex(mLock);

	delete client;
}

SessionManager::~SessionManager()
{
    CloseHandle(mLock);
}

