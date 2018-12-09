#include "main.hpp"
#include "configure.hpp"
#include "premiereParams.hpp"
#include "prstring.hpp"
#include "export_settings.hpp"
#include "exporter/exporter.hpp"
#include "configure.hpp"
#include <codecvt>
#include <vector>
#include <locale>

csSDK_int32 GetNumberOfAudioChannels(csSDK_int32 audioChannelType);
static void renderAndWriteAllAudio(exDoExportRec *exportInfoP, prMALError &error, MovieWriter *writer);

class StringForPr
{
public:
    StringForPr(const std::wstring &from)
    : prString_(from.size() + 1) {
        copyConvertStringLiteralIntoUTF16(from.c_str(), prString_.data());
    };
    const prUTF16Char *get() const {
        return prString_.data();
    }
private:
    std::vector<prUTF16Char> prString_;
};

DllExport PREMPLUGENTRY xSDKExport(csSDK_int32 selector, exportStdParms* stdParmsP, void* param1, void* param2)
{
	prMALError result = exportReturn_Unsupported;

	switch (selector)
	{
	case exSelStartup:
		result = startup(stdParmsP, reinterpret_cast<exExporterInfoRec*>(param1));
		break;

	case exSelBeginInstance:
		result = beginInstance(stdParmsP, reinterpret_cast<exExporterInstanceRec*>(param1));
		break;

	case exSelEndInstance:
		result = endInstance(stdParmsP, reinterpret_cast<exExporterInstanceRec*>(param1));
		break;

	case exSelGenerateDefaultParams:
		result = generateDefaultParams(stdParmsP, reinterpret_cast<exGenerateDefaultParamRec*>(param1));
		break;

	case exSelPostProcessParams:
		result = postProcessParams(stdParmsP, reinterpret_cast<exPostProcessParamsRec*>(param1));
		break;

	case exSelGetParamSummary:
		result = getParamSummary(stdParmsP, reinterpret_cast<exParamSummaryRec*>(param1));
		break;

	case exSelQueryOutputSettings:
		result = queryOutputSettings(stdParmsP, reinterpret_cast<exQueryOutputSettingsRec*>(param1));
		break;

	case exSelQueryExportFileExtension:
		result = fileExtension(stdParmsP, reinterpret_cast<exQueryExportFileExtensionRec*>(param1));
		break;

	case exSelParamButton:
		result = paramButton(stdParmsP, reinterpret_cast<exParamButtonRec*>(param1));
		break;

	case exSelValidateParamChanged:
		result = validateParamChanged(stdParmsP, reinterpret_cast<exParamChangedRec*>(param1));
		break;

	case exSelValidateOutputSettings:
		result = malNoError;
		break;

	case exSelExport:
		result = doExport(stdParmsP, reinterpret_cast<exDoExportRec*>(param1));
		break;

	default:
		break;
	}
	return result;
}

prMALError startup(exportStdParms* stdParms, exExporterInfoRec* infoRec)
{
    if (infoRec->exportReqIndex == 0)
    {
        infoRec->classID = HAP_VIDEOCLSS;
        infoRec->fileType = HAP_VIDEOFILETYPE;
        infoRec->hideInUI = kPrFalse;
        infoRec->isCacheable = kPrFalse;
        infoRec->exportReqIndex = 0;
        infoRec->canExportVideo = kPrTrue;
        infoRec->canExportAudio = kPrTrue;
        infoRec->canEmbedCaptions = kPrFalse;
        infoRec->canConformToMatchParams = kPrTrue;
        infoRec->singleFrameOnly = kPrFalse;
        infoRec->wantsNoProgressBar = kPrFalse;
        infoRec->doesNotSupportAudioOnly = kPrTrue;
        infoRec->interfaceVersion = EXPORTMOD_VERSION;
        copyConvertStringLiteralIntoUTF16(HAP_VIDEO_FILE_NAME, infoRec->fileTypeName);
        copyConvertStringLiteralIntoUTF16(HAP_VIDEOFILEEXT, infoRec->fileTypeDefaultExtension);
        return exportReturn_IterateExporter;
    }

    return exportReturn_IterateExporterDone;
}

