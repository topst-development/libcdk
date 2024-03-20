/* libFLAC - Free Lossless Audio Codec library
 * Copyright (C) 2001-2009  Josh Coalson
 * Copyright (C) 2011-2014  Xiph.Org Foundation
 * Copyright (C) 2007-2017  Telechips
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * - Neither the name of the Xiph.org Foundation nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "tcas_typedef.h"
#include "flac_dec_callback.h"
#include "tcc_flac_assert.h"
#include "tcc_flac_format.h"
#include "tcc_flac_streamdec.h"

typedef struct 
{
	TCAS_U64				m_ulSamplesProcessed;
	TCAS_U32				m_uiFrameCounter;

	TCAS_S32				m_iGotStreamInfo;
	TCAS_U64				m_ulTotalSamples;
	TCAS_U32				m_uiBitsPerSample;
	TCAS_U32				m_uiChannels;
	TCAS_U32				m_uiSampleRate;

//	TCAS_S32				m_iHasMd5Sum;
//	TCAS_U32				m_uiChannelMask;

	TCC_FLAC_StreamDecoder	*m_pstStereamDecInfo;
	
	TCAS_S32				**m_ppsChannel;
	
	//TCAS_S32				m_iChOffset;
	TCAS_S32				m_iPcmInterleavingFlag;
	TCAS_U32				m_uiDownMixMode;
	TCAS_U32				m_uiOutputPcmBits;
} TCC_FLAC_DecoderInfo;

typedef enum 
{
	TCC_FLAC_DECODER_ERROR_NONE = 0,
	TCC_FLAC_DECODER_END_OF_STREAM,
	TCC_FLAC_DECODER_CHANNEL_ERROR,
	TCC_FLAC_DECODER_BLOCKSIZE_ERROR,	
	TCC_FLAC_DECODER_SEEK_ERROR,
	TCC_FLAC_DECODER_ABORTED,
	TCC_FLAC_DECODER_MEMORY_ALLOCATION_ERROR,
	TCC_FLAC_DECODER_UNINITIALIZED,
	TCC_FLAC_DECODER_METADATA_ERROR,
	TCC_FLAC_DECODER_STREAM_ERROR,
    TCC_FLAC_DECODER_INPUT_BUFFER_ERROR,
} TCC_FLAC_DecoderState;

/* local routines */
TCAS_S32 TC_FLAC_CloseCodec(TCC_FLAC_DecoderInfo *pstDecoderInfo, flac_callback_t *pCallback);
static TCC_FLAC_StreamDecoderWriteStatus TC_FLAC_PCM_Out(const TCC_FLAC_StreamDecoder *pstDecoder, const TCC_FLAC_Frame *pstFrame, TCAS_S32 *buffer[], TCASVoid *client_data );


TCAS_S32 FLAC_Decoder_Setbuffer(TCC_FLAC_StreamDecoder *pstDecoder, flac_callback_t *pCallback);
TCC_FLAC_StreamDecoderState TCC_FLAC_stream_decoder_set_state(const TCC_FLAC_StreamDecoder *pstDecoder, TCC_FLAC_StreamDecoderState state);

TCAS_S32 FLAC_Decoder_GetSampleSize(TCC_FLAC_StreamDecoder *pstDecoder);
//TCAS_S32 FLAC_Decoder_GetDecodedSample(TCC_FLAC_StreamDecoder *pstDecoder);
TCASVoid WritePCM16_Stereo(TCAS_S16  *, TCAS_S16  *, const TCAS_S32 *, TCAS_U32);
TCASVoid WritePCM16_Mono(TCAS_S16  *, const TCAS_S32 *, TCAS_U32);
TCAS_S32 TCC_FLAC_stream_decoder_set_input(TCC_FLAC_StreamDecoder *pstDecoder, TCAS_S8 **buffer, TCAS_S32 *length);
TCAS_S32 TCC_FLAC_stream_decoder_get_input_state(TCC_FLAC_StreamDecoder *pstDecoder, TCAS_S8 **buffer, TCAS_S32 *length);

