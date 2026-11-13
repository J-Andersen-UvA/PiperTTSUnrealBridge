#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "HAL/PlatformMisc.h"
#include "PiperBlueprintLibrary.h"
#include "PiperTTSTypes.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPiperTTSSynthesisTest,
    "PiperTTS.Synthesis.DutchAlex",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPiperTTSSynthesisTest::RunTest(const FString& parameters)
{
    const FString runtimeDirectory =
        FPlatformMisc::GetEnvironmentVariable(TEXT("PIPER_TEST_RUNTIME"));
    const FString modelPath =
        FPlatformMisc::GetEnvironmentVariable(TEXT("PIPER_TEST_MODEL"));
    const FString espeakDataDirectory =
        FPlatformMisc::GetEnvironmentVariable(TEXT("PIPER_TEST_ESPEAK_DATA"));

    if (runtimeDirectory.IsEmpty() ||
        modelPath.IsEmpty() ||
        espeakDataDirectory.IsEmpty())
    {
        AddWarning(TEXT("Piper test paths are not configured."));
        return true;
    }

    UPiperVoiceProfile* profile = NewObject<UPiperVoiceProfile>();
    profile->piperRuntimeDirectory.Path = runtimeDirectory;
    profile->voiceModel.FilePath = modelPath;
    profile->espeakDataDirectory.Path = espeakDataDirectory;
    profile->noiseScale = 0.5f;
    profile->noiseWidthScale = 0.333f;

    const FPiperSynthesisResult result =
        UPiperBlueprintLibrary::synthesizeSpeech(
            TEXT("Hallo wereld. Dit is een Nederlandse test."),
            profile);

    TestTrue(*result.errorMessage, result.success);
    TestEqual(TEXT("Sample rate"), result.audio.sampleRate, 22050);
    TestTrue(TEXT("Generated samples"), !result.audio.samples.IsEmpty());
    TestTrue(TEXT("Positive duration"), result.audio.getDurationSeconds() > 0.0f);
    return true;
}

#endif
