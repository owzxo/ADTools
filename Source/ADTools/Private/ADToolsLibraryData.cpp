#include "ADToolsLibraryData.h"

#include "ADTools.h"
#include "ADToolsGeneric.h"
#include "UObject/Object.h"
#include "Engine/Texture2D.h"
#include "EngineUtils.h"
#include "Engine/StaticMesh.h"
#include "MaterialStatsCommon.h"
#include "Materials/Material.h"
#include "Misc/FileHelper.h" 
#include "Materials/MaterialInstanceConstant.h"
#include "ImageUtils.h"
#include "ObjectTools.h"
#include "TextureCompiler.h"
#include "UnrealEdGlobals.h"
#include "AutoReimport/AssetSourceFilenameCache.h"
#include "Dom/JsonObject.h"
#include "Editor/UnrealEdEngine.h"
#include "EditorFramework/AssetImportData.h"
#include "Engine/ObjectLibrary.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "ImageCore.h"
#include "Misc/ScopedSlowTask.h"


#include "ThumbnailRendering/ThumbnailManager.h"

// --------------------------------------------------------------------
// --- ASSET LIBRARY FUNCTIONS
// --------------------------------------------------------------------

DECLARE_STATS_GROUP(TEXT("Promethean_AI"), STATGROUP_PrometheanLibraryData, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Promethean - SaveThumbnail"), STAT_SaveThumbnail, STATGROUP_PrometheanLibraryData);

/** Generates a thumbnail for the specified object and caches it */
FObjectThumbnail* GenerateHighResolutionThumbnail( UObject* InObject )
{
	return NULL;
	if constexpr (!bGenerateHighResThumbnails)
	{
		return nullptr;
	}
	
	// Does the object support thumbnails?
	FThumbnailRenderingInfo* RenderInfo = GUnrealEd ? GUnrealEd->GetThumbnailManager()->GetRenderingInfo( InObject ) : nullptr;
	if( RenderInfo != NULL && RenderInfo->Renderer != NULL )
	{
		// Set the size of cached thumbnails
		const int32 ImageWidth = RenderedThumbnailSize;
		const int32 ImageHeight = RenderedThumbnailSize;

		// For cached thumbnails we want to make sure that textures are fully streamed in so that the thumbnail we're saving won't have artifacts
		// However, this can add 30s - 100s to editor load
		//@todo - come up with a cleaner solution for this, preferably not blocking on texture streaming at all but updating when textures are fully streamed in
		ThumbnailTools::EThumbnailTextureFlushMode::Type TextureFlushMode = ThumbnailTools::EThumbnailTextureFlushMode::NeverFlush;

#if ENGINE_MAJOR_VERSION >= 5
		if ( UTexture* Texture = Cast<UTexture>(InObject) )
		{
			FTextureCompilingManager::Get().FinishCompilation({Texture});
			Texture->WaitForStreaming();
		}
#endif

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1
		// When generating a material thumbnail to save in a package, make sure we finish compilation on the material first
		if ( UMaterial* InMaterial = Cast<UMaterial>(InObject) )
		{
			FScopedSlowTask SlowTask(0, NSLOCTEXT( "ObjectTools", "FinishingCompilationStatus", "Finishing Shader Compilation..." ) );
			SlowTask.MakeDialog();

			// Block until the shader maps that we will save have finished being compiled
			FMaterialResource* CurrentResource = InMaterial->GetMaterialResource(GMaxRHIFeatureLevel);
			if (CurrentResource)
			{
				if (!CurrentResource->IsGameThreadShaderMapComplete())
				{
					CurrentResource->SubmitCompileJobs_GameThread(EShaderCompileJobPriority::High);
				}
				CurrentResource->FinishCompilation();
			}
		}
#endif

		// Generate the thumbnail
		FObjectThumbnail NewThumbnail;
		ThumbnailTools::RenderThumbnail(
			InObject, ImageWidth, ImageHeight, TextureFlushMode, NULL,
			&NewThumbnail );		// Out

		UPackage* MyOutermostPackage = InObject->GetOutermost();
		return ThumbnailTools::CacheThumbnail( InObject->GetFullName(), &NewThumbnail, MyOutermostPackage );
	}

	return NULL;
}

// start of this is copied from https://forums.unrealengine.com/development-discussion/c-gameplay-programming/76797-how-to-save-utexture2d-to-png-file
// but repurposed to work with object thumbnail
bool SaveThumbnail(FObjectThumbnail* ObjectThumbnail, const FString& ImagePath, const bool bIsCompressed)
{
	if (!ObjectThumbnail || ObjectThumbnail->IsEmpty())
	{
		return false;
	}

	SCOPE_CYCLE_COUNTER(STAT_SaveThumbnail);
    
	const int32 w = ObjectThumbnail->GetImageWidth();
	const int32 h = ObjectThumbnail->GetImageHeight();

	if (bIsCompressed)
	{
		ObjectThumbnail->DecompressImageData();
	}
	const TArray<uint8>& ImageData = ObjectThumbnail->AccessImageData();

	const FImageView ImageView( (void*)ImageData.GetData(), h, w, ERawImageFormat::BGRA8);
	const bool bSaved = FImageUtils::SaveImageByExtension(*ImagePath, ImageView);

	UE_LOG(LogADTools, Display, TEXT("ThumbnailSize: %d %d"), w, h);
	UE_LOG(LogADTools, Display, TEXT("SaveThumbnail: %s %d"), *ImagePath, bSaved == true ? 1 : 0);
	return bSaved;
}

void GetAssetLibraryData(const FString& DataTypeName)
{
	UE_LOG(LogADTools, Display, TEXT("Getting asset library data..."));
	
	UClass* cls = TSubclassOf<class UObject>();
	
	UObjectLibrary* ObjectLibrary = UObjectLibrary::CreateLibrary(cls, false, GIsEditor);
	
	if (ObjectLibrary != nullptr)
	{
		ObjectLibrary->AddToRoot();

		const FString NewPath = TEXT("/Game");  // "/Game/StarterContent/Architecture"
		int32 NumOfAssetDatas = ObjectLibrary->LoadAssetDataFromPath(NewPath);

		TArray<FAssetData> AssetDatas;
		ObjectLibrary->GetAssetDataList(AssetDatas);

		UE_LOG(LogADTools, Display, TEXT("About to go through all asset datas... Datas Num: %i"), AssetDatas.Num());
		
		TArray<TSharedPtr<FJsonValue>> JSONDictArray;  // for output
		for (int32 i = 0; i < AssetDatas.Num(); ++i)  // can't print AssetDatas.Num() - crashes!
		{
			FAssetData& AssetData = AssetDatas[i];
			if (AssetData.IsValid())
			{
				// currently get only all static meshes
				if (AssetData.AssetClassPath.GetAssetName().IsEqual(*DataTypeName))
				{									
					TSharedPtr<FJsonObject> JSONDict = MakeShared<FJsonObject>();
					if (DataTypeName == "MaterialInstanceConstant")
					{
						JSONDict = GetMaterialInstanceAssetData(AssetData);
					}
					else if (DataTypeName == "Material")
					{
						JSONDict = GetMaterialAssetData(AssetData);
					}
					else if (DataTypeName == "StaticMesh")
					{
						JSONDict = GetStaticMeshAssetData(AssetData);
					}
					else if (DataTypeName == "Texture2D")
					{
						JSONDict = GetTextureAssetData(AssetData);
					}
					else if (DataTypeName == "World")
					{
						// JSONDict = getLevelAssetData(AssetData);
					}
					TSharedRef<FJsonValueObject> JSONDictValue = MakeShared<FJsonValueObject>(JSONDict);
					UE_LOG(LogADTools, Display, TEXT("================================================"));
					// -- Add To Array
					JSONDictArray.Add(JSONDictValue);
				}				
			}
		}
		// output json data to string
		const TSharedPtr<FJsonObject> RootObject = MakeShared<FJsonObject>();
		RootObject->SetArrayField(TEXT("asset_data"), JSONDictArray);
		FString OutputString;
		const TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&OutputString);
		FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);
		SaveToFile(OutFolderPath + "/ue4_asset_data_" + DataTypeName + ".json", OutputString);
	}
}