/* public routines */
TCC_FLAC_DecoderInfo *FLAC_Decoder_Init( flac_callback_t *pCallback, TCAS_U8 *pucExtra, TCAS_S32 iExtraLength, TCAS_U32 uiPcmFormat )
{
	TCC_FLAC_DecoderInfo *pstDecoderInfo;
	TCC_FLAC_StreamDecoderInitStatus init_status;

	TCAS_S32 initerr;
	TCAS_U8 *streaminfo_start;

	pstDecoderInfo = (TCC_FLAC_DecoderInfo *)pCallback->m_pfMalloc(sizeof(TCC_FLAC_DecoderInfo));

	pstDecoderInfo->m_ulSamplesProcessed = 0;
	pstDecoderInfo->m_uiFrameCounter = 0;

	pstDecoderInfo->m_ulTotalSamples = 0;
	pstDecoderInfo->m_iGotStreamInfo = false;
	pstDecoderInfo->m_uiBitsPerSample = 0;
	pstDecoderInfo->m_uiChannels = 0;
	pstDecoderInfo->m_uiSampleRate = 0;

	pstDecoderInfo->m_uiOutputPcmBits = uiPcmFormat;
//	pstDecoderInfo->m_iHasMd5Sum = false;
//	pstDecoderInfo->m_uiChannelMask = 0;
	pstDecoderInfo->m_pstStereamDecInfo = 0;

	pstDecoderInfo->m_pstStereamDecInfo = TCC_FLAC_stream_decoder_new(pCallback);
	if( pstDecoderInfo->m_pstStereamDecInfo == NULL )
	{
		//printf("ERROR creating the decoder instance\n");
		TC_FLAC_CloseCodec(pstDecoderInfo, pCallback);
		return NULL;
	}

	//pstDecoderInfo->m_pstStereamDecInfo->max_samplerate =  ( ( limit48khz == 1 ) ? 48000 : 96000 );

	streaminfo_start = NULL;

	/* parsing extradata (from parser) */
	if( pucExtra != NULL && iExtraLength >= 34)
	{
		if( (pucExtra[0] == 'f') && (pucExtra[1] == 'L') && (pucExtra[2] == 'a') && (pucExtra[3] == 'C') )
		{
			//full header
			streaminfo_start = &pucExtra[8];
			iExtraLength -= 8;
			//if( size < 8 + TCC_FLAC_STREAM_METADATA_STREAMINFO_LENGTH)
			//{	
				//printf("extra-data size %d uiBytes.\n", *size); 
			//	streaminfo_start = NULL;
			//}
			//printf("flac extradata format, full header streaminfo \n"); 
		}
		else
		{
			//STREAMINFO only
			//printf("flac extradata format, only streaminfo \n"); 
			streaminfo_start = &pucExtra[0];
		}
		//if(streaminfo_start != NULL)
		{
			if(TCC_FLAC_stream_decoder_parse_streaminfo(pstDecoderInfo->m_pstStereamDecInfo, streaminfo_start, iExtraLength) < 0 )
			{
				streaminfo_start = NULL;
			}
		}
	}

	if( streaminfo_start == NULL )
	{
		pstDecoderInfo->m_pstStereamDecInfo->m_uiMaxCh = TCC_FLAC_MAX_CHANNELS;
		pstDecoderInfo->m_pstStereamDecInfo->m_uiMaxBlock = TCC_FLAC_MAX_BLOCK_SIZE;
		//pstDecoderInfo->m_pstStereamDecInfo->max_block = ( ( limit48khz == 1 ) ? TCC_FLAC_SUBSET_MAX_BLOCK_SIZE_48000HZ : TCC_FLAC_MAX_BLOCK_SIZE );
	}
	else
	{
		if(pstDecoderInfo->m_pstStereamDecInfo->m_uiMaxCh > TCC_FLAC_MAX_CHANNELS)
		{
			pstDecoderInfo->m_pstStereamDecInfo->m_uiMaxCh = TCC_FLAC_MAX_CHANNELS;
		}
		if( pstDecoderInfo->m_pstStereamDecInfo->m_uiMaxBlock > TCC_FLAC_MAX_BLOCK_SIZE )
		{
			pstDecoderInfo->m_pstStereamDecInfo->m_uiMaxBlock = TCC_FLAC_MAX_BLOCK_SIZE;
		}

		pstDecoderInfo->m_iGotStreamInfo = true;

		pstDecoderInfo->m_uiBitsPerSample = pstDecoderInfo->m_pstStereamDecInfo->m_uiBitsPerSample;
		pstDecoderInfo->m_uiChannels = pstDecoderInfo->m_pstStereamDecInfo->m_uiChannels;
		pstDecoderInfo->m_uiSampleRate = pstDecoderInfo->m_pstStereamDecInfo->m_uiSampleRate;
		//pstDecoderInfo->m_uiBitsPerSample = pstDecoderInfo->m_pstStereamDecInfo->

		//printf("max-ch %d, max-block %d\n", pstDecoderInfo->m_pstStereamDecInfo->max_ch,  pstDecoderInfo->m_pstStereamDecInfo->max_block);
	}

	/* Protection Code */
	initerr = 0;//CalcCodecScale();
	pstDecoderInfo->m_pstStereamDecInfo->m_uiTCCxxxx = !initerr;

#ifdef MD5_CHECKING
	TCC_FLAC_stream_decoder_set_md5_checking(pstDecoderInfo->m_pstStereamDecInfo, false);
#endif

	if( FLAC_Decoder_Setbuffer(pstDecoderInfo->m_pstStereamDecInfo, pCallback) < 0 )
	{
		TC_FLAC_CloseCodec(pstDecoderInfo, pCallback);
		return NULL;
	}

	init_status = TCC_FLAC_stream_decoder_init(pstDecoderInfo->m_pstStereamDecInfo, TC_FLAC_PCM_Out, pstDecoderInfo);

	if(init_status != TCC_FLAC_STREAM_DECODER_INIT_STATUS_OK) 
	{
		//printf("ERROR initializing decoder");
		TC_FLAC_CloseCodec(pstDecoderInfo, pCallback);
		return NULL;
	}
	
	return pstDecoderInfo;
}

