#include "premiereParams.hpp"
#include "export_settings.hpp"
#include "prstring.hpp"
#include "configure.hpp"

const int k_chunkingMin = 1;
const int k_chunkingMax = 64;
const int k_chunkingBlockSize = 1920*1200;
enum HapChunkOption {
    kHapChunkNone = 0,
    kHapChunkAuto = 1,
    kHapChunkManual = 2
};
void chunkSettingsHandler(const csSDK_uint32 exID, ExportSettings *settings);

prMALError generateDefaultParams(exportStdParms *stdParms, exGenerateDefaultParamRec *generateDefaultParamRec)
{
    prMALError result = malNoError;
    ExportSettings* settings = reinterpret_cast<ExportSettings*>(generateDefaultParamRec->privateData);
    PrSDKExportParamSuite* exportParamSuite = settings->exportParamSuite;
    PrSDKExportInfoSuite* exportInfoSuite = settings->exportInfoSuite;
    PrSDKTimeSuite* timeSuite = settings->timeSuite;
    csSDK_int32	exporterPluginID = generateDefaultParamRec->exporterPluginID;
    csSDK_int32	mgroupIndex = 0;
    PrParam	hasVideo,
        hasAudio,
        seqWidth,
        seqHeight,
        seqFrameRate,
        seqChannelType,
        seqSampleRate;
    prUTF16Char tempString[256];

    if (exportInfoSuite)
    {
        exportInfoSuite->GetExportSourceInfo(exporterPluginID, kExportInfo_SourceHasVideo, &hasVideo);
        exportInfoSuite->GetExportSourceInfo(exporterPluginID, kExportInfo_SourceHasAudio, &hasAudio);
        exportInfoSuite->GetExportSourceInfo(exporterPluginID, kExportInfo_VideoWidth, &seqWidth);
        exportInfoSuite->GetExportSourceInfo(exporterPluginID, kExportInfo_VideoHeight, &seqHeight);

        if (seqWidth.mInt32 == 0)
            seqWidth.mInt32 = 1920;

        if (seqHeight.mInt32 == 0)
            seqHeight.mInt32 = 1080;

        exportInfoSuite->GetExportSourceInfo(exporterPluginID, kExportInfo_VideoFrameRate, &seqFrameRate);
        exportInfoSuite->GetExportSourceInfo(exporterPluginID, kExportInfo_AudioChannelsType, &seqChannelType);
        exportInfoSuite->GetExportSourceInfo(exporterPluginID, kExportInfo_AudioSampleRate, &seqSampleRate);
    }

    if (exportParamSuite)
    {
        exportParamSuite->AddMultiGroup(exporterPluginID, &mgroupIndex);
        copyConvertStringLiteralIntoUTF16(TOP_VIDEO_PARAM_GROUP_NAME, tempString);
        exportParamSuite->AddParamGroup(exporterPluginID, mgroupIndex, ADBETopParamGroup, ADBEVideoTabGroup, tempString, kPrFalse, kPrFalse, kPrFalse);
        copyConvertStringLiteralIntoUTF16(VIDEO_CODEC_PARAM_GROUP_NAME, tempString);
        exportParamSuite->AddParamGroup(exporterPluginID, mgroupIndex, ADBEVideoTabGroup, ADBEVideoCodecGroup, tempString, kPrFalse, kPrFalse, kPrFalse);
        copyConvertStringLiteralIntoUTF16(BASIC_VIDEO_PARAM_GROUP_NAME, tempString);
        exportParamSuite->AddParamGroup(exporterPluginID, mgroupIndex, ADBEVideoTabGroup, ADBEBasicVideoGroup, tempString, kPrFalse, kPrFalse, kPrFalse);
        copyConvertStringLiteralIntoUTF16(CODEC_SPECIFIC_PARAM_GROUP_NAME, tempString);
        exportParamSuite->AddParamGroup(exporterPluginID, mgroupIndex, ADBEVideoTabGroup, HAPSpecificCodecGroup, tempString, kPrFalse, kPrFalse, kPrFalse);
        exNewParamInfo widthParam;
        exParamValues widthValues;
        safeStrCpy(widthParam.identifier, 256, ADBEVideoWidth);
        widthParam.paramType = exParamType_int;
        widthParam.flags = exParamFlag_none;
        widthValues.rangeMin.intValue = 16;
        widthValues.rangeMax.intValue = 16384;
        widthValues.value.intValue = seqWidth.mInt32;
        widthValues.disabled = kPrFalse;
        widthValues.hidden = kPrFalse;
        widthParam.paramValues = widthValues;
        exportParamSuite->AddParam(exporterPluginID, mgroupIndex, ADBEBasicVideoGroup, &widthParam);

        exNewParamInfo heightParam;
        exParamValues heightValues;
        safeStrCpy(heightParam.identifier, 256, ADBEVideoHeight);
        heightParam.paramType = exParamType_int;
        heightParam.flags = exParamFlag_none;
        heightValues.rangeMin.intValue = 16;
        heightValues.rangeMax.intValue = 16384;
        heightValues.value.intValue = seqHeight.mInt32;
        heightValues.disabled = kPrFalse;
        heightValues.hidden = kPrFalse;
        heightParam.paramValues = heightValues;
        exportParamSuite->AddParam(exporterPluginID, mgroupIndex, ADBEBasicVideoGroup, &heightParam);

		exNewParamInfo hapSubcodecParam;
		exParamValues hapSubcodecValues;
		safeStrCpy(hapSubcodecParam.identifier, 256, ADBEVideoCodec);
		hapSubcodecParam.paramType = exParamType_int;
		hapSubcodecParam.flags = exParamFlag_none;
		hapSubcodecValues.rangeMin.intValue = 0;
		hapSubcodecValues.rangeMax.intValue = 4;
		auto temp = kHapAlphaCodecSubType;
		hapSubcodecValues.value.intValue = reinterpret_cast<int32_t&>(temp); //!!! seqHapSubcodec.mInt32;
		hapSubcodecValues.disabled = kPrFalse;
		hapSubcodecValues.hidden = kPrFalse;
		hapSubcodecParam.paramValues = hapSubcodecValues;
		exportParamSuite->AddParam(exporterPluginID, mgroupIndex, ADBEBasicVideoGroup, &hapSubcodecParam);

        exNewParamInfo hapQualityParam;
        exParamValues hapQualityValues;
        safeStrCpy(hapQualityParam.identifier, 256, ADBEVideoQuality);
        hapQualityParam.paramType = exParamType_int;
        hapQualityParam.flags = exParamFlag_none;
        hapQualityValues.rangeMin.intValue = 0;
        hapQualityValues.rangeMax.intValue = 1;
        hapQualityValues.value.intValue = kSquishEncoderNormalQuality;
        hapQualityValues.disabled = kPrFalse;
        hapQualityValues.hidden = kPrFalse;
        hapQualityParam.paramValues = hapQualityValues;
        exportParamSuite->AddParam(exporterPluginID, mgroupIndex, ADBEBasicVideoGroup, &hapQualityParam);

		exNewParamInfo frameRateParam;
        exParamValues frameRateValues;
        safeStrCpy(frameRateParam.identifier, 256, ADBEVideoFPS);
        frameRateParam.paramType = exParamType_ticksFrameRate;
        frameRateParam.flags = exParamFlag_none;
        frameRateValues.rangeMin.timeValue = 1;
        timeSuite->GetTicksPerSecond(&frameRateValues.rangeMax.timeValue);
        frameRateValues.value.timeValue = seqFrameRate.mInt64;
        frameRateValues.disabled = kPrFalse;
        frameRateValues.hidden = kPrFalse;
        frameRateParam.paramValues = frameRateValues;
        exportParamSuite->AddParam(exporterPluginID, mgroupIndex, ADBEBasicVideoGroup, &frameRateParam);

        exNewParamInfo chunkParam;
        exParamValues chunkValues;
        safeStrCpy(chunkParam.identifier, 256, HAPChunk);
        chunkParam.paramType = exParamType_int;
        chunkParam.flags = exParamFlag_none;
        chunkValues.rangeMin.intValue = 0;
        chunkValues.rangeMax.intValue = 2;
        chunkValues.value.intValue = kHapChunkAuto; // TODO: decide default value for Release: Auto or None?
        chunkValues.disabled = kPrFalse;
        chunkValues.hidden = kPrFalse;
        chunkParam.paramValues = chunkValues;
        exportParamSuite->AddParam(exporterPluginID, mgroupIndex, HAPSpecificCodecGroup, &chunkParam);

        exNewParamInfo chunkCountParam;
        exParamValues chunkCountValues;
        safeStrCpy(chunkCountParam.identifier, 256, HAPChunkCount);
        chunkCountParam.paramType = exParamType_int;
        chunkCountParam.flags = exParamFlag_slider;
        chunkCountValues.rangeMin.intValue = k_chunkingMin;
        chunkCountValues.rangeMax.intValue = k_chunkingMax;
        chunkCountValues.value.intValue = 1;
        chunkCountValues.disabled = kPrTrue;
        chunkCountValues.hidden = kPrFalse;
        chunkCountParam.paramValues = chunkCountValues;
        exportParamSuite->AddParam(exporterPluginID, mgroupIndex, HAPSpecificCodecGroup, &chunkCountParam);

        // Audio parameters
        copyConvertStringLiteralIntoUTF16(TOP_AUDIO_PARAM_GROUP_NAME, tempString);
        exportParamSuite->AddParamGroup(exporterPluginID, mgroupIndex, ADBETopParamGroup, ADBEAudioTabGroup, tempString, kPrFalse, kPrFalse, kPrFalse);
        copyConvertStringLiteralIntoUTF16(BASIC_AUDIO_PARAM_GROUP_NAME, tempString);
        exportParamSuite->AddParamGroup(exporterPluginID, mgroupIndex, ADBEAudioTabGroup, ADBEBasicAudioGroup, tempString, kPrFalse, kPrFalse, kPrFalse);

        // Sample rate
        exNewParamInfo sampleRateParam;
        exParamValues sampleRateValues;
        safeStrCpy(sampleRateParam.identifier, 256, ADBEAudioRatePerSecond);
        sampleRateParam.paramType = exParamType_float;
        sampleRateParam.flags = exParamFlag_none;
        sampleRateValues.value.floatValue = 44100.0f; // disguise servers default samplerate
        sampleRateValues.disabled = kPrFalse;
        sampleRateValues.hidden = kPrFalse;
        sampleRateParam.paramValues = sampleRateValues;
        exportParamSuite->AddParam(exporterPluginID, mgroupIndex, ADBEBasicAudioGroup, &sampleRateParam);
        
        // Channel type
        exNewParamInfo channelTypeParam;
        exParamValues channelTypeValues;
        safeStrCpy(channelTypeParam.identifier, 256, ADBEAudioNumChannels);
        channelTypeParam.paramType = exParamType_int;
        channelTypeParam.flags = exParamFlag_none;
        channelTypeValues.value.intValue = kPrAudioChannelType_Stereo;
        channelTypeValues.disabled = kPrFalse; // TODO in Release disable to simplify user expirience: only stereo
        channelTypeValues.hidden = kPrFalse;
        channelTypeParam.paramValues = channelTypeValues;
        exportParamSuite->AddParam(exporterPluginID, mgroupIndex, ADBEBasicAudioGroup, &channelTypeParam);

        exportParamSuite->SetParamsVersion(exporterPluginID, 5);
    }

    return result;
}

