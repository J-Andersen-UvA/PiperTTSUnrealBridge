#include "PiperTTSModule.h"

#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"
#include "Misc/ScopeLock.h"

namespace
{
constexpr int32 piperOk = 0;
constexpr int32 piperDone = 1;

struct FPiperNativeAudioChunk
{
    const float* samples;
    SIZE_T sampleCount;
    int32 sampleRate;
    bool isLast;
    const char32_t* phonemes;
    SIZE_T phonemeCount;
    const int32* phonemeIds;
    SIZE_T phonemeIdCount;
    const int32* alignments;
    SIZE_T alignmentCount;
};

struct FPiperNativeSynthesizeOptions
{
    int32 speakerId;
    float lengthScale;
    float noiseScale;
    float noiseWidthScale;
};
}

struct FPiperTTSModule::FNativeApi
{
    using FCreate = void* (*)(const char*, const char*, const char*);
    using FFree = void (*)(void*);
    using FDefaultOptions = FPiperNativeSynthesizeOptions (*)(void*);
    using FStart = int32 (*)(void*, const char*, const FPiperNativeSynthesizeOptions*);
    using FNext = int32 (*)(void*, FPiperNativeAudioChunk*);

    void* onnxRuntimeHandle = nullptr;
    void* onnxProvidersHandle = nullptr;
    void* piperHandle = nullptr;
    FString runtimeDirectory;
    FCreate create = nullptr;
    FFree free = nullptr;
    FDefaultOptions defaultOptions = nullptr;
    FStart start = nullptr;
    FNext next = nullptr;
};

struct FPiperTTSModule::FCachedSynthesizer
{
    void* handle = nullptr;
};

FPiperTTSModule& FPiperTTSModule::get()
{
    return FModuleManager::LoadModuleChecked<FPiperTTSModule>(TEXT("PiperTTS"));
}

void FPiperTTSModule::ShutdownModule()
{
    FScopeLock lock(&synthesisMutex);
    unloadRuntime();
}

FPiperSynthesisResult FPiperTTSModule::synthesize(
    const FString& text,
    const UPiperVoiceProfile& profile)
{
    FPiperSynthesisResult result;
    if (text.TrimStartAndEnd().IsEmpty())
    {
        result.errorMessage = TEXT("Text is empty.");
        return result;
    }
    if (!profile.validate(result.errorMessage))
    {
        return result;
    }

    FScopeLock lock(&synthesisMutex);
    if (!loadRuntime(profile.getRuntimeDirectory(), result.errorMessage))
    {
        return result;
    }

    FCachedSynthesizer* synthesizer =
        getOrCreateSynthesizer(profile, result.errorMessage);
    if (synthesizer == nullptr)
    {
        return result;
    }

    FPiperNativeSynthesizeOptions options =
        nativeApi->defaultOptions(synthesizer->handle);
    options.speakerId = profile.speakerId;
    options.lengthScale = profile.lengthScale;
    options.noiseScale = profile.noiseScale;
    options.noiseWidthScale = profile.noiseWidthScale;

    const FTCHARToUTF8 utf8Text(*text);
    if (nativeApi->start(synthesizer->handle, utf8Text.Get(), &options) != piperOk)
    {
        result.errorMessage = TEXT("Piper could not start synthesis.");
        return result;
    }

    while (true)
    {
        FPiperNativeAudioChunk chunk{};
        const int32 status = nativeApi->next(synthesizer->handle, &chunk);
        if (status != piperOk && status != piperDone)
        {
            result.audio = FPiperAudioData();
            result.errorMessage = TEXT("Piper failed while generating audio.");
            return result;
        }
        if (chunk.samples != nullptr && chunk.sampleCount > 0 && chunk.sampleRate > 0)
        {
            if (result.audio.sampleRate == 0)
            {
                result.audio.sampleRate = chunk.sampleRate;
            }
            else if (result.audio.sampleRate != chunk.sampleRate)
            {
                result.audio = FPiperAudioData();
                result.errorMessage = TEXT("Piper returned inconsistent sample rates.");
                return result;
            }
            result.audio.samples.Append(
                chunk.samples,
                static_cast<int32>(chunk.sampleCount));
        }
        if (status == piperDone)
        {
            break;
        }
    }

    result.success = result.audio.isValid();
    if (!result.success)
    {
        result.errorMessage = TEXT("Piper returned no audio samples.");
    }
    return result;
}

