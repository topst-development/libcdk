/*
  Copyright (C) 2016 Telechips
 
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

#include <stdio.h>
#include <stdlib.h>

#if defined(_MSC_VER)

#define inline __inline

#endif
#include "aenc.h"
#include "libAACenc/include/aacenc_lib.h"
#include "libSYS/include/FDK_audio.h"

#ifdef __cplusplus
extern "C"
{
#endif

TCAS_S32 TCC_AAC_ENC( TCAS_S32 iOpCode, TCAS_SLONG* pHandle, TCASVoid* pParam1, TCASVoid* pParam2 );

#ifdef __cplusplus
}
#endif

#define LOGD(...)     //{printf("[AACENC:D][%s:%d] ",__func__,__LINE__);printf(__VA_ARGS__);printf("\x1b[0m\n");}
#define LOGI(...)     //{printf("[AACENC:I][%s:%d] ",__func__,__LINE__);printf(__VA_ARGS__);printf("\x1b[0m\n");}
#define LOGW(...)     //{printf("[AACENC:W][%s:%d] ",__func__,__LINE__);printf(__VA_ARGS__);printf("\x1b[0m\n");}
#define LOGE(...)     //{printf("[AACENC:E][%s:%d] ",__func__,__LINE__);printf(__VA_ARGS__);printf("\x1b[0m\n");}


typedef struct aac_enc_info_t
{
	HANDLE_AACENCODER mAACEncoder;
	aenc_callback_func_t 	*m_psCallback;

	TCAS_S32	transport;	// AAC Transport type : ( 0: RAW-AAC, 1: ADTS, 2: ADIF, 3: LATM )
	TCAS_S32	profile;	//AAC Stream Profile type : ( 0 : LC, 1 : HE, 2 : HE_PS )
	TCAS_S32	bitrate;
	TCAS_S32	channel;
	TCAS_S32	samplerate;
	TCAS_S32	encSampleCnt;
	TCAS_S32	vbrmode;
	AUDIO_OBJECT_TYPE	aot;
} aac_enc_info_t;


static TRANSPORT_TYPE getTransportType(TCAS_S32 headertype) {
    TRANSPORT_TYPE user_tptype = TT_UNKNOWN;
    switch (headertype) {
        case 0: user_tptype = TT_MP4_RAW; LOGI("Transport : RAW"); break;
        case 1: user_tptype = TT_MP4_ADTS; LOGI("Transport : ADTS"); break;
        case 2: user_tptype = TT_MP4_ADIF; LOGI("Transport : ADIF"); break;
        case 3: user_tptype = TT_MP4_LOAS; LOGI("Transport : LATM"); break;
        default: user_tptype = TT_UNKNOWN; LOGI("Transport : Unknown");
    }
    return user_tptype;
}

static CHANNEL_MODE getChannelMode(TCAS_S32 nChannels) {
    CHANNEL_MODE chMode = MODE_INVALID;
    switch (nChannels) {
        case 1: chMode = MODE_1; break;
        case 2: chMode = MODE_2; break;
        case 3: chMode = MODE_1_2; break;
        case 4: chMode = MODE_1_2_1; break;
        case 5: chMode = MODE_1_2_2; break;
        case 6: chMode = MODE_1_2_2_1; break;
        default: chMode = MODE_INVALID;
    }
    return chMode;
}

static AUDIO_OBJECT_TYPE getAOTFromProfile(TCAS_S32 profile) {
	AUDIO_OBJECT_TYPE ret;
    if (profile == 0 ) { //LC
    	LOGI("AAC LC profile");
        ret = AOT_AAC_LC;
    } else if (profile == 1) { //HE
    	LOGI("AAC HE profile");
        ret = AOT_SBR;
    } else if (profile == 2) { //HE_PS
    	LOGI("AAC HE-PS profile");
        ret = AOT_PS;
    }
/*
	else if (profile == 3) { //LD
    	LOGI("AAC LD profile");
        ret = AOT_ER_AAC_LD;
    } else if (profile == 4) { //ELD
    	LOGI("AAC ELD profile");
        ret = AOT_ER_AAC_ELD;
    } else if (profile == 5) { //LTP
    	LOGI("AAC LTP profile");
        ret = AOT_AAC_LTP;
    } else if (profile == 6) { //SCALABLE
    	LOGI("AAC SCALABLE profile");
        ret = AOT_AAC_SCAL;
    } 
*/
	else {
        LOGW("Unsupported AAC profile - defaulting to AAC-LC");
        ret = AOT_AAC_LC;
    }

	return ret;
}