prMALError postProcessParams(exportStdParms *stdParmsP, exPostProcessParamsRec *postProcessParamsRecP)
{
    const csSDK_uint32 exID = postProcessParamsRecP->exporterPluginID;
    ExportSettings* settings = reinterpret_cast<ExportSettings*>(postProcessParamsRecP->privateData);
    PrTime ticksPerSecond = 0;
    exParamValues hapSubcodec, qualityToValidate;
	exOneParamValueRec tempHapSubcodec;
	CodecSubType HAPsubcodecs[] = { kHapCodecSubType, kHapAlphaCodecSubType, kHapYCoCgCodecSubType, kHapYCoCgACodecSubType, kHapAOnlyCodecSubType };
    exOneParamValueRec tempHapQuality;
    SquishEncoderQuality HAPquality[] = { kSquishEncoderFastQuality, kSquishEncoderNormalQuality };
    exOneParamValueRec tempFrameRate;
    PrTime frameRates[] = { 10, 15, 23, 24, 25, 29, 30, 50, 59, 60 };
    PrTime frameRateNumDens[][2] = { { 10, 1 }, { 15, 1 }, { 24000, 1001 }, { 24, 1 }, { 25, 1 }, { 30000, 1001 }, { 30, 1 }, { 50, 1 }, { 60000, 1001 }, { 60, 1 } };

    exOneParamValueRec tempSampleRate;
    float sampleRates[] = {44100.0f, 48000.0f};
    exOneParamValueRec tempChannelType;
    csSDK_int32 channelTypes[] = {kPrAudioChannelType_Mono, kPrAudioChannelType_Stereo, kPrAudioChannelType_51};

    prUTF16Char tempString[256];
    const wchar_t* frameRateStrings[] = { STR_FRAME_RATE_10, STR_FRAME_RATE_15, STR_FRAME_RATE_23976, STR_FRAME_RATE_24, STR_FRAME_RATE_25, STR_FRAME_RATE_2997, STR_FRAME_RATE_30, STR_FRAME_RATE_50, STR_FRAME_RATE_5994, STR_FRAME_RATE_60 };
	const wchar_t *hapSubcodecStrings[] = { STR_HAP_SUBCODEC_0, STR_HAP_SUBCODEC_1, STR_HAP_SUBCODEC_2, STR_HAP_SUBCODEC_3, STR_HAP_SUBCODEC_4 };
    const wchar_t *hapQualityStrings[] = { STR_HAP_QUALITY_0, STR_HAP_QUALITY_1 };
    const wchar_t *sampleRateStrings[] = {STR_SAMPLE_RATE_441, STR_SAMPLE_RATE_48};
    const wchar_t *channelTypeStrings[] = {STR_CHANNEL_TYPE_MONO, STR_CHANNEL_TYPE_STEREO, STR_CHANNEL_TYPE_51};

    exOneParamValueRec tempHapChunksParam;
    HapChunkOption hapChunkOptions[] = { kHapChunkNone, kHapChunkAuto, kHapChunkManual };
    const wchar_t *hapChunkStrings[] = {STR_HAP_CHUNKS_NONE, STR_HAP_CHUNKS_AUTO, STR_HAP_CHUNKS_MANUAL};

	settings->timeSuite->GetTicksPerSecond(&ticksPerSecond);
    for (csSDK_int32 i = 0; i < sizeof(frameRates) / sizeof(PrTime); i++)
        frameRates[i] = ticksPerSecond / frameRateNumDens[i][0] * frameRateNumDens[i][1];

    copyConvertStringLiteralIntoUTF16(VIDEO_CODEC_PARAM_GROUP_NAME, tempString);
    settings->exportParamSuite->SetParamName(exID, 1, ADBEVideoCodecGroup, tempString);

    copyConvertStringLiteralIntoUTF16(STR_CODEC, tempString);
    settings->exportParamSuite->SetParamName(exID, 0, ADBEVideoCodec, tempString);
    copyConvertStringLiteralIntoUTF16(STR_CODEC_TOOLTIP, tempString);
    settings->exportParamSuite->SetParamDescription(exID, 0, ADBEVideoCodec, tempString);

    copyConvertStringLiteralIntoUTF16(BASIC_VIDEO_PARAM_GROUP_NAME, tempString);
    settings->exportParamSuite->SetParamName(exID, 0, ADBEBasicVideoGroup, tempString);

    copyConvertStringLiteralIntoUTF16(STR_WIDTH, tempString);
    settings->exportParamSuite->SetParamName(exID, 0, ADBEVideoWidth, tempString);

    copyConvertStringLiteralIntoUTF16(STR_HEIGHT, tempString);
    settings->exportParamSuite->SetParamName(exID, 0, ADBEVideoHeight, tempString);

	copyConvertStringLiteralIntoUTF16(STR_HAP_SUBCODEC, tempString);
	settings->exportParamSuite->SetParamName(exID, 0, ADBEVideoCodec, tempString);
    settings->exportParamSuite->ClearConstrainedValues(exID, 0, ADBEVideoCodec);
    for (csSDK_int32 i = 0; i < sizeof(HAPsubcodecs) / sizeof(HAPsubcodecs[0]); i++)
    {
        tempHapSubcodec.intValue = reinterpret_cast<int32_t &>(HAPsubcodecs[i][0]);
        copyConvertStringLiteralIntoUTF16(hapSubcodecStrings[i], tempString);
        settings->exportParamSuite->AddConstrainedValuePair(exID, 0, ADBEVideoCodec, &tempHapSubcodec, tempString);
    }

    copyConvertStringLiteralIntoUTF16(STR_HAP_QUALITY, tempString);
    settings->exportParamSuite->SetParamName(exID, 0, ADBEVideoQuality, tempString);
    settings->exportParamSuite->ClearConstrainedValues(exID, 0, ADBEVideoQuality);
    for (csSDK_int32 i = 0; i < sizeof(HAPquality) / sizeof(HAPquality[0]); i++)
    {
        tempHapQuality.intValue = HAPquality[i];
        copyConvertStringLiteralIntoUTF16(hapQualityStrings[i], tempString);
        settings->exportParamSuite->AddConstrainedValuePair(exID, 0, ADBEVideoQuality, &tempHapQuality, tempString);
    }

    // quality setting is not supported by all subcodecs
    settings->exportParamSuite->GetParamValue(exID, 0, ADBEVideoCodec, &hapSubcodec);
    const auto codecSubtype = reinterpret_cast<CodecSubType&>(hapSubcodec.value.intValue);
    bool enableQuality = Codec::getCapabilities(codecSubtype).hasQuality;
    settings->exportParamSuite->GetParamValue(exID, 0, ADBEVideoQuality, &qualityToValidate);
    qualityToValidate.disabled = !enableQuality;
    qualityToValidate.hidden = !enableQuality;
    settings->exportParamSuite->ChangeParam(exID, 0, ADBEVideoQuality, &qualityToValidate);


    copyConvertStringLiteralIntoUTF16(STR_FRAME_RATE, tempString);
    settings->exportParamSuite->SetParamName(exID, 0, ADBEVideoFPS, tempString);
    settings->exportParamSuite->ClearConstrainedValues(exID, 0, ADBEVideoFPS);
    for (csSDK_int32 i = 0; i < sizeof(frameRates) / sizeof(PrTime); i++)
    {
        tempFrameRate.timeValue = frameRates[i];
        copyConvertStringLiteralIntoUTF16(frameRateStrings[i], tempString);
        settings->exportParamSuite->AddConstrainedValuePair(exID, 0, ADBEVideoFPS, &tempFrameRate, tempString);
    }

    copyConvertStringLiteralIntoUTF16(CODEC_SPECIFIC_PARAM_GROUP_NAME, tempString);
    settings->exportParamSuite->SetParamName(exID, 0, HAPSpecificCodecGroup, tempString);

	copyConvertStringLiteralIntoUTF16(STR_HAP_CHUNKS, tempString);
	settings->exportParamSuite->SetParamName(exID, 0, HAPChunk, tempString);
    settings->exportParamSuite->ClearConstrainedValues(exID, 0, HAPChunk);
    for (csSDK_int32 i = 0; i < sizeof(hapChunkOptions) / sizeof(hapChunkOptions[0]); i++)
    {
        tempHapChunksParam.intValue = hapChunkOptions[i];
        copyConvertStringLiteralIntoUTF16(hapChunkStrings[i], tempString);
        settings->exportParamSuite->AddConstrainedValuePair(exID, 0, HAPChunk, &tempHapChunksParam, tempString);
    }
    
    copyConvertStringLiteralIntoUTF16(STR_HAP_CHUNKING, tempString);
    settings->exportParamSuite->SetParamName(exID, 0, HAPChunkCount, tempString);
    chunkSettingsHandler(exID, settings);

    copyConvertStringLiteralIntoUTF16(BASIC_AUDIO_PARAM_GROUP_NAME, tempString);
    settings->exportParamSuite->SetParamName(exID, 0, ADBEBasicAudioGroup, tempString);

    copyConvertStringLiteralIntoUTF16(STR_SAMPLE_RATE, tempString);
    settings->exportParamSuite->SetParamName(exID, 0, ADBEAudioRatePerSecond, tempString);
    settings->exportParamSuite->ClearConstrainedValues(exID, 0, ADBEAudioRatePerSecond);
    for (csSDK_int32 i = 0; i < sizeof(sampleRates) / sizeof(float); i++)
    {
        tempSampleRate.floatValue = sampleRates[i];
        copyConvertStringLiteralIntoUTF16(sampleRateStrings[i], tempString);
        settings->exportParamSuite->AddConstrainedValuePair(exID, 0, ADBEAudioRatePerSecond, &tempSampleRate, tempString);
    }

    copyConvertStringLiteralIntoUTF16(STR_CHANNEL_TYPE, tempString);
    settings->exportParamSuite->SetParamName(exID, 0, ADBEAudioNumChannels, tempString);
    settings->exportParamSuite->ClearConstrainedValues(exID, 0, ADBEAudioNumChannels);
    for (csSDK_int32 i = 0; i < sizeof(channelTypes) / sizeof(csSDK_int32); i++)
    {
        tempChannelType.intValue = channelTypes[i];
        copyConvertStringLiteralIntoUTF16(channelTypeStrings[i], tempString);
        settings->exportParamSuite->AddConstrainedValuePair(exID, 0, ADBEAudioNumChannels, &tempChannelType, tempString);
    }

    return malNoError;
}