prMALError beginInstance(exportStdParms* stdParmsP, exExporterInstanceRec* instanceRecP)
{
	SPErr spError = kSPNoError;
	PrSDKMemoryManagerSuite* memorySuite;
	SPBasicSuite* spBasic = stdParmsP->getSPBasicSuite();

	if (spBasic == nullptr)
		return exportReturn_ErrMemory;

	spError = spBasic->AcquireSuite(kPrSDKMemoryManagerSuite, kPrSDKMemoryManagerSuiteVersion, const_cast<const void**>(reinterpret_cast<void**>(&memorySuite)));
	void *settingsMem = (memorySuite->NewPtrClear(sizeof(ExportSettings)));

	if (settingsMem == nullptr)
		return exportReturn_ErrMemory;

	ExportSettings* settings = new(settingsMem) ExportSettings();

	settings->fileType = instanceRecP->fileType;
	settings->spBasic = spBasic;
	settings->memorySuite = memorySuite;
    spError = spBasic->AcquireSuite(kPrSDKExporterUtilitySuite, kPrSDKExporterUtilitySuiteVersion, const_cast<const void**>(reinterpret_cast<void**>(&(settings->exporterUtilitySuite))));
    spError = spBasic->AcquireSuite(kPrSDKExportParamSuite, kPrSDKExportParamSuiteVersion, const_cast<const void**>(reinterpret_cast<void**>(&(settings->exportParamSuite))));
    spError = spBasic->AcquireSuite(kPrSDKExportProgressSuite, kPrSDKExportProgressSuiteVersion, const_cast<const void**>(reinterpret_cast<void**>(&(settings->exportProgressSuite))));
	spError = spBasic->AcquireSuite(kPrSDKExportFileSuite, kPrSDKExportFileSuiteVersion, const_cast<const void**>(reinterpret_cast<void**>(&(settings->exportFileSuite))));
	spError = spBasic->AcquireSuite(kPrSDKExportInfoSuite, kPrSDKExportInfoSuiteVersion, const_cast<const void**>(reinterpret_cast<void**>(&(settings->exportInfoSuite))));
	spError = spBasic->AcquireSuite(kPrSDKErrorSuite, kPrSDKErrorSuiteVersion3, const_cast<const void**>(reinterpret_cast<void**>(&(settings->errorSuite))));
	spError = spBasic->AcquireSuite(kPrSDKClipRenderSuite, kPrSDKClipRenderSuiteVersion, const_cast<const void**>(reinterpret_cast<void**>(&(settings->clipRenderSuite))));
	spError = spBasic->AcquireSuite(kPrSDKMarkerSuite, kPrSDKMarkerSuiteVersion, const_cast<const void**>(reinterpret_cast<void**>(&(settings->markerSuite))));
	spError = spBasic->AcquireSuite(kPrSDKPPixSuite, kPrSDKPPixSuiteVersion, const_cast<const void**>(reinterpret_cast<void**>(&(settings->ppixSuite))));
	spError = spBasic->AcquireSuite(kPrSDKTimeSuite, kPrSDKTimeSuiteVersion, const_cast<const void**>(reinterpret_cast<void**>(&(settings->timeSuite))));
    spError = spBasic->AcquireSuite(kPrSDKWindowSuite, kPrSDKWindowSuiteVersion, const_cast<const void**>(reinterpret_cast<void**>(&(settings->windowSuite))));
    spError = spBasic->AcquireSuite(kPrSDKAudioSuite, kPrSDKAudioSuiteVersion, const_cast<const void**>(reinterpret_cast<void**>(&(settings->audioSuite))));
    spError = spBasic->AcquireSuite(kPrSDKSequenceAudioSuite, kPrSDKSequenceAudioSuiteVersion1, const_cast<const void**>(reinterpret_cast<void**>(&(settings->sequenceAudioSuite))));

    // convenience callback
    auto report = settings->exporterUtilitySuite->ReportEvent;
    auto pluginId = instanceRecP->exporterPluginID;
    settings->reportError = [report, pluginId](const std::string& error) {

        StringForPr title(L"HAP encoder plugin");
        StringForPr detail(std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}.from_bytes(error));

        report(
            pluginId, PrSDKErrorSuite2::kEventTypeError,
            title.get(),
            detail.get());
    };

	instanceRecP->privateData = reinterpret_cast<void*>(settings);

	return malNoError;
}