DECLARE_CYCLE_STAT(TEXT("Promethean - GetStaticMeshAssetData"), STAT_GetStaticMeshAssetData, STATGROUP_PrometheanLibraryData);
DECLARE_CYCLE_STAT(TEXT("Promethean - ExtractAssetImportInfo"), STAT_ExtractAssetImportInfo, STATGROUP_PrometheanLibraryData);
DECLARE_CYCLE_STAT(TEXT("Promethean - LoadObjectStuff"), STAT_LoadObjectStuff, STATGROUP_PrometheanLibraryData);
TSharedPtr<FJsonObject> GetStaticMeshAssetData(const FAssetData& AssetData, bool bPerformHeavyOperations)
{
	SCOPE_CYCLE_COUNTER(STAT_GetStaticMeshAssetData);
	TSharedPtr<FJsonObject> JsonDict = MakeShared<FJsonObject>();	
	TMap<FName,FString> AllTags;
	if (AssetData.AssetClassPath.GetAssetName().IsEqual("StaticMesh"))

	{
		// ---- Unique ID - Name
    	const FString AssetPath = AssetData.GetSoftObjectPath().ToString();
    	JsonDict->SetStringField(TEXT("path"), AssetPath);
    	FString FileName = AssetPath;
    	FileName = FileName.Replace(TEXT("/"), TEXT("--"));
    	FString OutputPath = OutFolderPath + "/thumbnails/" + FileName;  // define path globally because it's reused for triangle too
    	if (bPerformHeavyOperations)
    	{
    		SCOPE_CYCLE_COUNTER(STAT_LoadObjectStuff);
    		// ---- WARNING! Have to load the actual StaticMesh here. Need it to get material paths and pivot offset
    		UStaticMesh* StaticMesh = Cast<UStaticMesh>(AssetData.GetAsset());
    		// ---- High Res Thumbs
    		GenerateHighResolutionThumbnail(StaticMesh);		
    		// ---- Material Paths
    		TArray<TSharedPtr<FJsonValue>> MaterialPathArray;
    		for (FStaticMaterial StaticMaterial : StaticMesh->GetStaticMaterials())
    		{
    			//UE_LOG(LogPromethean, Display, TEXT("Material Used: %s"), *StaticMaterial.MaterialInterface->GetPathName());  // gets material slots on the static mesh
    			MaterialPathArray.Add(MakeShareable(new FJsonValueString(StaticMaterial.MaterialInterface->GetPathName())));
    		}
    		JsonDict->SetArrayField(TEXT("material_paths"), MaterialPathArray);
    		// ---- Size (Instead of approx size, update bounding box when object is loaded
    		//UE_LOG(LogPromethean, Display, TEXT("Size: %s"), *StaticMesh->GetBoundingBox().GetSize().ToString());
    		FVector Size = StaticMesh->GetBoundingBox().GetSize();
    		TArray<TSharedPtr<FJsonValue>> MeshScaleArray;
    		MeshScaleArray.Add(MakeShareable(new FJsonValueNumber(Size.X)));
    		MeshScaleArray.Add(MakeShareable(new FJsonValueNumber(Size.Y)));
    		MeshScaleArray.Add(MakeShareable(new FJsonValueNumber(Size.Z)));
    		JsonDict->SetArrayField(TEXT("bounding_box"), MeshScaleArray);		
    		// ---- Pivot Offset
    		FVector PivotOffset = StaticMesh->GetBoundingBox().GetCenter();		
    		PivotOffset[2] = StaticMesh->GetBoundingBox().Min[2];  // min height
    		TArray<TSharedPtr<FJsonValue>> MeshPivotOffsetArray;
    		MeshPivotOffsetArray.Add(MakeShareable(new FJsonValueNumber(-PivotOffset.X)));
    		MeshPivotOffsetArray.Add(MakeShareable(new FJsonValueNumber(-PivotOffset.Y)));
    		MeshPivotOffsetArray.Add(MakeShareable(new FJsonValueNumber(-PivotOffset.Z)));
    		JsonDict->SetArrayField(TEXT("pivot_offset"), MeshPivotOffsetArray);
    		
    		// ---- Raw Triangles
    		TArray<FVector> Vertexes = GetMeshTrianglePositionsFromLibraryStaticMesh(StaticMesh);
    		if (Vertexes.Num() != 0)
    		{
    			FString VertexString = FVectorArrayToJsonString(Vertexes, TEXT("verts"));
    			SaveToFile(OutputPath, VertexString);  // no file format is currently reserved for vertexes
    			JsonDict->SetStringField(TEXT("verts"), OutputPath);
    		}
    	}
    		
		for (const auto& TagAndValuePair : AssetData.TagsAndValues)
		{
			// UE_LOG(LogPromethean, Display, TEXT("Tag NameValue Pair: %s : %s"), *TagAndValuePair.Key.ToString(), *TagAndValuePair.Value);			
			// ---- Material Count
			if (TagAndValuePair.Key.IsEqual("Materials"))
			{
				//UE_LOG(LogPromethean, Display, TEXT("Material Count: %i"), FCString::Atoi(*TagAndValuePair.Value.GetValue()));  // gets material slots on the static mesh
				JsonDict->SetNumberField(TEXT("material_count"), FCString::Atoi(*TagAndValuePair.Value.GetValue()));
			}
			// ---- Face Count
			else if (TagAndValuePair.Key.IsEqual("Triangles"))
			{
				//UE_LOG(LogPromethean, Display, TEXT("Faces: %i"), FCString::Atoi(*TagAndValuePair.Value.GetValue()));
				JsonDict->SetNumberField(TEXT("face_count"), FCString::Atoi(*TagAndValuePair.Value.GetValue()));
			}
			// ---- Vertex Count
			else if (TagAndValuePair.Key.IsEqual("Vertices"))
			{
				//UE_LOG(LogPromethean, Display, TEXT("Vertices: %i"), FCString::Atoi(*TagAndValuePair.Value.GetValue()));
				JsonDict->SetNumberField(TEXT("vertex_count"), FCString::Atoi(*TagAndValuePair.Value.GetValue()));
			}
			// ---- UV Channels
			else if (TagAndValuePair.Key.IsEqual("UVChannels"))
			{
				//UE_LOG(LogPromethean, Display, TEXT("UV Channels: %i"), FCString::Atoi(*TagAndValuePair.Value.GetValue()));
				JsonDict->SetNumberField(TEXT("uv_sets"), FCString::Atoi(*TagAndValuePair.Value.GetValue()));
			}			
			// ---- Size
			// Get precise size from the static mesh when we load it
			else if (TagAndValuePair.Key.IsEqual("ApproxSize"))
			{				
				TArray<FString> DimensionsList;
				FString TagValue = TagAndValuePair.Value.GetValue();
				TagValue.ParseIntoArray(DimensionsList, TEXT("x"), true);  // value is "8x16x4"
				if (DimensionsList.Num() == 3)  // make sure we split correctly into 3 items. unreal crashes if index is out of range for whatever unlikely reason
				{
					UE_LOG(LogADTools, Display, TEXT("Size: %s"), *TagValue);
					TArray<TSharedPtr<FJsonValue>> MeshScaleArray;
					MeshScaleArray.Add(MakeShareable(new FJsonValueNumber(FCString::Atoi(*DimensionsList[0]))));
					MeshScaleArray.Add(MakeShareable(new FJsonValueNumber(FCString::Atoi(*DimensionsList[2]))));  // swizzle
					MeshScaleArray.Add(MakeShareable(new FJsonValueNumber(FCString::Atoi(*DimensionsList[1]))));  // swizzle
					JsonDict->SetArrayField(TEXT("bounding_box"), MeshScaleArray);
				}
			}
			// ---- LODs
			else if (TagAndValuePair.Key.IsEqual("LODs"))
			{				
				//UE_LOG(LogPromethean, Display, TEXT("Lod Count: %i"), FCString::Atoi(*TagAndValuePair.Value.GetValue()));
				JsonDict->SetNumberField(TEXT("lod_count"), FCString::Atoi(*TagAndValuePair.Value.GetValue()));
			}
			
		}		
		//UE_LOG(LogPromethean, Display, TEXT("Asset Path: %s"), *AssetPath);
		// ---- Name
		JsonDict->SetStringField(TEXT("name"), AssetData.AssetName.ToString());
		// ---- Type
		JsonDict->SetStringField(TEXT("type"), "mesh");
		// ---- Vertex Color Channels   // TODO?
		JsonDict->SetNumberField(TEXT("vertex_color_channels"), 1);
		{
			SCOPE_CYCLE_COUNTER(STAT_ExtractAssetImportInfo);
			// ---- Source Files
			TOptional<FAssetImportInfo> ImportInfo = FAssetSourceFilenameCache::ExtractAssetImportInfo(AssetData);
			if (ImportInfo.IsSet())
			{
				FString SourcePaths;
				for (const auto& File : ImportInfo->SourceFiles)
				{
					SourcePaths += " " + File.RelativeFilename;
				}
				// JSONDict->SetStringField(TEXT("source_path"), SourcePaths);  // TODO: non english characters prevent json from being written :( 
				//UE_LOG(LogPromethean, Display, TEXT("Source Assets: %s"), *SourcePaths);
			}
		}
		
		// ---- Thumbnail Image -----
		TArray<FString> FileList;
		OutputPath.ParseIntoArray(FileList, TEXT("."), true);  // need to make sure we don't repeat the name second time after the '.' i.e. --Props--SM_Chair.SM_Chair.bmp
		const FString ImagePath = FString::Printf(TEXT("%s.bmp"), *FileList[0]);  // need to add bmp extension to force CreateBitmap not ue4 add numbers at the end
		JsonDict->SetStringField(TEXT("thumbnail"), ImagePath);
		
		FString ObjectFullName = AssetData.GetFullName();
		TArray<FName> ObjectFullNames = { *ObjectFullName };
		FThumbnailMap LoadedThumbnails;  // out tmap of <name:thumbnail>
		if (ThumbnailTools::ConditionallyLoadThumbnailsForObjects(ObjectFullNames, LoadedThumbnails))
		{			
			FObjectThumbnail* ObjectThumbnail = LoadedThumbnails.Find(*ObjectFullName);
		 			
			if (ObjectThumbnail && !ObjectThumbnail->IsEmpty())
			{
				//UE_LOG(LogPromethean, Display, TEXT("Thumbnail Found. Size: %i"), ObjectThumbnail->GetCompressedDataSize());			
				SaveThumbnail(ObjectThumbnail, ImagePath);  // will add extension at the end
			}
			else
			{
				UE_LOG(LogADTools, Warning, TEXT("ObjectThumbnail not valid for %s!"), *ObjectFullName);
			}
		}
	}
	return JsonDict;
}

