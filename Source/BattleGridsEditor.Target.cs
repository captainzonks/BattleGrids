// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class BattleGridsEditorTarget : TargetRules
{
	public BattleGridsEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;

		ExtraModuleNames.AddRange(new[] {"BattleGrids"});
	}
}