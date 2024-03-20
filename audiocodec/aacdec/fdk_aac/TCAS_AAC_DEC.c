/*
 * Copyright (C) 2016 Telechips
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>

#if defined(_MSC_VER)

#define inline (__inline)

#endif

#include "libAACdec/include/aacdecoder_lib.h"
#include "libFDK/include/FDK_bitstream.h"
#include "adec.h"

#define TCC_AACCODEC_VERSION ("v0.01.05.16")

#define FDK_DEBUG

#ifdef __cplusplus
extern "C"
{
#endif

TCAS_S32 TCC_AAC_DEC(TCAS_S32 Op, TCAS_SLONG* pHandle, void* pParam1, void* pParam2);

#ifdef __cplusplus
}
#endif

//! Data structure for AAC decoding
typedef struct AacDecoderPrivate
{
    //!     AAC Object type : (2 : AAC_LC, 4: LTP, 5: SBR, 22: BSAC, ...)
    TCAS_S32		m_iAACObjectType;
    //!     AAC Stream Header type : ( 0 : RAW-AAC, 1: ADTS, 2: ADIF)
    TCAS_S32		m_iAACHeaderType;
    //!     m_iAACForceUpsampling -> deprecated
    TCAS_S32		m_iAACForceUpsampling;
    //!		upmix (mono to stereo) flag (0 : disable, 1: enable)
    //!     only, if( ( m_iAACForceUpmix == 1 ) && ( channel == mono ) ), then out_channel = 2;
    TCAS_S32		m_iAACForceUpmix;
    //!		Dynamic Range Control
    //!		Dynamic Range Control, Enable Dynamic Range Control (0 : disable (default), 1: enable)
    TCAS_S32		m_iEnableDRC;
    //!		Dynamic Range Control, Scaling factor for boosting gain value, range: 0 (not apply) ~ 127 (fully apply)
    TCAS_S32		m_iDrcBoostFactor;
    //!		Dynamic Range Control, Scaling factor for cutting gain value, range: 0 (not apply) ~ 127 (fully apply)
    TCAS_S32		m_iDrcCutFactor;
    //!		Dynamic Range Control, Target reference level, range: 0 (full scale) ~ 127 (31.75 dB below full-scale)
    TCAS_S32		m_iDrcReferenceLevel;
    //!		Dynamic Range Control, Enable DVB specific heavy compression (aka RF mode), (0 : disable (default), 1: enable)
    TCAS_S32		m_iDrcHeavyCompression;
    //!		m_uiChannelMasking -> deprecated
    TCAS_S32		m_uiChannelMasking;
    //!		m_uiDisableHEAACDecoding -> deprecated
    TCAS_S32		m_uiDisableHEAACDecoding;
    //!		Disable signal level limiting. \n
    //!     1: Turn off PCM limiter, Otherwise: Auto-config. Enable limiter for all non-lowdelay configurations by default.
    TCAS_S32		m_uiDisablePCMLimiter;
    //!		RESERVED
    TCAS_S32		reserved[32-12];
} AacDecoderPrivate;

#define FILEREAD_MAX_LAYERS (2)

typedef struct {
    HANDLE_AACDECODER mAACDecoder;
    CStreamInfo *mStreamInfo;

    TRANSPORT_TYPE mType;
    TCAS_U32 mCheckFormat;

    TCAS_U8 mConfig[4];
    TCAS_U32 mPcmInterleaved;
    TCAS_S32 mCopied;
    TCAS_S32 mConsumed;

    TCAS_U32 mFlag;
    TCAS_S32 mFlush;
    TCAS_S32 mDownMix;
    TCAS_U32 mMaxChannel;

    // DRC
    TCAS_S32 mEnableDRC;
    TCAS_S32 mDrcBoostFactor;
    TCAS_S32 mDrcCutFactor;
    TCAS_S32 mDrcReferenceLevel;
    TCAS_S32 mDrcHeavyCompression;

    void *mShmem;
} TCC_AACDecoder;

static void MakeAudioSpecificConfig(
        TCAS_U8 *config, TCAS_S32 profile, TCAS_S32 samplerate, TCAS_S32 samplerate_idx, TCAS_S32 numChannel, TCAS_S32 frameLen)
{
    TCAS_U32 frameLengthFlag;
    static const TCAS_S32 kSamplingFreq[] = {
        96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000, 7350, 0
    };

    if (config != NULL)
    {
        TCAS_S32 sampling_freq_index = samplerate_idx;
        if (sampling_freq_index == -1) {
            for (sampling_freq_index = 0; sampling_freq_index < 14; sampling_freq_index++)
            {
                if (samplerate == kSamplingFreq[sampling_freq_index])
                {
                    break;
                }
            }
        }

        frameLengthFlag = (frameLen == 960) ? 1U : 0U;

        config[0] = (TCAS_U8)((((TCAS_U32)profile + 1u) << 3) | ((TCAS_U32)sampling_freq_index >> 1));
        config[1] = (TCAS_U8)((((TCAS_U32)sampling_freq_index << 7) & 0x80u) | ((TCAS_U32)numChannel << 3) | ((TCAS_U32)frameLengthFlag << 2));
    }
}

static enum_channel_type_t convertChannelType(TCAS_S32 *elements, AUDIO_CHANNEL_TYPE *type) {
    TCAS_S32 numelementCh;
    TCAS_S32 numOutCh;
    TCAS_S32 i;
    TCAS_S32 numFRONT=0;
    TCAS_S32 numSIDE=0;
    TCAS_S32 numBACK=0;
    TCAS_S32 numLFE=0;
    enum_channel_type_t ret_ch;

    numelementCh = elements[ID_SCE] + (2*elements[ID_CPE]) + elements[ID_LFE];

    for (i = 0; i < 8; i++) {
        //channelType[type[i]]++;
        switch(type[i]) {
            case ACT_FRONT:
				numFRONT++;
				break;
            case ACT_SIDE:
				numSIDE++;
				break;
            case ACT_BACK:
				numBACK++;
				break;
            case ACT_LFE:
				numLFE++;
				break;
            default:
				break;
        }
    }

    numOutCh = numFRONT + numSIDE + numBACK + numLFE;

    switch (numOutCh) {
        case 1:
            ret_ch = TCAS_CH_MONO;
            break;
        case 0:	// dual mono
        case 2:
            if (numelementCh > 2){
                ret_ch = TCAS_CH_STEREO;	// down-mix
            } else {
                if (elements[ID_CPE] != 0) {
                    ret_ch = TCAS_CH_STEREO;
                } else if (elements[ID_SCE] == 2) {
                    ret_ch = TCAS_CH_DUAL;
                } else if (elements[ID_SCE] == 1) {
                    ret_ch = TCAS_CH_STEREO;	// PS or Upmix
                } else {
                    ret_ch = TCAS_CH_UNKNOWN;
                }
            }
            break;
        case 3:
            ret_ch = TCAS_CH_3F;
            break;
        case 4:
            ret_ch = TCAS_CH_3F1R;
            break;
        case 6:
            ret_ch = TCAS_CH_3F2R;
            break;
        default:
            ret_ch = TCAS_CH_UNKNOWN;
            break;
    }
    return ret_ch;
}

static AAC_DECODER_ERROR TCC_AAC_IS_ADTS(
        UCHAR *pBuffer, TCAS_U32 length)
{
    UCHAR *data;
    INT valid = (INT)length;
    AAC_DECODER_ERROR ret;
    UINT synch = 0;
    INT n = 0;

    data = (UCHAR *)FDKmalloc(length);
    (void)FDKmemcpy(data,pBuffer,length);

    ret = AAC_DEC_UNKNOWN;

    // search sync word
    while(((valid > 6) && (n < (valid-6))) && (synch == 0U)) {
      if (((UINT)data[n] == 0xffu) && (((UINT)data[n+1] & 0xf6u) == 0xf0u)) {
        /* This looks like an ADTS frame header but we need at least 6 bytes to proceed */
        //if ((valid - n) >= 6) 
        {
          UINT framesize;
          UINT channels;
          framesize = ((((UINT)data[n+3] & 0x03u) << 11) | ((UINT)data[n+4] << 3) | (((UINT)data[n+5] & 0xe0u) >> 5));
          channels  = ((((UINT)data[n+2] & 0x01u) << 2) | (((UINT)data[n+3] & 0xc0u) >> 6));
          if ((channels != 0U) && (framesize != 0U) && ((valid - n) >= (INT)framesize)) {
            synch = 1;
            ret = AAC_DEC_OK;
          }
        }
      }
      n++;
    }

    FDKfree(data);

    return ret;
}

