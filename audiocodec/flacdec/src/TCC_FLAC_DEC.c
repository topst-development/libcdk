/* -----------------------------------------
  Copyright (C) 2007 Telechips
 
  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at
 
       http://www.apache.org/licenses/LICENSE-2.0
 
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 */

#include <stddef.h>
#include "adec.h"
#include "tcas_typedef.h"
#include "flac_dec_callback.h"

#include "tcc_flac_dec.h"
#include "tcc_flac_dec_core.h"

/* Channel definitions */
#define FRONT_CHANNEL_LEFT   (0)
#define FRONT_CHANNEL_RIGHT  (1)
#define FRONT_CHANNEL_CENTER (2)
#define LFE_CHANNEL          (3)
#define BACK_CHANNEL_LEFT    (4)
#define BACK_CHANNEL_RIGHT   (5)

#define SIDE_CHANNEL_LEFT    (6)
#define SIDE_CHANNEL_RIGHT   (7)
/*
1 channel: mono
2 channels: left, right
3 channels: left, right, center
4 channels: front left, front right, back left, back right
5 channels: front left, front right, front center, back/surround left, back/surround right
6 channels: front left, front right, front center, LFE, back/surround left, back/surround right
7 channels: front left, front right, front center, LFE, back center, side left, side right
8 channels: front left, front right, front center, LFE, back left, back right, side left, side right
*/

#define UNKNOWN_CHANNEL      (8)

static TCAS_S32 ConvertCh_TCAS_To_FLAC(TCAS_U8 **pChannel, TCAS_U8 **pFLAC_CH, TCAS_S32 *nChannelOrder, TCAS_S32 *nChannelOffset, TCAS_S32 pcm_interleaved)
{
	TCAS_S32 ch;
	TCAS_S32 srcCh;
	TCAS_S32 TargetCh = 1;

	srcCh = 0;

	if ((pChannel == NULL) || (pFLAC_CH == NULL) || (nChannelOrder == NULL) || (nChannelOffset == NULL)) {
		return -1;
	}

	do
	{
		for(ch = 0 ; ch < TCAS_MAX_CHANNEL; ch++)
		{
			if(nChannelOrder[ch] == TargetCh){
				break;
			}
		}

		switch(ch)
		{
		case CH_LEFT_FRONT:
			if(pcm_interleaved == 0){
				pFLAC_CH[FRONT_CHANNEL_LEFT] = pChannel[srcCh];
			}else{
				pFLAC_CH[FRONT_CHANNEL_LEFT] = (pChannel[0] + nChannelOffset[srcCh]);
			}
			break;
		case CH_CENTER:
			if(pcm_interleaved == 0){
				pFLAC_CH[FRONT_CHANNEL_CENTER] = pChannel[srcCh];
			}else{
				pFLAC_CH[FRONT_CHANNEL_CENTER] = (pChannel[0] + nChannelOffset[srcCh]);
			}
			break;
		case CH_RIGHT_FRONT:
			if(pcm_interleaved == 0){
				pFLAC_CH[FRONT_CHANNEL_RIGHT] = pChannel[srcCh];
			}else{
				pFLAC_CH[FRONT_CHANNEL_RIGHT] = (pChannel[0] + nChannelOffset[srcCh]);
			}
			break;
		case CH_LEFT_REAR:
			if(pcm_interleaved == 0){
				pFLAC_CH[BACK_CHANNEL_LEFT] = pChannel[srcCh];
			}else{
				pFLAC_CH[BACK_CHANNEL_LEFT] = (pChannel[0] + nChannelOffset[srcCh]);
			}
			break;
		case CH_RIGHT_REAR:
			if(pcm_interleaved == 0){
				pFLAC_CH[BACK_CHANNEL_RIGHT] = pChannel[srcCh];
			}else{
				pFLAC_CH[BACK_CHANNEL_RIGHT] = (pChannel[0] + nChannelOffset[srcCh]);
			}
			break;
		case CH_LEFT_SIDE:
		case CH_RIGHT_SIDE:
			break;
		case CH_LFE:
		default:
			if(pcm_interleaved == 0){
				pFLAC_CH[LFE_CHANNEL] = pChannel[srcCh];
			}else{
				pFLAC_CH[LFE_CHANNEL] = (pChannel[0] + nChannelOffset[srcCh]);
			}
			break;
		}
		TargetCh++;
		srcCh++;
	}while(TargetCh < TCAS_MAX_CHANNEL);

	return 0;
}

