#pragma once
#include "Engine/StaticMeshActor.h"

#include <limits> // infinity stuff


const float NegativeInfinity = -std::numeric_limits<float>::infinity();
const int NegativeInfinityInt = -std::numeric_limits<int>::infinity();
const FVector NegativeInfinityVector = FVector(NegativeInfinity, NegativeInfinity, NegativeInfinity);
const FRotator NegativeInfinityRotator = FRotator(NegativeInfinity, NegativeInfinity, NegativeInfinity);


TArray<TSharedPtr<class FJsonValue>> GetVectorJsonData(FVector UEVector);
TArray<TSharedPtr<FJsonValue>> GetRotationJsonData(FRotator ActorRotation);

// --------------------------------------------------------------------
// --- Editor Functions
// --------------------------------------------------------------------
/**
 * @brief 获取编辑的World对象.
 * @return  World
 */
UWorld* GetEditorWorld();

// --------------------------------------------------------------------
// --- Transform Functions
// --------------------------------------------------------------------

/**
 * @brief SetActorTransform
 * @param ObjectNames 
 * @param Transform 
 * @param World 
 */
void TransformActorsByName(const TArray<FString>& ObjectNames,const FTransform Transform,UWorld* World);
void TransformActorByName(FString ObjectName,FTransform Transform,UWorld* World);

/**
 * @brief SetActorLocation
 * @param ObjectNames 
 * @param NewLocation 
 * @param World 
 */
void TranslateActorsByName(const TArray<FString>& ObjectNames,FVector NewLocation,UWorld* World);
void TranslateActorByName(FString ObjectName,FVector NewLocation,UWorld* World);

/**
 * @brief SetActorLocation RayTrace HitLocation
 * @param ObjectNames 
 * @param NewLocation 
 * @param RaytraceDistance 
 * @param MaxNormalDeviation 
 * @param IgnoreObjectNames 
 * @param World 
 */
void TranslateAndRayTraceActorsByName(const TArray<FString>& ObjectNames, FVector NewLocation, float RaytraceDistance, float MaxNormalDeviation, const TArray<FString>& IgnoreObjectNames, UWorld* World);
void TranslateAndRaytraceActorByName(FString ObjectName, FVector NewLocation, float RaytraceDistance, float MaxNormalDeviation, TArray<AActor*> IgnoreActors, UWorld* World);

/**
 * @brief AddActorWorldOffset
 * @param ObjectNames 
 * @param LocationDelta 
 * @param World 
 */
void RelativeTranslateActorsByName(const TArray<FString>& ObjectNames, FVector LocationDelta, UWorld* World);
void RelativeTranslateActorByName(FString ObjectName, FVector LocationDelta, UWorld* World);

/**
 * @brief SetActorRotation
 * @param ObjectNames 
 * @param NewRotationVec 
 * @param World 
 */
void RotateActorsByName(const TArray<FString>& ObjectNames, FVector NewRotationVec, UWorld* World);
void RotateActorByName(FString ObjectName,FVector NewRotationVec, UWorld* World);

/**
 * @brief AddActorLocalRotation
 * @param ObjectNames 
 * @param NewRotationVec 
 * @param World 
 */
void RelativeRotateActorsByName(const TArray<FString>& ObjectNames, FVector NewRotationVec, UWorld* World);
void RelationRotateActorByName(FString ObjectName, FVector NewRotationVec, UWorld* World);

/**
 * @brief SetActorScale3D
 * @param ObjectNames 
 * @param NewScale 
 * @param World 
 */
void ScaleActorsByName(const TArray<FString>& ObjectNames,FVector NewScale, UWorld* World);
void ScaleActorByName(FString ObjectName, FVector NewScale,UWorld* World);

/**
 * @brief SetActorScale3D
 * @param ObjectNames 
 * @param NewScale 
 * @param World 
 */
void RelativeScaleActorsByName(const TArray<FString>& ObjectNames, FVector NewScale,UWorld* World);
void RelativeScaleActorByName(FString ObjectName,FVector NewScale,UWorld* World);