TCAS_S32 TCC_AAC_DEC(TCAS_S32 Op, TCAS_SLONG* pHandle, void* pParam1, void* pParam2){
    TCAS_S32 eTcasError = TCAS_SUCCESS;
    TCC_AACDecoder *pAACDEC;

    if (pHandle == NULL) {
        return (TCAS_S32)TCAS_ERROR_NULL_INSTANCE;
    }

    switch (Op)
    {
        case AUDIO_INIT:
        {
            TCAS_S32 hasConfigData;
            AAC_DECODER_ERROR decoderErr;
            AacDecoderPrivate *pAacPrivate;
            UCHAR *pCSD;
            TCAS_S32 iCSDLen;
            UCHAR* inBuffer[FILEREAD_MAX_LAYERS];
            UINT inBufferLength[FILEREAD_MAX_LAYERS] = {0};
            UINT bits_per_sample = 16U;

            adec_init_t *pAdecInit = (adec_init_t *)pParam1;
            if ((pAdecInit == NULL) || (pAdecInit->m_psAudiodecInput == NULL) || (pAdecInit->m_psAudiodecOutput == NULL)) {
                return (TCAS_S32)TCAS_ERROR_INVALID_OPINFO;
            }

            pAACDEC = (TCC_AACDecoder *)FDKcalloc(1, sizeof(TCC_AACDecoder));

            if (pAACDEC == NULL)
            {
                *pHandle = 0;
#if defined(FDK_DEBUG)
                FDKprintfErr("Error! Memory allocation failed.\n");
#endif
                return (TCAS_S32)TCAS_ERROR_NULL_INSTANCE;
            }

            pAacPrivate = (AacDecoderPrivate *)(&pAdecInit->m_unAudioCodecParams);

            hasConfigData = (TCAS_S32)((pAdecInit->m_pucExtraData != NULL) && (pAdecInit->m_iExtraDataLen > 1));
            if (hasConfigData == 1) {
                pCSD = pAdecInit->m_pucExtraData;
                iCSDLen = pAdecInit->m_iExtraDataLen;
            }

            switch (pAacPrivate->m_iAACHeaderType)
            {
            case 0:
                pAACDEC->mType = TT_MP4_ADIF; // don't use TT_MP4_RAW for RAW format (causes CTS failure)
                pAACDEC->mCheckFormat = 1;
                if (hasConfigData == 0) { // need Audio Specific Config
#if defined(FDK_DEBUG)
                    FDKprintf("Warning! Audio Specific Config data is required.\n");
#endif
                    MakeAudioSpecificConfig(pAACDEC->mConfig,
                                            pAacPrivate->m_iAACObjectType - 1,
                                            (TCAS_S32)pAdecInit->m_psAudiodecInput->m_eSampleRate,
                                            -1,
                                            (TCAS_S32)pAdecInit->m_psAudiodecInput->m_uiNumberOfChannel,
                                            (TCAS_S32)pAdecInit->m_psAudiodecInput->m_uiSamplesPerChannel);

                    pCSD = (UCHAR *)pAACDEC->mConfig;
                    iCSDLen = 2;
                    hasConfigData = 2;
                }
                break;
            case 1:
                pAACDEC->mType = TT_MP4_ADTS;
                break;
            case 2:
                pAACDEC->mType = TT_MP4_ADIF;
                break;
            case 3:
                pAACDEC->mType = TT_MP4_LOAS;
                break;
            case 4:
                pAACDEC->mType = TT_MP4_LATM_MCP1;
                if (hasConfigData == 0) {
#if defined(FDK_DEBUG)
                    FDKprintf("Warning! Stream Mux Config data is required.\n");
#endif
                }
                break;
            case 5:
                pAACDEC->mType = TT_MP4_LATM_MCP0;
                if (hasConfigData == 0) { // need Stream Mux Config
#if defined(FDK_DEBUG)
                    FDKprintfErr("Error! Stream Mux Config data is required.\n");
#endif
                    eTcasError = TCAS_ERROR_INVALID_OPINFO;
                    goto exit_error;
                }
                break;

            default:
#if defined(FDK_DEBUG)
                FDKprintfErr("Error! Not supported transport type (%d)\n", pAacPrivate->m_iAACHeaderType);
#endif
                eTcasError = TCAS_ERROR_NOT_SUPPORT_FORMAT;
                break;
            }

            if (eTcasError == TCAS_ERROR_NOT_SUPPORT_FORMAT) {
                goto exit_error;
            }

            pAACDEC->mAACDecoder = aacDecoder_Open(pAACDEC->mType, /* num layers */ 1);
            if (pAACDEC->mAACDecoder == NULL) {
#if defined(FDK_DEBUG)
                FDKprintfErr("Error! aacDecoder_Open failed.\n");
#endif
                eTcasError = TCAS_ERROR_OPEN_FAIL;
                goto exit_error;
            }

            pAACDEC->mStreamInfo = aacDecoder_GetStreamInfo(pAACDEC->mAACDecoder);

            /* Audio Specific Config (ASC) or Stream Mux Config (SMC) */
            if ((hasConfigData != 0) && (pAACDEC->mType != TT_MP4_LOAS)) {
                inBuffer[0] = (UCHAR *)(pCSD);
                inBufferLength[0] = (UINT)iCSDLen;
                decoderErr = aacDecoder_ConfigRaw(pAACDEC->mAACDecoder, inBuffer, inBufferLength);
                if (decoderErr != AAC_DEC_OK) {
#if defined(FDK_DEBUG)
                    FDKprintfErr("Error! aacDecoder_ConfigRaw failed (%d)\n", decoderErr);
#endif
                    eTcasError = TCAS_ERROR_INVALID_OPINFO;
                    goto exit_error;
                }

                // Apply information from configuration data.
                if (pAACDEC->mStreamInfo->channelConfig != 0) {
                    pAdecInit->m_psAudiodecOutput->m_uiNumberOfChannel = (TCAS_U32)pAACDEC->mStreamInfo->channelConfig;
                } else {
                    // fix-me : use PCE instead
                    pAdecInit->m_psAudiodecOutput->m_uiNumberOfChannel = pAdecInit->m_psAudiodecInput->m_uiNumberOfChannel;
                }
                if (pAACDEC->mStreamInfo->extSamplingRate != 0) {
                    pAdecInit->m_psAudiodecOutput->m_eSampleRate = (enum_samplerate_t)pAACDEC->mStreamInfo->extSamplingRate;
                } else {
                    pAdecInit->m_psAudiodecOutput->m_eSampleRate = (enum_samplerate_t)pAACDEC->mStreamInfo->aacSampleRate;
                }

                pAdecInit->m_psAudiodecOutput->m_uiSamplesPerChannel = (TCAS_U32)pAACDEC->mStreamInfo->aacSamplesPerFrame;
                if (pAACDEC->mStreamInfo->extAot == AOT_SBR) {
                    (pAdecInit->m_psAudiodecOutput->m_uiSamplesPerChannel) <<= 1;
                }
                pAdecInit->m_psAudiodecOutput->m_uiBitsPerSample = bits_per_sample;
                pAdecInit->m_psAudiodecInput->m_uiNumberOfChannel = pAdecInit->m_psAudiodecOutput->m_uiNumberOfChannel;
                pAdecInit->m_psAudiodecInput->m_eSampleRate = (enum_samplerate_t)pAACDEC->mStreamInfo->aacSampleRate;
            } else {
                // No configuration data. So let pass input as it is.
                pAdecInit->m_psAudiodecOutput->m_uiNumberOfChannel = pAdecInit->m_psAudiodecInput->m_uiNumberOfChannel;
                pAdecInit->m_psAudiodecOutput->m_eSampleRate = pAdecInit->m_psAudiodecInput->m_eSampleRate;
                if (pAdecInit->m_psAudiodecInput->m_uiSamplesPerChannel != 0U) {
                    pAdecInit->m_psAudiodecOutput->m_uiSamplesPerChannel = pAdecInit->m_psAudiodecInput->m_uiSamplesPerChannel;
                } else {
                    pAdecInit->m_psAudiodecOutput->m_uiSamplesPerChannel = 1024;
                }
                pAdecInit->m_psAudiodecOutput->m_uiBitsPerSample = 16;
            }

            // Apply user settings.
            pAACDEC->mDownMix = pAdecInit->m_iDownMixMode;
            if (pAACDEC->mDownMix == 1) { // stereo mix-down
                pAACDEC->mMaxChannel = 2;
                (void)aacDecoder_SetParam(pAACDEC->mAACDecoder, AAC_PCM_MAX_OUTPUT_CHANNELS, 2);
                pAdecInit->m_psAudiodecOutput->m_uiNumberOfChannel = 2;

            } else { // Disable downmixing feature.
                pAACDEC->mMaxChannel = 8;
                (void)aacDecoder_SetParam(pAACDEC->mAACDecoder, AAC_PCM_MAX_OUTPUT_CHANNELS, -1);
            }

            pAACDEC->mPcmInterleaved = (UINT)pAdecInit->m_psAudiodecOutput->m_ePcmInterLeaved;
            if ((pAACDEC->mPcmInterleaved == 0U) && (pAdecInit->m_psAudiodecOutput->m_uiNumberOfChannel > 2U)) {
#if defined(FDK_DEBUG)
                FDKprintfErr("Error! not-interleaved PCM format is not yet supported due to FDK-AAC's Bug.\n");
#endif
                eTcasError = TCAS_ERROR_INVALID_OPINFO;
                goto exit_error;
            } else {
                (void)aacDecoder_SetParam(pAACDEC->mAACDecoder, AAC_PCM_OUTPUT_INTERLEAVED, 1);
            }

            if (pAacPrivate->m_iAACForceUpmix == 1) {
                (void)aacDecoder_SetParam(pAACDEC->mAACDecoder, AAC_PCM_MIN_OUTPUT_CHANNELS, 2);
            }

            // No longer needed.
            //pAACDEC->mForceUpSample = pAacPrivate->m_iAACForceUpsampling;

            pAACDEC->mEnableDRC = pAacPrivate->m_iEnableDRC;
            if (pAacPrivate->m_iEnableDRC == 0) {
                (void)aacDecoder_SetParam(pAACDEC->mAACDecoder, AAC_DRC_BOOST_FACTOR, 0);
                (void)aacDecoder_SetParam(pAACDEC->mAACDecoder, AAC_DRC_ATTENUATION_FACTOR, 0);
                (void)aacDecoder_SetParam(pAACDEC->mAACDecoder, AAC_DRC_REFERENCE_LEVEL, -1);
            } else {
                pAACDEC->mDrcBoostFactor = pAacPrivate->m_iDrcBoostFactor;
                (void)aacDecoder_SetParam(pAACDEC->mAACDecoder, AAC_DRC_BOOST_FACTOR, pAACDEC->mDrcBoostFactor);

                pAACDEC->mDrcCutFactor = pAacPrivate->m_iDrcCutFactor;
                (void)aacDecoder_SetParam(pAACDEC->mAACDecoder, AAC_DRC_ATTENUATION_FACTOR, pAACDEC->mDrcCutFactor);

                pAACDEC->mDrcReferenceLevel = pAacPrivate->m_iDrcReferenceLevel;
                (void)aacDecoder_SetParam(pAACDEC->mAACDecoder, AAC_DRC_REFERENCE_LEVEL, pAACDEC->mDrcReferenceLevel);

                pAACDEC->mDrcHeavyCompression = pAacPrivate->m_iDrcHeavyCompression;
                (void)aacDecoder_SetParam(pAACDEC->mAACDecoder, AAC_DRC_HEAVY_COMPRESSION, pAACDEC->mDrcHeavyCompression);
            }

            if (pAacPrivate->m_uiDisablePCMLimiter == 1) {
                // Disable limiter
                (void)aacDecoder_SetParam(pAACDEC->mAACDecoder, AAC_PCM_LIMITER_ENABLE, 0);
            }

            // enable error concealment
            // pAACDEC->mFlag |= AACDEC_CONCEAL;
            // AAC_CONCEAL_METHOD
            //(void)aacDecoder_SetParam(pAACDEC->mAACDecoder, AAC_CONCEAL_METHOD, 1);

            FDKprintf("fdk-aac ver %s: init done\n", TCC_AACCODEC_VERSION);

#ifdef KERNEL_MODE
            pAACDEC->mShmem = tcc_aac_init_shmemory(8192 * 2);
            if (pAACDEC->mShmem == NULL) {
                eTcasError = TCAS_ERROR_NULL_INSTANCE;
                goto exit_error;
            }

            aacDecoder_SetShmemHandle(pAACDEC->mAACDecoder, pAACDEC->mShmem);
#endif

            *pHandle = (TCAS_SLONG)pAACDEC;

            eTcasError = TCAS_SUCCESS;
            break;

    exit_error:
            if (pAACDEC->mAACDecoder != NULL) {
                aacDecoder_Close(pAACDEC->mAACDecoder);
                pAACDEC->mAACDecoder = NULL;
            }

            (void)FDKfree(pAACDEC);
            *pHandle = 0;
            break;
        }

        case AUDIO_DECODE:
        {
            INT_PCM *outBuffer;
            UCHAR* inBuffer[FILEREAD_MAX_LAYERS];
            UINT inBufferLength[FILEREAD_MAX_LAYERS] = {0};
            UINT bytesValid[FILEREAD_MAX_LAYERS] = {0};

            adec_input_t *pBitInfo = (adec_input_t *)pParam1;
            adec_output_t *pPcmInfo = (adec_output_t *)pParam2;
            AAC_DECODER_ERROR decoderErr;	// = AAC_DEC_NOT_ENOUGH_BITS;

            pAACDEC = (TCC_AACDecoder *)(*pHandle);

            if ((pAACDEC == NULL) || (pAACDEC->mAACDecoder == NULL)) {
                return (TCAS_S32)TCAS_ERROR_NULL_INSTANCE;
            }

            if ((pPcmInfo == NULL) || (pPcmInfo->m_pvChannel[0] == NULL)) {
                return (TCAS_S32)TCAS_ERROR_NULL_PCMBUFF;
            }

            if ((pBitInfo == NULL) || (pBitInfo->m_pcStream == NULL) || (pBitInfo->m_iStreamLength <= 0)) {
                return (TCAS_S32)TCAS_ERROR_INVALID_BUFSTATE;
            }

            if (pBitInfo->m_iStreamLength < 2) {
                return (TCAS_S32)TCAS_ERROR_MORE_DATA;
            }

#ifdef KERNEL_MODE
            tcc_aac_reset_shmemory(pAACDEC->mShmem);
#endif

            if (pAACDEC->mCheckFormat == 1U) {
                AAC_DECODER_ERROR local_ret;
                if (pBitInfo->m_iStreamLength < 7) {
                    return (TCAS_S32)TCAS_ERROR_MORE_DATA;
                }

                pAACDEC->mCheckFormat = 0;
                local_ret = TCC_AAC_IS_ADTS((UCHAR *)(pBitInfo->m_pcStream), (TCAS_U32)pBitInfo->m_iStreamLength);
                if (local_ret == AAC_DEC_OK) {
                    if (pAACDEC->mType != TT_MP4_ADTS) {
                        local_ret = aacDecoder_SetFormat(pAACDEC->mAACDecoder, TT_MP4_ADTS);
                        if (local_ret != AAC_DEC_OK) {
                            return (TCAS_S32)TCAS_ERROR_INVALID_OPINFO;
                        }
                        pAACDEC->mType = TT_MP4_ADTS;
                    }
                } else if ((pBitInfo->m_pcStream[0] == 0x56) && (((TCAS_U32)pBitInfo->m_pcStream[1] & 0xE0u) == 0xE0u)) {
                    TCAS_U32 uiFrameSize = (TCAS_U32)(((((TCAS_U32)pBitInfo->m_pcStream[1] << 8) | (TCAS_U32)pBitInfo->m_pcStream[2]) & 0x1FFFU) + 3u);
                    if (pAACDEC->mType != TT_MP4_LOAS) {
                        local_ret = aacDecoder_SetFormat(pAACDEC->mAACDecoder, TT_MP4_LOAS);
                        if (local_ret != AAC_DEC_OK) {
                            return (TCAS_S32)TCAS_ERROR_INVALID_OPINFO;
                        }
                        pAACDEC->mType = TT_MP4_LOAS;
					}
                } else {
				}
            }

            outBuffer = (INT_PCM *)pPcmInfo->m_pvChannel[0];

            inBuffer[0] = (UCHAR *)(pBitInfo->m_pcStream);
            inBufferLength[0] = (UINT)pBitInfo->m_iStreamLength;
            bytesValid[0] = inBufferLength[0];

            (void)aacDecoder_Fill(pAACDEC->mAACDecoder, inBuffer, inBufferLength, bytesValid);
            pAACDEC->mCopied = ((INT)inBufferLength[0] - (INT)bytesValid[0]);
            //pBitInfo->m_pcStream += pAACDEC->mCopied;
            //pBitInfo->m_iStreamLength -= pAACDEC->mCopied;

            pAACDEC->mConsumed = (INT)pAACDEC->mStreamInfo->numTotalBytes;
            if (pAACDEC->mFlush == 1) {
                pAACDEC->mFlag |= (/*AACDEC_FLUSH | */AACDEC_INTR | AACDEC_CLRHIST);
            }

            decoderErr = aacDecoder_DecodeFrame(pAACDEC->mAACDecoder, outBuffer, (2048 * (INT)pAACDEC->mMaxChannel), (UINT)pAACDEC->mFlag);

            if (pAACDEC->mFlush == 1) {
                pAACDEC->mFlush = 0;
                pAACDEC->mFlag &= ~ (/*AACDEC_FLUSH | */AACDEC_INTR | AACDEC_CLRHIST);
            }

            pAACDEC->mConsumed = (INT)pAACDEC->mStreamInfo->numTotalBytes - pAACDEC->mConsumed;

            if (pAACDEC->mCopied != pAACDEC->mConsumed) {
                (void)aacDecoder_SetParam(pAACDEC->mAACDecoder, AAC_TPDEC_CLEAR_BUFFER, 1);
            }
            pBitInfo->m_pcStream += pAACDEC->mConsumed;
            pBitInfo->m_iStreamLength -= pAACDEC->mConsumed;

            if (decoderErr == AAC_DEC_NOT_ENOUGH_BITS) {
#if defined(FDK_DEBUG)
                FDKprintf("Not enough bits, bytesValid %d\n", bytesValid[0]);
#endif
                return (TCAS_S32)TCAS_ERROR_MORE_DATA;
            }

            if (decoderErr != AAC_DEC_OK) {
#if defined(FDK_DEBUG)
                FDKprintfErr("decode error %d\n", decoderErr);
#endif
                (void)aacDecoder_SetParam(pAACDEC->mAACDecoder, AAC_TPDEC_CLEAR_BUFFER, 1);
                if ((pAACDEC->mType == TT_MP4_RAW) || (pAACDEC->mType == TT_MP4_ADIF)) {
                    pBitInfo->m_iStreamLength = 0;
                }

                if (pAACDEC->mStreamInfo->frameSize == 0) {
                    return (TCAS_S32)TCAS_ERROR_DECODE;
                }
            }

            if ((pAACDEC->mPcmInterleaved == 0U) && (pAACDEC->mStreamInfo->numChannels == 2)) {
                INT_PCM *pcmbuff = aacDecoder_GetPcmBuffer(pAACDEC->mAACDecoder);
                INT_PCM *left = (INT_PCM *)pPcmInfo->m_pvChannel[0];
                INT_PCM *right = (INT_PCM *)pPcmInfo->m_pvChannel[1];
                INT sample;

                for (sample = 0; sample < pAACDEC->mStreamInfo->frameSize; sample+=2) {
                    left[0] = pcmbuff[0];
                    right[0] = pcmbuff[1];
                    left[1] = pcmbuff[2];
                    right[1] = pcmbuff[3];
                    left += 2;
					right += 2;
					pcmbuff += 4;
                }
            }

            pPcmInfo->m_uiNumberOfChannel = (TCAS_U32)pAACDEC->mStreamInfo->numChannels;
            pPcmInfo->m_eSampleRate = (enum_samplerate_t)pAACDEC->mStreamInfo->sampleRate;
            pPcmInfo->m_uiSamplesPerChannel = (TCAS_U32)pAACDEC->mStreamInfo->frameSize;
            pPcmInfo->m_ePcmInterLeaved = (enum_function_switch_t)pAACDEC->mPcmInterleaved;
            pPcmInfo->m_uiBitsPerSample = 16;
            pPcmInfo->m_eChannelType = convertChannelType(pAACDEC->mStreamInfo->pElements, pAACDEC->mStreamInfo->pChannelType);

            /* update original stream info */
            pBitInfo->m_uiNumberOfChannel = (TCAS_U32)pAACDEC->mStreamInfo->aacNumChannels;

            if (decoderErr != AAC_DEC_OK) {
                return (TCAS_S32)TCAS_ERROR_CONCEALMENT_APPLIED;
            }
            eTcasError = TCAS_SUCCESS;
            break;
        }

        case AUDIO_CLOSE:

            pAACDEC = (TCC_AACDecoder *)(*pHandle);

            if (pAACDEC == NULL) {
                eTcasError = TCAS_ERROR_NULL_INSTANCE;
            } else {

              if (pAACDEC->mAACDecoder != NULL) {
                  (void)aacDecoder_Close(pAACDEC->mAACDecoder);
                  pAACDEC->mAACDecoder = NULL;
              }

#ifdef KERNEL_MODE
              tcc_aac_close_shmemory(pAACDEC->mShmem);
              pAACDEC->mShmem = NULL;
#endif
              FDKfree(pAACDEC);
              *pHandle = 0;
              eTcasError = TCAS_SUCCESS;
            }
            break;

        case AUDIO_FLUSH:

            pAACDEC = (TCC_AACDecoder *)*pHandle;

            if ((pAACDEC == NULL) || (pAACDEC->mAACDecoder == NULL))
            {
                return (TCAS_S32)TCAS_ERROR_NULL_INSTANCE;
            }

            (void)aacDecoder_SetParam(pAACDEC->mAACDecoder, AAC_TPDEC_CLEAR_BUFFER, 1);
            pAACDEC->mFlush = 1; 

            eTcasError = TCAS_SUCCESS;
            break;

        default:
            eTcasError = TCAS_ERROR_NOT_SUPPORT_FORMAT;
            break;
    }
    return eTcasError;
}
