// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ADToolsSettings.generated.h"

/**
 * 
 */
UCLASS(config = ADTools,DefaultConfig)
class ADTOOLS_API UADToolsSettings : public UObject
{
	GENERATED_BODY()
	UADToolsSettings() {

	};
	UPROPERTY(config, EditAnywhere, meta = (EditCondition = "true", DisplayName = "ADTools", ToolTip = "ADTools", Category = "ADTools", DisplayPriority = 1))
	FString PrevName = "";
};
