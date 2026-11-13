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

float UPiperBlueprintLibrary::getAudioDuration(const FPiperAudioData& audio)
{
    return audio.getDurationSeconds();
}