static TCAS_S32 AllocatePCMBuffer(TCAS_S32 **pChannel, TCAS_S32 **pFLAC_CH, TCAS_S32 ChannelType)
{
	TCAS_S32 i;
	TCAS_S32 ch = 0;
	TCAS_S32 ret = 0;
	TCC_FLAC_Channel_Type eChannel = (TCC_FLAC_Channel_Type)ChannelType;

	if ((pChannel == NULL) || (pFLAC_CH == NULL)) {
		return -1;
	}

	switch(eChannel)
	{
		case	FLAC_CH1_MONO:
			pChannel[ch] = pFLAC_CH[FRONT_CHANNEL_LEFT];
			ch++;
			break;
		case	FLAC_CH2_STEREO:
		case	FLAC_CH2_LEFT_SIDE:
		case	FLAC_CH2_RIGHT_SIDE:
		case	FLAC_CH2_MID_SIDE:
			pChannel[ch] = pFLAC_CH[FRONT_CHANNEL_LEFT];
			ch++;
			pChannel[ch] = pFLAC_CH[FRONT_CHANNEL_RIGHT];
			ch++;
			break;
		case	FLAC_CH3_3F:
			pChannel[ch] = pFLAC_CH[FRONT_CHANNEL_LEFT];
			ch++;
			pChannel[ch] = pFLAC_CH[FRONT_CHANNEL_RIGHT];
			ch++;
			pChannel[ch] = pFLAC_CH[FRONT_CHANNEL_CENTER];
			ch++;
			break;
		case	FLAC_CH4_2F2R:
			pChannel[ch] = pFLAC_CH[FRONT_CHANNEL_LEFT];
			ch++;
			pChannel[ch] = pFLAC_CH[FRONT_CHANNEL_RIGHT];
			ch++;
			pChannel[ch] = pFLAC_CH[BACK_CHANNEL_LEFT];
			ch++;
			pChannel[ch] = pFLAC_CH[BACK_CHANNEL_RIGHT];
			ch++;
			break;
		case	FLAC_CH5_3F2R:
			pChannel[ch] = pFLAC_CH[FRONT_CHANNEL_LEFT];
			ch++;
			pChannel[ch] = pFLAC_CH[FRONT_CHANNEL_RIGHT];
			ch++;
			pChannel[ch] = pFLAC_CH[FRONT_CHANNEL_CENTER];
			ch++;
			pChannel[ch] = pFLAC_CH[BACK_CHANNEL_LEFT];
			ch++;
			pChannel[ch] = pFLAC_CH[BACK_CHANNEL_RIGHT];
			ch++;
			break;
		case	FLAC_CH6_3F_LFE_2R:
			pChannel[ch] = pFLAC_CH[FRONT_CHANNEL_LEFT];
			ch++;
			pChannel[ch] = pFLAC_CH[FRONT_CHANNEL_RIGHT];
			ch++;
			pChannel[ch] = pFLAC_CH[FRONT_CHANNEL_CENTER];
			ch++;
			pChannel[ch] = pFLAC_CH[LFE_CHANNEL];
			ch++;
			pChannel[ch] = pFLAC_CH[BACK_CHANNEL_LEFT];
			ch++;
			pChannel[ch] = pFLAC_CH[BACK_CHANNEL_RIGHT];
			ch++;
			break;
		case	FLAC_CH7:
			pChannel[ch] = pFLAC_CH[FRONT_CHANNEL_LEFT];
			ch++;
			pChannel[ch] = pFLAC_CH[FRONT_CHANNEL_RIGHT];
			ch++;
			pChannel[ch] = pFLAC_CH[FRONT_CHANNEL_CENTER];
			ch++;
			pChannel[ch] = pFLAC_CH[LFE_CHANNEL];
			ch++;
			pChannel[ch] = pFLAC_CH[BACK_CHANNEL_LEFT];
			ch++;// back center
			pChannel[ch] = pFLAC_CH[SIDE_CHANNEL_LEFT];
			ch++;//
			pChannel[ch] = pFLAC_CH[SIDE_CHANNEL_RIGHT];
			ch++;//
			break;
		case	FLAC_CH8:
			pChannel[ch] = pFLAC_CH[FRONT_CHANNEL_LEFT];
			ch++;
			pChannel[ch] = pFLAC_CH[FRONT_CHANNEL_RIGHT];
			ch++;
			pChannel[ch] = pFLAC_CH[FRONT_CHANNEL_CENTER];
			ch++;
			pChannel[ch] = pFLAC_CH[LFE_CHANNEL];
			ch++;
			pChannel[ch] = pFLAC_CH[BACK_CHANNEL_LEFT];
			ch++;
			pChannel[ch] = pFLAC_CH[BACK_CHANNEL_RIGHT];
			ch++;
			pChannel[ch] = pFLAC_CH[SIDE_CHANNEL_LEFT];
			ch++;//
			pChannel[ch] = pFLAC_CH[SIDE_CHANNEL_RIGHT];
			ch++;//
			break;

		case	FLAC_CH_UNKNOWN:
		default:
			ret = -1;
			break;
	}

	if (ret == 0) {
		for (i = 0; i < ch; i++)
		{
			if (pChannel[i] == NULL){
				ret = -1;
				break;
			}
		}

		if (ret == 0) {
			for( ; ch < 8; ch++)
			{
				pChannel[ch] = NULL;
			}
		}
	}

	return ret;
}

