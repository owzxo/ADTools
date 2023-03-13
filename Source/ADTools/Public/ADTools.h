// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

class FADToolsModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command. */
	void PluginButtonClicked();

	
private:

	void RegisterMenus();
	void BindCommands();

	void LangSwitcher() const;
	void RestartEditor();
	
	
	TSharedRef< class SWidget > GenerateComboMenu(TSharedPtr<class FUICommandList> InCommands);
	 void OnToorBarButtonClick() const;

private:

	TSharedPtr<FUICommandList> ADToolsCommands;
};
