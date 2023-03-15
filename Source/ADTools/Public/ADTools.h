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
	


	inline static bool bRedIcon = false;

	inline static int iconIndex;
	
private:

	void RegisterMenus();
	void BindCommands();
	void OpenSetting() const;

	void LangSwitcher() const;
	void RestartEditor() const;
	void OpenGitHubUrl() const;
	bool HandleSettingsSaved();

	TSharedRef< class SWidget > GenerateComboMenu(TSharedPtr<class FUICommandList> InCommands);
	 void OnToorBarButtonClick() const;

private:

	
	
	TSharedPtr<FUICommandList> ADToolsCommands;
};
