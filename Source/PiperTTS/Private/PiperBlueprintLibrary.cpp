#include "PiperBlueprintLibrary.h"

#include "PiperTTSModule.h"

FPiperSynthesisResult UPiperBlueprintLibrary::synthesizeSpeech(
    const FString& text,
    const UPiperVoiceProfile* profile)
{
    if (profile == nullptr)
    {
        FPiperSynthesisResult result;
        result.errorMessage = TEXT("A Piper voice profile is required.");
        return result;
    }
    return FPiperTTSModule::get().synthesize(text, *profile);
}

FString UPiperBlueprintLibrary::textToPhonemes(
    const FString& text, const UPiperVoiceProfile* profile, FString& errorMessage)
{
    errorMessage.Reset();
    if (profile == nullptr)
    {
        errorMessage = TEXT("A Piper voice profile is required.");
        return {};
    }

    FString phonemes;
    const auto result = FPiperTTSModule::get().synthesize(text, *profile, &phonemes);
    if (!result.success)
    {
        errorMessage = result.errorMessage;
        return {};
    }
    if (phonemes.IsEmpty())
    {
        errorMessage = TEXT("Piper returned no phoneme text for this voice and input.");
    }
    return phonemes;
}

float UPiperBlueprintLibrary::getAudioDuration(const FPiperAudioData& audio)
{
    return audio.getDurationSeconds();
}
