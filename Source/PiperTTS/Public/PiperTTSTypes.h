#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PiperTTSTypes.generated.h"

UCLASS(BlueprintType)
class PIPERTTS_API UPiperVoiceProfile : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Piper")
    FText displayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Piper")
    FDirectoryPath piperRuntimeDirectory;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Piper", meta = (FilePathFilter = "onnx"))
    FFilePath voiceModel;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Piper", meta = (FilePathFilter = "json"))
    FFilePath voiceConfig;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Piper")
    FDirectoryPath espeakDataDirectory;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Piper", meta = (ClampMin = "0"))
    int32 speakerId = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Piper", meta = (ClampMin = "0.05"))
    float lengthScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Piper", meta = (ClampMin = "0.0"))
    float noiseScale = 0.667f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Piper", meta = (ClampMin = "0.0"))
    float noiseWidthScale = 0.8f;

    FString getRuntimeDirectory() const;
    FString getModelPath() const;
    FString getConfigPath() const;
    FString getEspeakDataPath() const;
    FString getCacheKey() const;
    bool validate(FString& errorMessage) const;
};

USTRUCT(BlueprintType)
struct PIPERTTS_API FPiperAudioData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Piper")
    int32 sampleRate = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Piper")
    TArray<float> samples;

    float getDurationSeconds() const;

    bool isValid() const;
};

USTRUCT(BlueprintType)
struct PIPERTTS_API FPiperSynthesisResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Piper")
    bool success = false;

    UPROPERTY(BlueprintReadOnly, Category = "Piper")
    FString errorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Piper")
    FPiperAudioData audio;
};
