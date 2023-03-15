// Copyright Epic Games, Inc. All Rights Reserved.

#include "ADToolsStyle.h"
#include "ADTools.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FADToolsStyle::StyleInstance = nullptr;

void FADToolsStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FADToolsStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FADToolsStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("ADToolsStyle"));
	return StyleSetName;
}


const FVector2D Icon2x2(2.0f, 2.0f);
const FVector2D Icon4x4(4.0f, 4.0f);
const FVector2D Icon12x12(12.0f, 12.0f);
const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);
const FVector2D Icon64x64(64.0f, 64.0f);

TSharedRef< FSlateStyleSet > FADToolsStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("ADToolsStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("ADTools")->GetBaseDir() / TEXT("Resources"));

	//Style->Set("ADTools.PluginAction", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));
	Style->Set("ADTools.PluginAction", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon40x40));
	Style->Set("ADTools.PluginAction_red", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon_red"), Icon40x40));
	Style->Set("ADTools.Settings", new IMAGE_BRUSH_SVG(TEXT("setting"), Icon20x20));
	Style->Set("ADTools.LangSwitcher", new IMAGE_BRUSH_SVG(TEXT("language"), Icon20x20));
	Style->Set("ADTools.ResetEditor", new IMAGE_BRUSH_SVG(TEXT("restart"), Icon20x20));
	Style->Set("ADTools.GitHubUrl", new IMAGE_BRUSH_SVG(TEXT("github"), Icon20x20));


	for (int i = 0; i <= 35; ++i)
	{
		Style->Set(FName(*FString::FromInt(i)), new IMAGE_BRUSH_SVG("icons/"+FString::FromInt(i), Icon20x20));
	}

	/*for (FName Icon : icons)
	{
		Style->Set(Icon, new IMAGE_BRUSH_SVG(FText::FromName(Icon).ToString(), Icon20x20));
	}*/


	
	
	//Style->Set("ADTools.PluginAction", new IMAGE_BRUSH(TEXT("ButtonIcon_40x"), Icon40x40));
	return Style;
}

void FADToolsStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FADToolsStyle::Get()
{
	return *StyleInstance;
}