TCAS_S32 TCC_AAC_ENC( TCAS_S32 iOpCode, TCAS_SLONG* pHandle, TCASVoid* pParam1, TCASVoid* pParam2 )
{
	TCAS_S32 ret = TCAS_SUCCESS;
	aac_enc_info_t *p_aac_enc;

	if (pHandle == NULL) {
		return (TCAS_S32)TCAS_ERROR_NULL_INSTANCE;
	}

	switch(iOpCode)
	{
		case AUDIO_INIT:
		{
			TCAS_S32 scale=0;
			TCAS_S32 valid_extrainfo = 0;
			audio_streaminfo_t *p_stream_info;
			audio_pcminfo_t *p_pcm_info;
			aenc_callback_func_t *p_callback;
			aenc_init_t *pAencInit = (aenc_init_t *)pParam1;

			p_callback = pAencInit->m_sCommonInfo.m_psAudioCallback;

			p_aac_enc = (aac_enc_info_t *)p_callback->m_pfMalloc(sizeof(aac_enc_info_t));
			if( p_aac_enc == NULL )
			{
				*pHandle = 0;
				return TCAS_ERROR_NULL_INSTANCE;
			}

			FDKmemset(p_aac_enc, 0, sizeof(aac_enc_info_t));

			p_aac_enc->m_psCallback = p_callback;

			p_pcm_info = pAencInit->m_sCommonInfo.m_psAudioEncInput;
			p_stream_info = pAencInit->m_sCommonInfo.m_psAudioEncOutput;

			p_aac_enc->profile = pAencInit->m_unAudioEncodeParams.m_unAAC.m_iAACProfileType;
			p_aac_enc->transport = pAencInit->m_unAudioEncodeParams.m_unAAC.m_iAACHeaderType ; 
			p_aac_enc->vbrmode = pAencInit->m_unAudioEncodeParams.m_unAAC.m_iBitrateMode;
			p_aac_enc->bitrate = p_stream_info->m_uiBitRates;
			p_aac_enc->channel = p_pcm_info->m_uiNumberOfChannel;
			p_aac_enc->samplerate = p_pcm_info->m_eSampleRate;

			p_aac_enc->aot = getAOTFromProfile(p_aac_enc->profile);	

   			LOGI("[input setting] %d ch, %d Hz, %d bps, profile %d, transport %d vbrmode %d", p_aac_enc->channel,p_aac_enc->samplerate,p_aac_enc->bitrate,p_aac_enc->aot,p_aac_enc->transport, p_aac_enc->vbrmode);

			/* Open aac encoder */
			LOGD("aacEncOpen Start");
/*
enum {
    ENC_MODE_FLAG_ALL  = 0x0000;
    ENC_MODE_FLAG_AAC  = 0x0001,
    ENC_MODE_FLAG_SBR  = 0x0002,
    ENC_MODE_FLAG_PS   = 0x0004,
    ENC_MODE_FLAG_SAC  = 0x0008,
    ENC_MODE_FLAG_META = 0x0010
};
*/
			if (aacEncOpen(&p_aac_enc->mAACEncoder, 0, p_aac_enc->channel) != AACENC_OK)
			{
				LOGE("Failed to open AAC encoder");
				return -1;
			}
			if (aacEncoder_SetParam(p_aac_enc->mAACEncoder, AACENC_AOT, p_aac_enc->aot) != AACENC_OK) 
			{
				LOGE("Failed to set AAC encoder AACENC_AOT parameters");
				return 1;
			}
			if (aacEncoder_SetParam(p_aac_enc->mAACEncoder, AACENC_SAMPLERATE, p_aac_enc->samplerate) != AACENC_OK) 
			{
				LOGE("Failed to set AAC encoder AACENC_SAMPLERATE parameters");
				return 1;
			}
			if (aacEncoder_SetParam(p_aac_enc->mAACEncoder, AACENC_CHANNELMODE, getChannelMode(p_aac_enc->channel)) != AACENC_OK) 
			{
				LOGE("Failed to set AAC encoder AACENC_CHANNELMODE parameters\n");
				return -1;
			}
			if (aacEncoder_SetParam(p_aac_enc->mAACEncoder, AACENC_CHANNELORDER, 0) != AACENC_OK) 
			{
				LOGE("Unable to set the MPEG channel order");
				return -1;
			}
			
			if (p_aac_enc->vbrmode)
			{
				if (aacEncoder_SetParam(p_aac_enc->mAACEncoder, AACENC_BITRATEMODE, p_aac_enc->vbrmode) != AACENC_OK) 
				{
					LOGE("Unable to set the VBR bitrate mode\n");
					return -1;
				}
			}
			else
			{
				if (aacEncoder_SetParam(p_aac_enc->mAACEncoder, AACENC_BITRATE, p_aac_enc->bitrate) != AACENC_OK) 
				{
					LOGE("Failed to set AAC encoder AACENC_BITRATE parameters\n");
					return -1;
				}
			}
			if (aacEncoder_SetParam(p_aac_enc->mAACEncoder, AACENC_TRANSMUX, getTransportType(p_aac_enc->transport)) != AACENC_OK) 
			{
				LOGE("Failed to set AAC encoder AACENC_TRANSMUX parameters\n");
				return -1;
			}

			if (aacEncoder_SetParam(p_aac_enc->mAACEncoder, AACENC_AFTERBURNER, 1) != AACENC_OK) 
			{
				LOGE("Unable to set the afterburner mode\n");
				return -1;
			}

			/* Initilize aac encoder */
			LOGD("aacEncEncode - initialize Start");
			if (AACENC_OK !=  aacEncEncode(p_aac_enc->mAACEncoder, NULL, NULL, NULL, NULL))
			{
				LOGE("Unable to initialize encoder for profile / sample-rate / bit-rate / channels");
				return -1;
			}
			LOGD("aacEncOpen Done");

			TCAS_U32 actualBitRate  = aacEncoder_GetParam(p_aac_enc->mAACEncoder, AACENC_BITRATE);
			if (p_aac_enc->bitrate != actualBitRate) {
				if (actualBitRate != -1) {
					LOGW("Requested bitrate %u unsupported, using %u", p_aac_enc->bitrate, actualBitRate);
				} else {
					LOGW("Requested bitrate %u unsupported, using VBR mode %u", p_aac_enc->bitrate, aacEncoder_GetParam(p_aac_enc->mAACEncoder, AACENC_BITRATEMODE));
				}
			}

			AACENC_InfoStruct encInfo;
			if (AACENC_OK != aacEncInfo(p_aac_enc->mAACEncoder, &encInfo)) {
				LOGE("Failed to get AAC encoder info");
				return;
			}

			if (encInfo.confSize) {
				FDKmemcpy(p_stream_info->m_pcStream, encInfo.confBuf,encInfo.confSize);
				p_stream_info->m_iStreamLength = encInfo.confSize;
			}

			// Limit input size
			p_aac_enc->encSampleCnt = encInfo.frameLength * p_aac_enc->channel;

			p_stream_info->m_uiNumberOfChannel = p_pcm_info->m_uiNumberOfChannel;
			p_stream_info->m_eSampleRate = p_pcm_info->m_eSampleRate;
			p_stream_info->m_uiSamplesPerChannel = encInfo.frameLength;
			p_pcm_info->m_uiSamplesPerChannel = p_stream_info->m_uiSamplesPerChannel;

			LOGI("need inputsize = %d (sample/channel)",p_stream_info->m_uiSamplesPerChannel);
			*pHandle = (TCAS_SLONG)p_aac_enc;

			return TCAS_SUCCESS;
		}

		case AUDIO_ENCODE:
		{
			TCAS_S16 *p_channel[2];
			aenc_input_t *p_pcm_info = (aenc_input_t *)pParam1;
			aenc_output_t *p_bit_info = (aenc_output_t *)pParam2;
			TCAS_S32 read_num;
			AACENC_InArgs inargs;
			AACENC_OutArgs outargs;

			p_aac_enc = (aac_enc_info_t *)*pHandle;

			if(p_aac_enc == NULL)
			{
				return TCAS_ERROR_NULL_INSTANCE;
			}

			read_num = p_pcm_info->m_uiNumberOfChannel * p_pcm_info->m_uiSamplesPerChannel * sizeof(TCAS_S16);
			if (read_num < (p_aac_enc->encSampleCnt * sizeof(TCAS_S16)))
			{
				return TCAS_ERROR_MORE_DATA;
			}

			FDKmemset(&inargs, 0, sizeof(inargs));
			FDKmemset(&outargs, 0, sizeof(outargs));
			void *mInputFrame = (void *)p_pcm_info->m_pvChannel[0];
			inargs.numInSamples = p_aac_enc->encSampleCnt;

			//void *inPtr = (TCAS_S16 *)omx_private->remainBuf;
			TCAS_S32   inBufferIds       = IN_AUDIO_DATA;
			TCAS_S32   inBufferSize      = read_num;
			TCAS_S32   inBufferElSize = sizeof(TCAS_S16);

			AACENC_BufDesc inBufDesc = { 0 };
			inBufDesc.numBufs           = 1;
			inBufDesc.bufs              = &mInputFrame; //omx_private->remainBuf;
			inBufDesc.bufferIdentifiers = &inBufferIds;
			inBufDesc.bufSizes          = &inBufferSize;
			inBufDesc.bufElSizes        = &inBufferElSize;

			void *outPtr = p_bit_info->m_pcStream;
			TCAS_S32   outBufferIds   = OUT_BITSTREAM_DATA;
			TCAS_S32   outBufferSize  = 4096;
			TCAS_S32   outBufferElSize= sizeof(UCHAR);

			AACENC_BufDesc outBufDesc = { 0 };
			outBufDesc.numBufs           = 1;
			outBufDesc.bufs              = &outPtr;
			outBufDesc.bufferIdentifiers = &outBufferIds;
			outBufDesc.bufSizes          = &outBufferSize;
			outBufDesc.bufElSizes        = &outBufferElSize;
			///////////////////////////////////////////////////

			/* Encode aac */
		    ret = aacEncEncode(p_aac_enc->mAACEncoder,
                               &inBufDesc,
                               &outBufDesc,
                               &inargs,
                               &outargs);

			if ((ret == 0) && (outargs.numOutBytes > 0))
			{
				//LOGI("[%d] encoded_size = %d",ret, outargs.numOutBytes);
				p_bit_info->m_iStreamLength = outargs.numOutBytes;
				// fwrite(outPtr,outargs.numOutBytes,1,outfile);
			}

			return TCAS_SUCCESS;
		}

		case AUDIO_CLOSE:
		{
			aenc_callback_func_t *p_callback;
			p_aac_enc = (aac_enc_info_t *)*pHandle;

			if(p_aac_enc == NULL)
			{
				return TCAS_ERROR_NULL_INSTANCE;
			}

			p_callback = p_aac_enc->m_psCallback;

			(void)aacEncClose(&p_aac_enc->mAACEncoder);

			p_callback->m_pfFree(p_aac_enc);

			*pHandle = 0;
			LOGI("[AAC] Close");
			return TCAS_SUCCESS;

		}

		default:
			return TCAS_ERROR_NOT_SUPPORT_FORMAT;
	}
}
