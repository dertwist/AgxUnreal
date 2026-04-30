# AgX Tonemap Switcher for Unreal Engine

[![Download](https://img.shields.io/github/v/release/dertwist/AgxUnreal?label=Download&style=for-the-badge)](https://github.com/dertwist/AgxUnreal/releases/latest)

A plugin to switch between stock and AgX tonemappers in UE 5.4-5.7.

## Overview
This plugin allows developers to toggle between the standard Unreal Engine tonemapper and the AgX tonemapper variant by swapping internal engine shaders. It includes support for multiple engine versions and a convenient toolbar menu for quick switching.

## Features
- Support for UE 5.4, 5.5, 5.6, and 5.7.
- One-click Enable/Restore via Tools menu or Toolbar.
- Automatic engine shader backup (.bak).
- Self-contained with all necessary shader variants.

## Installation
1. **Download**: Get the latest `AgxTonemapSwitcher-vX.X.X.zip` from the [Releases](https://github.com/dertwist/AgxUnreal/releases) page.
2. **Setup**: Create a `Plugins` folder in your project root if it doesn't exist.
3. **Install**: Extract the `AgxTonemapSwitcher` folder into your project's `Plugins` directory.
4. **Enable**: Open your project, go to **Edit -> Plugins**, and ensure **AgX Tonemap Switcher** is enabled.
5. **Restart**: Restart the Unreal Editor to compile and load the plugin.
6. **Switch**: Use the **Tools -> AgX Tonemap** menu or the toolbar button to switch modes.

> [!NOTE]
> After switching modes, you must restart the editor again to trigger the shader rebuild.

## Credits
Created by Twist.