/**
 * @brief Destroy
 * @param ObjectNames 
 * @param World 
 */
void RemoveActorsByName(const TArray<FString>& ObjectNames, UWorld* World);
void RemoveActorByName(FString ObjectName, UWorld* World);

/**
 * @brief Destroy Descendents Actor
 * @param ObjectNames 
 * @param World 
 */
void RemoveDescendentsFromActorsByName(const TArray<FString>& ObjectNames, UWorld* World);
void RemoveDescendentsFromActorByName(FString ObjectName, UWorld* World);
/**
 * @brief Destroy
 */
void RemoveSelectedActors();

FString GetValidActorsTransformJsonDict(const TArray<AActor*>& ValidActors);
FString GetValidActorsExpandedTransformJsonDict(const TArray<AActor*>& ValidActors, UWorld* World);
FString GetValidActorsLocationJsonDict(const TArray<AActor*>& ValidActors);
FString GetValidActorsBottomCenterLocationJsonDict(const TArray<AActor*>& ValidActors);

FString GetStaticMeshActorsTransformJsonDictByName(const TArray<FString>& ObjectNames, UWorld* World);
FString GetStaticMeshActorsTransformJsonDict(const TArray<AStaticMeshActor*>& StaticMeshActors);

FString GetCameraInfoJsonDict(const bool ObjectsOnScreen = true);
// --------------------------------------------------------------------
// --- Utility Functions
// --------------------------------------------------------------------

AActor* ValidateActor(AActor* Actor);
/**
 * @brief 查找有效的Actors
 * @param ActorName 
 * @param World 
 * @return 
 */
AActor* GetValidActorByName(const FString& ActorName,UWorld* World);
/**
 * @brief 提取Scene组件
 * @param Actor 
 * @return 
 */
USceneComponent* GetValidSceneComponent(AActor* Actor);
/**
 * @brief 提取Mesh组件
 * @param Actor 
 * @return 
 */
UMeshComponent* GetValidMeshComponent(AActor* Actor);
/**
 * @brief 提取StaticMesh组件
 * @param Actor 
 * @return 
 */
UStaticMeshComponent* GetValidStaticMeshComponent(AActor* Actor);
/**
 * @brief 名字
 * @param ActorNames 
 * @param World 
 * @return 
 */
TArray<AActor*> GetNamedValidActors(const TArray<FString>& ActorNames,UWorld* World);
/**
 * @brief 场景中的Actors
 * @param World 
 * @return 
 */
TArray<AActor*> GetWorldValidActors(const UWorld* World);
/**
 * @brief 已选中的Actors
 * @return 
 */
TArray<AActor*> GetSelectedValidActors();
/**
 * @brief 已渲染Actors
 * @return 
 */
TArray<AActor*> GetRenderedValidActors();
/**
 * @brief 使用的资产路径
 * @param AssetPath 
 * @param World 
 * @return 
 */
TArray<AActor*> GetValidActorsByMeshAssetPath(const FString& AssetPath,const UWorld* World);
/**
 * @brief 使用的材质路径
 * @param MaterialPath 
 * @param World 
 * @return 
 */
TArray<AActor*> GetValidActorsByMaterialPath(const FString& MaterialPath, const UWorld* World);




// --------------------------------------------------------------------
// --- Material Functions
// --------------------------------------------------------------------
/**
 * @brief 
 * @param Actor SetMaterial Attribute -FLinearColor
 * @param AttributeName 
 * @param Value 
 * @param Index 
 */
void SetVecMatAttrsForValidActor(AActor* Actor, const FString AttributeName, const FLinearColor Value, const int Index);
/**
 * @brief SetMaterial Attribute -Scalar
 * @param Actor 
 * @param AttributeName 
 * @param Value 
 */
void SetScalarMatAttrsForValidActor(AActor* Actor, const FString AttributeName, const float Value);
/**
 * @brief SetMaterial
 * @param MaterialPath 
 * @param Index 
 */