prMALError getParamSummary(exportStdParms *stdParmsP, exParamSummaryRec *summaryRecP)
{
    wchar_t videoSummary[256], audioSummary[256];
    exParamValues width, height, frameRate, hapSubcodec, hapQuality, sampleRate, channelType;
    ExportSettings* settings = reinterpret_cast<ExportSettings*>(summaryRecP->privateData);
    PrSDKExportParamSuite* paramSuite = settings->exportParamSuite;
    PrSDKTimeSuite* timeSuite = settings->timeSuite;
    PrTime ticksPerSecond;
    const csSDK_int32 mgroupIndex = 0;
    const csSDK_int32 exporterPluginID = summaryRecP->exporterPluginID;

    if (!paramSuite)
        return malNoError;

    paramSuite->GetParamValue(exporterPluginID, mgroupIndex, ADBEVideoWidth, &width);
    paramSuite->GetParamValue(exporterPluginID, mgroupIndex, ADBEVideoHeight, &height);
    paramSuite->GetParamValue(exporterPluginID, mgroupIndex, ADBEVideoFPS, &frameRate);
	paramSuite->GetParamValue(exporterPluginID, mgroupIndex, ADBEVideoCodec, &hapSubcodec);
    paramSuite->GetParamValue(exporterPluginID, mgroupIndex, ADBEVideoQuality, &hapQuality);
    paramSuite->GetParamValue(exporterPluginID, mgroupIndex, ADBEAudioRatePerSecond, &sampleRate);
    paramSuite->GetParamValue(exporterPluginID, mgroupIndex, ADBEAudioNumChannels, &channelType);
    timeSuite->GetTicksPerSecond(&ticksPerSecond);

    std::wstring hapSubcodecSummary;
    switch(reinterpret_cast<CodecSubType&>(hapSubcodec.value.intValue)[3])
    {
        case '1':
            hapSubcodecSummary = STR_HAP_SUBCODEC_0;
            break;
        case '5':
            hapSubcodecSummary = STR_HAP_SUBCODEC_1;
            break;
        case 'Y':
            hapSubcodecSummary = STR_HAP_SUBCODEC_2;
            break;
        case 'M':
            hapSubcodecSummary = STR_HAP_SUBCODEC_3;
            break;
        case 'A':
            hapSubcodecSummary = STR_HAP_SUBCODEC_4;
            break;
        default:
            hapSubcodecSummary = L"Unknown";
    }

    swprintf(videoSummary, 256, L"%ix%i, %ls, %.2f fps",
            width.value.intValue, height.value.intValue,
            hapSubcodecSummary.c_str(), 
            static_cast<float>(ticksPerSecond) / static_cast<float>(frameRate.value.timeValue));
    copyConvertStringLiteralIntoUTF16(videoSummary, summaryRecP->videoSummary);

    if (summaryRecP->exportAudio)
    {
        std::wstring audioChannelSummary;
        switch (channelType.value.intValue)
        {
        case kPrAudioChannelType_Mono:
            audioChannelSummary = STR_CHANNEL_TYPE_MONO;
            break;
        case kPrAudioChannelType_Stereo:
            audioChannelSummary = STR_CHANNEL_TYPE_STEREO;
            break;
        case kPrAudioChannelType_51:
            audioChannelSummary = STR_CHANNEL_TYPE_51;
            break;
        default:
            audioChannelSummary = L"Unknown";
        }

        swprintf(audioSummary, 256, L"Uncompressed, %.0f Hz, %ls, 16bit",
                 sampleRate.value.floatValue,
                 audioChannelSummary.c_str());
        copyConvertStringLiteralIntoUTF16(audioSummary, summaryRecP->audioSummary);
    }

    return malNoError;
}

