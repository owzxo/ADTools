// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "ADToolsStyle.h"

class FADToolsCommands : public TCommands<FADToolsCommands>
{
public:

	FADToolsCommands()
		: TCommands<FADToolsCommands>(TEXT("ADTools"), NSLOCTEXT("Contexts", "ADTools", "ADTools Plugin"), NAME_None, FADToolsStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;

	/**
	 * 重启编辑器.
	 */
	TSharedPtr< FUICommandInfo > Settings;

	/**
	 * 中英文切换.
	 */
	TSharedPtr< FUICommandInfo > LangSwitcher;

	/**
	 * 重启编辑器.
	 */
	TSharedPtr< FUICommandInfo > ResetEditor;
	
	/**
	 * 重启编辑器.
	 */
	TSharedPtr< FUICommandInfo > GitHubUrl;
};