TCC_FLAC_DecoderState FLAC_Decoder_Frame( TCC_FLAC_DecoderInfo *pstDecoderInfo, TCAS_S32 **ppsChannel, TCAS_S8 **ppucInBuffer, TCAS_S32 *piLength )
{
	TCAS_S32 iResult;
	//TCC_FLAC_StreamDecoderState state;
	
	iResult = TCC_FLAC_stream_decoder_set_input( pstDecoderInfo->m_pstStereamDecInfo, ppucInBuffer, piLength );
	if( iResult < 0 )
	{
		return TCC_FLAC_DECODER_INPUT_BUFFER_ERROR;	//TCAS_ERROR_NULL_INSTANCE
	}

	//state = TCC_FLAC_stream_decoder_get_state(pstDecoderInfo->m_pstStereamDecInfo);
	//if( state != TCC_FLAC_STREAM_DECODER_SEARCH_FOR_FRAME_SYNC )
	//	return TCC_FLAC_DECODER_END_OF_STREAM;
	
	pstDecoderInfo->m_ppsChannel = ppsChannel;

	if(!TCC_FLAC_stream_decoder_process_single(pstDecoderInfo->m_pstStereamDecInfo)) 
	{
		TCC_FLAC_stream_decoder_get_input_state( pstDecoderInfo->m_pstStereamDecInfo, ppucInBuffer, piLength );
		//state = TCC_FLAC_stream_decoder_get_state( pstDecoderInfo->m_pstStereamDecInfo );		//v123
		TCC_FLAC_stream_decoder_set_state( pstDecoderInfo->m_pstStereamDecInfo, TCC_FLAC_STREAM_DECODER_SEARCH_FOR_FRAME_SYNC );
		return TCC_FLAC_DECODER_STREAM_ERROR;
	}

	// Input buffer update
	TCC_FLAC_stream_decoder_get_input_state( pstDecoderInfo->m_pstStereamDecInfo, ppucInBuffer, piLength );
	
	if( TCC_FLAC_stream_decoder_get_state(pstDecoderInfo->m_pstStereamDecInfo) > TCC_FLAC_STREAM_DECODER_END_OF_STREAM ) 
	{
		TCC_FLAC_stream_decoder_set_state( pstDecoderInfo->m_pstStereamDecInfo, TCC_FLAC_STREAM_DECODER_SEARCH_FOR_FRAME_SYNC );
		return TCC_FLAC_DECODER_STREAM_ERROR;
	}

	return TCC_FLAC_DECODER_ERROR_NONE;
}

TCAS_S32 FLAC_Decoder_Get_Samplerate( TCC_FLAC_DecoderInfo *pstDecoderInfo )
{
	if(pstDecoderInfo->m_pstStereamDecInfo) 
	{
		return (TCAS_S32)pstDecoderInfo->m_uiSampleRate;
	}
	return 0;
}

TCAS_S32 FLAC_Decoder_Reset( TCC_FLAC_DecoderInfo *pstDecoderInfo )
{
	if(pstDecoderInfo->m_pstStereamDecInfo) 
	{
		TCC_FLAC_stream_decoder_reset( pstDecoderInfo->m_pstStereamDecInfo );
	}
	return 0;
}

TCAS_S32 FLAC_Decoder_Get_Channels(TCC_FLAC_DecoderInfo *pstDecoderInfo)
{
	if(pstDecoderInfo->m_pstStereamDecInfo) 
	{
		return (TCAS_S32)pstDecoderInfo->m_uiChannels;
	}
	return 0;
}

#if 0
TCAS_S32 FLAC_Decoder_Get_TotalSamples(TCC_FLAC_DecoderInfo *pstDecoderInfo)
{
	if(pstDecoderInfo->m_pstStereamDecInfo) 
	{
		return pstDecoderInfo->m_ulTotalSamples;
	}
	return 0;
}

TCAS_S32 FLAC_Decoder_Get_SamplesProcessed(TCC_FLAC_DecoderInfo *pstDecoderInfo)
{
	if(pstDecoderInfo->m_pstStereamDecInfo) 
	{
		return pstDecoderInfo->m_ulSamplesProcessed;
	}
	return 0;
}
#endif

TCAS_S32 FLAC_Decoder_Get_Current_PCM_Size(TCC_FLAC_DecoderInfo *pstDecoderInfo)
{
	return FLAC_Decoder_GetSampleSize(pstDecoderInfo->m_pstStereamDecInfo);
}

#if 0
TCAS_S32 FLAC_Decoder_Get_DecodedSamples(TCC_FLAC_DecoderInfo *pstDecoderInfo)
{
	return FLAC_Decoder_GetDecodedSample(pstDecoderInfo->m_pstStereamDecInfo);
}
#endif

TCAS_S32 FLAC_Decoder_Set_PCMInfo(TCC_FLAC_DecoderInfo *pstDecoderInfo, TCAS_S32 iChOffset, TCAS_U32 uiDownmixMode)
{
	pstDecoderInfo->m_iPcmInterleavingFlag = iChOffset;
	pstDecoderInfo->m_uiDownMixMode = uiDownmixMode;
	return 0;
}

TCAS_S32 TC_FLAC_CloseCodec( TCC_FLAC_DecoderInfo *pstDecoderInfo, flac_callback_t *pCallback )
{
	if( pstDecoderInfo->m_pstStereamDecInfo ) 
	{
		TCC_FLAC_stream_decoder_delete( pstDecoderInfo->m_pstStereamDecInfo, pCallback );
	}

	pCallback->m_pfFree( pstDecoderInfo );
	
	return 1;
}