void SetMaterialForSelectedValidActors(const FString MaterialPath, const int Index);
/**
 * @brief SetMaterial
 * @param Actor 
 * @param MaterialPath 
 * @param Index 
 */
void SetMaterialForValidActor(AActor* Actor, const FString& MaterialPath, int Index);
/**
 * @brief SetMaterial
 * @param MeshComponent 
 * @param MaterialPath 
 * @param Index 
 * @return 
 */
bool SetMaterialForMeshComponent(UMeshComponent* MeshComponent, const FString& MaterialPath, int Index);
/**
 * @brief GetMaterial Attribute - FLinearColor
 * @param ValidActors 
 * @param AttributeName 
 * @param Index 
 * @return 
 */
TArray<FLinearColor> GetVecMatAttrsForValidActors(const TArray<AActor*>& ValidActors, const FString& AttributeName, int Index);
TArray<FLinearColor> GetVecMatAttrsForValidActor(AActor* ValidActor, const FString& AttributeName, int Index);
/**
 * @brief GetMaterial Attribute - Scalar
 * @param ValidActors 
 * @param AttributeName 
 * @return 
 */
TArray<float> GetScalarMatAttrsForValidActors(const TArray<AActor*>& ValidActors, FString AttributeName);
TArray<float> GetScalarMatAttrsForValidActor(AActor* ValidActor, FString AttributeName);
/**
 * @brief GetMaterialAttributeNames
 * @param AttributeType 
 * @return 
 */
TArray<FString> GetMaterialAttributeNamesFromSelectedStaticMeshActors(FString AttributeType);
TArray<FString> GetMaterialAttributeNamesFromValidActors(const TArray<FString>& ActorNames, FString AttributeType, UWorld* World);
TArray<FString> GetMaterialAttributeNamesFromValidActor(AActor* Actor, FString AttributeType);

/**
 * @brief GetMaterialPath
 * @return 
 */
TArray<FString> GetMaterialPathsFromSelectedValidActors();
FString GetMaterialPathJsonDictFromActorsByName(const TArray<FString>& ActorNames, UWorld* World);
TArray<FString> GtMaterialPathsFromStaticMeshActorByName(FString ActorName, UWorld* World);
TArray<FString> GetMaterialPathsFromValidActor(AActor* Actor);

TArray<UMaterialFunctionInterface*> GetMaterialFunctionDependencies(const UMaterial& Material);
/**
 * @brief CreateMaterialInstances
 * @param JsonString 
 */
void CreateMaterialInstancesFromJsonString(const FString& JsonString);
/**
 * @brief SetMeshAssetMaterial
 * @param JsonString 
 */
void SetMeshAssetMaterialFromJsonString(const FString& JsonString);
/**
 * @brief RemoveMaterial
 * @param ActorName 
 * @param World 
 */
void RemoveMaterialOverride(const FString& ActorName, UWorld* World);

/**
 * @brief 获取Actor上所有材质.
 * @param Actor 
 * @return 
 */
TArray<UMaterialInterface*> GetAllMaterialInterfacesFromValidActor(AActor* Actor);



// --------------------------------------------------------------------
// --- Physics Simulation Functions
// --------------------------------------------------------------------

/**
 * @brief 获取物理模拟状态
 * @param ActorNames 
 * @return 
 */
FString GetActorsPhysicsSimulationStateByName(const TArray<FString>& ActorNames);
/**
 * @brief 设置物理模拟状态
 * @param ActorNames 
 * @param State 
 */
void SetActorsToBePhysicallySimulatedByName(const TArray<FString>& ActorNames, bool State);
void SetActorToBePhysicallySimulatedByName(const FString& ActorName, bool State);
void SetActorToBePhysicallySimulated(AActor* Actor, bool State);

TArray<AActor*> GetSimulatingActorsByName(const TArray<FString>& ActorNames);
FString GetSimulatingActorsTransformJSONDictByNames(const TArray<FString>& ActorNames);
bool GetActorsPhysicalSimulationState(const AActor* Actor);

template<class ActorType>
ActorType* GetSimulatingActorByName(const FString& ActorName);