prMALError endInstance(exportStdParms* stdParmsP, exExporterInstanceRec* instanceRecP)
{
	prMALError result = malNoError;
	ExportSettings* settings = reinterpret_cast<ExportSettings*>(instanceRecP->privateData);
	SPBasicSuite* spBasic = stdParmsP->getSPBasicSuite();

	if (spBasic == nullptr || settings == nullptr)
		return malNoError;

    if (settings->exporterUtilitySuite)
        result = spBasic->ReleaseSuite(kPrSDKExporterUtilitySuite, kPrSDKExporterUtilitySuiteVersion);

    if (settings->exportParamSuite)
        result = spBasic->ReleaseSuite(kPrSDKExportParamSuite, kPrSDKExportParamSuiteVersion);

    if (settings->exportProgressSuite)
		result = spBasic->ReleaseSuite(kPrSDKExportProgressSuite, kPrSDKExportProgressSuiteVersion);

	if (settings->exportFileSuite)
		result = spBasic->ReleaseSuite(kPrSDKExportFileSuite, kPrSDKExportFileSuiteVersion);

	if (settings->exportInfoSuite)
		result = spBasic->ReleaseSuite(kPrSDKExportInfoSuite, kPrSDKExportInfoSuiteVersion);

	if (settings->errorSuite)
		result = spBasic->ReleaseSuite(kPrSDKErrorSuite, kPrSDKErrorSuiteVersion3);

	if (settings->clipRenderSuite)
		result = spBasic->ReleaseSuite(kPrSDKClipRenderSuite, kPrSDKClipRenderSuiteVersion);

	if (settings->markerSuite)
		result = spBasic->ReleaseSuite(kPrSDKMarkerSuite, kPrSDKMarkerSuiteVersion);

	if (settings->ppixSuite)
		result = spBasic->ReleaseSuite(kPrSDKPPixSuite, kPrSDKPPixSuiteVersion);

	if (settings->timeSuite)
		result = spBasic->ReleaseSuite(kPrSDKTimeSuite, kPrSDKTimeSuiteVersion);

	if (settings->windowSuite)
		result = spBasic->ReleaseSuite(kPrSDKWindowSuite, kPrSDKWindowSuiteVersion);

    if (settings->audioSuite)
        result = spBasic->ReleaseSuite(kPrSDKAudioSuite, kPrSDKAudioSuiteVersion);

    if (settings->sequenceAudioSuite)
        result = spBasic->ReleaseSuite(kPrSDKSequenceAudioSuite, kPrSDKSequenceAudioSuiteVersion1);

	settings->~ExportSettings();

	if (settings->memorySuite)
	{
		PrSDKMemoryManagerSuite* memorySuite = settings->memorySuite;
		memorySuite->PrDisposePtr(reinterpret_cast<PrMemoryPtr>(settings));
		result = spBasic->ReleaseSuite(kPrSDKMemoryManagerSuite, kPrSDKMemoryManagerSuiteVersion);
	}

	return result;
}