static TCASVoid tc_flacdec_pcm_16bit(TCAS_S32 *buffer, TCAS_S16 *outbuf, TCAS_S32 length, TCAS_S32 chOffset)
{
//	TCAS_S32 i;
	/*
	for (i = 0; i < length; i++) 
	{
		*outbuf = (TCAS_S16)buffer[i];
		outbuf += chOffset;
	}*/
	if(length & 3)
	{
		do
		{
			*outbuf = *buffer++;
			outbuf += chOffset;
			length--;
		}while(length & 3);

		if(length <= 0){
			return;
		}
	}
	do
	{
		TCAS_S16 iD0, iD1, iD2, iD3;
		iD0 = *buffer++; iD1 = *buffer++; iD2 = *buffer++; iD3 = *buffer++;
		*outbuf = iD0; outbuf += chOffset;
		*outbuf = iD1; outbuf += chOffset;
		*outbuf = iD2; outbuf += chOffset;
		*outbuf = iD3; outbuf += chOffset;
		length -= 4;
	}while(length);
}

static TCASVoid tc_flacdec_small16_to_16(TCAS_S32 *buffer, TCAS_S16 *outbuf, TCAS_S32 length, TCAS_S32 chOffset, TCAS_S32 bps)
{
	TCAS_S32 i;
	TCAS_S32 scale = (16 - bps);

	for(i = 0; i < length; i++)
	{
		*outbuf = (TCAS_S16)(buffer[i] << scale);
		outbuf += chOffset;
	}
}

static TCASVoid tc_flacdec_large16_to_16(TCAS_S32 *buffer, TCAS_S16 *outbuf, TCAS_S32 length, TCAS_S32 chOffset, TCAS_S32 bps)
{
#if 0
	TCAS_S32 i;
	TCAS_S32 scale = (bps - 16);

	for(i = 0; i < length; i++)
	{
#if 0
		TCAS_S32 iTmp;
		
		if(buffer[i] >= 0)
			iTmp = buffer[i]+(TCAS_U32)(1<<(scale-1));
		else
			iTmp = buffer[i]-(TCAS_U32)(1<<(scale-1));

		CLIP_2N(iTmp, bps-1);

		*outbuf = (TCAS_S16)(iTmp >> scale);
#else
		*outbuf = (TCAS_S16)(buffer[i] >> scale);
#endif
		outbuf += chOffset;
	}
#else

	TCAS_S32 scale = (bps - 16);

	if(length & 3)
	{
		do
		{
			*outbuf = (TCAS_S16)((*buffer++) >> scale);
			outbuf += chOffset;
			length--;
		}while(length & 3);

		if(length <= 0){
			return;
		}
	}
	do
	{
		TCAS_S32 iD0, iD1, iD2, iD3;
		iD0 = *buffer++; iD1 = *buffer++; iD2 = *buffer++; iD3 = *buffer++;
		*outbuf = (iD0 >> scale); outbuf += chOffset;
		*outbuf = (iD1 >> scale); outbuf += chOffset;
		*outbuf = (iD2 >> scale); outbuf += chOffset;
		*outbuf = (iD3 >> scale); outbuf += chOffset;
		length -= 4;
	}while(length);

#endif
}

// 24bit
static TCASVoid tc_flacdec_pcm_24bit(TCAS_S32 *buffer, TCAS_S32 *outbuf, TCAS_S32 length, TCAS_S32 chOffset)
{
//	TCAS_S32 i;

	if(length & 3)
	{
		do
		{
			*outbuf = *buffer++;
			outbuf += chOffset;
			length--;
		}while(length & 3);

		if(length <= 0){
			return;
		}
	}
	do
	{
		TCAS_S32 iD0, iD1, iD2, iD3;
		iD0 = *buffer++; iD1 = *buffer++; iD2 = *buffer++; iD3 = *buffer++;
		*outbuf = iD0; outbuf += chOffset;
		*outbuf = iD1; outbuf += chOffset;
		*outbuf = iD2; outbuf += chOffset;
		*outbuf = iD3; outbuf += chOffset;
		length -= 4;
	}while(length);
}

static TCASVoid tc_flacdec_small24_to_24(TCAS_S32 *buffer, TCAS_S32 *outbuf, TCAS_S32 length, TCAS_S32 chOffset, TCAS_S32 bps)
{
	TCAS_S32 i;
	TCAS_S32 scale = (24 - bps);

	for(i = 0; i < length; i++)
	{
		*outbuf = (TCAS_S32)(buffer[i] << scale);
		outbuf += chOffset;
	}
}

static TCASVoid tc_flacdec_large24_to_24(TCAS_S32 *buffer, TCAS_S32 *outbuf, TCAS_S32 length, TCAS_S32 chOffset, TCAS_S32 bps)
{
	TCAS_S32 i;
	TCAS_S32 scale = (bps - 24);

	for(i = 0; i < length; i++)
	{
		//*outbuf = (TCAS_S32)((buffer[i]+(TCAS_U32)(1<<(scale-1))) >> scale);
		*outbuf = (TCAS_S32)(buffer[i] >> scale);
		outbuf += chOffset;
	}
}

#define CLIP_2N(uiVal, uiBit ) {                               \
        if ((uiVal) >> 31 != (uiVal) >> (uiBit)){                \
            (uiVal) = ((uiVal) >> 31) ^ ((1 << (uiBit)) - 1);   \
		}  \
    }

#if defined (__arm) && defined (__ARMCC_VERSION)
static __inline TCAS_S32 SMULWb(TCAS_S32 coef, TCAS_S32 data) //smulwb 
{
	TCAS_S32 rN;
	__asm 
	{
		smulwb	rN,data,coef
	}
	return rN;
}
static __inline TCAS_S32 SMULWt(TCAS_S32 coef, TCAS_S32 data) //smulwb 
{
	TCAS_S32 rN;
	__asm 
	{
		smulwt	rN,data,coef
	}
	return rN;
}
static __inline TCAS_S32 SMLAWb(TCAS_S32 coef, TCAS_S32 data, TCAS_S32 acc )
{
	__asm 
	{
		smlawb	acc,data,coef,acc
	}
	return acc;
}