TSharedPtr<FJsonObject> GetMaterialAssetData(const FAssetData& AssetData, const bool bPerformHeavyOperations)
{
	TSharedPtr<FJsonObject> JsonDict = MakeShared<FJsonObject>();
	if (AssetData.AssetClassPath.GetAssetName().IsEqual("Material"))
	{		
		// ---- Unique ID - Name
		const FString AssetPath = AssetData.GetSoftObjectPath().ToString();

		JsonDict->SetStringField(TEXT("path"), AssetPath);
		UE_LOG(LogADTools, Display, TEXT("Asset Path: %s"), *AssetPath);
		// ---- Name
		JsonDict->SetStringField(TEXT("name"), AssetData.AssetName.ToString());
		// ---- Type
		JsonDict->SetStringField(TEXT("type"), "material");
		// ---- Is Instance
		JsonDict->SetNumberField(TEXT("is_instance"), 0);
		// ---- Instruction Counts
		JsonDict->SetNumberField(TEXT("instructions_static"), 0);  // set default value
		JsonDict->SetNumberField(TEXT("instructions_dynamic"), 0);  // set default value
		JsonDict->SetNumberField(TEXT("instructions_vertex"), 0);  // set default value

		FString FileName = AssetPath;
		FileName = FileName.Replace(TEXT("/"), TEXT("--"));
		const FString OutputPath = OutFolderPath + "/thumbnails/" + FileName;
		
		if (bPerformHeavyOperations)
		{
			// turn AssetData to UMaterial
			UMaterial* MaterialInstance = Cast<UMaterial>(AssetData.GetAsset());
			// High-res Thumbnail
			GenerateHighResolutionThumbnail(MaterialInstance);  // if couldn't load - generate
			// ---- Parent Material
			JsonDict->SetStringField(TEXT("parent_path"), MaterialInstance->GetBaseMaterial()->GetPathName());
			UE_LOG(LogADTools, Display, TEXT("Parent Name: %s"), *MaterialInstance->GetBaseMaterial()->GetPathName());

			// ---- Number of textures
			TArray<UTexture*> MaterialTextures;
			//MaterialInstance->GetUsedTextures(MaterialTextures, EMaterialQualityLevel::Num, true, GMaxRHIFeatureLevel, true);
			MaterialInstance->GetUsedTextures(MaterialTextures, EMaterialQualityLevel::Num, true, ERHIFeatureLevel::SM5, true);
			JsonDict->SetNumberField(TEXT("texture_count"), MaterialTextures.Num());
			UE_LOG(LogADTools, Display, TEXT("Textures: %i"), MaterialTextures.Num());
			// ---- Texture Paths
			TArray<TSharedPtr<FJsonValue>> TexturePathArray;
			for (const UTexture* Texture : MaterialTextures)
			{
				UE_LOG(LogADTools, Display, TEXT("Texture Used: %s"), *Texture->GetPathName());  // gets material slots on the static mesh
				TexturePathArray.Add(MakeShareable(new FJsonValueString(Texture->GetPathName())));
			}
			JsonDict->SetArrayField(TEXT("texture_paths"), TexturePathArray);
			// ---- Two Sided
			JsonDict->SetNumberField(TEXT("is_two_sided"), MaterialInstance->TwoSided);
			UE_LOG(LogADTools, Display, TEXT("Two Sided: %i"), MaterialInstance->TwoSided);
			// ---- Masked
			JsonDict->SetNumberField(TEXT("is_masked"), MaterialInstance->IsMasked());
			UE_LOG(LogADTools, Display, TEXT("Masked: %i"), MaterialInstance->IsMasked());
			
			// ---- Material Functions Used
			auto MaterialFunctions = GetMaterialFunctionDependencies(*MaterialInstance);
			TArray<TSharedPtr<FJsonValue>> MaterialFunctionPaths;
			for (const auto& Function : MaterialFunctions)
			{
				UE_LOG(LogADTools, Display, TEXT("Material Function Used: %s"), *Function->GetPathName());
				MaterialFunctionPaths.Add(MakeShareable(new FJsonValueString(Function->GetPathName())));
			}
			JsonDict->SetArrayField(TEXT("material_functions"), MaterialFunctionPaths);
		}

		// ---- Thumbnail Image -----
		TArray<FString> FileList;
		OutputPath.ParseIntoArray(FileList, TEXT("."), true);  // need to make sure we don't repeat the name second time after the '.' i.e. --Props--SM_Chair.SM_Chair.bmp
		const FString ImagePath = FString::Printf(TEXT("%s.bmp"), *FileList[0]);  // need to add bmp extension to force CreateBitmap not ue4 add numbers at the end
		JsonDict->SetStringField(TEXT("thumbnail"), ImagePath);

		const FString ObjectFullName = AssetData.GetFullName();
		const TArray<FName> ObjectFullNames = { *ObjectFullName };
		FThumbnailMap LoadedThumbnails;  // out tmap of <name:thumbnail>
		if (ThumbnailTools::ConditionallyLoadThumbnailsForObjects(ObjectFullNames, LoadedThumbnails))
		{			
			FObjectThumbnail* ObjectThumbnail = LoadedThumbnails.Find(*ObjectFullName);
		 			
			if (ObjectThumbnail && !ObjectThumbnail->IsEmpty())
			{
				//UE_LOG(LogPromethean, Display, TEXT("Thumbnail Found. Size: %i"), ObjectThumbnail->GetCompressedDataSize());			
				SaveThumbnail(ObjectThumbnail, ImagePath);  // will add extension at the end
			}
			else
			{
				UE_LOG(LogADTools, Warning, TEXT("ObjectThumbnail not valid for %s!"), *ObjectFullName);
			}
		}
	}
	return JsonDict;
}

