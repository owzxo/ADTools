// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

DECLARE_LOG_CATEGORY_EXTERN(LogADTools, Log, All);
class FADToolsModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	


	inline static bool bRedIcon = false;

	inline static int iconIndex;
	
private:

	void RegisterMenus();
	void BindCommands();

	void LangSwitcher() const;
	void RestartEditor() const;
	void OpenGitHubUrl() const;
	void OpenSettings() const;
	bool HandleSettingsSaved() const;

	TSharedRef< class SWidget > GenerateComboMenu(TSharedPtr<class FUICommandList> InCommands) const;
	 void OnToorBarButtonClick() const;

private:

	
	
	TSharedPtr<FUICommandList> ADToolsCommands;
};