static __inline TCAS_S32 SMLAWt(TCAS_S32 coef, TCAS_S32 data, TCAS_S32 acc)
{
	__asm 
	{
		smlawt	acc,data,coef,acc
	}
	return acc;
}

#elif defined(__GNUC__) && defined(__arm__)
static inline TCAS_S32 SMULWb (const TCAS_S32 coef, const TCAS_S32 data)
{
  TCAS_S32 result ;
  __asm__ ("smulwb %0, %1, %2"
    : "=r" (result)
    : "r" (data), "r" (coef)) ;
  return result ;
}
static inline TCAS_S32 SMULWt (const TCAS_S32 coef, const TCAS_S32 data)
{
  TCAS_S32 result ;
  __asm__ ("smulwt %0, %1, %2"
    : "=r" (result)
    : "r" (data), "r" (coef)) ;
  return result ;
}

static inline TCAS_S32 SMLAWb (const TCAS_S32 coef, const TCAS_S32 data, TCAS_S32 acc)
{
	TCAS_S32 result ;
	__asm__ ("smlawb %0, %1, %2, %3"
		: "=r"(result)
		: "r"(data), "r"(coef), "r"(acc));
	return result ;
}

static inline TCAS_S32 SMLAWt (const TCAS_S32 coef, const TCAS_S32 data, TCAS_S32 acc)
{
	TCAS_S32 result ;
	__asm__ ("smlawt %0, %1, %2, %3"
		: "=r"(result)
		: "r"(data), "r"(coef), "r"(acc));
	return result ;
}

#else
static __inline TCAS_S32 SMULWb(const TCAS_S32 coef, const TCAS_S32 data) //smulwb 
{
	TCAS_S32 rN;
	rN =  (TCAS_S32)(((TCAS_S64)(data) * (TCAS_S32)((coef<<16)>>16)) >> 16 );
	return rN;
}
static __inline TCAS_S32 SMULWt(const TCAS_S32 coef, const TCAS_S32 data) //smulwt 
{
	TCAS_S32 rN;
	rN =  (TCAS_S32)(((TCAS_S64)(data) * (TCAS_S32)(coef>>16)) >> 16 );
	return rN;
}

static __inline TCAS_S32 SMLAWb(TCAS_S32 coef, TCAS_S32 data, TCAS_S32 acc )
{
	acc +=  (TCAS_S32)(((TCAS_S64)(data) * (TCAS_S32)((coef<<16)>>16)) >> 16 );
	return acc;
}

static __inline TCAS_S32 SMLAWt(TCAS_S32 coef, TCAS_S32 data, TCAS_S32 acc )
{
	acc +=  (TCAS_S32)(((TCAS_S64)(data) * (TCAS_S32)(coef>>16)) >> 16 );;
	return acc;
}
#endif

// 7.1 ch --> stereo
// 8 channels: front left, front right, front center, LFE, back left, back right, side left, side right
TCASVoid TCC_FLAC_DownMix_8CH_ToStereo(TCAS_S32 **pChannel, TCAS_U32 frameLength, TCAS_S32 nch, TCAS_S32 bps)
{
	{
		TCAS_S32 *pCenter, *pLeftFront, *pRightFront, *pLeftBack, *pRigthBack, *pSideLeft, *pSideRight;
		TCAS_U32 i;

		pLeftFront = pChannel[0];
		pRightFront = pChannel[1];
		pCenter = pChannel[2];
		pLeftBack = pChannel[4];
		pRigthBack = pChannel[5];
		pSideLeft = pChannel[6];
		pSideRight = pChannel[7];

		for(i = 0; i < frameLength; i++)
		{
			TCAS_S32 c, l, r;

			// 0x5A9E (-3db), 0x47FB (-5db)
			// 0x4027 (-6db), 0x2D6B (-9db)
			c = SMULWb (0x5A9E47FB, pCenter[i]);		// c * -5dB

			l = SMLAWt (0x5A9E47FB, pLeftFront[i], c);	// l * -3dB
			r = SMLAWt (0x5A9E47FB, pRightFront[i], c);	// r * -3dB

			l = SMLAWt (0x40272D6B, pLeftBack[i], l);	// ls * -6dB
			r = SMLAWb (0x40272D6B, pLeftBack[i], r);	// ls * -9dB

			l = SMLAWb (0x40272D6B, pRigthBack[i], l);	// rs * -9dB
			r = SMLAWt (0x40272D6B, pRigthBack[i], r);	// rs * -6dB

			l = SMLAWb (0x40272D6B, pSideLeft[i], l);	// rs * -9dB
			r = SMLAWb (0x40272D6B, pSideRight[i], r);	// rs * -9dB

			l = l << 1;
			r = r << 1;

			CLIP_2N(l, bps-1);
			CLIP_2N(r, bps-1);

			pLeftFront[i]  = l;
			pRightFront[i] = r;
		}
	}
}