prMALError queryOutputSettings(exportStdParms *stdParmsP, exQueryOutputSettingsRec *outputSettingsP)
{
	const csSDK_uint32 exID = outputSettingsP->exporterPluginID;
    exParamValues width, height, frameRate, hapSubcodec; // , fieldType;
	ExportSettings* privateData = reinterpret_cast<ExportSettings*>(outputSettingsP->privateData);
	PrSDKExportParamSuite* paramSuite = privateData->exportParamSuite;
	const csSDK_int32 mgroupIndex = 0;
	float fps = 0.0f;

	if (outputSettingsP->inExportVideo)
	{
		paramSuite->GetParamValue(exID, mgroupIndex, ADBEVideoWidth, &width);
		outputSettingsP->outVideoWidth = width.value.intValue;
		paramSuite->GetParamValue(exID, mgroupIndex, ADBEVideoHeight, &height);
		outputSettingsP->outVideoHeight = height.value.intValue;
		paramSuite->GetParamValue(exID, mgroupIndex, ADBEVideoFPS, &frameRate);
		outputSettingsP->outVideoFrameRate = frameRate.value.timeValue;
		paramSuite->GetParamValue(exID, mgroupIndex, ADBEVideoCodec, &hapSubcodec);
		privateData->hapSubcodec = reinterpret_cast<CodecSubType &>(hapSubcodec.value.intValue);
		outputSettingsP->outVideoAspectNum = 1;
		outputSettingsP->outVideoAspectDen = 1;
		// paramSuite->GetParamValue(exID, mgroupIndex, ADBEVideoFieldType, &fieldType);
		outputSettingsP->outVideoFieldType = prFieldsNone;
	}

	// Calculate bitrate
	PrTime ticksPerSecond = 0;
	csSDK_uint32 videoBitrate = 0;

	if (outputSettingsP->inExportVideo)
	{
		privateData->timeSuite->GetTicksPerSecond(&ticksPerSecond);
		fps = static_cast<float>(ticksPerSecond) / frameRate.value.timeValue;
		paramSuite->GetParamValue(exID, mgroupIndex, "HAPSubcodec", &hapSubcodec);
		videoBitrate = static_cast<csSDK_uint32>(width.value.intValue * height.value.intValue * getPixelFormatSize(hapSubcodec.value.intValue) * fps);
	}

    if (outputSettingsP->inExportAudio)
    {
        exParamValues sampleRate, channelType;
        paramSuite->GetParamValue(exID, mgroupIndex, ADBEAudioRatePerSecond, &sampleRate);
        paramSuite->GetParamValue(exID, mgroupIndex, ADBEAudioNumChannels, &channelType);
        
        outputSettingsP->outAudioChannelType = static_cast<PrAudioChannelType>(channelType.value.intValue);
        outputSettingsP->outAudioSampleRate = sampleRate.value.floatValue;
        outputSettingsP->outAudioSampleType = kPrAudioSampleType_16BitInt;
    }

	outputSettingsP->outBitratePerSecond = videoBitrate * 8 / 1000;

	return malNoError;
}

prMALError fileExtension(exportStdParms* stdParmsP, exQueryExportFileExtensionRec* exportFileExtensionRecP)
{
	copyConvertStringLiteralIntoUTF16(HAP_VIDEOFILEEXT, exportFileExtensionRecP->outFileExtension);

	return malNoError;
}

static prMALError c_onFrameComplete(
    csSDK_uint32 inWhichPass,
    csSDK_uint32 inFrameNumber,
    csSDK_uint32 inFrameRepeatCount,
    PPixHand inRenderedFrame,
    void* inCallbackData)
{
    ExportSettings* settings = reinterpret_cast<ExportSettings*>(inCallbackData);
    try
    {
        char* bgra_buffer;
        int32_t bgra_stride;
        prMALError error = settings->ppixSuite->GetPixels(inRenderedFrame, PrPPixBufferAccess_ReadOnly, &bgra_buffer);
        if (malNoError != error)
            throw std::runtime_error("could not GetPixels on completed frame");

        error = settings->ppixSuite->GetRowBytes(inRenderedFrame, &bgra_stride);
        if (malNoError != error)
            throw std::runtime_error("could not GetRowBytes on completed frame");

        settings->exporter->dispatch(inFrameNumber, (uint8_t*)bgra_buffer, bgra_stride);
    }
    catch (const std::exception& ex)
    {
        settings->reportError(ex.what());
        return malUnknownError;
    }
    catch (...)
    {
        settings->reportError("unspecified error while processing frame");
        return malUnknownError;
    }
    return malNoError;
}