prMALError paramButton(exportStdParms *stdParmsP, exParamButtonRec *getFilePrefsRecP)
{
    return malNoError;
}

prMALError validateParamChanged(exportStdParms *stdParmsP, exParamChangedRec *validateParamChangedRecP)
{
    const csSDK_uint32 exID = validateParamChangedRecP->exporterPluginID;
    ExportSettings* settings = reinterpret_cast<ExportSettings*>(validateParamChangedRecP->privateData);
    exParamValues hapSubcodec, toValidate;

    if (settings->exportParamSuite == nullptr)
        return exportReturn_ErrMemory;

    // quality setting is not supported by all subcodecs
    if (strcmp(validateParamChangedRecP->changedParamIdentifier, ADBEVideoCodec) == 0)
    {
        settings->exportParamSuite->GetParamValue(exID, 0, ADBEVideoCodec, &hapSubcodec);
        const auto codecSubtype = reinterpret_cast<CodecSubType&>(hapSubcodec.value.intValue);
        bool enableQuality = Codec::getCapabilities(codecSubtype).hasQuality;

        settings->exportParamSuite->GetParamValue(exID,
            validateParamChangedRecP->multiGroupIndex,
            ADBEVideoQuality,
            &toValidate);

        toValidate.disabled = !enableQuality;
        toValidate.hidden = !enableQuality;
        settings->exportParamSuite->ChangeParam(exID, 0, ADBEVideoQuality, &toValidate);
    }

    // trigger handler on both settings change and resolution change
    if (strcmp(validateParamChangedRecP->changedParamIdentifier, HAPChunk) == 0 
     || strcmp(validateParamChangedRecP->changedParamIdentifier, ADBEVideoWidth) == 0 
     || strcmp(validateParamChangedRecP->changedParamIdentifier, ADBEVideoHeight) == 0)
    {
        chunkSettingsHandler(exID, settings);
    }

    return malNoError;
}