TSharedPtr<FJsonObject> GetMaterialInstanceAssetData(const FAssetData& AssetData, const bool bPerformHeavyOperations)
{
	TSharedPtr<FJsonObject> JsonDict = MakeShared<FJsonObject>();
	if (AssetData.AssetClassPath.GetAssetName().IsEqual("MaterialInstanceConstant"))
	{
		// ---- Unique ID - Name
		const FString AssetPath = AssetData.GetSoftObjectPath().ToString();
		JsonDict->SetStringField(TEXT("path"), AssetPath);
		UE_LOG(LogADTools, Display, TEXT("Asset Path: %s"), *AssetPath);
		// ---- Name		
		JsonDict->SetStringField(TEXT("name"), AssetData.AssetName.ToString());
		// ---- Type
		JsonDict->SetStringField(TEXT("type"), "material");
		// ---- Is Instance
		JsonDict->SetNumberField(TEXT("is_instance"), 1);
		// ---- Instruction Counts
		JsonDict->SetNumberField(TEXT("instructions_static"), 0);  // set default value
		JsonDict->SetNumberField(TEXT("instructions_dynamic"), 0);  // set default value
		JsonDict->SetNumberField(TEXT("instructions_vertex"), 0);  // set default value

		FString FileName = AssetPath;
		FileName = FileName.Replace(TEXT("/"), TEXT("--"));
		const FString OutputPath = OutFolderPath + "/thumbnails/" + FileName;

		if (bPerformHeavyOperations)
		{
			// turn AssetData to UMaterialInstanceConstant
			UMaterialInstanceConstant* MaterialInstance = Cast<UMaterialInstanceConstant>(AssetData.GetAsset());
			GenerateHighResolutionThumbnail(MaterialInstance);
			// ---- Parent Material				
			FString ParentPath = "";
			const UMaterial* ParentMaterial = MaterialInstance->GetBaseMaterial();
			if (IsValid(ParentMaterial))  // need to check if material exists first - otherwise hard crash
				ParentPath = ParentMaterial->GetPathName();
			else
				UE_LOG(LogADTools, Display, TEXT("Warning! Can't find a valid parent for this MaterialInstance - it might be corrupted"));
			JsonDict->SetStringField(TEXT("parent_path"), ParentPath);
			UE_LOG(LogADTools, Display, TEXT("Parent Name: %s"), *ParentPath);

			// ---- Number of textures					
			TArray<UTexture*> MaterialTextures;
			MaterialInstance->GetUsedTextures(MaterialTextures, EMaterialQualityLevel::Num, true, GMaxRHIFeatureLevel, true);
			JsonDict->SetNumberField(TEXT("texture_count"), MaterialTextures.Num());
			UE_LOG(LogADTools, Display, TEXT("Textures: %i"), MaterialTextures.Num());
			// ---- Texture Paths
			TArray<TSharedPtr<FJsonValue>> TexturePathArray;
			FString TexturePath = "";
			for (const UTexture* Texture : MaterialTextures)
			{
				TexturePath = Texture->GetFullName();
				UE_LOG(LogADTools, Display, TEXT("Texture Used: %s"), *TexturePath);  // gets material slots on the static mesh
				TexturePathArray.Add(MakeShareable(new FJsonValueString(TexturePath)));
			}
			JsonDict->SetArrayField(TEXT("texture_paths"), TexturePathArray);
			// ---- Two Sided
			JsonDict->SetNumberField(TEXT("is_two_sided"), MaterialInstance->TwoSided);
			UE_LOG(LogADTools, Display, TEXT("Two Sided: %i"), MaterialInstance->TwoSided);
			// ---- Masked
			JsonDict->SetNumberField(TEXT("is_masked"), MaterialInstance->IsMasked());
			UE_LOG(LogADTools, Display, TEXT("Masked: %i"), MaterialInstance->IsMasked());

		}
		
		// ---- Thumbnail Image -----
		TArray<FString> FileList;
		OutputPath.ParseIntoArray(FileList, TEXT("."), true);  // need to make sure we don't repeat the name second time after the '.' i.e. --Props--SM_Chair.SM_Chair.bmp
		const FString ImagePath = FString::Printf(TEXT("%s.bmp"), *FileList[0]);  // need to add bmp extension to force CreateBitmap not ue4 add numbers at the end
		JsonDict->SetStringField(TEXT("thumbnail"), ImagePath);

		const FString ObjectFullName = AssetData.GetFullName();
		const TArray<FName> ObjectFullNames = { *ObjectFullName };
		FThumbnailMap LoadedThumbnails;  // out tmap of <name:thumbnail>
		if (ThumbnailTools::ConditionallyLoadThumbnailsForObjects(ObjectFullNames, LoadedThumbnails))
		{			
			FObjectThumbnail* ObjectThumbnail = LoadedThumbnails.Find(*ObjectFullName);
		 			
			if (ObjectThumbnail && !ObjectThumbnail->IsEmpty())
			{
				//UE_LOG(LogPromethean, Display, TEXT("Thumbnail Found. Size: %i"), ObjectThumbnail->GetCompressedDataSize());			
				SaveThumbnail(ObjectThumbnail, ImagePath);  // will add extension at the end
			}
			else
			{
				UE_LOG(LogADTools, Warning, TEXT("ObjectThumbnail not valid for %s!"), *ObjectFullName);
			}
		}
	}
	return JsonDict;
}