static void renderAndWriteAllVideo(exDoExportRec* exportInfoP, prMALError& error)
{
	const csSDK_uint32 exID = exportInfoP->exporterPluginID;
	ExportSettings* settings = reinterpret_cast<ExportSettings*>(exportInfoP->privateData);
	exParamValues ticksPerFrame, width, height, hapSubcodec, hapQuality, chunkCount;
	PrTime ticksPerSecond;

	settings->exportParamSuite->GetParamValue(exID, 0, ADBEVideoFPS, &ticksPerFrame);
	settings->exportParamSuite->GetParamValue(exID, 0, ADBEVideoWidth, &width);
	settings->exportParamSuite->GetParamValue(exID, 0, ADBEVideoHeight, &height);
    settings->exportParamSuite->GetParamValue(exID, 0, ADBEVideoCodec, &hapSubcodec);
    settings->exportParamSuite->GetParamValue(exID, 0, ADBEVideoQuality, &hapQuality);
    settings->exportParamSuite->GetParamValue(exID, 0, HAPChunkCount, &chunkCount);
    settings->timeSuite->GetTicksPerSecond(&ticksPerSecond);
    const int64_t frameRateNumerator = ticksPerSecond;
    const int64_t frameRateDenominator = ticksPerFrame.value.timeValue;

    unsigned int chunkCountAfterAutoApplied = std::max(1, chunkCount.value.intValue);
    HapChunkCounts chunkCounts{ chunkCountAfterAutoApplied, chunkCountAfterAutoApplied };

	std::unique_ptr<Codec> codec = std::unique_ptr<Codec>(
        Codec::create(reinterpret_cast<CodecSubType&>(hapSubcodec.value.intValue),
					  FrameDef(width.value.intValue, height.value.intValue),
                      chunkCounts,
                      static_cast<SquishEncoderQuality>(hapQuality.value.intValue)));

    //--- this error flag may be overwritten fairly deeply in callbacks so original error may be
    //--- passed up to Adobe
    error = settings->exportFileSuite->Open(exportInfoP->fileObject);
    if (malNoError != error)
        throw std::runtime_error("couldn't open output file");

    // cache some things
    auto file = exportInfoP->fileObject;
    auto Write = settings->exportFileSuite->Write;
    auto Seek = settings->exportFileSuite->Seek;
    auto Close = settings->exportFileSuite->Close;

    std::unique_ptr<MovieWriter> movieWriter = std::make_unique<MovieWriter>(
        codec->subType(),
        width.value.intValue, height.value.intValue,
        frameRateNumerator, frameRateDenominator,
        [&, &error=error](const uint8_t* buffer, size_t size) {
            prMALError writeError = Write(file, (void *)buffer, (int32_t)size);
            if (malNoError != writeError) {
                settings->reportError("Could not write to file");
                return -1;
            }
            return 0;
        },
        [&, &error=error](int64_t offset, int whence) {
            int64_t newPosition;
            ExFileSuite_SeekMode seekMode;
            if (whence == SEEK_SET)
                seekMode = fileSeekMode_Begin;
            else if (whence == SEEK_END)
                seekMode = fileSeekMode_End;
            else if (whence == SEEK_CUR)
                seekMode = fileSeekMode_Current;
            else
                throw std::runtime_error("unhandled file seek mode");
            prMALError seekError = Seek(file, offset, newPosition, seekMode);
            if (malNoError != seekError) {
                settings->reportError("Could not seek in file");
                return -1;
            }
            return 0;
        },
        [&, &error=error]() {
            return (malNoError == Close(file)) ? 0 : -1;
        },
        [&](const char *msg) { settings->reportError(msg); } );

        // TODO move this outside to DoExport()
        auto writer = movieWriter.get();
        if (exportInfoP->exportAudio) 
        {
            exParamValues sampleRate, channelType;
            settings->exportParamSuite->GetParamValue(exID, 0, ADBEAudioRatePerSecond, &sampleRate);
            settings->exportParamSuite->GetParamValue(exID, 0, ADBEAudioNumChannels, &channelType);
            csSDK_int32 numAudioChannels = GetNumberOfAudioChannels(channelType.value.intValue);
            
            writer->addAudioStream(numAudioChannels, (int)sampleRate.value.floatValue);
        }

        writer->writeHeader();

        if (exportInfoP->exportAudio)
            renderAndWriteAllAudio(exportInfoP, error, writer);

    try {
        settings->exporter = std::make_unique<Exporter>(std::move(codec), std::move(movieWriter));

        ExportLoopRenderParams renderParams;

        renderParams.inRenderParamsSize = sizeof(ExportLoopRenderParams);
        renderParams.inRenderParamsVersion = kPrSDKExporterUtilitySuiteVersion;
        renderParams.inFinalPixelFormat = PrPixelFormat_BGRA_4444_8u;
        renderParams.inStartTime = exportInfoP->startTime;
        renderParams.inEndTime = exportInfoP->endTime;
        renderParams.inReservedProgressPreRender = 0.0; //!!!
        renderParams.inReservedProgressPostRender = 0.0; //!!!

        prMALError multipassExportError = settings->exporterUtilitySuite->DoMultiPassExportLoop(
            exportInfoP->exporterPluginID,
            &renderParams,
            1,  // number of passes
            c_onFrameComplete,
            settings
        );
        if (malNoError != multipassExportError)
        {
            if (error == malNoError)  // retain error if it was set in per-frame export
                error = multipassExportError;
            throw std::runtime_error("DoMultiPassExportLoop failed");
        }

        // this may throw
        settings->exporter->close();
    }
    catch (...)
    {
        settings->exporter.reset(nullptr);
        throw;
    }

    settings->exporter.reset(nullptr);
}

