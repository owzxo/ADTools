
#include "Network/HttpManager.h"
#include "ADTools.h"

#include "HttpPath.h"
#include "HttpRouteHandle.h"
#include "IHttpRouter.h"


void LaunchHttp()
{
	HttpServerInstance = &FHttpServerModule::Get();
	HttpRouter = HttpServerInstance->GetHttpRouter(HttpServerPort);
	const FHttpPath HttpPath(TEXT("/ADToolsHttpServer"));
	const FHttpRequestHandler RequestHandler = []
		(const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
	{
		for (auto QueryParam : Request.QueryParams)
		{
			UE_LOG(LogTemp, Log, TEXT("QueryParam: %s : %s"), *QueryParam.Key, *QueryParam.Value);
		}

		for (auto PathParam : Request.PathParams)
		{
			UE_LOG(LogTemp, Log, TEXT("PathParam: %s : %s"), *PathParam.Key, *PathParam.Value);
		}

		FString RequestAsFString = UTF8_TO_TCHAR(reinterpret_cast<const char*>(Request.Body.GetData()));
		UE_LOG(LogTemp, Log, TEXT("Body: %s"), *RequestAsFString);

		/*if (OnReceivedRequest.IsBound())
		{
			OnReceivedRequest.Broadcast(RequestAsFString);
		}*/

		OnComplete(FHttpServerResponse::Ok());
		
		return true;
	};
	
	HttpRouteHandle = HttpRouter->BindRoute(HttpPath, EHttpServerRequestVerbs::VERB_GET|EHttpServerRequestVerbs::VERB_POST, RequestHandler);

	HttpServerInstance->StartAllListeners();
	
}

void StopHttpListeners()
{
	HttpServerInstance->StopAllListeners();
	StopHttpServer();
}
void StartHttpServer()
{
	if (!HttpRouter)
	{
		HttpRouter = FHttpServerModule::Get().GetHttpRouter(HttpServerPort, /* bFailOnBindFailure = */ true);
		if (!HttpRouter)
		{
			UE_LOG(LogADTools, Error, TEXT("Web Remote Call server couldn't be started on port %d"), HttpServerPort);
			return;
		}

		for (FADToolsRoute& Route : RegisteredHttpRoutes)
		{
			StartRoute(Route);
		}

		const FHttpRequestHandler ValidationRequestHandler = FHttpRequestHandler([](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
		{
			TUniquePtr<FHttpServerResponse> Response = CreateHttpResponse();
			TArray<FString> ValueArray = {};
			if (Request.Headers.Find(PassphraseHeader))
			{
				ValueArray = Request.Headers[PassphraseHeader];
			}
			const FString Passphrase = !ValueArray.IsEmpty() ? ValueArray.Last() : "";
			if (!CheckPassphrase(Passphrase))
			{
				CreateUTF8ErrorMessage(FString::Printf(TEXT("Given Passphrase is not correct!")), Response->Body);
				Response->Code = EHttpServerResponseCodes::Denied;
				OnComplete(MoveTemp(Response));
				return true;
			}

			return false;
		});

		HttpRouter->RegisterRequestPreprocessor(ValidationRequestHandler);
		
		// Go through externally registered request pre-processors and register them with the http router.
		for (const TPair<FDelegateHandle, FHttpRequestHandler>& Handler : PreprocessorsToRegister)
		{
			// Find the pre-processors HTTP-handle from the one we generated.
			FDelegateHandle& Handle = PreprocessorsHandleMappings.FindChecked(Handler.Key);
			if (Handle.IsValid())
			{
				HttpRouter->UnregisterRequestPreprocessor(Handle);
			}

			// Update the preprocessor handle mapping.
			Handle = HttpRouter->RegisterRequestPreprocessor(Handler.Value);
		}

		FHttpServerModule::Get().StartAllListeners();

		/*bIsHttpServerRunning = true;
		OnHttpServerStartedDelegate.Broadcast(HttpServerPort);*/
	}
}
void StartRoute(const FADToolsRoute& Route)
{
	// The handler is wrapped in a lambda since HttpRouter::BindRoute only accepts TFunctions
	ActiveRouteHandles.Add(GetTypeHash(Route), HttpRouter->BindRoute(Route.Path, Route.Verb, [ Handler = Route.Handler](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) { return Handler.Execute(Request, OnComplete); }));
}

void RegisterRoutes()
{
	/*// Misc
	RegisterRoute({
		TEXT("Get information about different routes available on this API."),
		FHttpPath(TEXT("/remote/info")),
		EHttpServerRequestVerbs::VERB_GET,
		FRequestHandlerDelegate::CreateRaw(this, &FWebRemoteControlModule::HandleInfoRoute)
		});

	RegisterRoute({
		TEXT("Allows cross-origin http requests to the API."),
		FHttpPath(TEXT("/remote")),
		EHttpServerRequestVerbs::VERB_OPTIONS,
		FRequestHandlerDelegate::CreateRaw(this, &FWebRemoteControlModule::HandleOptionsRoute)
		});
		*/

	

}

void RegisterRoute(const FADToolsRoute& Route)
{
	RegisteredHttpRoutes.Add(Route);

	// If the route is registered after the server is already started.
	if (HttpRouter)
	{
		StartRoute(Route);
	}
}

void UnregisterRoute(const FADToolsRoute& Route)
{
	RegisteredHttpRoutes.Remove(Route);
	const uint32 RouteHash = GetTypeHash(Route);
	if (FHttpRouteHandle* Handle = ActiveRouteHandles.Find(RouteHash))
	{
		if (HttpRouter)
		{
			HttpRouter->UnbindRoute(*Handle);
		}
		ActiveRouteHandles.Remove(RouteHash);
	}
}

void StopHttpServer()
{
	if (FHttpServerModule::IsAvailable())
	{
		FHttpServerModule::Get().StopAllListeners();
	}

	if (HttpRouter)
	{
		for (const TPair<uint32, FHttpRouteHandle>& Tuple : ActiveRouteHandles)
		{
			if (Tuple.Key)
			{
				HttpRouter->UnbindRoute(Tuple.Value);
			}
		}

		ActiveRouteHandles.Reset();
	}

	HttpRouter.Reset();
	/*bIsHttpServerRunning = false;
	OnHttpServerStoppedDelegate.Broadcast();*/
}

TUniquePtr<FHttpServerResponse> CreateHttpResponse(EHttpServerResponseCodes InResponseCode)
{
	TUniquePtr<FHttpServerResponse> Response = MakeUnique<FHttpServerResponse>();
	AddCORSHeaders(Response.Get());
	AddContentTypeHeaders(Response.Get(), TEXT("application/json"));
	Response->Code = InResponseCode;
	return Response;
}
void AddCORSHeaders(FHttpServerResponse* InOutResponse)
{
	check(InOutResponse != nullptr);
	InOutResponse->Headers.Add(TEXT("Access-Control-Allow-Origin"), { TEXT("*") });
	InOutResponse->Headers.Add(TEXT("Access-Control-Allow-Methods"), { TEXT("PUT, POST, GET, OPTIONS") });
	InOutResponse->Headers.Add(TEXT("Access-Control-Allow-Headers"), { TEXT("Origin, X-Requested-With, Content-Type, Accept") });
	InOutResponse->Headers.Add(TEXT("Access-Control-Max-Age"), { TEXT("600") });
}
void AddContentTypeHeaders(FHttpServerResponse* InOutResponse, FString InContentType)
{
	InOutResponse->Headers.Add(TEXT("content-type"), { MoveTemp(InContentType) });
}
void CreateUTF8ErrorMessage(const FString& InMessage, TArray<uint8>& OutUTF8Message)
{
	ConvertToUTF8(FString::Printf(TEXT("{ \"errorMessage\": \"%s\" }"), *InMessage), OutUTF8Message);
}
void ConvertToUTF8(TConstArrayView<uint8> InTCHARPayload, TArray<uint8>& OutUTF8Payload)
{
	int32 StartIndex = OutUTF8Payload.Num();
	OutUTF8Payload.AddUninitialized(FPlatformString::ConvertedLength<UTF8CHAR>((TCHAR*)InTCHARPayload.GetData(), InTCHARPayload.Num() / sizeof(TCHAR)) * sizeof(ANSICHAR));
	FPlatformString::Convert((UTF8CHAR*)(OutUTF8Payload.GetData() + StartIndex), (OutUTF8Payload.Num() - StartIndex) / sizeof(ANSICHAR), (TCHAR*)InTCHARPayload.GetData(), InTCHARPayload.Num() / sizeof(TCHAR));
}
void ConvertToUTF8(const FString& InString, TArray<uint8>& OutUTF8Payload)
{
	int32 StartIndex = OutUTF8Payload.Num();
	OutUTF8Payload.AddUninitialized(FPlatformString::ConvertedLength<UTF8CHAR>(*InString, InString.Len()) * sizeof(ANSICHAR));
	FPlatformString::Convert((UTF8CHAR*)(OutUTF8Payload.GetData() + StartIndex), (OutUTF8Payload.Num() - StartIndex) / sizeof(ANSICHAR), *InString, InString.Len());
}

bool CheckPassphrase(const FString& HashedPassphrase)
{
	bool bOutResult = true;//!(GetMutableDefault<FADToolsRoute>()->bUseRemoteControlPassphrase);

	if (bOutResult)
	{
		return true;
	}

	TArray<FString> HashedPassphrases;//GetMutableDefault<FADToolsRoute>()->GetHashedPassphrases();
	if (HashedPassphrases.IsEmpty())
	{
		return true;
	}
	
	for (const FString& InPassphrase : HashedPassphrases)
	{
		bOutResult = bOutResult || InPassphrase == HashedPassphrase;

		if (bOutResult)
		{
			break;
		}
	}
	
	return bOutResult;
}