// Handle chunk setting changes and recalculate auto value
void chunkSettingsHandler(const csSDK_uint32 exID, ExportSettings *settings)
{
	exParamValues hapChunksParam, chunkCountValues, width, height;
	settings->exportParamSuite->GetParamValue(exID, 0, HAPChunk, &hapChunksParam);
	settings->exportParamSuite->GetParamValue(exID, 0, HAPChunkCount, &chunkCountValues);
    chunkCountValues.rangeMin.intValue = k_chunkingMin;
    chunkCountValues.rangeMax.intValue = k_chunkingMax;

	switch (hapChunksParam.value.intValue)
	{
	case kHapChunkNone: // No chunks in fact mean 1 chunk
		chunkCountValues.value.intValue = 1;
		chunkCountValues.disabled = kPrTrue;
		chunkCountValues.hidden = kPrTrue;
		break;
	case kHapChunkAuto: // Adjust chunks count to ~ 1920*1200px for each chunk
		settings->exportParamSuite->GetParamValue(exID, 0, ADBEVideoWidth, &width);
		settings->exportParamSuite->GetParamValue(exID, 0, ADBEVideoHeight, &height);
		chunkCountValues.value.intValue = std::min(k_chunkingMax,
			(int)ceil((float)width.value.intValue * height.value.intValue / k_chunkingBlockSize));
		chunkCountValues.disabled = kPrTrue;
		chunkCountValues.hidden = kPrFalse;
		break;
	case kHapChunkManual: // Enable slider for manual setting
		chunkCountValues.disabled = kPrFalse;
		chunkCountValues.hidden = kPrFalse;
		break;
	}
	settings->exportParamSuite->ChangeParam(exID, 0, HAPChunkCount, &chunkCountValues);
}
