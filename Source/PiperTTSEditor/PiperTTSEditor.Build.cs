using UnrealBuildTool;

public class PiperTTSEditor : ModuleRules
{
    public PiperTTSEditor(ReadOnlyTargetRules target) : base(target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Cpp20;

        PublicDependencyModuleNames.AddRange(
            new[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "PiperTTS",
                "UnrealEd"
            });
    }
}
