#include "AgxTonemapSwitcher.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "ToolMenus.h"
#include "HAL/FileManager.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FAgxTonemapSwitcherModule"

DEFINE_LOG_CATEGORY(LogAgxTonemapSwitcher);

static const FName AgxPluginName = TEXT("AgxTonemapSwitcher");

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
		if (Menu)
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
		if (ToolbarMenu)
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
	if (ENGINE_MAJOR_VERSION != 5 || ENGINE_MINOR_VERSION < 4 || ENGINE_MINOR_VERSION > 7)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("UnsupportedVersion", "This version of Unreal Engine is not supported by the AgX Switcher."));
		return;
	}

	FString VersionStr = FString::Printf(TEXT("%d.%d"), ENGINE_MAJOR_VERSION, ENGINE_MINOR_VERSION);
	FString SourceFileName = FString::Printf(TEXT("PostProcessCombineLUTs_%s.usf"), *VersionStr);

	if (CopyShaderFile(SourceFileName, true))
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("AgxEnabled", "AgX CombineLUTs ({0}) enabled successfully.\n\nPlease restart the editor to rebuild shaders."), FText::FromString(VersionStr)));
	}
}

void FAgxTonemapSwitcherModule::OnRestoreOriginal()
{
	if (ENGINE_MAJOR_VERSION != 5 || ENGINE_MINOR_VERSION < 4 || ENGINE_MINOR_VERSION > 7)
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("UnsupportedVersion", "This version of Unreal Engine is not supported by the AgX Switcher."));
		return;
	}

	IFileManager& FileManager = IFileManager::Get();
	const FString EngineShaderDir = FPaths::Combine(FPaths::EngineDir(), TEXT("Shaders"), TEXT("Private"));
	const FString TargetFilePath = FPaths::Combine(EngineShaderDir, TEXT("PostProcessCombineLUTs.usf"));
	const FString BackupFilePath = FPaths::Combine(EngineShaderDir, TEXT("PostProcessCombineLUTs.usf.agx_backup"));

	// 1. Try to restore from the actual backup file created by the plugin
	if (FileManager.FileExists(*BackupFilePath))
	{
		if (FileManager.Copy(*TargetFilePath, *BackupFilePath, true) == COPY_OK)
		{
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("OriginalRestoredFromBak", "Original shader restored from backup (.agx_backup) successfully.\n\nPlease restart the editor to rebuild shaders."));
			return;
		}
		else
		{
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("RestorationFromBakFailed", "Failed to restore from .agx_backup file.\n\nThis usually happens due to missing write permissions in the Engine folder. On Windows, try running the Editor as Administrator. On Mac, ensure the Engine folder is writable. Falling back to internal stock shader..."));
		}
	}

	// 2. Fallback: use the stock shader from the plugin resources
	FString VersionStr = FString::Printf(TEXT("%d.%d"), ENGINE_MAJOR_VERSION, ENGINE_MINOR_VERSION);
	FString SourceFileName = FString::Printf(TEXT("PostProcessCombineLUTs_%s_ORIGINAL.usf"), *VersionStr);

	if (CopyShaderFile(SourceFileName, false))
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("OriginalRestored", "Original CombineLUTs ({0}) restored from plugin resources successfully.\n\nPlease restart the editor to rebuild shaders."), FText::FromString(VersionStr)));
	}
}

bool FAgxTonemapSwitcherModule::CopyShaderFile(const FString& SourceFileName, bool bIsAgx)
{
	IFileManager& FileManager = IFileManager::Get();

	// 1. Locate Engine Shader Path
	const FString EngineShaderDir = FPaths::Combine(FPaths::EngineDir(), TEXT("Shaders"), TEXT("Private"));
	const FString TargetFilePath = FPaths::Combine(EngineShaderDir, TEXT("PostProcessCombineLUTs.usf"));
	const FString BackupFilePath = FPaths::Combine(EngineShaderDir, TEXT("PostProcessCombineLUTs.usf.agx_backup"));

	// 2. Locate Source Shader Path (in plugin's Resources/Shaders folder)
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(AgxPluginName.ToString());
	if (!Plugin.IsValid())
	{
		UE_LOG(LogAgxTonemapSwitcher, Error, TEXT("Could not find AgxTonemapSwitcher plugin directory."));
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("PluginNotFound", "Could not find AgxTonemapSwitcher plugin directory."));
		return false;
	}
	
	const FString SourceFilePath = FPaths::Combine(Plugin->GetBaseDir(), TEXT("Resources"), TEXT("Shaders"), SourceFileName);

	if (!FileManager.FileExists(*SourceFilePath))
	{
		UE_LOG(LogAgxTonemapSwitcher, Error, TEXT("Source shader file not found at: %s"), *SourceFilePath);
		FMessageDialog::Open(EAppMsgType::Ok, FText::Format(LOCTEXT("SourceNotFound", "Source shader file not found at: {0}"), FText::FromString(SourceFilePath)));
		return false;
	}

	// 3. Create Backup if it doesn't exist
	if (!FileManager.FileExists(*BackupFilePath))
	{
		if (FileManager.Copy(*BackupFilePath, *TargetFilePath) != COPY_OK)
		{
			UE_LOG(LogAgxTonemapSwitcher, Error, TEXT("Failed to create backup of the original shader file to: %s"), *BackupFilePath);
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("BackupFailed", "Failed to create backup of the original shader file.\n\nThis usually happens due to missing write permissions in the Engine folder. On Windows, try running the Editor as Administrator. On Mac, ensure the Engine folder is writable."));
			return false;
		}
		UE_LOG(LogAgxTonemapSwitcher, Log, TEXT("Created shader backup at: %s"), *BackupFilePath);
	}

	// 4. Handle Read-Only attributes on target
	if (FileManager.FileExists(*TargetFilePath) && FileManager.IsReadOnly(*TargetFilePath))
	{
		UE_LOG(LogAgxTonemapSwitcher, Warning, TEXT("Target file is read-only. Attempting to remove attribute: %s"), *TargetFilePath);
		if (!FileManager.SetReadOnly(*TargetFilePath, false))
		{
			UE_LOG(LogAgxTonemapSwitcher, Error, TEXT("Failed to remove read-only attribute from: %s"), *TargetFilePath);
		}
	}

	// 5. Copy the new file
	if (FileManager.Copy(*TargetFilePath, *SourceFilePath, true) != COPY_OK)
	{
		UE_LOG(LogAgxTonemapSwitcher, Error, TEXT("Failed to copy shader file from %s to %s"), *SourceFilePath, *TargetFilePath);
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("CopyFailed", "Failed to copy shader file.\n\nThis usually happens due to missing write permissions in the Engine folder. On Windows, try running the Editor as Administrator. On Mac, ensure the Engine folder is writable."));
		return false;
	}

	UE_LOG(LogAgxTonemapSwitcher, Log, TEXT("Successfully swapped shader to: %s"), *SourceFileName);
	return true;
}


#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FAgxTonemapSwitcherModule, AgxTonemapSwitcher)
