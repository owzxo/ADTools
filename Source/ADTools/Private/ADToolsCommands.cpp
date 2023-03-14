// Copyright Epic Games, Inc. All Rights Reserved.

#include "ADToolsCommands.h"

#define LOCTEXT_NAMESPACE "FADToolsModule"

void FADToolsCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "ADTools", "Execute ADTools action", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(Settings, "Settings", "Settings", EUserInterfaceActionType::Button, FInputChord());

	
	
	UI_COMMAND(LangSwitcher, "语言切换(中/英)", "语言切换(中/英)", EUserInterfaceActionType::Button,  FInputChord(EModifierKey::Alt | EModifierKey::Shift,EKeys::L));
	UI_COMMAND(ResetEditor, "重启编辑器", "重启编辑器", EUserInterfaceActionType::Button, FInputChord());

	UI_COMMAND(GitHubUrl, "GitHub", "GitHub", EUserInterfaceActionType::Button, FInputChord());
	
}

#undef LOCTEXT_NAMESPACE