bool FPiperTTSModule::loadRuntime(
    const FString& runtimeDirectory,
    FString& errorMessage)
{
    if (nativeApi.IsValid())
    {
        if (FPaths::IsSamePath(nativeApi->runtimeDirectory, runtimeDirectory))
        {
            return true;
        }
        errorMessage = TEXT("A different Piper runtime is already loaded.");
        return false;
    }

    TUniquePtr<FNativeApi> loadedApi = MakeUnique<FNativeApi>();
    loadedApi->runtimeDirectory = runtimeDirectory;
    const FString onnxRuntimePath =
        FPaths::Combine(runtimeDirectory, TEXT("onnxruntime.dll"));
    const FString onnxProvidersPath =
        FPaths::Combine(runtimeDirectory, TEXT("onnxruntime_providers_shared.dll"));
    const FString piperPath = FPaths::Combine(runtimeDirectory, TEXT("piper.dll"));

    loadedApi->onnxRuntimeHandle = FPlatformProcess::GetDllHandle(*onnxRuntimePath);
    if (loadedApi->onnxRuntimeHandle == nullptr)
    {
        errorMessage = FString::Printf(
            TEXT("Could not load %s."),
            *onnxRuntimePath);
        return false;
    }
    if (FPaths::FileExists(onnxProvidersPath))
    {
        loadedApi->onnxProvidersHandle =
            FPlatformProcess::GetDllHandle(*onnxProvidersPath);
    }
    loadedApi->piperHandle = FPlatformProcess::GetDllHandle(*piperPath);
    if (loadedApi->piperHandle == nullptr)
    {
        FPlatformProcess::FreeDllHandle(loadedApi->onnxRuntimeHandle);
        if (loadedApi->onnxProvidersHandle != nullptr)
        {
            FPlatformProcess::FreeDllHandle(loadedApi->onnxProvidersHandle);
        }
        errorMessage = FString::Printf(TEXT("Could not load %s."), *piperPath);
        return false;
    }

    loadedApi->create = reinterpret_cast<FNativeApi::FCreate>(
        FPlatformProcess::GetDllExport(loadedApi->piperHandle, TEXT("piper_create")));
    loadedApi->free = reinterpret_cast<FNativeApi::FFree>(
        FPlatformProcess::GetDllExport(loadedApi->piperHandle, TEXT("piper_free")));
    loadedApi->defaultOptions = reinterpret_cast<FNativeApi::FDefaultOptions>(
        FPlatformProcess::GetDllExport(
            loadedApi->piperHandle,
            TEXT("piper_default_synthesize_options")));
    loadedApi->start = reinterpret_cast<FNativeApi::FStart>(
        FPlatformProcess::GetDllExport(
            loadedApi->piperHandle,
            TEXT("piper_synthesize_start")));
    loadedApi->next = reinterpret_cast<FNativeApi::FNext>(
        FPlatformProcess::GetDllExport(
            loadedApi->piperHandle,
            TEXT("piper_synthesize_next")));

    if (loadedApi->create == nullptr ||
        loadedApi->free == nullptr ||
        loadedApi->defaultOptions == nullptr ||
        loadedApi->start == nullptr ||
        loadedApi->next == nullptr)
    {
        FPlatformProcess::FreeDllHandle(loadedApi->piperHandle);
        if (loadedApi->onnxProvidersHandle != nullptr)
        {
            FPlatformProcess::FreeDllHandle(loadedApi->onnxProvidersHandle);
        }
        FPlatformProcess::FreeDllHandle(loadedApi->onnxRuntimeHandle);
        errorMessage = TEXT("The Piper runtime does not expose the required C API.");
        return false;
    }

    nativeApi = MoveTemp(loadedApi);
    return true;
}

void FPiperTTSModule::unloadRuntime()
{
    if (!nativeApi.IsValid())
    {
        return;
    }
    for (TPair<FString, TUniquePtr<FCachedSynthesizer>>& entry : synthesizers)
    {
        if (entry.Value.IsValid() && entry.Value->handle != nullptr)
        {
            nativeApi->free(entry.Value->handle);
        }
    }
    synthesizers.Reset();

    if (nativeApi->piperHandle != nullptr)
    {
        FPlatformProcess::FreeDllHandle(nativeApi->piperHandle);
    }
    if (nativeApi->onnxProvidersHandle != nullptr)
    {
        FPlatformProcess::FreeDllHandle(nativeApi->onnxProvidersHandle);
    }
    if (nativeApi->onnxRuntimeHandle != nullptr)
    {
        FPlatformProcess::FreeDllHandle(nativeApi->onnxRuntimeHandle);
    }
    nativeApi.Reset();
}

FPiperTTSModule::FCachedSynthesizer* FPiperTTSModule::getOrCreateSynthesizer(
    const UPiperVoiceProfile& profile,
    FString& errorMessage)
{
    const FString cacheKey = profile.getCacheKey();
    if (TUniquePtr<FCachedSynthesizer>* cached = synthesizers.Find(cacheKey))
    {
        return cached->Get();
    }

    const FTCHARToUTF8 modelPath(*profile.getModelPath());
    const FTCHARToUTF8 configPath(*profile.getConfigPath());
    const FTCHARToUTF8 espeakDataPath(*profile.getEspeakDataPath());
    void* handle = nativeApi->create(
        modelPath.Get(),
        configPath.Get(),
        espeakDataPath.Get());
    if (handle == nullptr)
    {
        errorMessage = TEXT("Piper could not load the voice profile.");
        return nullptr;
    }

    TUniquePtr<FCachedSynthesizer> synthesizer =
        MakeUnique<FCachedSynthesizer>();
    synthesizer->handle = handle;
    FCachedSynthesizer* result = synthesizer.Get();
    synthesizers.Add(cacheKey, MoveTemp(synthesizer));
    return result;
}

IMPLEMENT_MODULE(FPiperTTSModule, PiperTTS)
