/*
 * Copyright (C) 2018 Telechips
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

#include "adec.h"

#include "include/pvmp3decoder_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

TCAS_S32 TCC_MP3_DEC(TCAS_S32 iOpCode, TCAS_SLONG* pHandle, void* pParam1, void* pParam2);

#ifdef __cplusplus
}
#endif

typedef struct {
    tPVMP3DecoderExternal *mConfig;
    void *mDecoderBuf;
    int32_t mNumChannels;
    int32_t mSamplingRate;
    void (*free)(void *ptr);
} TCC_MP3Decoder;

TCAS_S32 TCC_MP3_DEC(TCAS_S32 iOpCode, TCAS_SLONG* pHandle, void* pParam1, void* pParam2)
{
    TCAS_S32 eTcasError;
    TCC_MP3Decoder *pMP3DEC;

    if (pHandle == NULL){
        return (TCAS_S32)TCAS_ERROR_NULL_INSTANCE;
    }
    switch (iOpCode)
    {
        case AUDIO_INIT:
        {
            adec_init_t *pAdecInit = (adec_init_t *)pParam1;

            if ((pAdecInit == NULL) || 
                (pAdecInit->m_psAudiodecInput == NULL) || 
                (pAdecInit->m_psAudiodecOutput == NULL) || 
                (pAdecInit->m_psAudiodecOutput->m_ePcmInterLeaved != TCAS_ENABLE) ||
                (pAdecInit->m_pfMalloc == NULL) ||
                (pAdecInit->m_pfFree == NULL)) 
            {
                eTcasError = TCAS_ERROR_INVALID_OPINFO;
            }
            else 
            {
                pMP3DEC = (TCC_MP3Decoder *)pAdecInit->m_pfMalloc(sizeof(TCC_MP3Decoder));
    
                if (pMP3DEC == NULL) {
                    *pHandle = 0;
                    eTcasError = TCAS_ERROR_NULL_INSTANCE;
                }
                else
                {    
                    uint32_t memRequirements;
                    pMP3DEC->mDecoderBuf = NULL;
                    pMP3DEC->free = pAdecInit->m_pfFree;
                    memRequirements = pvmp3_decoderMemRequirements();
        
                    pMP3DEC->mConfig = (tPVMP3DecoderExternal *)pAdecInit->m_pfMalloc(sizeof(tPVMP3DecoderExternal));
                    pMP3DEC->mDecoderBuf = pAdecInit->m_pfMalloc(memRequirements);
                    if ((pMP3DEC->mConfig == NULL) || (pMP3DEC->mDecoderBuf == NULL)) {
                        if (pMP3DEC->mDecoderBuf != NULL) {
                            pAdecInit->m_pfFree(pMP3DEC->mDecoderBuf);
                            pMP3DEC->mDecoderBuf = NULL;
                        }
            
                        if (pMP3DEC->mConfig != NULL) {
                            pAdecInit->m_pfFree(pMP3DEC->mConfig);
                            pMP3DEC->mConfig = NULL;
                        }

                        pAdecInit->m_pfFree(pMP3DEC);
                        *pHandle = 0;
                        eTcasError = TCAS_ERROR_OPEN_FAIL;
                    }
                    else
                    {
                        pMP3DEC->mConfig->equalizerType = flat;
                        pMP3DEC->mConfig->crcEnabled = 0;

                        pvmp3_InitDecoder(pMP3DEC->mConfig, pMP3DEC->mDecoderBuf);

                        pAdecInit->m_psAudiodecOutput->m_uiSamplesPerChannel = 1152;
                        pAdecInit->m_psAudiodecOutput->m_uiBitsPerSample = 16;

                        pMP3DEC->mNumChannels = (int32_t)pAdecInit->m_psAudiodecInput->m_uiNumberOfChannel;
                        pMP3DEC->mSamplingRate = (int32_t)pAdecInit->m_psAudiodecInput->m_eSampleRate;

                        *pHandle = (TCAS_SLONG)pMP3DEC;
                        eTcasError = TCAS_SUCCESS;
                    }
                }
            }
            break;
        }

        case AUDIO_DECODE:
        {
            ERROR_CODE decoderErr;
            adec_input_t *pBitInfo = (adec_input_t *)pParam1;
            adec_output_t *pPcmInfo = (adec_output_t *)pParam2;

            pMP3DEC = (TCC_MP3Decoder *)*pHandle;

            if (pMP3DEC == NULL) {
                eTcasError = TCAS_ERROR_NULL_INSTANCE;
            } else if ((pPcmInfo == NULL) || (pPcmInfo->m_pvChannel[0] == NULL)) {
                eTcasError = TCAS_ERROR_NULL_PCMBUFF;
            } else if ((pBitInfo == NULL) || (pBitInfo->m_pcStream == NULL) || (pBitInfo->m_iStreamLength <= 0)) {
                eTcasError = TCAS_ERROR_INVALID_BUFSTATE;
            } else {

                pMP3DEC->mConfig->pInputBuffer = (uint8 *)(pBitInfo->m_pcStream);
                pMP3DEC->mConfig->inputBufferCurrentLength = pBitInfo->m_iStreamLength;
    
                pMP3DEC->mConfig->inputBufferMaxLength = 0;
                pMP3DEC->mConfig->inputBufferUsedLength = 0;
    
                pMP3DEC->mConfig->outputFrameSize = 4608;
                pMP3DEC->mConfig->pOutputBuffer = (int16 *)(pPcmInfo->m_pvChannel[0]);
                decoderErr = pvmp3_framedecoder(pMP3DEC->mConfig, pMP3DEC->mDecoderBuf);
                if (decoderErr != NO_DECODING_ERROR) {
                    if (decoderErr == NO_ENOUGH_MAIN_DATA_ERROR) {
                        eTcasError = TCAS_ERROR_MORE_DATA;
                    } else {
                        pMP3DEC->mConfig->inputBufferUsedLength = pBitInfo->m_iStreamLength;
                        eTcasError = TCAS_ERROR_DECODE;
                    }
                } else {
                    if ((pMP3DEC->mConfig->samplingRate != pMP3DEC->mSamplingRate) || (pMP3DEC->mConfig->num_channels != pMP3DEC->mNumChannels)) {
                        pMP3DEC->mSamplingRate = pMP3DEC->mConfig->samplingRate;
                        pMP3DEC->mNumChannels = pMP3DEC->mConfig->num_channels;
                    }
                    pPcmInfo->m_eSampleRate = (enum_samplerate_t)(pMP3DEC->mConfig->samplingRate);
                    pPcmInfo->m_uiNumberOfChannel = (TCAS_U32)(pMP3DEC->mConfig->num_channels);
                    pPcmInfo->m_uiSamplesPerChannel = (TCAS_U32)((TCAS_U32)pMP3DEC->mConfig->outputFrameSize / (TCAS_U32)pMP3DEC->mConfig->num_channels);
                    pPcmInfo->m_ePcmInterLeaved = TCAS_ENABLE;
                    pPcmInfo->m_uiBitsPerSample = 16;
                    eTcasError = TCAS_SUCCESS;
                }
                pBitInfo->m_pcStream += pMP3DEC->mConfig->inputBufferUsedLength;
                pBitInfo->m_iStreamLength -= pMP3DEC->mConfig->inputBufferUsedLength;
            }

            break;
        }

        case AUDIO_CLOSE:
        {
            pMP3DEC = (TCC_MP3Decoder *)(*pHandle);

            if (pMP3DEC == NULL) {
                eTcasError = TCAS_ERROR_NULL_INSTANCE;
            } else {
                if (pMP3DEC->mDecoderBuf != NULL) {
                    pMP3DEC->free(pMP3DEC->mDecoderBuf);
                    pMP3DEC->mDecoderBuf = NULL;
                }

                if(pMP3DEC->mConfig != NULL) {
                    pMP3DEC->free(pMP3DEC->mConfig);
                    pMP3DEC->mConfig = NULL;
                }

                pMP3DEC->free(pMP3DEC);

                *pHandle = 0;
                eTcasError = TCAS_SUCCESS;
            }
            break;	
        }

        case AUDIO_FLUSH:
        {
            pMP3DEC = (TCC_MP3Decoder *)*pHandle;

            if (pMP3DEC == NULL) {
                eTcasError = TCAS_ERROR_NULL_INSTANCE;
            } else {
                pvmp3_InitDecoder(pMP3DEC->mConfig, pMP3DEC->mDecoderBuf);
                eTcasError = TCAS_SUCCESS;
            }
            break;	
        }

        default:
            eTcasError = TCAS_ERROR_NOT_SUPPORT_FORMAT;
            break;
    }
    return eTcasError;
}