static TCAS_S32 ConvertChToChType(TCAS_S32 numberOfChannel)
{
	TCAS_S32 conv_chtype;
	switch(numberOfChannel)
	{
		case	1:
			conv_chtype = (TCAS_S32)FLAC_CH1_MONO;
			break;
		case	2:
			conv_chtype = (TCAS_S32)FLAC_CH2_STEREO; /* FLAC_CH2_LEFT_SIDE, FLAC_CH2_RIGHT_SIDE, FLAC_CH2_MID_SIDE */
			break;
		case	3:
			conv_chtype = (TCAS_S32)FLAC_CH3_3F;
			break;
		case	4:
			conv_chtype = (TCAS_S32)FLAC_CH4_2F2R;
			break;
		case	5:
			conv_chtype = (TCAS_S32)FLAC_CH5_3F2R;
			break;
		case	6:
			conv_chtype = (TCAS_S32)FLAC_CH6_3F_LFE_2R;
			break;
		case    7:
			conv_chtype = (TCAS_S32)FLAC_CH7;
			break;
		case	8:
			conv_chtype = (TCAS_S32)FLAC_CH8;
			break;
		default:		/* FLAC_CH7, FLAC_CH8, FLAC_CH_UNKNOWN */
			conv_chtype = (TCAS_S32)FLAC_CH_UNKNOWN;
			break;
	}
	return conv_chtype;
}