/**
 * @brief Play PIE
 */
void EnablePlayInEditor();
/**
 * @brief Stop PIE
 */
void DisablePlayInEditor();



// --------------------------------------------------------------------
// --- Scene Management Functions
// --------------------------------------------------------------------

/**
 * @brief AddStaticMesh
 * @param World 
 * @param JsonString 
 * @return 
 */
FString AddStaticMeshActors(UWorld* World,FString JsonString);
TArray<AStaticMeshActor*> AddStaticMeshActorOnSelection(FString MeshPath, UWorld* World);
TArray<AStaticMeshActor*> AddStaticMeshActorsOnSelection(TArray<FString> MeshPaths, UWorld* World);

/**
 * @brief AddActorToWorld
 * @tparam ActorType 
 * @param World 
 * @param name 
 * @param Location 
 * @param RotationVec 
 * @param Scale 
 * @return 
 */
template<class ActorType>
ActorType* AddActorToWorld(UWorld* World, const FString& name, const FVector& Location, const FVector& RotationVec = FVector::ZeroVector, const FVector& Scale = FVector::OneVector);
// Could probably be replaced by templated version above, AddActorToWorld(..).
AStaticMeshActor* AddStaticMeshActor(UWorld* World,FString MeshPath, FString Name = "", FVector Location = FVector(0,0,0),FVector ROtationVec = FVector(0,0,0), FVector Scale = FVector(1,1,1));
// Could probably be replaced by templated version above, AddActorToWorld(..).
AStaticMeshActor* AddEmptyStaticMeshActor(UWorld* World,FString Name = "", FVector Location = FVector(0,0,0),FVector RotationVec = FVector(0,0,0),FVector Scale = FVector(1,1,1));
AActor* AddBlueprintActor(UWorld* World, FString MeshPath, FString Name, FVector Location, FVector RotationVec, FVector Scale);
/**
 * @brief SetStaticMesh
 * @param ValidActor 
 * @param MeshPath 
 */
void SetStaticMeshAssetForValidActor(AActor* ValidActor, FString MeshPath);
/**
 * @brief GetActor
 * @tparam ActorType 
 * @param ActorName 
 * @param World 
 * @return 
 */
template<class ActorType>
ActorType* GetActorByName(const FString& ActorName, UWorld* World);
TArray<AActor*> GetRenderedActors();
/**
 * @brief 
 * @param Actor 
 * @return 
 */
bool ActorHasStaticMeshComponent(const AActor* Actor);

/**
 * @brief 编辑器中隐藏显示
 * @param ObjectNames 
 * @param World 
 */
void SetActorsHiddenByName(const TArray<FString>& ObjectNames, UWorld* World);
void SetActorsVisibleByName(const TArray<FString>& ObjectsNames, UWorld* World);
void SetStaticMeshActorsHiddenByName(const TArray<FString>& ObjectNames, UWorld* World);
void SetStaticMeshActorsVisibleByName(const TArray<FString>& ObjectNames, UWorld* World);

/**
 * @brief 定位Actor
 * @param ObjectNames 
 * @param World 
 */
void FocusOnActorsByName(const TArray<FString>& ObjectNames, UWorld* World);
/**
 * @brief 选中指定Actor
 * @param ObjectNames 
 */
void SelectSceneActorsByName(const TArray<FString>& ObjectNames);

/**
 * @brief Get OverlappingActors
 * @param Objects 
 * @return 
 */
TArray<AActor*> GetAllObjectsIntersectingGivenObjects(TArray<AActor*> Objects);
/**
 * @brief GetParentsForObjectsByName
 * @param ObjectNames 
 * @param World 
 * @return 
 */
TArray<FString> GetParentsForObjectsByName(const TArray<FString>& ObjectNames, UWorld* World);
FString GetParentForObjectByName(FString ObjectName, UWorld* World);
TArray<AActor*> GetAllDescendentsForObjectsRecursive(TArray<AActor*> Objects, UWorld* World);
/**
 * @brief GetChildActors
 * @param Object 
 * @param World 
 * @return 
 */
