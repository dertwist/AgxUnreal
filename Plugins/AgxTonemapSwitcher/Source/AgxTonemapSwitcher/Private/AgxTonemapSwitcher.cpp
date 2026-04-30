#include "AgxTonemapSwitcher.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "ToolMenus.h"
#include "HAL/FileManager.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FAgxTonemapSwitcherModule"

void FAgxTonemapSwitcherModule::StartupModule()
{
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FAgxTonemapSwitcherModule::RegisterMenus));
}

void FAgxTonemapSwitcherModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
}

void FAgxTonemapSwitcherModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	// Register Menu Entries
	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
		{
			FToolMenuSection& Section = Menu->AddSection("AgxTonemap", LOCTEXT("AgxTonemap", "AgX Tonemap"));
			
			Section.AddMenuEntry(
				"EnableAgx",
				LOCTEXT("EnableAgx", "Enable AgX CombineLUTs"),
				LOCTEXT("EnableAgxTooltip", "Swaps the engine's CombineLUTs shader with the AgX version."),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateRaw(this, &FAgxTonemapSwitcherModule::OnEnableAgx))
			);

			Section.AddMenuEntry(
				"RestoreOriginal",
				LOCTEXT("RestoreOriginal", "Restore original CombineLUTs"),
				LOCTEXT("RestoreOriginalTooltip", "Restores the original engine CombineLUTs shader."),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateRaw(this, &FAgxTonemapSwitcherModule::OnRestoreOriginal))
			);
		}
	}

	// Register Toolbar Button
	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			
			Section.AddEntry(FToolMenuEntry::InitComboButton(
				"AgxTonemapToolbar",
				FUIAction(),
				FOnGetContent::CreateLambda([this]()
				{
					FMenuBuilder MenuBuilder(true, nullptr);
					
					MenuBuilder.BeginSection("AgxTonemap", LOCTEXT("AgxTonemap", "AgX Tonemap"));
					{
						MenuBuilder.AddMenuEntry(
							LOCTEXT("EnableAgx", "Enable AgX CombineLUTs"),
							LOCTEXT("EnableAgxTooltip", "Swaps the engine's CombineLUTs shader with the AgX version."),
							FSlateIcon(),
							FUIAction(FExecuteAction::CreateRaw(this, &FAgxTonemapSwitcherModule::OnEnableAgx))
						);

						MenuBuilder.AddMenuEntry(
							LOCTEXT("RestoreOriginal", "Restore original CombineLUTs"),
							LOCTEXT("RestoreOriginalTooltip", "Restores the original engine CombineLUTs shader."),
							FSlateIcon(),
							FUIAction(FExecuteAction::CreateRaw(this, &FAgxTonemapSwitcherModule::OnRestoreOriginal))
						);
					}
					MenuBuilder.EndSection();

					return MenuBuilder.MakeWidget();
				}),
				LOCTEXT("AgxTonemap", "AgX Tonemap"),
				LOCTEXT("AgxTonemapTooltip", "AgX Tonemap Switching Tools"),
				FSlateIcon(FAppStyle::GetAppStyleSetName(), "EditorViewport.Visualizers")
			));
		}
	}
}

void FAgxTonemapSwitcherModule::OnEnableAgx()
{
	FString VersionStr = FString::Printf(TEXT("%d.%d"), ENGINE_MAJOR_VERSION, ENGINE_MINOR_VERSION);
	FString SourceFileName = FString::Printf(TEXT("PostProcessCombineLUTs_%s.usf"), *VersionStr);

	if (CopyShaderFile(SourceFileName, true))
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("AgxEnabled", "AgX CombineLUTs ({0}) enabled successfully.\n\nPlease restart the editor to rebuild shaders."), FText::FromString(VersionStr)));
	}
}

void FAgxTonemapSwitcherModule::OnRestoreOriginal()
{
	FString VersionStr = FString::Printf(TEXT("%d.%d"), ENGINE_MAJOR_VERSION, ENGINE_MINOR_VERSION);
	FString SourceFileName = FString::Printf(TEXT("PostProcessCombineLUTs_%s_ORIGINAL.usf"), *VersionStr);

	if (CopyShaderFile(SourceFileName, false))
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("OriginalRestored", "Original CombineLUTs ({0}) restored successfully.\n\nPlease restart the editor to rebuild shaders."), FText::FromString(VersionStr)));
	}
}

bool FAgxTonemapSwitcherModule::CopyShaderFile(const FString& SourceFileName, bool bIsAgx)
{
	IFileManager& FileManager = IFileManager::Get();

	// 1. Locate Engine Shader Path
	FString EngineShaderDir = FPaths::EngineDir() / TEXT("Shaders/Private/");
	FString TargetFilePath = EngineShaderDir / TEXT("PostProcessCombineLUTs.usf");
	FString BackupFilePath = EngineShaderDir / TEXT("PostProcessCombineLUTs.usf.bak");

	// 2. Locate Source Shader Path (in plugin's Resources/Shaders folder)
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("AgxTonemapSwitcher"));
	if (!Plugin.IsValid())
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("PluginNotFound", "Could not find AgxTonemapSwitcher plugin directory."));
		return false;
	}
	FString SourceFilePath = Plugin->GetBaseDir() / TEXT("Resources/Shaders") / SourceFileName;

	if (!FileManager.FileExists(*SourceFilePath))
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("SourceNotFound", "Source shader file not found at: {0}"), FText::FromString(SourceFilePath)));
		return false;
	}

	// 3. Create Backup if it doesn't exist
	if (!FileManager.FileExists(*BackupFilePath))
	{
		if (FileManager.Copy(*BackupFilePath, *TargetFilePath) != COPY_OK)
		{
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("BackupFailed", "Failed to create backup of the original shader file.\n\nMake sure you have write access to the engine folder."));
			return false;
		}
	}

	// 4. Copy the new file
	if (FileManager.Copy(*TargetFilePath, *SourceFilePath, true) != COPY_OK)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("CopyFailed", "Failed to copy shader file.\n\nMake sure you have write access to the engine folder."));
		return false;
	}

	return true;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAgxTonemapSwitcherModule, AgxTonemapSwitcher)