TCAS_S32 TCC_FLAC_DEC( TCAS_U32 Op, H_FLAC_DEC *pHandle, TCASVoid* pParam1, TCASVoid* pParam2 )
{
	TCC_FLACDecoder *pFLACDEC;
	TCAS_S32	eTcasError;

	if (pHandle == NULL) {
		return (TCAS_S32)TCAS_ERROR_NULL_INSTANCE;
	}

	switch(Op)
	{
		// Decode a frame of data.
		case AUDIO_DECODE:
		{
			TCAS_S32* pFLACCH[TCAS_MAX_CHANNEL];
			audio_pcminfo_t *pPcmInfo;
			audio_streaminfo_t *pBitInfo;

			// The first parameter is a pointer to the FLAC persistent state.
			pFLACDEC = (TCC_FLACDecoder *)(*pHandle);

			if(pFLACDEC == NULL){
				eTcasError = TCAS_ERROR_NULL_INSTANCE;
				break;
			}

			pBitInfo = (audio_streaminfo_t *)pParam1;
			pPcmInfo = (audio_pcminfo_t *)pParam2;

            if (pPcmInfo == NULL) {
                eTcasError = TCAS_ERROR_NULL_PCMBUFF;
				break;
            }
			if (pBitInfo == NULL) {
                eTcasError = TCAS_ERROR_INVALID_BUFSTATE;
				break;
            }

			pFLACDEC->m_sCallbackInfo.m_pfMemset(pFLACCH, 0, sizeof(pFLACCH));

			(void)ConvertCh_TCAS_To_FLAC((TCAS_U8 **)pPcmInfo->m_pvChannel, (TCAS_U8 **)pFLACCH, 
				pPcmInfo->m_iNchannelOrder, pPcmInfo->m_iNchannelOffsets, 
				(TCAS_S32)pPcmInfo->m_ePcmInterLeaved);

			if((pFLACDEC->down_mix_mode == 1U) && (pFLACDEC->nChannelType >= FLAC_CH3_3F)){
				pFLACDEC->nChannelType = FLAC_CH2_STEREO;
			}

			if(AllocatePCMBuffer(pFLACDEC->pFLAC_CH, pFLACCH, (TCAS_S32)pFLACDEC->nChannelType) != 0){
				eTcasError = TCAS_ERROR_NULL_PCMBUFF;
				break;
			}

			(void)FLAC_Decoder_Set_PCMInfo(pFLACDEC->hFLACDEC, (TCAS_S32)pPcmInfo->m_ePcmInterLeaved, pFLACDEC->down_mix_mode);

			pFLACDEC->status = TCC_FLAC_DECODER_ERROR_NONE;

			pFLACDEC->status = (TCC_FLAC_DecoderState)FLAC_Decoder_Frame(pFLACDEC->hFLACDEC, pFLACDEC->pFLAC_CH, (TCAS_S8 *)&pBitInfo->m_pcStream, &pBitInfo->m_iStreamLength);

			if(pFLACDEC->status == TCC_FLAC_DECODER_ERROR_NONE)
			{
				pPcmInfo->m_uiSamplesPerChannel = (TCAS_U32)FLAC_Decoder_Get_Current_PCM_Size(pFLACDEC->hFLACDEC);
				pPcmInfo->m_eSampleRate = (enum_samplerate_t)FLAC_Decoder_Get_Samplerate(pFLACDEC->hFLACDEC);
				pPcmInfo->m_uiNumberOfChannel = (TCAS_U32)FLAC_Decoder_Get_Channels(pFLACDEC->hFLACDEC);
				pFLACDEC->nChannelType = (TCC_FLAC_Channel_Type)ConvertChToChType((TCAS_S32)pPcmInfo->m_uiNumberOfChannel);
				pPcmInfo->m_uiBitsPerSample = pFLACDEC->uiOutFormat;

				if(pFLACDEC->nChannelType == FLAC_CH_UNKNOWN)
				{
					eTcasError = TCAS_ERROR_NOT_SUPPORT_FORMAT;
				}
				else {
					if((pFLACDEC->down_mix_mode != 0U) && (pFLACDEC->nChannelType >= FLAC_CH3_3F)){
						pPcmInfo->m_uiNumberOfChannel = 2;
					}
					eTcasError = TCAS_SUCCESS;
				}
			}
			else
			{
				eTcasError = TCAS_ERROR_INVALID_DATA;
			}
			break;
		}

		// Prepare the codec to decode a file.
		case AUDIO_INIT:
		{
			adec_init_t *pAdecInit = (adec_init_t *)pParam1;

            if (pAdecInit == NULL)
            {
                eTcasError = TCAS_ERROR_INVALID_OPINFO;
				break;
            }

			pFLACDEC = (TCC_FLACDecoder *)pAdecInit->m_pfMalloc(sizeof(TCC_FLACDecoder));
			if(pFLACDEC == NULL)
			{
				*pHandle = NULL;
				eTcasError = TCAS_ERROR_NULL_INSTANCE;
			}
			else
			{
				pAdecInit->m_pfMemset(pFLACDEC, 0, sizeof(TCC_FLACDecoder));
		
				pFLACDEC->m_sCallbackInfo.m_pfMalloc = pAdecInit->m_pfMalloc;
				pFLACDEC->m_sCallbackInfo.m_pfFree = pAdecInit->m_pfFree;
				pFLACDEC->m_sCallbackInfo.m_pfMemcpy = pAdecInit->m_pfMemcpy;
				pFLACDEC->m_sCallbackInfo.m_pfMemset = pAdecInit->m_pfMemset;
				pFLACDEC->m_sCallbackInfo.m_pfMemmove = pAdecInit->m_pfMemmove;
		
				if((pAdecInit->m_psAudiodecOutput->m_uiBitsPerSample != 16U) && (pAdecInit->m_psAudiodecOutput->m_uiBitsPerSample != 24U))
				{
					pAdecInit->m_psAudiodecOutput->m_uiBitsPerSample = 16;
				}
				pFLACDEC->uiOutFormat = pAdecInit->m_psAudiodecOutput->m_uiBitsPerSample;
		
				pFLACDEC->hFLACDEC = FLAC_Decoder_Init( &pFLACDEC->m_sCallbackInfo, pAdecInit->m_pucExtraData, pAdecInit->m_iExtraDataLen, pFLACDEC->uiOutFormat );
		
				if(pFLACDEC->hFLACDEC == NULL)
				{
					pFLACDEC->m_sCallbackInfo.m_pfFree(pFLACDEC->pInstance);
					pFLACDEC->m_sCallbackInfo.m_pfFree(pFLACDEC);
					*pHandle = (H_FLAC_DEC)0;
					eTcasError = TCAS_ERROR_OPEN_FAIL;
				}
				else
				{
		 			pAdecInit->m_psAudiodecOutput->m_eSampleRate = pAdecInit->m_psAudiodecInput->m_eSampleRate;
		 			pAdecInit->m_psAudiodecOutput->m_uiNumberOfChannel = pAdecInit->m_psAudiodecInput->m_uiNumberOfChannel;
		 			pAdecInit->m_psAudiodecOutput->m_uiSamplesPerChannel = pAdecInit->m_psAudiodecInput->m_uiSamplesPerChannel;
		
		 			pFLACDEC->nChannelType = (TCC_FLAC_Channel_Type)ConvertChToChType((TCAS_S32)pAdecInit->m_psAudiodecOutput->m_uiNumberOfChannel);
		 			if( pFLACDEC->nChannelType == FLAC_CH_UNKNOWN )
		 			{
		 				pFLACDEC->nChannelType = FLAC_CH2_STEREO;	//default
		 			}
		 			if(pAdecInit->m_psAudiodecOutput->m_uiNumberOfChannel == 0U){
		 				pAdecInit->m_psAudiodecOutput->m_uiNumberOfChannel = 2; //default
		 			}
		 			if((TCAS_S32)(pAdecInit->m_psAudiodecOutput->m_eSampleRate) == 0){
		 				pAdecInit->m_psAudiodecOutput->m_eSampleRate = TCAS_SR_44100; //default
		 			}
		
		 			pFLACDEC->down_mix_mode = (TCAS_U32)pAdecInit->m_iDownMixMode;
		 			if((pAdecInit->m_psAudiodecOutput->m_uiNumberOfChannel > 2U) && (pAdecInit->m_iDownMixMode == 1))
		 			{
		 				pAdecInit->m_psAudiodecOutput->m_uiNumberOfChannel = 2;
		 			}
		
		 			if(pAdecInit->m_psAudiodecOutput->m_uiSamplesPerChannel <= 0U){
		 				pAdecInit->m_psAudiodecOutput->m_uiSamplesPerChannel = 4608; //default
		 			}
		
		 			if(pAdecInit->m_psAudiodecOutput->m_ePcmInterLeaved == TCAS_ENABLE)
		 			{	
		 				TCAS_S32 i;
						TCAS_S32 iOffset;
		
		 				iOffset = (pAdecInit->m_psAudiodecOutput->m_uiBitsPerSample == 24U) ? 4 : 2;
		 				for(i = 0; i < TCAS_MAX_CHANNEL; i++)
		 				{
		 					pAdecInit->m_psAudiodecOutput->m_iNchannelOffsets[i] = i* (iOffset);
		 				}
		 			}
		
		 			*pHandle = (H_FLAC_DEC)pFLACDEC;
		 			eTcasError = TCAS_SUCCESS;
				}
			}
			break;
		}
		
		case AUDIO_FLUSH:
		{
			pFLACDEC = (TCC_FLACDecoder *)*pHandle;
			
			if(pFLACDEC == NULL){
				eTcasError = TCAS_ERROR_NULL_INSTANCE;
			} else {
				(void)FLAC_Decoder_Reset( pFLACDEC->hFLACDEC );
				eTcasError = TCAS_SUCCESS;
			}
			break;
		}
		// Seek to the specified time position.
		case FLAC_DEC_SEEK:
		{
			eTcasError = TCAS_ERROR_NOT_SUPPORT_FORMAT;
			break;
		}
		
		// Cleanup after the codec.
		case AUDIO_CLOSE:
		{
			pFLACDEC = (TCC_FLACDecoder *)*pHandle;
			
			if(pFLACDEC == NULL){
				eTcasError = TCAS_ERROR_NULL_INSTANCE;
			}
			else if(pFLACDEC->m_sCallbackInfo.m_pfFree == NULL){
				eTcasError = TCAS_ERROR_CLOSE_FAIL;
			}
			else
			{
				(void)TC_FLAC_CloseCodec(pFLACDEC->hFLACDEC, &pFLACDEC->m_sCallbackInfo);	
				pFLACDEC->m_sCallbackInfo.m_pfFree(pFLACDEC);
				*pHandle = (H_FLAC_DEC)0;
				eTcasError = TCAS_SUCCESS;
			}
			break;
		}

		default:
		{
			// Return a failure.
			eTcasError = TCAS_ERROR_NOT_SUPPORT_FORMAT;
			break;
		}
	}
	return eTcasError;
}