TArray<AActor*> GetAllDescendentsForObjectRecursive(const AActor* Object, UWorld* World);

/**
 * @brief SetParent
 * @param ParentName 
 * @param ChildName 
 * @param World 
 */
void ParentObjectsByName(const FString ParentName, const FString ChildName, UWorld* World);
void ParentAllObjectsByName(const FString ParentName, const TArray<FString>& ChildNames, UWorld* World);
/**
 * @brief UnsetParent
 * @param Names 
 * @param World 
 */
void UnparentObjectsByName(const TArray<FString>& Names, UWorld* World);
void UnparentObjectByName(FString Name, UWorld* World);
/**
 * @brief 更新ActorLabel
 * @param World 
 */
void FixObjectNames(const UWorld* World);
/**
 * @brief RenameActor
 * @param SourceName 
 * @param TargetName 
 * @param World 
 */
void RenameActor(FString SourceName, FString TargetName, UWorld* World);
void ToggleKillNamesOnSelection();

/**
 * @brief OpenLevel
 * @param LevelPath 
 */
void OpenLevel(FString LevelPath);
/**
 * @brief LoadLevel
 * @param LevelPath 
 * @param World 
 */
void LoadLevel(FString LevelPath, UWorld* World);
/**
 * @brief UnloadLevel
 * @param LevelPath 
 * @param World 
 */
void UnloadLevel(FString LevelPath, const UWorld* World);
void SetLevelVisibility(FString LevelPath, bool IsVisible, const UWorld* World);
void SetLevelCurrent(FString LevelPath, UWorld* World);
FString GetCurrentLevelPath(const UWorld* World);

// --------------------------------------------------------------------
// --- Asset Library Functions
// --------------------------------------------------------------------

/**
 * 获取关卡名字
 */
FString GetSceneName(const UWorld* World);
/**
 * 获取选中的资产.
 */
FString GetAssetBrowserSelection();
/**
 * 设置选中的资产.
 */
void SetAssetBrowserSelection(const TArray<UObject*>& SelectedObjects);

FString GetJsonAssetDataFromAssetBrowserSelection();

template<class AssetType>
FString GetJsonImportSourceDataFromAsset(const FString& AssetPath);
void EditAsset(FString AssetReferencePath);
void FindAsset(FString AssetReferencePath);
void FindAssets(TArray<FString> AssetReferencePaths);
void LoadAssets(TArray<FString> AssetReferencePaths);
void ImportAssetToCurrentPath();
void ImportAsset(FString AssetImportPath);
/**
 * @brief DeleteSelectedAssets
 */
void DeleteSelectedAssets();

TArray<UObject*> ImportFBXAssetsFromJsonString(const FString& JsonString);
TArray<UObject*> ImportTextureAssetsFromJsonString(const FString& JsonString);
UObject* LoadFBXFile(const FString& AbsoluteSourcePath, const FString& AssetTarget);
UObject* LoadTextureFile(const FString& AbsoluteSourcePath, const FString& AssetTarget);
template<class ObjectType>
UObject* AddObjectToContentBrowser(ObjectType* AssetObject, const FString& TargetPath);
FString GetAllExistingAssetByType(FString OutputPath, FString AssetType);

USceneComponent* GetBiggestVolumeBoundComponent(const TArray<UStaticMeshComponent*>& Components);

// --------------------------------------------------------------------
// --- Ray Tracing
// --------------------------------------------------------------------

