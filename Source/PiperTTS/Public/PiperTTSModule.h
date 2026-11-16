#pragma once

#include "Modules/ModuleManager.h"
#include "PiperTTSTypes.h"

class FPiperTTSModule : public IModuleInterface
{
public:
    static FPiperTTSModule& get();

    virtual void ShutdownModule() override;

    FPiperSynthesisResult synthesize(
        const FString& text,
        const UPiperVoiceProfile& profile, FString* phonemes = nullptr);

private:
    struct FNativeApi;
    struct FCachedSynthesizer;

    bool loadRuntime(const FString& runtimeDirectory, FString& errorMessage);
    void unloadRuntime();
    FCachedSynthesizer* getOrCreateSynthesizer(
        const UPiperVoiceProfile& profile,
        FString& errorMessage);

    TUniquePtr<FNativeApi> nativeApi;
    TMap<FString, TUniquePtr<FCachedSynthesizer>> synthesizers;
    FCriticalSection synthesisMutex;
};
