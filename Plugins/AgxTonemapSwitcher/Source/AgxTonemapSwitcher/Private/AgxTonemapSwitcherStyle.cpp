#include "AgxTonemapSwitcherStyle.h"
#include "AgxTonemapSwitcher.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Interfaces/IPluginManager.h"
#include "Slate/SlateGameResources.h"

TSharedPtr<FSlateStyleSet> FAgxTonemapSwitcherStyle::StyleInstance = nullptr;

void FAgxTonemapSwitcherStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FAgxTonemapSwitcherStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FAgxTonemapSwitcherStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("AgxTonemapSwitcherStyle"));
	return StyleSetName;
}

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( StyleInstance->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )

TSharedRef<FSlateStyleSet> FAgxTonemapSwitcherStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet(FAgxTonemapSwitcherStyle::GetStyleSetName()));
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(AgxPluginName.ToString());
	if (Plugin.IsValid())
	{
		Style->SetContentRoot(Plugin->GetBaseDir() / TEXT("Resources"));
	}

	Style->Set("AgxTonemapSwitcher.Icon", new IMAGE_BRUSH(TEXT("Icon128"), FVector2D(40.0f, 40.0f)));

	return Style;
}

#undef IMAGE_BRUSH

void FAgxTonemapSwitcherStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FAgxTonemapSwitcherStyle::Get()
{
	return *StyleInstance;
}
