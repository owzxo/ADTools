#pragma once

#include "Misc/ObjectThumbnail.h"
#include "UObject/Object.h"
#include "Runtime/Launch/Resources/Version.h"
#include "MaterialStatsCommon.h"
#include "Serialization/JsonSerializer.h"
class FJsonObject;

void GetAssetLibraryData(const FString& DataTypeName);

/** bPerformHeavyOperations to load the actual object and get higher res thumbnails, pivot offset, material paths, etc */
TSharedPtr<FJsonObject> GetStaticMeshAssetData(const FAssetData& AssetData,bool bPreformHeavyOperations = true);
/** bPerformHeavyOperations to load the actual object and get higher res thumbnails, pivot offset, material paths, etc */
TSharedPtr<FJsonObject> GetMaterialAssetData(const FAssetData& AssetData, const bool bPerformHeavyOperations = true);
/** bPerformHeavyOperations to load the actual object and get higher res thumbnails, pivot offset, material paths, etc */
TSharedPtr<FJsonObject> GetMaterialInstanceAssetData(const FAssetData& AssetData, const bool bPerformHeavyOperations = true);
/** bPerformHeavyOperations to load the actual object and get higher res thumbnails, pivot offset, material paths, etc */
TSharedPtr<FJsonObject> GetTextureAssetData(const FAssetData& AssetData, const bool bPerformHeavyOperations = true);
/** bPerformHeavyOperations to load the actual object and get higher res thumbnails, pivot offset, material paths, etc */
TSharedPtr<FJsonObject> GetLevelAssetData(const FAssetData& AssetData, const bool bPerformHeavyOperations = true);
/** bPerformHeavyOperations to load the actual object and get higher res thumbnails, pivot offset, material paths, etc */
TSharedPtr<FJsonObject> GetBlueprintAssetData(const FAssetData& AssetData, const bool bPerformHeavyOperations = true);
TSharedPtr<FJsonObject> GetUnknownAssetData();
TArray<FString> GetCurrentLevelArtAssets();

const FString OutFolderPath = "C:/PrometheanAITemp/";
constexpr bool bGenerateHighResThumbnails = false;
constexpr int32 RenderedThumbnailSize = 512;

/**
 *	Save FObjectThumbnail to a filename. TODO: there is seemingly a Gamma issue happening that could use some investigating
 *	@return
 */
bool SaveThumbnail(FObjectThumbnail* ObjectThumbnail, const FString& ImagePath, const bool bIsCompressed = true);

// --------------------------------------------------------------------
// --- Working around necessary data being private. 
// --------------------------------------------------------------------
// structure used to store various statistics extracted from compiled shaders
// original definition is private in MaterialStats.h so copying it here for now
//TODO: future proof and find a way to tie it back to source


#if ENGINE_MAJOR_VERSION >= 4  && ENGINE_MINOR_VERSION > 19  // MaterialStatsCommon missing on 4.19 anyway so compiling out for now
/** structure used to store various statistics extracted from compiled shaders */
struct FShaderStatsInfo
{
	struct FContent
	{
		FString StrDescription;
		FString StrDescriptionLong;
	};

	TMap<ERepresentativeShader, FContent> ShaderInstructionCount;
	FContent SamplersCount;
	FContent InterpolatorsCount;
	FContent TextureSampleCount;
	FContent VirtualTextureLookupCount;
	FString StrShaderErrors;

	void Reset()
	{
		ShaderInstructionCount.Empty();

		SamplersCount.StrDescription = TEXT("Compiling...");
		SamplersCount.StrDescriptionLong = TEXT("Compiling...");

		InterpolatorsCount.StrDescription = TEXT("Compiling...");
		InterpolatorsCount.StrDescriptionLong = TEXT("Compiling...");

		TextureSampleCount.StrDescription = TEXT("Compiling...");
		TextureSampleCount.StrDescriptionLong = TEXT("Compiling...");

		VirtualTextureLookupCount.StrDescription = TEXT("Compiling...");
		VirtualTextureLookupCount.StrDescriptionLong = TEXT("Compiling...");

		StrShaderErrors.Empty();
	}

	void Empty()
	{
		ShaderInstructionCount.Empty();

		SamplersCount.StrDescription.Empty();
		SamplersCount.StrDescriptionLong.Empty();

		InterpolatorsCount.StrDescription.Empty();
		InterpolatorsCount.StrDescriptionLong.Empty();

		TextureSampleCount.StrDescription.Empty();
		TextureSampleCount.StrDescriptionLong.Empty();

		VirtualTextureLookupCount.StrDescription.Empty();
		VirtualTextureLookupCount.StrDescriptionLong.Empty();

		StrShaderErrors.Empty();
	}

	bool HasErrors()
	{
		return !StrShaderErrors.IsEmpty();
	}
};
# endif
