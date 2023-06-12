// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ADToolsLibrary.generated.h"

/**
 * 
 */
UCLASS()
class ADTOOLS_API UADToolsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category=ADTools)
	static UWorld* GetEditorWorld();
};