TSharedPtr<FJsonObject> GetTextureAssetData(const FAssetData& AssetData, const bool bPerformHeavyOperations)
{
	TSharedPtr<FJsonObject> JsonDict = MakeShared<FJsonObject>();
	if (AssetData.AssetClassPath.GetAssetName().IsEqual("Texture2D"))

	{		
		for (const auto& TagAndValuePair : AssetData.TagsAndValues)
		{	// tags include: Format, AssetImportData, SRGB, CompressionSettings, Filter, NeverStream, LODGroup, AddressY, AddressX, Dimensions
			// UE_LOG(LogPromethean, Display, TEXT("Tag NameValue Pair: %s : %s"), *TagAndValuePair.Key.ToString(), *TagAndValuePair.Value);
			if (TagAndValuePair.Key.ToString() == "Dimensions")
			{
				// ---- Texture Size
				TArray<FString> DimensionsList;				
				TagAndValuePair.Value.GetValue().ParseIntoArray(DimensionsList, TEXT("x"), true);  // value is "512x512"
				if (DimensionsList.Num() == 2)  // make sure we split correctly into 2 items. unreal crashes if index is out of range for whatever unlikely reason
				{
					int width = FCString::Atoi(*DimensionsList[0]);
					int height = FCString::Atoi(*DimensionsList[1]);
					JsonDict->SetNumberField(TEXT("width"), width);
					JsonDict->SetNumberField(TEXT("height"), height);
					UE_LOG(LogADTools, Display, TEXT("Width: %i"), width);
					UE_LOG(LogADTools, Display, TEXT("Height: %i"), height);
				}
			}
			else if (TagAndValuePair.Key.ToString() == "HasAlphaChannel")
			{
				// ---- Masked or Not
				int has_alpha_int_bool = 0;
				if (TagAndValuePair.Value.GetValue() == "True")
					has_alpha_int_bool = 1;
				JsonDict->SetNumberField(TEXT("has_alpha"), has_alpha_int_bool);
				UE_LOG(LogADTools, Display, TEXT("Has Alpha: %i"), has_alpha_int_bool);
			}
		}		
		// ---- Unique ID - Name		
		const FString AssetPath = AssetData.GetSoftObjectPath().ToString();

		JsonDict->SetStringField(TEXT("path"), AssetPath);
		UE_LOG(LogADTools, Display, TEXT("Asset Path: %s"), *AssetPath);
		// ---- Name		
		JsonDict->SetStringField(TEXT("name"), AssetData.AssetName.ToString());
		// ---- Type
		JsonDict->SetStringField(TEXT("type"), "texture");

		FString FileName = AssetPath;
		FileName = FileName.Replace(TEXT("/"), TEXT("--"));
		const FString OutputPath = OutFolderPath + "/thumbnails/" + FileName;
		
		// --- Get source files paths of the asset
		TOptional<FAssetImportInfo> ImportInfo = FAssetSourceFilenameCache::ExtractAssetImportInfo(AssetData);
		if (ImportInfo.IsSet())
		{
			FString SourcePaths;
			for (const auto& File : ImportInfo->SourceFiles)
			{
				SourcePaths += " " + File.RelativeFilename;
			}
			JsonDict->SetStringField(TEXT("source_path"), SourcePaths);
			UE_LOG(LogADTools, Display, TEXT("Source Assets: %s"), *SourcePaths);
		}

		// Get source file path of the asset
		JsonDict->SetStringField(TEXT("source_path"), GetJsonImportSourceDataFromAsset<UTexture>(AssetData.GetAsset()->GetPathName()));
		
		if (bPerformHeavyOperations)
		{
			// turn AssetData to UMaterialInstanceConstant
			UTexture2D* Texture2D = Cast<UTexture2D>(AssetData.GetAsset());
			GenerateHighResolutionThumbnail(Texture2D);
		}
		
		// ---- Thumbnail Image -----
		TArray<FString> FileList;
		OutputPath.ParseIntoArray(FileList, TEXT("."), true);  // need to make sure we don't repeat the name second time after the '.' i.e. --Props--SM_Chair.SM_Chair.bmp
		const FString ImagePath = FString::Printf(TEXT("%s.bmp"), *FileList[0]);  // need to add bmp extension to force CreateBitmap not ue4 add numbers at the end
		JsonDict->SetStringField(TEXT("thumbnail"), ImagePath);

		const FString ObjectFullName = AssetData.GetFullName();
		const TArray<FName> ObjectFullNames = { *ObjectFullName };
		FThumbnailMap LoadedThumbnails;  // out tmap of <name:thumbnail>
		if (ThumbnailTools::ConditionallyLoadThumbnailsForObjects(ObjectFullNames, LoadedThumbnails))
		{			
			FObjectThumbnail* ObjectThumbnail = LoadedThumbnails.Find(*ObjectFullName);
		 			
			if (ObjectThumbnail && !ObjectThumbnail->IsEmpty())
			{
				//UE_LOG(LogPromethean, Display, TEXT("Thumbnail Found. Size: %i"), ObjectThumbnail->GetCompressedDataSize());			
				SaveThumbnail(ObjectThumbnail, ImagePath);  // will add extension at the end
			}
			else
			{
				UE_LOG(LogADTools, Warning, TEXT("ObjectThumbnail not valid for %s!"), *ObjectFullName);
			}
		}
	}
	return JsonDict;
}

