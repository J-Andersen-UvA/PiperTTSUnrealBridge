#include "PiperEditorBlueprintLibrary.h"

#include "Editor.h"
#include "Sound/SoundWaveProcedural.h"

bool UPiperEditorBlueprintLibrary::previewPiperAudio(
    const FPiperAudioData& audio,
    FString& errorMessage)
{
    errorMessage.Reset();
    if (!audio.isValid())
    {
        errorMessage = TEXT("Piper audio is empty.");
        return false;
    }
    if (GEditor == nullptr)
    {
        errorMessage = TEXT("The Unreal Editor is not available.");
        return false;
    }
    if (!GEditor->CanPlayEditorSound())
    {
        errorMessage = TEXT("Editor sounds are disabled or unavailable.");
        return false;
    }

    USoundWaveProcedural* soundWave =
        NewObject<USoundWaveProcedural>(GetTransientPackage());
    soundWave->SetSampleRate(static_cast<uint32>(audio.sampleRate));
    soundWave->SetNumFrames(static_cast<uint32>(audio.samples.Num()));
    soundWave->NumChannels = 1;
    soundWave->Duration = audio.getDurationSeconds();

    TArray<int16> pcmSamples;
    pcmSamples.SetNumUninitialized(audio.samples.Num());
    for (int32 index = 0; index < audio.samples.Num(); ++index)
    {
        const float sample = FMath::Clamp(audio.samples[index], -1.0f, 1.0f);
        pcmSamples[index] = static_cast<int16>(FMath::RoundToInt(sample * 32767.0f));
    }

    soundWave->QueueAudio(
        reinterpret_cast<const uint8*>(pcmSamples.GetData()),
        pcmSamples.Num() * sizeof(int16));

    if (GEditor->PlayPreviewSound(soundWave) == nullptr)
    {
        errorMessage = TEXT("Unreal could not start editor audio preview.");
        return false;
    }
    return true;
}
