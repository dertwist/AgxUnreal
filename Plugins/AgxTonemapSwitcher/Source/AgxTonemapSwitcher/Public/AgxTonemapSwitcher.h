#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAgxTonemapSwitcher, Log, All);

class FAgxTonemapSwitcherModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to our Command. */
	void OnEnableAgx();
	void OnRestoreOriginal();

private:
	void RegisterMenus();
	bool CopyShaderFile(const FString& SourceName, bool bIsAgx);
};

DECLARE_LOG_CATEGORY_EXTERN(LogAgxTonemapSwitcher, Log, All);
static const FName AgxPluginName = TEXT("AgxTonemapSwitcher");