TSharedPtr<FJsonObject> GetLevelAssetData(const FAssetData& AssetData, const bool bPerformHeavyOperations)
{
	TSharedPtr<FJsonObject> JsonDict = MakeShared<FJsonObject>();
	if (AssetData.AssetClassPath.GetAssetName().IsEqual("World"))
	{
		// ---- Thumbnail - do thumbnail sooner so it has more time to save
		
		// ---- Unique ID - Name		
		const FString AssetPath = AssetData.GetSoftObjectPath().ToString();
		JsonDict->SetStringField(TEXT("path"), AssetPath);
		
		FString FileName = AssetPath;
		FileName = FileName.Replace(TEXT("/"), TEXT("--"));
		const FString OutputPath = OutFolderPath + "/thumbnails/" + FileName + ".png";  // needs to be bmp as expected by standalone promethean app
		CaptureScreenshot(OutputPath);
		JsonDict->SetStringField(TEXT("thumbnail"), OutputPath);
		UE_LOG(LogADTools, Display, TEXT("Asset Path: %s"), *AssetPath);
		
		// ---- Name		
		JsonDict->SetStringField(TEXT("name"), AssetData.AssetName.ToString());
		// ---- Type
		JsonDict->SetStringField(TEXT("type"), "level");

		if (bPerformHeavyOperations)
		{
			// ---- Open Level, get assets
			OpenLevel(AssetData.GetSoftObjectPath().ToString());
			// ---- Get All Asset Paths
			TArray<FString> AssetPathArray = GetCurrentLevelArtAssets();
			TArray<TSharedPtr<FJsonValue>> AssetPathJsonArray;
			for (FString Path : AssetPathArray)
				AssetPathJsonArray.Add(MakeShared<FJsonValueString>(Path));
			JsonDict->SetArrayField(TEXT("asset_paths"), AssetPathJsonArray);
		}
	}
	return JsonDict;
}