// 6.1 ch --> stereo
// 7 channels: front left, front right, front center, LFE, back center, side left, side right
TCASVoid TCC_FLAC_DownMix_7CH_ToStereo(TCAS_S32 **pChannel, TCAS_U32 frameLength, TCAS_S32 nch, TCAS_S32 bps)
{
	TCAS_S32 *pCenter, *pLeftFront, *pRightFront, *pBackCenter, *pLeftSide, *pRigthSide;
	TCAS_U32 i;
	// front 3 channels
	pLeftFront = pChannel[0];
	pRightFront = pChannel[1];
	pCenter = pChannel[2];
	
	// back 3 channels
	pBackCenter = pChannel[4];
	pLeftSide = pChannel[5];
	pRigthSide = pChannel[6];

	for(i = 0; i < frameLength; i++)
	{
		TCAS_S32 c, l, r;
		TCAS_S32 bc, sl, sr;

		// 0x5A9E (-3db), 0x47FB (-5db)
		// 0x4027 (-6db), 0x2D6B (-9db)
		c = SMULWb (0x5A9E47FB, pCenter[i]);		// c * -5dB
		l = SMLAWt (0x5A9E47FB, pLeftFront[i], c);	// l * -3dB
		r = SMLAWt (0x5A9E47FB, pRightFront[i], c);	// r * -3dB

		bc = SMULWb (0x40272D6B, pBackCenter[i]);		// c * -9dB
		sl = SMLAWt (0x40272D6B, pLeftSide[i], bc);		// ls * -6dB
		sr = SMLAWt (0x40272D6B, pRigthSide[i], bc);	// rs * -6dB

		l = (l << 1) + (sl << 1);
		r = (r << 1) + (sr << 1);

		CLIP_2N(l, bps-1);
		CLIP_2N(r, bps-1);

		pLeftFront[i]  = l;
		pRightFront[i] = r;
	}
}

TCASVoid TCC_FLAC_DownMix_5_6CH_ToStereo(TCAS_S32 **pChannel, TCAS_U32 frameLength, TCAS_S32 nch, TCAS_S32 bps)
{
#ifdef ARM_OPT
	if (bps == 16)
		tcc_flac_downmix16 (pChannel, frameLength, nch);
	else if (bps == 24)
		tcc_flac_downmix24 (pChannel, frameLength, nch);
	else
#endif
	{
		TCAS_S32 *pCenter, *pLeftFront, *pRightFront, *pLeftBack, *pRigthBack;
		TCAS_U32 i;

		pLeftFront = pChannel[0];
		pRightFront = pChannel[1];
		pCenter = pChannel[2];
		if(nch == 5)
		{
			pLeftBack = pChannel[3];
			pRigthBack = pChannel[4];
		}
		else
		{
			pLeftBack = pChannel[4];
			pRigthBack = pChannel[5];
		}

		for(i = 0; i < frameLength; i++)
		{
			TCAS_S32 c, l, r;

			// 0x5A9E (-3db), 0x47FB (-5db)
			// 0x4027 (-6db), 0x2D6B (-9db)
			c = SMULWb (0x5A9E47FB, pCenter[i]);		// c * -5dB

			l = SMLAWt (0x5A9E47FB, pLeftFront[i], c);	// l * -3dB
			r = SMLAWt (0x5A9E47FB, pRightFront[i], c);	// r * -3dB

			l = SMLAWt (0x40272D6B, pLeftBack[i], l);	// ls * -6dB
			r = SMLAWb (0x40272D6B, pLeftBack[i], r);	// ls * -9dB

			l = SMLAWb (0x40272D6B, pRigthBack[i], l);	// rs * -9dB
			r = SMLAWt (0x40272D6B, pRigthBack[i], r);	// rs * -6dB

			l = l << 1;
			r = r << 1;

			CLIP_2N(l, bps-1);
			CLIP_2N(r, bps-1);

			pLeftFront[i]  = l;
			pRightFront[i] = r;
		}
	}
}

TCASVoid TCC_FLAC_DownMix_3CH_ToStereo(TCAS_S32 **pChannel, TCAS_U32 frameLength, TCAS_S32 bps)
{
	TCAS_S32 *pCenter, *pLeftFront, *pRightFront;
	TCAS_U32 i;

	pLeftFront = pChannel[0];
	pRightFront = pChannel[1];
	pCenter = pChannel[2];

	for(i = 0; i < frameLength; i++)
	{
		TCAS_S32 common, dataL, dataR;
		common = pCenter[i];
		
		dataL = common + pLeftFront[i];
		dataR = common + pRightFront[i];

		CLIP_2N(dataL, bps-1);
		CLIP_2N(dataR, bps-1);
		pLeftFront[i]  = dataL;
		pRightFront[i] = dataR;
	}
}

TCASVoid TCC_FLAC_DownMix_4CH_ToStereo(TCAS_S32 **pChannel, TCAS_U32 frameLength, TCAS_S32 bps)
{
	TCAS_S32 *pLeftFront, *pRightFront, *pLeftBack, *pRigthBack;
	TCAS_U32 i;

	pLeftFront = pChannel[0];
	pRightFront = pChannel[1];
	pLeftBack = pChannel[2];
	pRigthBack = pChannel[3];

	for(i = 0; i < frameLength; i++)
	{
		TCAS_S32 dataL, dataR;
		
		dataL = pLeftFront[i] + pLeftBack[i];
		dataR = pRightFront[i] + pRigthBack[i];

		CLIP_2N(dataL, bps-1);
		CLIP_2N(dataR, bps-1);
		pLeftFront[i]  = dataL;
		pRightFront[i] = dataR;
	}
}

