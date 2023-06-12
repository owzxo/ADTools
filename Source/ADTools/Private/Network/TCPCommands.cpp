#include "Network/TCPCommands.h"
#include "ADTools.h"
#include "ADToolsGeneric.h"
#include "Network/TCPManager.h"
//解析TCP命令.



/**
 * 提取命令行参数.
 */
static void EXtractCommandParameters(const FString& TCPMessage,FString& CommandNameOut,FString& CommandParameterStringOut)
{
	int32 ParameterSectionIndex;
	if(!TCPMessage.FindChar(TEXT(' '),ParameterSectionIndex))
	{
		CommandNameOut = TCPMessage;
		return;
	}
	CommandParameterStringOut = TCPMessage.RightChop(ParameterSectionIndex + 1);
	CommandNameOut = TCPMessage.Left(ParameterSectionIndex);
}

void ExecuteTCPCommand(FString MetaCommandStr)
{
	UE_LOG(LogADTools,Display,TEXT("Executing TCP Command:\n%s"),*MetaCommandStr);
	TArray<FString> MetaCommandList;//命令列表
	MetaCommandStr.ParseIntoArrayLines(MetaCommandList,true);
	UWorld* EditorWorld = GetEditorWorld();

	if(EditorWorld==nullptr)
	{
		return;
	}

	GEditor->BeginTransaction(FText::FromString("ADTools Command"));
	for (auto& CommandStr:MetaCommandList)
	{
		FString CommandName;
		FString CommandParameterString;
		TArray<FString> CommandParameterList;

		EXtractCommandParameters(CommandStr,CommandName,CommandParameterString);
		CommandParameterString.ParseIntoArray(CommandParameterList,TEXT(" "),true);

		if(CommandName == "scene_name")
		{
			FString SceneName = GetSceneName(EditorWorld);
			UE_LOG(LogADTools,Display,TEXT("Current Scene Name %s"),*SceneName);
			TCPSend(SceneName);
		}
		else if(CommandName == "get_selected_and_visible_static_mesh_actors")
		{
			TArray<AActor*> SelectedActors = GetSelectedValidActors();
			TArray<AActor*> RenderedActors = GetRenderedActors();
			
		}
			


		
	}
	GEditor->EndTransaction();
	UE_LOG(LogADTools,Display,TEXT("TCP命令执行完成"));
	GEditor->NoteSelectionChange(true);
	
}
