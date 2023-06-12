#pragma once

#include "HttpManager.h"
#include "HttpRouteHandle.h"
#include "HTTPServer/Public/HttpServerModule.h"

DECLARE_DELEGATE_RetVal_TwoParams(bool, FRequestHandlerDelegate, const FHttpServerRequest&, const FHttpResultCallback&);

struct FADToolsRoute
{
 FADToolsRoute(FString InRouteDescription, FHttpPath InPath, EHttpServerRequestVerbs InVerb, FRequestHandlerDelegate InHandler)
  : RouteDescription(MoveTemp(InRouteDescription))
  , Path(MoveTemp(InPath))
  , Verb(InVerb)
  , Handler(MoveTemp(InHandler))
 {
 }
 /** A description of how the route should be used. */
 FString RouteDescription;
 /** Relative path (ie. /remote/object) */
 FHttpPath Path;
 /** The desired HTTP verb (ie. GET, PUT..) */
 EHttpServerRequestVerbs Verb = EHttpServerRequestVerbs::VERB_GET;
 /** The handler called when the route is accessed. */
 FRequestHandlerDelegate Handler;

 friend uint32 GetTypeHash(const FADToolsRoute& Route) { return HashCombine(GetTypeHash(Route.Path), GetTypeHash(Route.Verb)); }
 friend bool operator==(const FADToolsRoute& LHS, const FADToolsRoute& RHS) { return LHS.Path == RHS.Path && LHS.Verb == RHS.Verb; }
};

static const TCHAR* WrappedRequestHeader = TEXT("UE-Wrapped-Request");
static const FString PassphraseHeader = TEXT("Passphrase");

const uint32 HttpServerPort = 8888;
const uint32 InvalidHttpRouterPort = TNumericLimits<uint16>::Max() + 1; // 65536

FHttpServerModule* HttpServerInstance;
TSharedPtr<IHttpRouter> HttpRouter;
FHttpRouteHandle HttpRouteHandle;
/** Mapping of routes to delegate handles */
TMap<uint32, FHttpRouteHandle> ActiveRouteHandles;
/** Set of routes that will be activated on http server start. */
TSet<FADToolsRoute> RegisteredHttpRoutes;

/** Whether the HTTP server has been started and has not been stopped. */
bool bIsHttpServerRunning = false;

/** List of preprocessor delegates that need to be registered when the server is started. */
TMap<FDelegateHandle, FHttpRequestHandler> PreprocessorsToRegister;

/**
  * Mappings of preprocessors delegate handles generated from the WebRC module to the ones generated from the Http Module.
  */
TMap<FDelegateHandle, FDelegateHandle> PreprocessorsHandleMappings;

void LaunchHttp();

/** Bind the route in the http router and add it to the list of active routes. */
void StartRoute(const FADToolsRoute& Route) ;

/** Register HTTP and Websocket routes. */
void RegisterRoutes();

/**
	 * Register a route to the API.
	 * @param Route The route to register.
	 */
void RegisterRoute(const FADToolsRoute& Route);

/**
 * Unregister a route to the API.
 * @param Route The route to unregister.
 */
void UnregisterRoute(const FADToolsRoute& Route);

/**
 * Start the web control server
 */
void StartHttpServer();

/**
 * Stop the web control server.
 */
void StopHttpServer();
TUniquePtr<FHttpServerResponse> CreateHttpResponse(EHttpServerResponseCodes InResponseCode = EHttpServerResponseCodes::BadRequest);

void AddCORSHeaders(FHttpServerResponse* InOutResponse);
void AddContentTypeHeaders(FHttpServerResponse* InOutResponse, FString InContentType);
void AddContentTypeHeaders(FHttpServerResponse* InOutResponse, FString InContentType);
void CreateUTF8ErrorMessage(const FString& InMessage, TArray<uint8>& OutUTF8Message);
void ConvertToUTF8(TConstArrayView<uint8> InTCHARPayload, TArray<uint8>& OutUTF8Payload);
void ConvertToUTF8(const FString& InString, TArray<uint8>& OutUTF8Payload);
/** Checking ApiKey using Md5. */
bool CheckPassphrase(const FString& HashedPassphrase);
