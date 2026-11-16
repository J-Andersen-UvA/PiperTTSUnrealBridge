#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PiperTTSTypes.h"
#include "PiperBlueprintLibrary.generated.h"

UCLASS()
class PIPERTTS_API UPiperBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Piper TTS", meta = (DisplayName = "Synthesize Speech"))
    static FPiperSynthesisResult synthesizeSpeech(
        const FString& text,
        const UPiperVoiceProfile* profile);

    UFUNCTION(BlueprintCallable, Category = "Piper TTS", meta = (DisplayName = "Text to Phonemes"))
    static FString textToPhonemes(const FString& text, const UPiperVoiceProfile* profile, FString& errorMessage);

    UFUNCTION(BlueprintPure, Category = "Piper TTS", meta = (DisplayName = "Get Piper Audio Duration"))
    static float getAudioDuration(const FPiperAudioData& audio);
};