TArray<FString> GetCurrentLevelArtAssets()
{
	UWorld* EditorWorld = GetEditorWorld();  // comes from PromehteanGeneric
	TArray<FString> OutPathArray;
	TArray<AActor*> AllValidActors = GetWorldValidActors(EditorWorld);
	for (AActor* CurrentLearnActor : AllValidActors)
	{
		USceneComponent* SceneComponent = GetValidSceneComponent(CurrentLearnActor);
		// --- skip if actor has no valid static mesh
		if (SceneComponent == nullptr)
			continue;

		const auto StaticMeshComponent = Cast<UStaticMeshComponent>(SceneComponent);
		if (StaticMeshComponent != nullptr) {
			const UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();
			// --- skip empty static meshes which are most likely there to signify semantic groups
			if (StaticMesh->GetPathName() == FString("None"))
				continue;
			// --- art asset path
			OutPathArray.Add(StaticMesh->GetPathName());
		}
	}
	return OutPathArray;
}

TArray<UStaticMeshComponent*> GetStaticMeshComponents(UBlueprint* blueprint)
{   // https://answers.unrealengine.com/questions/140647/get-default-object-for-blueprint-class.html
	TArray<UStaticMeshComponent*> ActorStaticMeshComponents;
	
	if (blueprint && blueprint->SimpleConstructionScript)
	{
		for (const auto scsnode : blueprint->SimpleConstructionScript->GetAllNodes())
		{
			if (scsnode)
			{
				if (auto itemComponent = Cast<UStaticMeshComponent>(scsnode->ComponentTemplate))
				{
					ActorStaticMeshComponents.AddUnique(itemComponent);
				}
			}
		}
	}

	return ActorStaticMeshComponents;
}