csSDK_int32 GetNumberOfAudioChannels(csSDK_int32 audioChannelType)
{
    csSDK_int32 numberOfChannels = -1;

    if (audioChannelType == kPrAudioChannelType_Mono)
        numberOfChannels = 1;

    else if (audioChannelType == kPrAudioChannelType_Stereo)
        numberOfChannels = 2;

    else if (audioChannelType == kPrAudioChannelType_51)
        numberOfChannels = 6;

    return numberOfChannels;
}

static void renderAndWriteAllAudio(exDoExportRec *exportInfoP, prMALError &error, MovieWriter *writer)
{
    // All audio calls to and from Premiere use arrays of buffers of 32-bit floats to pass audio.
    // Audio is not interleaved, rather separate channels are stored in separate buffers.
    const int kAudioSampleSizePremiere = sizeof(float_t);

    // Assume we export 16bit audio and pack up to 1024 samples per packet
    const int kAudioSampleSizeOutput = sizeof(int16_t);
    const int kMaxAudioSamplesPerPacket = 1024;

    const csSDK_uint32 exID = exportInfoP->exporterPluginID;
    ExportSettings *settings = reinterpret_cast<ExportSettings *>(exportInfoP->privateData);
    exParamValues ticksPerFrame, sampleRate, channelType;

    settings->exportParamSuite->GetParamValue(exID, 0, ADBEVideoFPS, &ticksPerFrame);
    settings->exportParamSuite->GetParamValue(exID, 0, ADBEAudioRatePerSecond, &sampleRate);
    settings->exportParamSuite->GetParamValue(exID, 0, ADBEAudioNumChannels, &channelType);
    csSDK_int32 numAudioChannels = GetNumberOfAudioChannels(channelType.value.intValue);

    csSDK_uint32 audioRenderID = 0;
    settings->sequenceAudioSuite->MakeAudioRenderer(exID,
                                                    exportInfoP->startTime,
                                                    (PrAudioChannelType)channelType.value.intValue,
                                                    kPrAudioSampleType_32BitFloat,
                                                    (float)sampleRate.value.floatValue,
                                                    &audioRenderID);

    PrTime ticksPerSample = 0;
    settings->timeSuite->GetTicksPerAudioSample((float)sampleRate.value.floatValue, &ticksPerSample);

    PrTime exportDuration = exportInfoP->endTime - exportInfoP->startTime;
    csSDK_int64 totalAudioSamples = exportDuration / ticksPerSample;
    csSDK_int64 samplesRemaining = totalAudioSamples;

    // Allocate audio buffers
    csSDK_int32 audioBufferSize = kMaxAudioSamplesPerPacket;
    auto audioBufferOut = (csSDK_int16 *)settings->memorySuite->NewPtr(numAudioChannels * audioBufferSize * kAudioSampleSizeOutput);
    float *audioBuffer[kMaxAudioChannelCount];
    for (csSDK_int32 bufferIndexL = 0; bufferIndexL < numAudioChannels; bufferIndexL++)
    {
        audioBuffer[bufferIndexL] = (float *)settings->memorySuite->NewPtr(audioBufferSize * kAudioSampleSizePremiere);
    }

    // Progress bar init with label
    float progress = 0.f;
    prUTF16Char tempStrProgress[256];
    copyConvertStringLiteralIntoUTF16(L"Preparing Audio...", tempStrProgress);
    settings->exportProgressSuite->SetProgressString(exID, tempStrProgress);

    // GetAudio loop
    csSDK_int32 samplesRequested, maxBlipSize;
    csSDK_int64 samplesExported = 0; // pts
    prMALError resultS = malNoError;
    while (samplesRemaining && (resultS == malNoError))
    {
        // Find size of blip to ask for
        settings->sequenceAudioSuite->GetMaxBlip(audioRenderID, ticksPerFrame.value.timeValue, &maxBlipSize);
        samplesRequested = std::min(audioBufferSize, maxBlipSize);
        if (samplesRequested > samplesRemaining)
            samplesRequested = (csSDK_int32)samplesRemaining;

        // Fill the buffer with audio
        resultS = settings->sequenceAudioSuite->GetAudio(audioRenderID, samplesRequested, audioBuffer, kPrFalse);
        if (resultS != malNoError)
            break;

        settings->audioSuite->ConvertAndInterleaveTo16BitInteger(audioBuffer, audioBufferOut,
                                                                 numAudioChannels, samplesRequested);

        // Write audioBufferOut as one packet
        writer->writeAudioFrame(reinterpret_cast<const uint8_t *>(audioBufferOut),
                                samplesRequested * numAudioChannels * kAudioSampleSizeOutput,
                                samplesExported);

        // Write audioBufferOut as separate samples
        // auto buf = reinterpret_cast<const uint8_t *>(audioBufferOut);
        // for (csSDK_int32 i = 0; i < samplesRequested; i++)
        // {
        //     csSDK_int32 offset = i * numAudioChannels * kAudioSampleSizeOutput;
        //     writer->writeAudioFrame(&buf[offset],
        //                             numAudioChannels * kAudioSampleSizeOutput,
        //                             samplesExported + i);
        // }

        // Calculate remaining audio
        samplesExported += samplesRequested;
        samplesRemaining -= samplesRequested;

        // Update progress bar percent
        progress = (float) samplesExported / totalAudioSamples;
        settings->exportProgressSuite->UpdateProgressPercent(exID, progress);
    }
    error = resultS;

    // Reset progress bar label
    copyConvertStringLiteralIntoUTF16(L"", tempStrProgress);
    settings->exportProgressSuite->SetProgressString(exID, tempStrProgress);

    // Free up
    settings->memorySuite->PrDisposePtr((PrMemoryPtr)audioBufferOut);
    for (csSDK_int32 bufferIndexL = 0; bufferIndexL < numAudioChannels; bufferIndexL++)
    {
        settings->memorySuite->PrDisposePtr((PrMemoryPtr)audioBuffer[bufferIndexL]);
    }
    settings->sequenceAudioSuite->ReleaseAudioRenderer(exID, audioRenderID);
}

prMALError doExport(exportStdParms* stdParmsP, exDoExportRec* exportInfoP)
{
    ExportSettings* settings = reinterpret_cast<ExportSettings*>(exportInfoP->privateData);
    prMALError error = malNoError;

    try {
        // if (exportInfoP->exportAudio)
        //     renderAndWriteAllAudio(exportInfoP, error);

        if (exportInfoP->exportVideo)
            renderAndWriteAllVideo(exportInfoP, error);
    }
    catch (const std::exception& ex)
    {
        settings->reportError(ex.what());
        return (error == malNoError) ? malUnknownError : error;
    }
    catch (...)
    {
        settings->reportError("unspecified error while rendering and writing video");
        return (error == malNoError) ? malUnknownError : error;
    }

    return 	malNoError;
}
