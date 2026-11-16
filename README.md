# PiperTTS

`PiperTTS` is a minimal Unreal Engine 5.7 bridge for the local Piper C API. It exposes one Blueprint-callable synthesis function:

```text
Synthesize Speech(Text, Profile) -> Result
```

`Result` contains success, an error message, the sample rate and mono float samples. The plugin loads Piper dynamically, so Piper binaries and voice models remain outside this repository.

## Profile

Create a `PiperVoiceProfile` Data Asset and set:

- Piper runtime directory: the directory containing `piper.dll` and `onnxruntime.dll`.
- Voice model: the `.onnx` voice model.
- Voice config: the matching `.onnx.json` file. Leave empty to use `<model>.json`.
- eSpeak data directory: Piper's `espeak-ng-data` directory.
- Speaker and synthesis values when the model requires non-default values.

For `nl_NL-alex-medium`, use `noiseScale = 0.5` and
`noiseWidthScale = 0.333`. These are the values in the voice's model card.

For a Piper checkout built with the commands below, the runtime and eSpeak
paths are:

```text
<piper-checkout>/libpiper/install-ue57/bin
<piper-checkout>/libpiper/install-ue57/share/espeak-ng-data
```

## Blueprint

Use `Synthesize Speech` with the input text and profile asset. The first request for a profile loads its voice model. The synthesizer is cached for later requests.

In an Editor Utility Widget, connect the returned `Audio` value to `Preview
Piper Audio`. The preview node plays through Unreal's editor audio device and
does not require PIE or a saved Sound Wave asset.

Synthesis is synchronous. Short words and phrases are the intended use.

## Piper

Piper is not bundled. Build or install Piper separately and review the Piper engine and selected voice-model licenses before redistribution.

The current `nl_NL-alex-medium` model is a 22,050 Hz single-speaker voice. Its
model card lists the dataset as CC0 and requires Piper 1.5 or newer. Keep the
model card beside the voice if the voice is redistributed.

## Unreal Engine 5.7 runtime compatibility

Unreal Engine 5.7 loads ONNX Runtime 1.20.1 before plugins. Build Piper with
the same version to avoid a Windows DLL version conflict:

```text
cmake -S libpiper -B libpiper/build-ue57-ort120 -DONNXRUNTIME_VERSION=1.20.1 -DCMAKE_INSTALL_PREFIX=libpiper/install-ue57
cmake --build libpiper/build-ue57-ort120 --config Release --target install
```

Current Piper uses one convenience method introduced after ONNX Runtime 1.20.
Apply `Patches/piper-onnxruntime-1.20.patch` to the Piper checkout before the
build. The patch uses the equivalent API available in ONNX Runtime 1.20.

```text
git apply --unidiff-zero /path/to/PiperTTS/Patches/piper-onnxruntime-1.20.patch
```

## Text to phonemes

Call `Text to Phonemes` with text and the same Piper voice profile used for speech.
The node returns the voice's phoneme string and an error message if extraction fails.
For eSpeak voices such as Dutch Alex, the symbols are IPA-style. The node uses
Piper's synthesis chunks and returns only the phoneme string. It does not expose
phoneme timing.
