#include "Network/TCPManager.h"
#include "ADTools.h"
#include "ADToolsGeneric.h"
#include "Common/TcpSocketBuilder.h"


bool LaunchTCP()
{

	//关闭链接,预防之前已经被打开.
	TCPCloseConnection();
	UE_LOG(LogADTools,Display,TEXT("---------->创建TCP链接..."));
	if(!StartTCPReceiver("ADToolsSocketListener","127.0.0.1",ADToolsPort))
	{
		UE_LOG(LogADTools,Display,TEXT("---------->TCP 链接失败"))
		return false;
	}
	UE_LOG(LogADTools,Display,TEXT("---------->TCP 链接成功"))
	return true;
}

bool StartTCPReceiver(const FString& SocketName, const FString& TheIP, const int32 ThePort)
{
	ListenerSocket  = CreateTCPListenerSocket(SocketName,TheIP,ThePort);
	if(!ListenerSocket)
	{
		UE_LOG(LogADTools,Error,TEXT("ADToolsTCPReceiver>> 无法创建Socket监听! ~> %s %d"), *TheIP, ThePort);
		if(GEngine !=nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1,5.0f,FColor::Red,FString::Printf(TEXT("ADTools 插件链接失败!")));
		}
		return false;
	}
	//开始监听....
	FTimerDelegate TimerCallback;
	TimerCallback.BindLambda([]
	{
		TCPConnectionListener();
	});
	GEditor->GetTimerManager()->SetTimer(TCPConnectionListenerTimerHandle,TimerCallback,GLOBAL_SERVER_HEARTBEAT,true);
	GEditor->AddOnScreenDebugMessage(-1,7.f,FColor::Green,FString::Printf(TEXT("ADTools 插件已经准备就绪!")));
	return true;
	
}

FSocket* CreateTCPListenerSocket(const FString& SocketName, const FString& TheIP, const int32 ThePort, const int32 ReceiveBufferSize)
{
	uint8 IPP4Nums[4];
	if(!FormatIP4ToNumber(TheIP,IPP4Nums))
	{
		return nullptr;
	}

	const FIPv4Endpoint Endpoint(FIPv4Address(IPP4Nums[0],IPP4Nums[1],IPP4Nums[2],IPP4Nums[3]),ThePort);

	FSocket* ListenSocket = FTcpSocketBuilder(*SocketName).AsReusable().BoundToEndpoint(Endpoint).Listening(8);
	if(!ListenSocket)
	{
		UE_LOG(LogADTools,Display,TEXT("通过配置文件创建Socket链接失败!"))
		return nullptr;
	}

	//Set Buffer Size
	int32 NewSize = 0;
	ListenSocket->SetReceiveBufferSize(ReceiveBufferSize,NewSize);
	return ListenSocket;
}

void TCPConnectionListener()
{
	if(!ListenerSocket)return;
	const TSharedRef<FInternetAddr> RemoteAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	bool Pending;
	ListenerSocket->HasPendingConnection(Pending);
	if(Pending)
	{
		if(ConnectionSocket)
		{
			ConnectionSocket->Close();
			ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ConnectionSocket);
		}
		ConnectionSocket=ListenerSocket->Accept(*RemoteAddress,TEXT("接收到Socket链接"));
		if(ConnectionSocket!=nullptr)
		{
			RemoteAddressForConnection = FIPv4Endpoint(RemoteAddress);
			FTimerDelegate TimerCallback;
			TimerCallback.BindLambda([]
			{
				TCPSocketListener();
			});
			GEditor->GetTimerManager()->SetTimer(TCPSocketListenerTimerHandle,TimerCallback,GLOBAL_SERVER_HEARTBEAT,true,0.0f);
		}
	}
}

void TCPSocketListener()
{
	if(!ConnectionSocket)return;
	TArray<uint8> ReceivedData;
	uint32 Size;
	while(ConnectionSocket->HasPendingData(Size))
	{
		ReceivedData.Init(FMath::Min(Size,65507u),Size);
		int32 Read = 0;
		ConnectionSocket->Recv(ReceivedData.GetData(),ReceivedData.Num(),Read);
	}
	if(ReceivedData.Num()<=0)return;
	const FString ReceivedUE4String = StringFromBinaryArray(ReceivedData);
	ExecuteTCPCommand(ReceivedUE4String);
}


void TCPCloseConnection()
{
	const UWorld* World = GetEditorWorld();

	World->GetTimerManager().ClearTimer(TCPConnectionListenerTimerHandle);
	World->GetTimerManager().ClearTimer(TCPSocketListenerTimerHandle);
	
	if(ConnectionSocket!=nullptr)
	{
		ConnectionSocket->Close();	
	}
	if(ListenerSocket!=nullptr)
	{
		ListenerSocket->Close();
	}
	
}

void TCPSend(FString ToSend)
{
	if(ConnectionSocket==nullptr)
	{
		return;
	}
	if(ToSend.IsEmpty())
	{
		ToSend = FString("None");
	}
	const int32 DestLen = TStringConvert<TCHAR,ANSICHAR>::ConvertedLength(*ToSend,ToSend.Len());
	uint8* Dest = new uint8[DestLen+1];
	TStringConvert<TCHAR,ANSICHAR>::Convert(reinterpret_cast<ANSICHAR*>(Dest),DestLen,*ToSend,ToSend.Len());
	Dest[DestLen] = '\0';
	int32 BytesSent = 0;

	if(!ConnectionSocket->Send(Dest,DestLen,BytesSent))
	{
		GEngine->AddOnScreenDebugMessage(-1,5.f,FColor::Red,FString::Printf(TEXT("消息发送错误!")));
	}
}



bool FormatIP4ToNumber(const FString& TheIP, uint8 (& Out)[4])
{
	TArray<FString> Parts;
	TheIP.ParseIntoArray(Parts,TEXT("."),true);
	if(Parts.Num()!=4)return false;

	for (int32 i=0;i<4;i++)
	{
		Out[i] = FCString::Atoi(*Parts[i]);
	}
	return true;
}


FString StringFromBinaryArray(TArray<uint8> BinaryArray)
{
	//Create a string from a byte array!
	const std::string Cstr(reinterpret_cast<const char*>(BinaryArray.GetData()),BinaryArray.Num());
	return FString(Cstr.c_str());
}
