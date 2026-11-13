#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PiperTTSTypes.h"
#include "PiperEditorBlueprintLibrary.generated.h"

UCLASS()
class PIPERTTSEDITOR_API UPiperEditorBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Piper TTS|Editor", meta = (DisplayName = "Preview Piper Audio"))
    static bool previewPiperAudio(
        const FPiperAudioData& audio,
        FString& errorMessage);
};