FString RayTraceFromPointsJSON(const TArray<FVector>& Points, FVector RayDirection, UWorld* World, TArray<AActor*> IgnoreActors);
FString RayTraceFromActorsByNameJSON(const TArray<FString>& ActorNames, FVector RayDirection, UWorld* World);
FVector RayTraceFromActorByName(FString ActorName, FVector RayDirection, UWorld* World);
FVector RayTraceFromActor(AActor* Actor, FVector RayDirection, UWorld* World);
FHitResult RayTrace(FVector StartPoint, FVector EndPoint, UWorld* World, TArray<AActor*> IgnoreActors = {});
FVector RayTraceFromCamera(UWorld* World);
// MULTI RAY TRACE
FString RayTraceMultiFromActorsByNameJson(const TArray<FString>& ActorNames, FVector RayDirection, UWorld* World);
FVector RayTraceMultiFromActorByName(FString ActorName, FVector RayDirection, UWorld* World);
FVector RayTraceMultiFromActor(AActor* Actor, FVector RayDirection, UWorld* World, TArray<AActor*> IgnoreActors = {});
TArray<FHitResult> RayTraceMulti(FVector StartPoint, FVector EndPoint, UWorld* World, TArray<AActor*> IgnoreActors = {});





// --------------------------------------------------------------------
// --- Mesh Functions
// --------------------------------------------------------------------

TArray<FVector> GetMeshVertexPositions(AStaticMeshActor* StaticMeshActor);
TArray<FVector> GetMeshTrianglePositionsFromActor(AActor* MeshActor);
TArray<FVector> GetMeshTrianglePositionsFromStaticMesh(const UStaticMeshComponent* StaticMeshComponent);
//TArray<FVector> GetMeshTrianglePositionsFromGeneratedMesh(APrometheanMeshActor* PrometheanMeshActor);
TArray<FVector> GetMeshTrianglePositionsFromLibraryStaticMesh(UStaticMesh* StaticMesh);

// --------------------------------------------------------------------
// --- Misc Functions
// --------------------------------------------------------------------

FQuat RotationFromNormal(FVector NormalVector);
TArray<FVector> GetSelectionPositions(UWorld* World);
TArray<TSharedPtr<class FJsonValue>> FVectorToJsonArray(FVector Vector);
TArray<TSharedPtr<FJsonValue>> FVectorArrayToJsonArray(const TArray<FVector>& VectorArray);
TArray<TSharedPtr<FJsonValue>> FStringArrayToJsonArray(TArray<FString> StringArray);
TSharedPtr<FJsonObject> FStringArrayToJsonIndexDict(const TArray<FString>& StringArray);
FString FVectorArrayToJsonString(const TArray<FVector>& VectorArray, FString Label);
void CaptureScreenshot(FString OutputPath);

/**
 *	Save a string to a text file
 *	@param OutputPath	- path where to save the file
 *	@param TextToSave	- str to write into the file. most likely will be a json string
 *	@return				
 */
void SaveToFile(FString OutputPath, FString TextToSave);

// string to stuff
FTransform StringArrayToTransform(const TArray<FString>& StringArray);
FVector StringToVector(FString String);
FVector StringArrayToVector(const TArray<FString>& StringArray);
FString StringArrayToString(const TArray<FString>& Strings);
FLinearColor StringToLinearColor(FString String);
// stuff to string
FString LinearColorArrayToString(const TArray<FLinearColor>& ColorArray);
FString FloatArrayToString(const TArray<float>& Floats);
FString FVectorToString(FVector Vector);
FString StaticMeshActorArrayToNamesString(const TArray<AStaticMeshActor*>& Actors);
FString StaticMeshActorArrayToAssetPathsString(const TArray<AStaticMeshActor*>& Actors);
FString ActorArrayToNamesString(const TArray<AActor*>& Actors);
FString ActorArrayToAssetPathsString(const TArray<AActor*>& Actors);
// stuff to string array
TArray<FString> ActorArrayToNamesArray(const TArray<AActor*>& Actors);
TArray<FString> ActorArrayToAssetPathsArray(const TArray<AActor*>& Actors);
TArray<FVector> ActorArrayToLocationsArray(TArray<AActor*> Actors);
TArray<FVector> ActorArrayToPivotArray(TArray<AActor*> Actors);

FRotator FRotatorFromXYZVec(FVector RotationVec);

bool ParseJsonString(const FString& String, TSharedPtr<FJsonObject>& Result);

USceneComponent* GetBiggestVolumeBoundComponent(const TArray<UStaticMeshComponent*>& Components);
bool SaveToAsset(UObject* ObjectToSave);