TSharedPtr<FJsonObject> GetBlueprintAssetData(const FAssetData& AssetData, const bool bPerformHeavyOperations)
{
	TSharedPtr<FJsonObject> JsonDict = MakeShared<FJsonObject>();
	if (AssetData.AssetClassPath.GetAssetName().IsEqual("Blueprint"))

	{
		// AssetData.PrintAssetData();
		// ---- Blueprint Flag
		JsonDict->SetBoolField(TEXT("blueprint"), true);
		// ---- Unique ID - Name
		const FString AssetPath = AssetData.GetSoftObjectPath().ToString();

		JsonDict->SetStringField(TEXT("path"), AssetPath);
		UE_LOG(LogADTools, Display, TEXT("Asset Path: %s"), *AssetPath);

		FString FileName = AssetPath;
		FileName = FileName.Replace(TEXT("/"), TEXT("--"));
		FString OutputPath = OutFolderPath + "/thumbnails/" + FileName;  // define path globally because it's reused for triangle too
		
		// ---- Name
		JsonDict->SetStringField(TEXT("name"), AssetData.AssetName.ToString());
		// ---- Type
		JsonDict->SetStringField(TEXT("type"), "mesh");
		// ---- Vertex Color Channels   // TODO?
		JsonDict->SetNumberField(TEXT("vertex_color_channels"), 1);
		// ---- Source Files
		TOptional<FAssetImportInfo> ImportInfo = FAssetSourceFilenameCache::ExtractAssetImportInfo(AssetData);
		if (ImportInfo.IsSet())
		{
			FString SourcePaths;
			for (const auto& File : ImportInfo->SourceFiles)
			{
				SourcePaths += " " + File.RelativeFilename;
			}
			// JSONDict->SetStringField(TEXT("source_path"), SourcePaths);  // TODO: non english characters prevent json from being written :( 
			UE_LOG(LogADTools, Display, TEXT("Source Assets: %s"), *SourcePaths);
		}

		if (bPerformHeavyOperations)
		{
			// ---- WARNING! Have to load the actual BP here.
			UBlueprint* BlueprintAsset = Cast<UBlueprint>(AssetData.GetAsset()); 
			// ---- High Res Thumbs
			GenerateHighResolutionThumbnail(BlueprintAsset);
			
			TArray<UStaticMeshComponent*> ActorStaticMeshComponents = GetStaticMeshComponents(BlueprintAsset);
			UE_LOG(LogADTools, Display, TEXT("Blueprint static mesh components: %i"), ActorStaticMeshComponents.Num());
			// get unique static meshes
			TArray<UStaticMesh*> StaticMeshes;
			for (auto StaticMeshComponent : ActorStaticMeshComponents)
			{
				UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();
				StaticMeshes.AddUnique(StaticMesh);
			}
			// get attributes
			TArray<int32> NumLods;
			TArray<int32> NumFaces;
			int32 NumVerts = 0;
			int32 NumUVSets = 0;
			int32 NumTris = 0;		
			for (auto StaticMesh : StaticMeshes)
			{
				NumLods.Add(StaticMesh->GetNumLODs());
				NumVerts += StaticMesh->GetNumVertices(0);  // TrianglesCount += StaticMesh->RenderData->LODResources[0].GetNumVertices();
				NumTris += StaticMesh->GetRenderData()->LODResources[0].GetNumTriangles();
				NumUVSets += StaticMesh->GetNumUVChannels(0);
			}

			// ---- Face Count
			UE_LOG(LogADTools, Display, TEXT("Faces: %i"), NumTris);
			JsonDict->SetNumberField(TEXT("face_count"), NumTris);
			// ---- Vertex Count
			UE_LOG(LogADTools, Display, TEXT("Vertices: %i"), NumVerts);
			JsonDict->SetNumberField(TEXT("vertex_count"), NumVerts);
			// ---- UV Channels
			UE_LOG(LogADTools, Display, TEXT("UV Channels: %i"), NumUVSets);
			JsonDict->SetNumberField(TEXT("uv_sets"), NumUVSets);
			// ---- Num Lods
			UE_LOG(LogADTools, Display, TEXT("Lod Count: %i"), NumLods.Max());
			JsonDict->SetNumberField(TEXT("lod_count"), NumLods.Max());
			// ---- Material Paths
			
			TArray<FString> MaterialPathStringArray;
			for (auto StaticMesh : StaticMeshes)
			{			
				for (FStaticMaterial StaticMaterial : StaticMesh->GetStaticMaterials())
				{
					UE_LOG(LogADTools, Display, TEXT("Material Used: %s"), *StaticMaterial.MaterialInterface->GetPathName());  // gets material slots on the static mesh
					MaterialPathStringArray.AddUnique(StaticMaterial.MaterialInterface->GetPathName());
				}			
			}
			TArray<TSharedPtr<FJsonValue>> MaterialPathArray;
			for (auto UniqueMatPath : MaterialPathStringArray)
			{
				MaterialPathArray.AddUnique(MakeShareable(new FJsonValueString(UniqueMatPath)));
			}
			JsonDict->SetArrayField(TEXT("material_paths"), MaterialPathArray);
			// ---- Material Count				
			UE_LOG(LogADTools, Display, TEXT("Material Count: %i"), MaterialPathArray.Num());
			JsonDict->SetNumberField(TEXT("material_count"), MaterialPathArray.Num());

			
			// ---- Size
			FBox CombinedBoundingBox;
			for (auto StaticMesh : StaticMeshes)
			{
				CombinedBoundingBox += StaticMesh->GetBoundingBox();
				
				// ---- Raw Triangles
				TArray<FVector> Vertexes = GetMeshTrianglePositionsFromLibraryStaticMesh(StaticMesh);
				if (Vertexes.Num() != 0)
				{
					FString VertexString = FVectorArrayToJsonString(Vertexes, TEXT("verts"));
					SaveToFile(OutputPath / StaticMesh->GetName(), VertexString);  // no file format is currently reserved for vertexes
					JsonDict->SetStringField(TEXT("verts"), OutputPath / StaticMesh->GetName());
				}
			}
			UE_LOG(LogADTools, Display, TEXT("Size: %s"), *CombinedBoundingBox.GetSize().ToString());
			FVector Size = CombinedBoundingBox.GetSize();
			TArray<TSharedPtr<FJsonValue>> MeshScaleArray;
			MeshScaleArray.Add(MakeShareable(new FJsonValueNumber(Size.X)));
			MeshScaleArray.Add(MakeShareable(new FJsonValueNumber(Size.Y)));
			MeshScaleArray.Add(MakeShareable(new FJsonValueNumber(Size.Z)));
			JsonDict->SetArrayField(TEXT("bounding_box"), MeshScaleArray);
			
			// ---- Pivot Offset
			FVector PivotOffset = CombinedBoundingBox.GetCenter();
			PivotOffset[2] = CombinedBoundingBox.Min[2];  // min height
			TArray<TSharedPtr<FJsonValue>> MeshPivotOffsetArray;
			MeshPivotOffsetArray.Add(MakeShareable(new FJsonValueNumber(-PivotOffset.X)));
			MeshPivotOffsetArray.Add(MakeShareable(new FJsonValueNumber(-PivotOffset.Y)));
			MeshPivotOffsetArray.Add(MakeShareable(new FJsonValueNumber(-PivotOffset.Z)));
			JsonDict->SetArrayField(TEXT("pivot_offset"), MeshPivotOffsetArray);
		}
		
		// ---- Thumbnail Image -----
		TArray<FString> FileList;
		OutputPath.ParseIntoArray(FileList, TEXT("."), true);  // need to make sure we don't repeat the name second time after the '.' i.e. --Props--SM_Chair.SM_Chair.bmp
		const FString ImagePath = FString::Printf(TEXT("%s.bmp"), *FileList[0]);  // need to add bmp extension to force CreateBitmap not ue4 add numbers at the end
		JsonDict->SetStringField(TEXT("thumbnail"), ImagePath);
		
		FString ObjectFullName = AssetData.GetFullName();
		TArray<FName> ObjectFullNames = { *ObjectFullName };
		FThumbnailMap LoadedThumbnails;  // out tmap of <name:thumbnail>
		if (ThumbnailTools::ConditionallyLoadThumbnailsForObjects(ObjectFullNames, LoadedThumbnails))
		{			
			FObjectThumbnail* ObjectThumbnail = LoadedThumbnails.Find(*ObjectFullName);
		 			
			if (ObjectThumbnail && !ObjectThumbnail->IsEmpty())
			{
				//UE_LOG(LogPromethean, Display, TEXT("Thumbnail Found. Size: %i"), ObjectThumbnail->GetCompressedDataSize());			
				SaveThumbnail(ObjectThumbnail, ImagePath);  // will add extension at the end
			}
			else
			{
				UE_LOG(LogADTools, Warning, TEXT("ObjectThumbnail not valid for %s!"), *ObjectFullName);
			}
		}
	}
	return JsonDict;
}

TSharedPtr<FJsonObject> GetUnknownAssetData()
{
	TSharedPtr<FJsonObject> JsonDict = MakeShared<FJsonObject>();	
	JsonDict->SetStringField(TEXT("error"), "Unsupported data type");
	return JsonDict;
}

