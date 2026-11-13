#include "PiperTTSTypes.h"

#include "Misc/Paths.h"

namespace
{
FString resolvePath(const FString& path)
{
    if (path.IsEmpty())
    {
        return FString();
    }
    return FPaths::ConvertRelativePathToFull(path);
}
}

FString UPiperVoiceProfile::getRuntimeDirectory() const
{
    return resolvePath(piperRuntimeDirectory.Path);
}

FString UPiperVoiceProfile::getModelPath() const
{
    return resolvePath(voiceModel.FilePath);
}

FString UPiperVoiceProfile::getConfigPath() const
{
    if (!voiceConfig.FilePath.IsEmpty())
    {
        return resolvePath(voiceConfig.FilePath);
    }
    const FString modelPath = getModelPath();
    return modelPath.IsEmpty() ? FString() : modelPath + TEXT(".json");
}

FString UPiperVoiceProfile::getEspeakDataPath() const
{
    return resolvePath(espeakDataDirectory.Path);
}

FString UPiperVoiceProfile::getCacheKey() const
{
    return FString::Printf(
        TEXT("%s|%s|%s"),
        *getModelPath(),
        *getConfigPath(),
        *getEspeakDataPath());
}

bool UPiperVoiceProfile::validate(FString& errorMessage) const
{
    if (!FPaths::DirectoryExists(getRuntimeDirectory()))
    {
        errorMessage = TEXT("The Piper runtime directory does not exist.");
        return false;
    }
    if (!FPaths::FileExists(getModelPath()))
    {
        errorMessage = TEXT("The Piper voice model does not exist.");
        return false;
    }
    if (!FPaths::FileExists(getConfigPath()))
    {
        errorMessage = TEXT("The Piper voice config does not exist.");
        return false;
    }
    if (!FPaths::DirectoryExists(getEspeakDataPath()))
    {
        errorMessage = TEXT("The eSpeak data directory does not exist.");
        return false;
    }
    if (lengthScale <= 0.0f)
    {
        errorMessage = TEXT("Length scale must be greater than zero.");
        return false;
    }
    return true;
}

float FPiperAudioData::getDurationSeconds() const
{
    return sampleRate > 0
        ? static_cast<float>(samples.Num()) / static_cast<float>(sampleRate)
        : 0.0f;
}

bool FPiperAudioData::isValid() const
{
    return sampleRate > 0 && !samples.IsEmpty();
}
