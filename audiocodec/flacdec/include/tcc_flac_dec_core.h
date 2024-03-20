/* libFLAC - Free Lossless Audio Codec library
 * Copyright (C) 2000-2009  Josh Coalson
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

#ifndef TCC_FLAC_DEC_CORE_H_
#define TCC_FLAC_DEC_CORE_H_

typedef TCASVoid *FLAC_Decoder_Handle;

#define FLAC_CHUNK_SIZE			(77580)

typedef struct
{
	FLAC_Decoder_Handle		hFLACDEC;
	TCC_FLAC_DecoderState	status;
	
	TCASVoid				*pInstance;
	TCAS_S32    			*pFLAC_CH[TCAS_MAX_CHANNEL];

	flac_callback_t			m_sCallbackInfo;

	TCAS_S8					*m_pcOpenFileName;

	TCAS_U64				fileSize;
	TCAS_S32				TotalSamples;
	TCC_FLAC_Channel_Type	nChannelType;

	TCAS_U32				ulFrameSize;
	TCAS_U32				usValid;
	TCAS_U32				usSampleRate;
	TCAS_U32				ucChannels;
	TCAS_U32				ulLength;
	TCAS_U32				down_mix_mode;
	TCAS_U32				uiOutFormat;
} TCC_FLACDecoder;

FLAC_Decoder_Handle FLAC_Decoder_Init(flac_callback_t *pCallback, TCAS_U8 *extra, TCAS_S32 iExtraLength, TCAS_U32 uiPcmFormat);
//TCAS_S32 FLAC_Decoder_Frame(FLAC_Decoder_Handle hFLACDecoder, TCAS_S16 **pChannel);
TCAS_S32 FLAC_Decoder_Set_PCMInfo(FLAC_Decoder_Handle hFLACDecoder, TCAS_S32 iChOffset, TCAS_U32 uiDownmixMode);
TCAS_S32 FLAC_Decoder_Frame(FLAC_Decoder_Handle hFLACDecoder,  TCAS_S32 **pChannel, TCAS_S8 *buffer, TCAS_S32 *length);
TCAS_S32 FLAC_Decoder_Get_Current_PCM_Size(FLAC_Decoder_Handle hFLACDecoder);
TCAS_S32 FLAC_Decoder_Get_Samplerate(FLAC_Decoder_Handle hFLACDecoder);
TCAS_S32 FLAC_Decoder_Get_Channels(FLAC_Decoder_Handle hFLACDecoder);
//TCAS_S32 FLAC_Decoder_Get_TotalSamples(FLAC_Decoder_Handle hFLACDecoder);
TCAS_S32 FLAC_Decoder_Get_Current_PCM_Size(FLAC_Decoder_Handle hFLACDecoder);
TCAS_S32 TC_FLAC_CloseCodec(FLAC_Decoder_Handle hFLACDecoder, flac_callback_t *pCallback);
TCAS_S32 FLAC_Decoder_Reset( FLAC_Decoder_Handle hFLACDecoder );

#endif	//TCC_FLAC_DEC_CORE_H_