static TCC_FLAC_StreamDecoderWriteStatus TC_FLAC_PCM_Out( const TCC_FLAC_StreamDecoder *pstDecoder, 
													   const TCC_FLAC_Frame *pstFrame, 
													   TCAS_S32 *buffer[], 
													   TCASVoid *client_data
)
{
	TCC_FLAC_DecoderInfo *pstDecoderInfo = (TCC_FLAC_DecoderInfo*)client_data;
	const TCAS_U32 bps = pstFrame->m_stHeader.m_uiBitsPerSample;
	TCAS_U32 channels = pstFrame->m_stHeader.m_uiChannels;

	TCAS_U32 channel;

	TCAS_S16  *L16buffer;
	TCAS_S16  *R16buffer;
	TCAS_U32 wide_samples = pstFrame->m_stHeader.m_uiBlockSize;
	
	L16buffer = (TCAS_S16  *)pstDecoderInfo->m_ppsChannel[0];
	R16buffer = (TCAS_S16  *)pstDecoderInfo->m_ppsChannel[1];

	if( pstDecoderInfo->m_uiBitsPerSample ) 
	{
		if( bps != pstDecoderInfo->m_uiBitsPerSample ) 
		{
			pstDecoderInfo->m_uiBitsPerSample = bps;
#if 0
			if( pstDecoderInfo->m_iGotStreamInfo )
				printf("ERROR, bits-per-sample is %u in frame but %u in STREAMINFO\n", bps, pstDecoderInfo->m_uiBitsPerSample);
			else
				printf("ERROR, bits-per-sample is %u in this frame but %u in previous frames\n", bps, pstDecoderInfo->m_uiBitsPerSample);
#endif
		}
	}
	else
	{
		/* must not have gotten STREAMINFO, save the bps from the frame header */
		TCC_FLAC_ASSERT(!pstDecoderInfo->m_iGotStreamInfo);
		pstDecoderInfo->m_uiBitsPerSample = bps;
	}
	
	if( pstDecoderInfo->m_uiChannels ) 
	{
		if( channels != pstDecoderInfo->m_uiChannels ) 
		{
			pstDecoderInfo->m_uiChannels = channels;
#if 0
			if( pstDecoderInfo->m_iGotStreamInfo )
				printf("ERROR, channels is %u in frame but %u in STREAMINFO\n", channels, pstDecoderInfo->m_uiChannels);
			else
				printf("ERROR, channels is %u in this frame but %u in previous frames\n", channels, pstDecoderInfo->m_uiChannels);
#endif
		}
	}
	else 
	{
		/* must not have gotten STREAMINFO, save the #channels from the frame header */
		TCC_FLAC_ASSERT( !pstDecoderInfo->m_iGotStreamInfo );
		pstDecoderInfo->m_uiChannels = channels;
	}

	if( pstDecoderInfo->m_uiSampleRate ) 
	{
		if( pstFrame->m_stHeader.m_uiSampleRate != pstDecoderInfo->m_uiSampleRate ) 
		{
			pstDecoderInfo->m_uiSampleRate = pstFrame->m_stHeader.m_uiSampleRate;
#if 0
			if( pstDecoderInfo->m_iGotStreamInfo )
				printf("ERROR, sample rate is %u in frame but %u in STREAMINFO\n", frame->header.m_uiSampleRate, pstDecoderInfo->m_uiSampleRate);
			else
				printf("ERROR, sample rate is %u in this frame but %u in previous frames\n", frame->header.m_uiSampleRate, pstDecoderInfo->m_uiSampleRate);
#endif
		}
	}
	else 
	{
		/* must not have gotten STREAMINFO, save the sample rate from the frame header */
		TCC_FLAC_ASSERT(!pstDecoderInfo->m_iGotStreamInfo);
		pstDecoderInfo->m_uiSampleRate = pstFrame->m_stHeader.m_uiSampleRate;
	}

	if(wide_samples > 0) 
	{
			TCAS_U32 channel_offset;
			pstDecoderInfo->m_ulSamplesProcessed += wide_samples;
			pstDecoderInfo->m_uiFrameCounter++;

			if(pstDecoderInfo->m_uiDownMixMode)
			{
				switch (channels)
				{
				case 3:
					TCC_FLAC_DownMix_3CH_ToStereo(buffer, wide_samples, bps);
					break;
				case 4:
					TCC_FLAC_DownMix_4CH_ToStereo(buffer, wide_samples, bps);
					break;
				case 5:
				case 6:
					TCC_FLAC_DownMix_5_6CH_ToStereo(buffer, wide_samples, channels, bps);
					break;
				case 7:
					TCC_FLAC_DownMix_7CH_ToStereo (buffer, wide_samples, channels, bps);
					break;
				case 8:
					TCC_FLAC_DownMix_8CH_ToStereo (buffer, wide_samples, channels, bps); // not yet tested
					break;
				}
				if (channels > 2)
					channels = 2;
			}
			
			channel_offset = channels;
			if(pstDecoderInfo->m_iPcmInterleavingFlag == 0)
			{
				channel_offset = 1;
			}
#if 1
			if(pstDecoderInfo->m_uiOutputPcmBits == 16)
			{
				if(bps == 16) 
				{
					for(channel = 0; channel < channels; channel++)
					{
						if(pstDecoderInfo->m_ppsChannel[channel])
							tc_flacdec_pcm_16bit(buffer[channel], (TCAS_S16 *)pstDecoderInfo->m_ppsChannel[channel], wide_samples, channel_offset);
					}
				}
				else if(bps < 16)
				{
					for(channel = 0; channel < channels; channel++)
					{
						if(pstDecoderInfo->m_ppsChannel[channel])
							tc_flacdec_small16_to_16(buffer[channel], (TCAS_S16 *)pstDecoderInfo->m_ppsChannel[channel], wide_samples, channel_offset, bps);
					}
				}
				else
				{
					for(channel = 0; channel < channels; channel++)
					{
						if(pstDecoderInfo->m_ppsChannel[channel])
							tc_flacdec_large16_to_16(buffer[channel], (TCAS_S16 *)pstDecoderInfo->m_ppsChannel[channel], wide_samples, channel_offset, bps);
					}
				}
			}
			else // 24bit
			{
				if(bps == 24) 
				{
					for(channel = 0; channel < channels; channel++)
					{
						if(pstDecoderInfo->m_ppsChannel[channel])
							tc_flacdec_pcm_24bit(buffer[channel], pstDecoderInfo->m_ppsChannel[channel], wide_samples, channel_offset);
					}
				}
				else if(bps < 24)
				{
					for(channel = 0; channel < channels; channel++)
					{
						if(pstDecoderInfo->m_ppsChannel[channel])
							tc_flacdec_small24_to_24(buffer[channel], pstDecoderInfo->m_ppsChannel[channel], wide_samples, channel_offset, bps);
					}
				}
				else
				{
					for(channel = 0; channel < channels; channel++)
					{
						if(pstDecoderInfo->m_ppsChannel[channel])
							tc_flacdec_large24_to_24(buffer[channel], pstDecoderInfo->m_ppsChannel[channel], wide_samples, channel_offset, bps);
					}
				}
			}
#else
			if(channels==2)
			{
				if(bps == 16) 
				{ //v119
#ifdef ARM_OPT
					WritePCM16_Stereo(L16buffer,R16buffer,buffer,wide_samples);
#else		
					for(wide_sample = 0; wide_sample < wide_samples; wide_sample++)
					{
						L16buffer[wide_sample] = (TCAS_S16)(buffer[0][wide_sample]);
						R16buffer[wide_sample] = (TCAS_S16)(buffer[1][wide_sample]);
					}
#endif			
				}
				else if(bps < 16)
				{
					TCAS_S32 scale = (16 - bps);
					for(wide_sample = 0; wide_sample < wide_samples; wide_sample++)	
					{
						L16buffer[wide_sample] = (TCAS_S16)(buffer[0][wide_sample] << scale);
						R16buffer[wide_sample] = (TCAS_S16)(buffer[1][wide_sample] << scale);
					}
				}
				else
				{	//24bit PCM //v119
					TCAS_S32 scale = (bps - 16);
					for(wide_sample = 0; wide_sample < wide_samples; wide_sample++)
					{
						L16buffer[wide_sample] = (TCAS_S16)((buffer[0][wide_sample]+(TCAS_U32)(1<<(scale-1))) >> scale);
						R16buffer[wide_sample] = (TCAS_S16)((buffer[1][wide_sample]+(TCAS_U32)(1<<(scale-1))) >> scale);
					}			
				}
			}
			else if(channels==1)
			{
				if(bps == 16) 
				{ //v119
#ifdef ARM_OPT	
					WritePCM16_Mono(L16buffer,buffer,wide_samples);
#else		
					for(wide_sample = 0; wide_sample < wide_samples; wide_sample++)
							L16buffer[wide_sample] = (TCAS_S16)(buffer[0][wide_sample]);
#endif					
				}
				else if(bps < 16)
				{
					TCAS_S32 scale = (16 - bps);
					for(wide_sample = 0; wide_sample < wide_samples; wide_sample++)
						L16buffer[wide_sample] = (TCAS_S16)(buffer[0][wide_sample] << scale);
				}
				else
				{	//24bit PCM //v119
					TCAS_S32 scale = (bps - 16);
					for(wide_sample = 0; wide_sample < wide_samples; wide_sample++)
						L16buffer[wide_sample] = (TCAS_S16)((buffer[0][wide_sample]+(TCAS_U32)(1<<(scale-1))) >> scale);
				}
			}
			else
			{
				if(bps == 16) 
				{ 
					for(channel = 0; channel < channels; channel++, sample++)
					{
						for(sample = wide_sample = 0; wide_sample < wide_samples; wide_sample++)
						{
							pstDecoderInfo->pChannel[channel][wide_sample] = (TCAS_S16)(buffer[channel][wide_sample]);
						}
					}
				}
				else if(bps < 16)
				{
					TCAS_S32 scale = (16 - bps);
					for(channel = 0; channel < channels; channel++, sample++)
					{
						for(sample = wide_sample = 0; wide_sample < wide_samples; wide_sample++)
						{
							pstDecoderInfo->pChannel[channel][wide_sample] = (TCAS_S16)(buffer[channel][wide_sample] << scale);
						}
					}
				}
				else
				{
					TCAS_S32 scale = (bps - 16);
					for(channel = 0; channel < channels; channel++, sample++)
					{
						for(sample = wide_sample = 0; wide_sample < wide_samples; wide_sample++)
						{
							pstDecoderInfo->pChannel[channel][wide_sample] = (TCAS_S16)((buffer[channel][wide_sample]+(TCAS_U32)(1<<(scale-1))) >> scale);
						}
					}
				}
			}
#endif
	}

				
	return TCC_FLAC_STREAM_DECODER_WRITE_STATUS_CONTINUE;
}
