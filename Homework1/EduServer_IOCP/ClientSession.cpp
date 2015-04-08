#include "stdafx.h"
#include "Exception.h"
#include "EduServer_IOCP.h"
#include "ClientSession.h"
#include "IocpManager.h"
#include "SessionManager.h"

bool ClientSession::OnConnect(SOCKADDR_IN* addr)
{
	//TODO: �� ���� lock���� ��ȣ �� ��
    WaitForSingleObject(mLock, INFINITE);
	CRASH_ASSERT(LThreadType == THREAD_MAIN_ACCEPT);

	/// make socket non-blocking
	u_long arg = 1 ;
	ioctlsocket(mSocket, FIONBIO, &arg) ;

	/// turn off nagle
	int opt = 1 ;
	setsockopt(mSocket, IPPROTO_TCP, TCP_NODELAY, (const char*)&opt, sizeof(int)) ;

	opt = 0;
	if (SOCKET_ERROR == setsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&opt, sizeof(int)) )
	{
		printf_s("[DEBUG] SO_RCVBUF change error: %d\n", GetLastError()) ;
		return false;
	}
	
	HANDLE handle = 0; //TODO: ���⿡�� CreateIoCompletionPort((HANDLE)mSocket, ...);����Ͽ� ������ ��
    handle = CreateIoCompletionPort((HANDLE)mSocket, GIocpManager->GetComletionPort(), (ULONG_PTR)this, 0);

	if (handle != GIocpManager->GetComletionPort())
	{
		printf_s("[DEBUG] CreateIoCompletionPort error: %d\n", GetLastError());
		return false;
	}

	memcpy(&mClientAddr, addr, sizeof(SOCKADDR_IN));
	mConnected = true ;

	printf_s("[DEBUG] Client Connected: IP=%s, PORT=%d\n", inet_ntoa(mClientAddr.sin_addr), ntohs(mClientAddr.sin_port));

	GSessionManager->IncreaseConnectionCount();

    ReleaseMutex(mLock);

	return PostRecv() ;
}

void ClientSession::Disconnect(DisconnectReason dr)
{
	//TODO: �� ���� lock���� ��ȣ�� ��
    WaitForSingleObject(mLock, INFINITE);

	if ( !IsConnected() )
		return ;
	
	LINGER lingerOption ;
	lingerOption.l_onoff = 1;
	lingerOption.l_linger = 0;

	/// no TCP TIME_WAIT
	if (SOCKET_ERROR == setsockopt(mSocket, SOL_SOCKET, SO_LINGER, (char*)&lingerOption, sizeof(LINGER)) )
	{
		printf_s("[DEBUG] setsockopt linger option error: %d\n", GetLastError());
	}

	printf_s("[DEBUG] Client Disconnected: Reason=%d IP=%s, PORT=%d \n", dr, inet_ntoa(mClientAddr.sin_addr), ntohs(mClientAddr.sin_port));
	
	GSessionManager->DecreaseConnectionCount();

	closesocket(mSocket) ;

    ReleaseMutex(mLock);

	mConnected = false ;
}

bool ClientSession::PostRecv() const
{
	if (!IsConnected())
		return false;

	OverlappedIOContext* recvContext = new OverlappedIOContext(this, IO_RECV);

	//TODO: WSARecv ����Ͽ� ������ ��
    DWORD recvSize = 0;
    DWORD flag = 0;
    recvContext->mWsaBuf.buf = recvContext->mBuffer;
    recvContext->mWsaBuf.len = BUFSIZE;

    memset(&(recvContext->mOverlapped), 0, sizeof(OVERLAPPED));

    int ret = WSARecv(mSocket, &(recvContext->mWsaBuf), 1, &recvSize, &flag, (LPWSAOVERLAPPED)recvContext, NULL);
    if (ret == SOCKET_ERROR)
    {
        int error = WSAGetLastError();
        if (error != ERROR_IO_PENDING)
        {
            printf("WSARecv error : %d \n", error);
            return false;
        }
    }

    printf("Recv - size : %d, socket : %d\n", recvSize, mSocket);

	return true;
}

bool ClientSession::PostSend(const char* buf, int len) const
{
	if (!IsConnected())
		return false;

	OverlappedIOContext* sendContext = new OverlappedIOContext(this, IO_SEND);

	/// copy for echoing back..
	memcpy_s(sendContext->mBuffer, BUFSIZE, buf, len);

	//TODO: WSASend ����Ͽ� ������ ��
    DWORD sendSize = 0;
    DWORD flag = 0;

    sendContext->mWsaBuf.buf = sendContext->mBuffer;
    sendContext->mWsaBuf.len = len;

    int ret = WSASend(mSocket, &(sendContext->mWsaBuf), 1, &sendSize, flag, (LPWSAOVERLAPPED)sendContext, 0);

    if (ret == SOCKET_ERROR)
    {
        int error = WSAGetLastError();
        if (error != ERROR_IO_PENDING)
        {
            printf("SendError\n");
            return false;
        }
    }

    printf("send - size : %d, socket : %d\n", sendSize, mSocket);

	return true;
}