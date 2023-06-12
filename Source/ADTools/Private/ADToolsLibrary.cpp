// Fill out your copyright notice in the Description page of Project Settings.


#include "ADToolsLibrary.h"

UWorld* UADToolsLibrary::GetEditorWorld()
{
	const auto WorldContext = GEditor->GetEditorWorldContext();
	return WorldContext.World();
}
