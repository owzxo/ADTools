#pragma once
#include "Sockets.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"





FSocket* ListenerSocket;
FSocket* ConnectionSocket;
FIPv4Endpoint RemoteAddressForConnection;

FTimerHandle TCPSocketListenerTimerHandle;
FTimerHandle TCPConnectionListenerTimerHandle;


const int32 ADToolsPort=62;
const int32 ADToolsBrowserPort = 1312;
const int32 ADToolsCommandLinePort = 1313;
const int32 ADToolsAssetDataPort = 1315;

TArray<FString> GlobalCOmmandStrArray;


bool isCommandExecuting = false;
//心跳,间隔时间.检测链接是否断开.
const float GLOBAL_SERVER_HEARTBEAT = 0.001f;




bool LaunchTCP();

void TCPSocketListener();
void TCPConnectionListener();
bool StartTCPReceiver(
	const FString& SocketName,
	const FString& TheIP,
	const int32 ThePort
	
);
FSocket* CreateTCPListenerSocket(
	const FString& SocketName,
	const FString& TheIP,
	const int32 ThePort,
	const int32 ReceiveBufferSize = 2*1024*1024
);
void TCPCloseConnection();
void TCPSend(FString ToSend);
void ExecuteTCPCommand(FString MetaCommandStr);
FString StringFromBinaryArray(TArray<uint8> BinaryArray);
//Format String IP4 to number array
bool FormatIP4ToNumber(const FString& TheIP,uint8(&Out)[4]);
