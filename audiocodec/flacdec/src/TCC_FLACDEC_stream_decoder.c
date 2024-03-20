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
#include "tcc_flac_streamdec_pt.h"
#include "tcc_flac_bitreader.h"
#include "tcc_flac_crc.h"
#include "tcc_flac_fixed.h"
#include "tcc_flac_lpc.h"
#include "tcc_flac_md5.h"
#include "flac_define.h"

#ifdef _MSC_VER
#define TCC_FLAC_U64L(x) x
#else
#define TCC_FLAC_U64L(x) x##LLU
#endif

/***********************************************************************
 *
 * Private class method prototypes
 *
 ***********************************************************************/

static TCASVoid SetDefaults(TCC_FLAC_StreamDecoder *pstDecoder);
static TCAS_S32 FrameSync(TCC_FLAC_StreamDecoder *pstDecoder);
static TCAS_S32 ReadFrame(TCC_FLAC_StreamDecoder *pstDecoder, TCAS_S32 *piGotAFrame, TCAS_S32 iFullDecodingFlag);

static TCAS_S32 ReadFrameHeader(TCC_FLAC_StreamDecoder *pstDecoder);
static TCAS_S32 ReadSubFrame(TCC_FLAC_StreamDecoder *pstDecoder, TCAS_U32 uiChannel, TCAS_U32 uiBitsPerSample, TCAS_S32 iFullDecodingFlag);
static TCAS_S32 ReadSubFrameConstant(TCC_FLAC_StreamDecoder *pstDecoder, TCAS_U32 uiChannel, TCAS_U32 uiBitsPerSample, TCAS_S32 iFullDecodingFlag);
static TCAS_S32 ReadSubFrameFixed(TCC_FLAC_StreamDecoder *pstDecoder, TCAS_U32 uiChannel, TCAS_U32 uiBitsPerSample, const TCAS_U32 uiOrder, TCAS_S32 iFullDecodingFlag);
static TCAS_S32 ReadSubFrameLPC(TCC_FLAC_StreamDecoder *pstDecoder, TCAS_U32 uiChannel, TCAS_U32 uiBitsPerSample, const TCAS_U32 uiOrder, TCAS_S32 iFullDecodingFlag);
static TCAS_S32 ReadSubFrameVerbatim(TCC_FLAC_StreamDecoder *pstDecoder, TCAS_U32 uiChannel, TCAS_U32 uiBitsPerSample, TCAS_S32 iFullDecodingFlag);
static TCAS_S32 ReadResidualPartitionedRice(TCC_FLAC_StreamDecoder *pstDecoder, TCAS_U32 uiPredictorOrder, TCAS_U32 uiPartitionOrder, /*TCC_FLAC_EntropyCodingMethod_PartitionedRiceContents *partitioned_rice_contents,*/ TCAS_S32 *piResidual, TCAS_S32 is_extended);
static TCAS_S32 ReadZeroPadding(TCC_FLAC_StreamDecoder *pstDecoder);

static TCC_FLAC_StreamDecoderWriteStatus write_audio_frame_to_client_(TCC_FLAC_StreamDecoder *pstDecoder, const TCC_FLAC_Frame *pstFrame, const TCAS_S32 * const buffer[]);

TCASVoid Channel_Assign_LeftSide(TCAS_S32 *pLeft, TCAS_S32 *pRigth, TCAS_U32 blocksize);
TCASVoid Channel_Assign_RightSide(TCAS_S32 *pLeft, TCAS_S32 *pRigth, TCAS_U32 n);
TCASVoid Channel_Assign_MidSide(TCAS_S32 *pLeft, TCAS_S32 *pRigth, TCAS_U32 n);
TCAS_S32 TCC_FLAC_MD5Accumulate_alloc(struct TCC_FLAC_MD5Context *ctx);

/***********************************************************************
 *
 * Private class data
 *
 ***********************************************************************/

typedef struct TCC_FLAC_StreamDecoderPrivate 
{
	TCC_FLAC_StreamDecoderWriteCallback m_pfWriteCallback;

	/* generic 32-bit datapath: */
	TCASVoid (*m_pfLpcRestoreSignal)(const TCAS_S32 piResidual[], TCAS_U32 data_len, const TCAS_S32 qlp_coeff[], TCAS_U32 uiOrder, TCAS_S32 lp_quantization, TCAS_S32 data[]);
	/* generic 64-bit datapath: */
	TCASVoid (*m_pfLpcRestoreSignal64bit)(const TCAS_S32 piResidual[], TCAS_U32 data_len, const TCAS_S32 qlp_coeff[], TCAS_U32 uiOrder, TCAS_S32 lp_quantization, TCAS_S32 data[]);
	/* for use when the signal is <= 16 bits-per-sample, or <= 15 bits-per-sample on a side channel (which requires 1 extra bit): */
	TCASVoid (*m_pfLpcRestoreSignal16bit)(const TCAS_S32 piResidual[], TCAS_U32 data_len, const TCAS_S32 qlp_coeff[], TCAS_U32 uiOrder, TCAS_S32 lp_quantization, TCAS_S32 data[]);
	/* for use when the signal is <= 16 bits-per-sample, or <= 15 bits-per-sample on a side channel (which requires 1 extra bit), AND order <= 8: */
	TCASVoid (*m_pfLpcRestoreSignal16bitOrder8)(const TCAS_S32 piResidual[], TCAS_U32 data_len, const TCAS_S32 qlp_coeff[], TCAS_U32 uiOrder, TCAS_S32 lp_quantization, TCAS_S32 data[]);
	TCAS_S32 (*m_pfReadRiceSignedBlock)( TCC_FLACDEC_BitParser *pstBitParser, TCAS_S32 piValue[], TCAS_U32 uiNumValues, TCAS_U32 uiParameter );
	TCASVoid (*m_pfLpcRestoreSignal_org)(const TCAS_S32 piResidual[], TCAS_U32 data_len, const TCAS_S32 qlp_coeff[], TCAS_U32 uiOrder, TCAS_S32 lp_quantization, TCAS_S32 data[]);
	
	TCASVoid				*m_pvClientDataPrivate;
	TCC_FLACDEC_BitParser	*m_pstInputBit;
	TCAS_S32				*m_piOutput[TCC_FLAC_MAX_CHANNELS];
	TCAS_S32				*m_piResidual[TCC_FLAC_MAX_CHANNELS]; /* WATCHOUT: these are the aligned pointers; the real pointers that should be free()'d are residual_unaligned[] below */
	
	TCAS_U32				m_uiOutputCapacity;
	TCAS_U32				m_uiOutputChannels;

	//TCAS_U32				m_uiLastFrameNumber;
	//TCAS_U32				m_uiLastBlockSize;

	TCAS_U32				m_uiFixedBlockSize;
	TCAS_U32				m_uiNextFixedBlockSize;

	TCAS_U64				m_uiSamplesDecoded;

	TCAS_S32				m_iHasStreamInfo;
	
	TCC_FLAC_StreamInfo		m_stStreamInfo;
	
	TCC_FLAC_Frame			m_stFrame;
	TCAS_S32				m_iCached;				/* true if there is a byte in lookahead */
	
	TCAS_U8					m_ucHeaderWarmup[2];	/* contains the sync code and reserved bits */
	TCAS_U8					m_ucLookAhead;			/* temp storage when we need to look ahead one byte in the stream */

	/* unaligned (original) pointers to allocated data */
	TCAS_S32				*m_iResidualUnaligned[TCC_FLAC_MAX_CHANNELS];

#ifdef MD5_CHECKING
	TCAS_S32				m_iMd5CheckFlag; 
	struct TCC_FLAC_MD5Context md5context;
	TCAS_U8 computed_md5sum[16];
#endif	

} TCC_FLAC_StreamDecoderPrivate;

/***********************************************************************
 *
 * Class constructor/destructor
 *
 ***********************************************************************/
TCC_FLAC_StreamDecoder *TCC_FLAC_stream_decoder_new( flac_callback_t *pCallback )
{
	TCC_FLAC_StreamDecoder *pstDecoder;
	TCAS_U32 i;

	pstDecoder = (TCC_FLAC_StreamDecoder*)pCallback->m_pfMalloc(sizeof(TCC_FLAC_StreamDecoder));
	if( pstDecoder == 0 ) 
	{
		return 0;
	}
	pCallback->m_pfMemset(pstDecoder, 0, sizeof(TCC_FLAC_StreamDecoder));

	pstDecoder->m_pstProtected = (TCC_FLAC_StreamDecoderProtected*)pCallback->m_pfMalloc(sizeof(TCC_FLAC_StreamDecoderProtected));
	if(pstDecoder->m_pstProtected == 0) 
	{
		return 0;
	}
	pCallback->m_pfMemset(pstDecoder->m_pstProtected, 0, sizeof(TCC_FLAC_StreamDecoderProtected));

	pstDecoder->m_pstPrivate = (TCC_FLAC_StreamDecoderPrivate*)pCallback->m_pfMalloc(sizeof(TCC_FLAC_StreamDecoderPrivate));
	if(pstDecoder->m_pstPrivate == 0) 
	{
		return 0;
	}
	pCallback->m_pfMemset(pstDecoder->m_pstPrivate, 0, sizeof(TCC_FLAC_StreamDecoderPrivate));

	pstDecoder->m_pstPrivate->m_pstInputBit = TCC_FLACDEC_CreateBitParser(pCallback);
	if(pstDecoder->m_pstPrivate->m_pstInputBit == 0) 
	{
		return 0;
	}

	for(i = 0; i < TCC_FLAC_MAX_CHANNELS; i++) 
	{
		pstDecoder->m_pstPrivate->m_piOutput[i] = 0;
		pstDecoder->m_pstPrivate->m_iResidualUnaligned[i] = pstDecoder->m_pstPrivate->m_piResidual[i] = 0;
	}

	pstDecoder->m_pstPrivate->m_uiOutputCapacity = 0;
	pstDecoder->m_pstPrivate->m_uiOutputChannels = 0;

	pstDecoder->m_pCallback = pCallback;

	SetDefaults(pstDecoder);

	pstDecoder->m_pstProtected->m_eStateProtected = TCC_FLAC_STREAM_DECODER_UNINITIALIZED;
	
	return pstDecoder;
}

TCAS_S32 TCC_FLAC_stream_decoder_parse_streaminfo(TCC_FLAC_StreamDecoder *pstDecoder, TCAS_S8 *pExtraData, TCAS_S32 length)
{

	TCC_FLAC_StreamInfo *pStreamInfo;
	TCC_FLACDEC_InitBitParser(pstDecoder->m_pstPrivate->m_pstInputBit, pstDecoder );
	TCC_FLACDEC_SetBitBuffer(pstDecoder->m_pstPrivate->m_pstInputBit, &pExtraData, &length);

	pStreamInfo = &pstDecoder->m_pstPrivate->m_stStreamInfo;

	pstDecoder->m_pstPrivate->m_iHasStreamInfo = false;

	if (!TCC_FLACDEC_ReadU32(pstDecoder->m_pstPrivate->m_pstInputBit, &pStreamInfo->m_uiMinBlockSize, TCC_FLAC_STREAM_METADATA_STREAMINFO_MIN_BLOCK_SIZE_LEN ))
	{
		return -1;
	}
	if (!TCC_FLACDEC_ReadU32(pstDecoder->m_pstPrivate->m_pstInputBit, &pStreamInfo->m_uiMaxBlockSize, TCC_FLAC_STREAM_METADATA_STREAMINFO_MAX_BLOCK_SIZE_LEN ))
	{
		return -1;
	}
	if (!TCC_FLACDEC_ReadU32(pstDecoder->m_pstPrivate->m_pstInputBit, &pStreamInfo->m_uiMinFrameSize, TCC_FLAC_STREAM_METADATA_STREAMINFO_MIN_FRAME_SIZE_LEN ))
	{
		return -1;
	}
	if (!TCC_FLACDEC_ReadU32(pstDecoder->m_pstPrivate->m_pstInputBit, &pStreamInfo->m_uiMaxFrameSize, TCC_FLAC_STREAM_METADATA_STREAMINFO_MAX_FRAME_SIZE_LEN ))
	{
		return -1;
	}
	if (!TCC_FLACDEC_ReadU32(pstDecoder->m_pstPrivate->m_pstInputBit, &pStreamInfo->m_uiSampleRate, TCC_FLAC_STREAM_METADATA_STREAMINFO_SAMPLE_RATE_LEN ))
	{
		return -1;
	}
	if (!TCC_FLACDEC_ReadU32(pstDecoder->m_pstPrivate->m_pstInputBit, &pStreamInfo->m_uiChannels, TCC_FLAC_STREAM_METADATA_STREAMINFO_CHANNELS_LEN ))
	{
		return -1;
	}
	pStreamInfo->m_uiChannels += 1;
	if (!TCC_FLACDEC_ReadU32(pstDecoder->m_pstPrivate->m_pstInputBit, &pStreamInfo->m_uiBitsPerSample, TCC_FLAC_STREAM_METADATA_STREAMINFO_BITS_PER_SAMPLE_LEN ))
	{
		return -1;
	}
	pStreamInfo->m_uiBitsPerSample += 1;
	if (!TCC_FLACDEC_ReadU64(pstDecoder->m_pstPrivate->m_pstInputBit, &pStreamInfo->m_ulTotalSamples, TCC_FLAC_STREAM_METADATA_STREAMINFO_TOTAL_SAMPLES_LEN ))
	{
		return -1;
	}
	
	//pStreamInfo->m_ucMd5Sum

	if (pStreamInfo->m_uiMinBlockSize > pStreamInfo->m_uiMaxBlockSize || pStreamInfo->m_uiMinFrameSize > pStreamInfo->m_uiMaxFrameSize)
	{
		return -1;
	}
	
	if( pStreamInfo->m_uiChannels > TCC_FLAC_MAX_CHANNELS )
	{
		return -1;
	}

	if( pStreamInfo->m_uiSampleRate < 8000 || pStreamInfo->m_uiSampleRate > 192000 )
	{
		return -1;
	}

	if(pStreamInfo->m_uiMaxBlockSize > TCC_FLAC_MAX_BLOCK_SIZE)
	{
		return -1;
	}

	pstDecoder->m_uiMaxCh = pStreamInfo->m_uiChannels;
	pstDecoder->m_uiMaxBlock = pStreamInfo->m_uiMaxBlockSize;

	pstDecoder->m_pstPrivate->m_iHasStreamInfo = true;

	pstDecoder->m_uiBitsPerSample = pStreamInfo->m_uiBitsPerSample;
	pstDecoder->m_uiChannels = pStreamInfo->m_uiChannels;
	pstDecoder->m_uiSampleRate = pStreamInfo->m_uiSampleRate;


	TCC_FLACDEC_ResetBitParser(pstDecoder->m_pstPrivate->m_pstInputBit);

	return 0;
}

TCASVoid TCC_FLAC_stream_decoder_delete( TCC_FLAC_StreamDecoder *pstDecoder, flac_callback_t *pCallback )
{
	TCAS_U32 i;

	//TCC_FLAC_ASSERT(0 != pstDecoder);
	//TCC_FLAC_ASSERT(0 != pstDecoder->m_pstProtected);
	//TCC_FLAC_ASSERT(0 != pstDecoder->m_pstPrivate);
	//TCC_FLAC_ASSERT(0 != pstDecoder->m_pstPrivate->m_pstInputBit);
    if (pstDecoder == 0) {
        return;
    }

    if (pstDecoder->m_pstPrivate == 0) {
        return;
    }

	for(i = 0; i < TCC_FLAC_MAX_CHANNELS; i++) 
	{
		if(pstDecoder->m_pstPrivate->m_piResidual[i])
		{
			pCallback->m_pfFree(pstDecoder->m_pstPrivate->m_piResidual[i]);
		}
	}
	
	for(i = 0; i < TCC_FLAC_MAX_CHANNELS; i++) 
	{
		if(pstDecoder->m_pstPrivate->m_piOutput[i])
		{
			pCallback->m_pfFree(pstDecoder->m_pstPrivate->m_piOutput[i]);
		}
	}

	TCC_FLACDEC_DestroyBitParser(pstDecoder->m_pstPrivate->m_pstInputBit, pCallback);
	
    pCallback->m_pfFree(pstDecoder->m_pstPrivate);

	if(pstDecoder->m_pstProtected)
	{
		pCallback->m_pfFree(pstDecoder->m_pstProtected);
	}

	pCallback->m_pfFree(pstDecoder);
}

/***********************************************************************
 *
 * Public class methods
 *
 ***********************************************************************/

TCC_FLAC_StreamDecoderInitStatus TCC_FLAC_stream_decoder_init(
	TCC_FLAC_StreamDecoder *pstDecoder,
	TCC_FLAC_StreamDecoderWriteCallback write_callback,
	TCASVoid *client_data
)
{
	TCC_FLAC_ASSERT(0 != pstDecoder);

	if( pstDecoder->m_pstProtected->m_eStateProtected != TCC_FLAC_STREAM_DECODER_UNINITIALIZED )
	{
		return TCC_FLAC_STREAM_DECODER_INIT_STATUS_ALREADY_INITIALIZED;
	}

	if( 0 == write_callback )
	{
		return pstDecoder->m_pstProtected->m_eStateProtected = TCC_FLAC_STREAM_DECODER_INIT_STATUS_INVALID_CALLBACKS;
	}

	/* first default to the non-asm routines */
#ifdef ARM_OPT	
	pstDecoder->m_pstPrivate->m_pfLpcRestoreSignal = TCC_FLAC_lpc_restore_signal_ARM;
	pstDecoder->m_pstPrivate->m_pfLpcRestoreSignal64bit = TCC_FLAC_lpc_restore_signal_wide_ARM;
	pstDecoder->m_pstPrivate->m_pfLpcRestoreSignal16bit = TCC_FLAC_lpc_restore_signal_ARM;
	pstDecoder->m_pstPrivate->m_pfLpcRestoreSignal16bitOrder8 = TCC_FLAC_lpc_restore_signal_ARM;
	pstDecoder->m_pstPrivate->m_pfReadRiceSignedBlock = TCC_FLACDEC_ReadRiceSignedBlock;
	pstDecoder->m_pstPrivate->m_pfLpcRestoreSignal_org = TCC_FLAC_lpc_restore_signal;
#else
	pstDecoder->m_pstPrivate->m_pfLpcRestoreSignal = TCC_FLAC_lpc_restore_signal;
	pstDecoder->m_pstPrivate->m_pfLpcRestoreSignal64bit = TCC_FLAC_lpc_restore_signal_wide;
	pstDecoder->m_pstPrivate->m_pfLpcRestoreSignal16bit = TCC_FLAC_lpc_restore_signal;
	pstDecoder->m_pstPrivate->m_pfLpcRestoreSignal16bitOrder8 = TCC_FLAC_lpc_restore_signal;
	pstDecoder->m_pstPrivate->m_pfReadRiceSignedBlock = TCC_FLACDEC_ReadRiceSignedBlock;
	pstDecoder->m_pstPrivate->m_pfLpcRestoreSignal_org = TCC_FLAC_lpc_restore_signal;
#endif

	if(!TCC_FLACDEC_InitBitParser(pstDecoder->m_pstPrivate->m_pstInputBit, pstDecoder )) 
	{
		pstDecoder->m_pstProtected->m_eStateProtected = TCC_FLAC_STREAM_DECODER_MEMORY_ALLOCATION_ERROR;
		return TCC_FLAC_STREAM_DECODER_INIT_STATUS_MEMORY_ALLOCATION_ERROR;
	}

	pstDecoder->m_pstPrivate->m_pfWriteCallback = write_callback;

	pstDecoder->m_pstPrivate->m_pvClientDataPrivate = client_data;
	//pstDecoder->m_pstPrivate->m_uiLastFrameNumber = 0;
	//pstDecoder->m_pstPrivate->m_uiLastBlockSize = 0;

	pstDecoder->m_pstPrivate->m_uiFixedBlockSize = 0;
	pstDecoder->m_pstPrivate->m_uiNextFixedBlockSize = 0;

	pstDecoder->m_pstPrivate->m_uiSamplesDecoded = 0;
	pstDecoder->m_pstPrivate->m_iCached = false;

#ifdef MD5_CHECKING
	pstDecoder->m_pstPrivate->m_iMd5CheckFlag = pstDecoder->m_pstProtected->m_iMd5CheckingProtected;
#endif
	
	TCC_FLAC_stream_decoder_reset(pstDecoder);

	return TCC_FLAC_STREAM_DECODER_INIT_STATUS_OK;
}


TCAS_S32 TCC_FLAC_stream_decoder_set_input( TCC_FLAC_StreamDecoder *pstDecoder, TCAS_S8 **buffer, TCAS_S32 *length )
{
	TCAS_S32 iResult;
	
	if( pstDecoder == 0 )
	{
		return -1;
	}
	
	if( !pstDecoder->m_uiTCCxxxx )
	{
		*buffer += 1;
		*length -= 1;
	}

	iResult = TCC_FLACDEC_SetBitBuffer(pstDecoder->m_pstPrivate->m_pstInputBit, buffer, length);

	return iResult;
}

TCAS_S32 TCC_FLAC_stream_decoder_get_input_state(TCC_FLAC_StreamDecoder *pstDecoder, TCAS_S8 **buffer, TCAS_S32 *length)
{
	TCC_FLAC_ASSERT(0 != pstDecoder);
	
	TCC_FLACDEC_GetBitBufferState(pstDecoder->m_pstPrivate->m_pstInputBit, buffer, length);

	return true;
}

#ifdef MD5_CHECKING
TCAS_S32 TCC_FLAC_stream_decoder_set_md5_checking(TCC_FLAC_StreamDecoder *pstDecoder, TCAS_S32 value)
{
	TCC_FLAC_ASSERT(0 != pstDecoder);
	TCC_FLAC_ASSERT(0 != pstDecoder->m_pstProtected);
	if(pstDecoder->m_pstProtected->m_eStateProtected != TCC_FLAC_STREAM_DECODER_UNINITIALIZED) {
		return false;
	}
	pstDecoder->m_pstProtected->m_iMd5CheckingProtected = value;
	return true;
}
#endif

TCC_FLAC_StreamDecoderState TCC_FLAC_stream_decoder_get_state(const TCC_FLAC_StreamDecoder *pstDecoder)
{
	TCC_FLAC_ASSERT(0 != pstDecoder);
	TCC_FLAC_ASSERT(0 != pstDecoder->m_pstProtected);
	return pstDecoder->m_pstProtected->m_eStateProtected;
}

TCC_FLAC_StreamDecoderState TCC_FLAC_stream_decoder_set_state(const TCC_FLAC_StreamDecoder *pstDecoder, TCC_FLAC_StreamDecoderState state)
{
	TCC_FLAC_ASSERT(0 != pstDecoder);
	TCC_FLAC_ASSERT(0 != pstDecoder->m_pstProtected);
	pstDecoder->m_pstProtected->m_eStateProtected = state;
	return true;
}

#ifdef MD5_CHECKING
TCAS_S32 TCC_FLAC_stream_decoder_get_md5_checking(const TCC_FLAC_StreamDecoder *pstDecoder)
{
	TCC_FLAC_ASSERT(0 != pstDecoder);
	TCC_FLAC_ASSERT(0 != pstDecoder->m_pstProtected);
	return pstDecoder->m_pstProtected->m_iMd5CheckingProtected;
}
#endif

TCAS_U64 TCC_FLAC_stream_decoder_get_total_samples(const TCC_FLAC_StreamDecoder *pstDecoder)
{
	TCC_FLAC_ASSERT(0 != pstDecoder);
	TCC_FLAC_ASSERT(0 != pstDecoder->m_pstProtected);
	return pstDecoder->m_pstPrivate->m_iHasStreamInfo ? pstDecoder->m_pstPrivate->m_stStreamInfo.m_ulTotalSamples : 0;
}
#if 0
TCAS_U32 TCC_FLAC_stream_decoder_get_channels(const TCC_FLAC_StreamDecoder *pstDecoder)
{
	TCC_FLAC_ASSERT(0 != pstDecoder);
	TCC_FLAC_ASSERT(0 != pstDecoder->m_pstProtected);
	return pstDecoder->m_pstProtected->m_uiChannelsProtected;
}

TCC_FLAC_ChannelAssignment TCC_FLAC_stream_decoder_get_channel_assignment(const TCC_FLAC_StreamDecoder *pstDecoder)
{
	TCC_FLAC_ASSERT(0 != pstDecoder);
	TCC_FLAC_ASSERT(0 != pstDecoder->m_pstProtected);
	return pstDecoder->m_pstProtected->m_eChannelAssignmentProtected;
}

TCAS_U32 TCC_FLAC_stream_decoder_get_bits_per_sample(const TCC_FLAC_StreamDecoder *pstDecoder)
{
	TCC_FLAC_ASSERT(0 != pstDecoder);
	TCC_FLAC_ASSERT(0 != pstDecoder->m_pstProtected);
	return pstDecoder->m_pstProtected->m_uiBitsPerSampleProtected;
}

TCAS_U32 TCC_FLAC_stream_decoder_get_sample_rate(const TCC_FLAC_StreamDecoder *pstDecoder)
{
	TCC_FLAC_ASSERT(0 != pstDecoder);
	TCC_FLAC_ASSERT(0 != pstDecoder->m_pstProtected);
	return pstDecoder->m_pstProtected->m_uiSampleRateProtected;
}

TCAS_U32 TCC_FLAC_stream_decoder_get_blocksize(const TCC_FLAC_StreamDecoder *pstDecoder)
{
	TCC_FLAC_ASSERT(0 != pstDecoder);
	TCC_FLAC_ASSERT(0 != pstDecoder->m_pstProtected);
	return pstDecoder->m_pstProtected->m_uiBlockSizeProtected;
}
#endif

TCAS_S32 TCC_FLAC_stream_decoder_flush(TCC_FLAC_StreamDecoder *pstDecoder)
{
	TCC_FLAC_ASSERT(0 != pstDecoder);
	TCC_FLAC_ASSERT(0 != pstDecoder->m_pstPrivate);
	TCC_FLAC_ASSERT(0 != pstDecoder->m_pstProtected);

	pstDecoder->m_pstPrivate->m_uiSamplesDecoded = 0;

#ifdef MD5_CHECKING
	pstDecoder->m_pstPrivate->m_iMd5CheckFlag = false;
#endif


	TCC_FLACDEC_ResetBitParser(pstDecoder->m_pstPrivate->m_pstInputBit);
	
	//pstDecoder->m_pstPrivate->m_uiLastFrameNumber = 0;
	//pstDecoder->m_pstPrivate->m_uiLastBlockSize = 0;

	pstDecoder->m_pstProtected->m_eStateProtected = TCC_FLAC_STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;

	return true;
}

TCAS_S32 TCC_FLAC_stream_decoder_reset( TCC_FLAC_StreamDecoder *pstDecoder )
{
	TCC_FLAC_ASSERT(0 != pstDecoder);
	TCC_FLAC_ASSERT(0 != pstDecoder->m_pstPrivate);
	TCC_FLAC_ASSERT(0 != pstDecoder->m_pstProtected);

	TCC_FLAC_stream_decoder_flush(pstDecoder);
	
	//pstDecoder->m_pstPrivate->m_iHasStreamInfo = false;

#ifdef MD5_CHECKING
	pstDecoder->m_pstPrivate->m_iMd5CheckFlag = pstDecoder->m_pstProtected->m_iMd5CheckingProtected;
	TCC_FLAC_MD5Init(&pstDecoder->m_pstPrivate->md5context);
#endif

	pstDecoder->m_pstPrivate->m_uiFixedBlockSize = 0;
	pstDecoder->m_pstPrivate->m_uiNextFixedBlockSize = 0;

	return true;
}

TCAS_S32 TCC_FLAC_stream_decoder_process_single( TCC_FLAC_StreamDecoder *pstDecoder )
{
	TCAS_S32 iGotAFrame;
	TCC_FLAC_ASSERT(0 != pstDecoder);
	TCC_FLAC_ASSERT(0 != pstDecoder->m_pstProtected);

	while(1) 
	{
		switch(pstDecoder->m_pstProtected->m_eStateProtected) 
		{
			case TCC_FLAC_STREAM_DECODER_SEARCH_FOR_METADATA:
					return false; /* above function sets the status for us */
			case TCC_FLAC_STREAM_DECODER_READ_METADATA:
					return false; /* above function sets the status for us */
			case TCC_FLAC_STREAM_DECODER_SEARCH_FOR_FRAME_SYNC:
				if(!FrameSync(pstDecoder)){
					return true; /* above function sets the status for us */
				}
				break;
			case TCC_FLAC_STREAM_DECODER_READ_FRAME:
				if(!ReadFrame(pstDecoder, &iGotAFrame, /*iFullDecodingFlag=*/true)){
					return false; /* above function sets the status for us */
				}
				if(iGotAFrame){
					return true; /* above function sets the status for us */
				}
				break;
			case TCC_FLAC_STREAM_DECODER_END_OF_STREAM:
			case TCC_FLAC_STREAM_DECODER_ABORTED:
				return true;
			default:
				TCC_FLAC_ASSERT(0);
				return false;
		}
	}
}

/***********************************************************************
 *
 * Private class methods
 *
 ***********************************************************************/

static TCASVoid SetDefaults( TCC_FLAC_StreamDecoder *pstDecoder )
{

	pstDecoder->m_pstPrivate->m_pfWriteCallback = 0;
	pstDecoder->m_pstPrivate->m_pvClientDataPrivate = 0;

#ifdef MD5_CHECKING
	pstDecoder->m_pstProtected->m_iMd5CheckingProtected = false;
#endif

}

#if 0
TCASVoid send_error_to_client_(const TCC_FLAC_StreamDecoder *pstDecoder, TCC_FLAC_StreamDecoderErrorStatus status)
{
	//flac__utils_printf(stderr, 1, "%s: *** Got error code %d:%s\n", status, TCC_FLAC_StreamDecoderErrorStatusString[status]);
}
#endif

//#define SKIPBYTESIZE		65536*2	//V122
#define SKIPBYTESIZE		4608*2*2*2	//V123

static TCAS_S32 FrameSync( TCC_FLAC_StreamDecoder *pstDecoder )
{
	//TCAS_S32 iFirstFlag = true;
	TCAS_U32 uiGetData, uiCount;	//v122

//	if( TCC_FLAC_stream_decoder_get_total_samples(pstDecoder) > 0 ) 
//	{
//		if( pstDecoder->m_pstPrivate->m_uiSamplesDecoded >= TCC_FLAC_stream_decoder_get_total_samples(pstDecoder) ) 
//		{
//			pstDecoder->m_pstProtected->m_eStateProtected = TCC_FLAC_STREAM_DECODER_END_OF_STREAM;
//			return true;
//		}
//	}

	/* make sure byte aligned */
	if( !TCC_FLACDEC_CheckByteAlignedConsumed(pstDecoder->m_pstPrivate->m_pstInputBit) ) 
	{
		if(!TCC_FLACDEC_ReadU32(pstDecoder->m_pstPrivate->m_pstInputBit, &uiGetData, TCC_FLACDEC_BitsLeftForByteAlign(pstDecoder->m_pstPrivate->m_pstInputBit)))
		{
			return false;
		}
	}

	uiCount = 0;	//v122

	while( 1 ) 
	{
		if( pstDecoder->m_pstPrivate->m_iCached ) 
		{
			uiGetData = (TCAS_U32)pstDecoder->m_pstPrivate->m_ucLookAhead;
			pstDecoder->m_pstPrivate->m_iCached = false;
		}
		else 
		{
			if(!TCC_FLACDEC_ReadU32(pstDecoder->m_pstPrivate->m_pstInputBit, &uiGetData, 8))
			{
				return false;
			}
		}
		if( uiGetData == 0xff ) 
		{	/* MAGIC NUMBER for the first 8 frame sync bits */
			pstDecoder->m_pstPrivate->m_ucHeaderWarmup[0] = (TCAS_U8)uiGetData;
			if(!TCC_FLACDEC_ReadU32(pstDecoder->m_pstPrivate->m_pstInputBit, &uiGetData, 8))
			{
				return false;
			}

			if( uiGetData == 0xff ) 
			{	/* MAGIC NUMBER for the first 8 frame sync bits */
				pstDecoder->m_pstPrivate->m_ucLookAhead = (TCAS_U8)uiGetData;
				pstDecoder->m_pstPrivate->m_iCached = true;
			}
			else if( uiGetData >> 2 == 0x3e ) 
			{ /* MAGIC NUMBER for the last 6 sync bits */
				pstDecoder->m_pstPrivate->m_ucHeaderWarmup[1] = (TCAS_U8)uiGetData;
				pstDecoder->m_pstProtected->m_eStateProtected = TCC_FLAC_STREAM_DECODER_READ_FRAME;
				return true;
			}
		}
#if 0
		if( iFirstFlag ) 
		{
			//send_error_to_client_(pstDecoder, TCC_FLAC_STREAM_DECODER_ERROR_STATUS_LOST_SYNC);
			iFirstFlag = false;
		}
#endif
		uiCount++;	//v122
		
		if(uiCount >= SKIPBYTESIZE)	//v123
		{
			return true;	//v122
		}
	}

	//return true;
}

static TCAS_S32 ReadFrame( TCC_FLAC_StreamDecoder *pstDecoder, TCAS_S32 *piGotAFrame, TCAS_S32 iFullDecodingFlag )
{
	TCAS_U32 uiChannel;
#ifndef ARM_OPT	
	TCAS_U32 i;
	TCAS_S32 uiChMid, uiChSide, uiChLeft, uiChRight;
#endif
	TCAS_U32 uiFrameCRC; /* the one we calculate from the input stream */
	TCAS_U32 uiGetData;

	*piGotAFrame = false;

	/* init the CRC */
	uiFrameCRC = 0;
	uiFrameCRC = TCC_FLAC_CRC16UPDATE(pstDecoder->m_pstPrivate->m_ucHeaderWarmup[0], uiFrameCRC);
	uiFrameCRC = TCC_FLAC_CRC16UPDATE(pstDecoder->m_pstPrivate->m_ucHeaderWarmup[1], uiFrameCRC);
	TCC_FLACDEC_ResetReadCRC16(pstDecoder->m_pstPrivate->m_pstInputBit, (TCAS_U16)uiFrameCRC);

	if(!ReadFrameHeader(pstDecoder))
	{
		//return false;
		pstDecoder->m_pstProtected->m_eStateProtected = TCC_FLAC_STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
		return false;	//v119
	}
	
	if( pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBlockSize > pstDecoder->m_uiMaxBlock )
	{
		//need more memory
		//return false;
		pstDecoder->m_pstProtected->m_eStateProtected = TCC_FLAC_STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
		return false;	//v119
	}
	if(pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiChannels > pstDecoder->m_uiMaxCh)	//v121
	{
		pstDecoder->m_pstProtected->m_eStateProtected = TCC_FLAC_STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;

		return false;
	}
#if 0
	if(pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiSampleRate > pstDecoder->max_samplerate)	//v121
	{
		pstDecoder->m_pstProtected->m_eStateProtected = TCC_FLAC_STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
		return false;
	}
#endif

	if(pstDecoder->m_pstProtected->m_eStateProtected == TCC_FLAC_STREAM_DECODER_SEARCH_FOR_FRAME_SYNC) /* means we didn't sync on a valid header */
	{
		return true;
	}

	for( uiChannel = 0; uiChannel < pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiChannels; uiChannel++)
	{
		/* first figure the correct bits-per-sample of the subframe */
		TCAS_U32 uiBitsPerSample = pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBitsPerSample;

		switch( pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_eChannelAssignment ) 
		{
			case TCC_FLAC_CHANNEL_ASSIGNMENT_INDEPENDENT: /* no adjustment needed */
				break;
			case TCC_FLAC_CHANNEL_ASSIGNMENT_LEFT_SIDE:
				TCC_FLAC_ASSERT(pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiChannels == 2);
				if(uiChannel == 1)
				{
					uiBitsPerSample++;
				}
				break;
			case TCC_FLAC_CHANNEL_ASSIGNMENT_RIGHT_SIDE:
				TCC_FLAC_ASSERT(pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiChannels == 2);
				if(uiChannel == 0)
				{
					uiBitsPerSample++;
				}
				break;
			case TCC_FLAC_CHANNEL_ASSIGNMENT_MID_SIDE:
				TCC_FLAC_ASSERT(pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiChannels == 2);
				if(uiChannel == 1)
				{
					uiBitsPerSample++;
				}
				break;
			default:
				TCC_FLAC_ASSERT(0);
		}

		/* now read it */
		if(!ReadSubFrame(pstDecoder, uiChannel, uiBitsPerSample, iFullDecodingFlag))
		{
			return false;
		}
		if(pstDecoder->m_pstProtected->m_eStateProtected == TCC_FLAC_STREAM_DECODER_SEARCH_FOR_FRAME_SYNC) /* means bad sync or got corruption */
		{
			return true;
		}
	}

	//test_overflow(pstDecoder->m_pstPrivate->m_piOutput[0], pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBlockSize);
	//test_overflow(pstDecoder->m_pstPrivate->m_piOutput[1], pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBlockSize);

	if(!ReadZeroPadding(pstDecoder))
	{
		return false;
	}

	if( pstDecoder->m_pstProtected->m_eStateProtected == TCC_FLAC_STREAM_DECODER_SEARCH_FOR_FRAME_SYNC) {/* means bad sync or got corruption (i.e. "zero bits" were not all zeroes) */
		return true;
	}

	/* Read the frame CRC-16 from the footer and check */
	uiFrameCRC = TCC_FLACDEC_GetReadCRC16(pstDecoder->m_pstPrivate->m_pstInputBit);
	if(!TCC_FLACDEC_ReadU32(pstDecoder->m_pstPrivate->m_pstInputBit, &uiGetData, TCC_FLAC_FRAME_FOOTER_CRC_LEN))
	{
		return false;
	}
	
	if(uiFrameCRC == uiGetData) 
	{
		if(iFullDecodingFlag) 
		{
			/* Undo any special channel coding */
			switch(pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_eChannelAssignment)
			{
				case TCC_FLAC_CHANNEL_ASSIGNMENT_INDEPENDENT:
					/* do nothing */
					break;
				case TCC_FLAC_CHANNEL_ASSIGNMENT_LEFT_SIDE:
					TCC_FLAC_ASSERT(pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiChannels == 2);
#ifdef ARM_OPT	
					Channel_Assign_LeftSide(&pstDecoder->m_pstPrivate->m_piOutput[0][0], &pstDecoder->m_pstPrivate->m_piOutput[1][0], pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBlockSize);
#else
					for(i = 0; i < pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBlockSize; i++){
						pstDecoder->m_pstPrivate->m_piOutput[1][i] = pstDecoder->m_pstPrivate->m_piOutput[0][i] - pstDecoder->m_pstPrivate->m_piOutput[1][i];
					}
#endif
					break;
				case TCC_FLAC_CHANNEL_ASSIGNMENT_RIGHT_SIDE:
					TCC_FLAC_ASSERT(pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiChannels == 2);
#ifdef ARM_OPT
					Channel_Assign_RightSide(&pstDecoder->m_pstPrivate->m_piOutput[0][0], &pstDecoder->m_pstPrivate->m_piOutput[1][0], pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBlockSize);
#else
					for(i = 0; i < pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBlockSize; i++){
						pstDecoder->m_pstPrivate->m_piOutput[0][i] += pstDecoder->m_pstPrivate->m_piOutput[1][i];
					}
#endif
					break;
				case TCC_FLAC_CHANNEL_ASSIGNMENT_MID_SIDE:
					TCC_FLAC_ASSERT(pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiChannels == 2);
#ifdef ARM_OPT
					Channel_Assign_MidSide(&pstDecoder->m_pstPrivate->m_piOutput[0][0], &pstDecoder->m_pstPrivate->m_piOutput[1][0], pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBlockSize);
#else
					for(i = 0; i < pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBlockSize; i++)
					{
#if 1
						//v1.1.4
						uiChMid = pstDecoder->m_pstPrivate->m_piOutput[0][i];
						uiChSide = pstDecoder->m_pstPrivate->m_piOutput[1][i];
						uiChMid <<= 1;
						if(uiChSide & 1) {/* i.e. if 'side' is odd... */
							uiChMid++;
						}
						uiChLeft = uiChMid + uiChSide;
						uiChRight = uiChMid - uiChSide;
						pstDecoder->m_pstPrivate->m_piOutput[0][i] = uiChLeft >> 1;
						pstDecoder->m_pstPrivate->m_piOutput[1][i] = uiChRight >> 1;
#else
						//v1.2.1
						uiChMid = pstDecoder->m_pstPrivate->m_piOutput[0][i];
						uiChSide = pstDecoder->m_pstPrivate->m_piOutput[1][i];
						uiChMid <<= 1;
						uiChMid |= ( uiChSide & 1 );
						uiChLeft = uiChMid + uiChSide;
						uiChRight = uiChMid - uiChSide;
						pstDecoder->m_pstPrivate->m_piOutput[0][i] = uiChLeft >> 1;
						pstDecoder->m_pstPrivate->m_piOutput[1][i] = uiChRight >> 1;
#endif
					}
#endif
					break;
				default:
					TCC_FLAC_ASSERT(0);
					break;
			}
		}
	}
	else {
		/* Bad frame, emit error and zero the output signal */
		//send_error_to_client_(pstDecoder, TCC_FLAC_STREAM_DECODER_ERROR_STATUS_FRAME_CRC_MISMATCH);
		if(iFullDecodingFlag) 
		{
			for( uiChannel = 0; uiChannel < pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiChannels; uiChannel++ ) 
			{
				pstDecoder->m_pCallback->m_pfMemset(pstDecoder->m_pstPrivate->m_piOutput[uiChannel], 0, sizeof(TCAS_S32) * pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBlockSize);
			}
		}
	}

	*piGotAFrame = true;

	/* we wait to update fixed_block_size until here, when we're sure we've got a proper frame and hence a correct blocksize */
	if( pstDecoder->m_pstPrivate->m_uiNextFixedBlockSize )	{ //v1.2.1
		pstDecoder->m_pstPrivate->m_uiFixedBlockSize = pstDecoder->m_pstPrivate->m_uiNextFixedBlockSize;
	}

	/* put the latest values into the public section of the pstDecoder instance */
	pstDecoder->m_pstProtected->m_uiChannelsProtected = pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiChannels;
	pstDecoder->m_pstProtected->m_eChannelAssignmentProtected = pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_eChannelAssignment;
	pstDecoder->m_pstProtected->m_uiBitsPerSampleProtected = pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBitsPerSample;
	pstDecoder->m_pstProtected->m_uiSampleRateProtected = pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiSampleRate;
	pstDecoder->m_pstProtected->m_uiBlockSizeProtected = pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBlockSize;

	TCC_FLAC_ASSERT(pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_eNumberType == TCC_FLAC_FRAME_NUMBER_TYPE_SAMPLE_NUMBER);
	pstDecoder->m_pstPrivate->m_uiSamplesDecoded = pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uNumber.m_ulSampleNumber + pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBlockSize;

	/* write it */
	if(iFullDecodingFlag )
	{
		if(write_audio_frame_to_client_(pstDecoder, &pstDecoder->m_pstPrivate->m_stFrame, (const TCAS_S32 * const *)pstDecoder->m_pstPrivate->m_piOutput) != TCC_FLAC_STREAM_DECODER_WRITE_STATUS_CONTINUE)
		{
			return false;
		}
	}

	pstDecoder->m_pstProtected->m_eStateProtected = TCC_FLAC_STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
	return true;
}

TCAS_S32 FLAC_Decoder_GetSampleSize(TCC_FLAC_StreamDecoder *pstDecoder)
{
	return pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBlockSize;
}
#if 0
TCAS_S32 FLAC_Decoder_GetDecodedSample(TCC_FLAC_StreamDecoder *pstDecoder)
{
	return pstDecoder->m_pstPrivate->m_uiSamplesDecoded;
}
#endif

static const TCAS_U8 TCC_FLAC_Crc8Table[256] = 
{
	/* CRC-8, poly = x^8 + x^2 + x^1 + x^0, init = 0 */
	0x0000, 0x0007, 0x000E, 0x0009, 0x001C, 0x001B, 0x0012, 0x0015, 0x0038, 0x003F, 0x0036, 0x0031, 0x0024, 0x0023, 0x002A, 0x002D,
	0x0070, 0x0077, 0x007E, 0x0079, 0x006C, 0x006B, 0x0062, 0x0065,	0x0048, 0x004F, 0x0046, 0x0041, 0x0054, 0x0053, 0x005A, 0x005D,
	0x00E0, 0x00E7, 0x00EE, 0x00E9, 0x00FC, 0x00FB, 0x00F2, 0x00F5,	0x00D8, 0x00DF, 0x00D6, 0x00D1, 0x00C4, 0x00C3, 0x00CA, 0x00CD,
	0x0090, 0x0097, 0x009E, 0x0099, 0x008C, 0x008B, 0x0082, 0x0085,	0x00A8, 0x00AF, 0x00A6, 0x00A1, 0x00B4, 0x00B3, 0x00BA, 0x00BD,
	0x00C7, 0x00C0, 0x00C9, 0x00CE, 0x00DB, 0x00DC, 0x00D5, 0x00D2,	0x00FF, 0x00F8, 0x00F1, 0x00F6, 0x00E3, 0x00E4, 0x00ED, 0x00EA,
	0x00B7, 0x00B0, 0x00B9, 0x00BE, 0x00AB, 0x00AC, 0x00A5, 0x00A2,	0x008F, 0x0088, 0x0081, 0x0086, 0x0093, 0x0094, 0x009D, 0x009A,
	0x0027, 0x0020, 0x0029, 0x002E, 0x003B, 0x003C, 0x0035, 0x0032,	0x001F, 0x0018, 0x0011, 0x0016, 0x0003, 0x0004, 0x000D, 0x000A,
	0x0057, 0x0050, 0x0059, 0x005E, 0x004B, 0x004C, 0x0045, 0x0042,	0x006F, 0x0068, 0x0061, 0x0066, 0x0073, 0x0074, 0x007D, 0x007A,
	0x0089, 0x008E, 0x0087, 0x0080, 0x0095, 0x0092, 0x009B, 0x009C,	0x00B1, 0x00B6, 0x00BF, 0x00B8, 0x00AD, 0x00AA, 0x00A3, 0x00A4,
	0x00F9, 0x00FE, 0x00F7, 0x00F0, 0x00E5, 0x00E2, 0x00EB, 0x00EC,	0x00C1, 0x00C6, 0x00CF, 0x00C8, 0x00DD, 0x00DA, 0x00D3, 0x00D4,
	0x0069, 0x006E, 0x0067, 0x0060, 0x0075, 0x0072, 0x007B, 0x007C,	0x0051, 0x0056, 0x005F, 0x0058, 0x004D, 0x004A, 0x0043, 0x0044,
	0x0019, 0x001E, 0x0017, 0x0010, 0x0005, 0x0002, 0x000B, 0x000C,	0x0021, 0x0026, 0x002F, 0x0028, 0x003D, 0x003A, 0x0033, 0x0034,
	0x004E, 0x0049, 0x0040, 0x0047, 0x0052, 0x0055, 0x005C, 0x005B,	0x0076, 0x0071, 0x0078, 0x007F, 0x006A, 0x006D, 0x0064, 0x0063,
	0x003E, 0x0039, 0x0030, 0x0037, 0x0022, 0x0025, 0x002C, 0x002B,	0x0006, 0x0001, 0x0008, 0x000F, 0x001A, 0x001D, 0x0014, 0x0013,
	0x00AE, 0x00A9, 0x00A0, 0x00A7, 0x00B2, 0x00B5, 0x00BC, 0x00BB,	0x0096, 0x0091, 0x0098, 0x009F, 0x008A, 0x008D, 0x0084, 0x0083,
	0x00DE, 0x00D9, 0x00D0, 0x00D7, 0x00C2, 0x00C5, 0x00CC, 0x00CB,	0x00E6, 0x00E1, 0x00E8, 0x00EF, 0x00FA, 0x00FD, 0x00F4, 0x00F3
};

static TCAS_U8 TCC_FLAC_crc8(const TCAS_U8 *pucData, TCAS_U32 uiLength )
{
	TCAS_U8 uCrc = 0;

	while( uiLength-- )
	{
		uCrc = TCC_FLAC_Crc8Table[uCrc ^ *pucData++];
	}

	return uCrc;
}


static TCAS_S32 ReadFrameHeader( TCC_FLAC_StreamDecoder *pstDecoder )
{
	TCAS_U64 ulX2;
	TCAS_U32 uiGetData;
	TCAS_U32 uiRawHeaderLen;
	TCAS_U32 i, uiBlockSizeHint = 0, uiSampleRateHint = 0;
	TCAS_U8 ucCRC8, TCC_FLAC_CheckCrc8[16];

	TCAS_S32 iUnparseableFlag = false;
	//const TCAS_S32 iVariableBlockSize = (pstDecoder->m_pstPrivate->m_iHasStreamInfo && pstDecoder->m_pstPrivate->m_stStreamInfo.m_uiMinBlockSize != pstDecoder->m_pstPrivate->m_stStreamInfo.m_uiMaxBlockSize);
	//const TCAS_S32 iFixedBlockSize = (pstDecoder->m_pstPrivate->m_iHasStreamInfo && pstDecoder->m_pstPrivate->m_stStreamInfo.m_uiMinBlockSize == pstDecoder->m_pstPrivate->m_stStreamInfo.m_uiMaxBlockSize);

	TCC_FLAC_ASSERT(TCC_FLACDEC_CheckByteAlignedConsumed(pstDecoder->m_pstPrivate->m_pstInputBit));

	/* init the raw header with the saved bits from synchronization */
	TCC_FLAC_CheckCrc8[0] = pstDecoder->m_pstPrivate->m_ucHeaderWarmup[0];
	TCC_FLAC_CheckCrc8[1] = pstDecoder->m_pstPrivate->m_ucHeaderWarmup[1];
	uiRawHeaderLen = 2;

	/* check to make sure that the reserved bits are 0 */
	if( TCC_FLAC_CheckCrc8[1] & 0x02 ) 
	{ 
		iUnparseableFlag = true;
	}

	for( i = 0; i < 2; i++ ) 
	{
		if( !TCC_FLACDEC_ReadU32(pstDecoder->m_pstPrivate->m_pstInputBit, &uiGetData, 8) )
		{
			return false;
		}

		if( uiGetData == 0xff ) 
		{ 
			pstDecoder->m_pstPrivate->m_ucLookAhead = (TCAS_U8)uiGetData;
			pstDecoder->m_pstPrivate->m_iCached = true;
			pstDecoder->m_pstProtected->m_eStateProtected = TCC_FLAC_STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
			return true;
		}

		TCC_FLAC_CheckCrc8[uiRawHeaderLen++] = (TCAS_U8)uiGetData;
	}

	switch( uiGetData = TCC_FLAC_CheckCrc8[2] >> 4 ) 
	{
		case 0:
			iUnparseableFlag = true;
			break;
		case 1:
			pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBlockSize = 192;
			break;
		case 2:
		case 3:
		case 4:
		case 5:
			pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBlockSize = 576 << (uiGetData-2);
			break;
		case 6:
		case 7:
			uiBlockSizeHint = uiGetData;
			break;
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
			pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBlockSize = 256 << (uiGetData-8);
			break;
		default:
			TCC_FLAC_ASSERT(0);
			break;
	}

	switch( uiGetData = TCC_FLAC_CheckCrc8[2] & 0x0f ) 
	{
		case 0:
			if( pstDecoder->m_pstPrivate->m_iHasStreamInfo )
			{
				pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiSampleRate = pstDecoder->m_pstPrivate->m_stStreamInfo.m_uiSampleRate;
			} else {
				iUnparseableFlag = true;
			}
			break;
		case 1:
			pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiSampleRate = 88200;
			break;
		case 2:
			pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiSampleRate = 176400;
			break;
		case 3:
			pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiSampleRate = 192000;
			break;
		case 4:
			pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiSampleRate = 8000;
			break;
		case 5:
			pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiSampleRate = 16000;
			break;
		case 6:
			pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiSampleRate = 22050;
			break;
		case 7:
			pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiSampleRate = 24000;
			break;
		case 8:
			pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiSampleRate = 32000;
			break;
		case 9:
			pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiSampleRate = 44100;
			break;
		case 10:
			pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiSampleRate = 48000;
			break;
		case 11:
			pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiSampleRate = 96000;
			break;
		case 12:
		case 13:
		case 14:
			uiSampleRateHint = uiGetData;
			break;
		case 15:
			//send_error_to_client_(pstDecoder, TCC_FLAC_STREAM_DECODER_ERROR_STATUS_BAD_HEADER);
			pstDecoder->m_pstProtected->m_eStateProtected = TCC_FLAC_STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
			return true;
		default:
			TCC_FLAC_ASSERT(0);
	}

	uiGetData = (TCAS_U32)(TCC_FLAC_CheckCrc8[3] >> 4);
	if( uiGetData & 8 ) 
	{
		pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiChannels = 2;
		switch( uiGetData & 7 ) 
		{
			case 0:
				pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_eChannelAssignment = TCC_FLAC_CHANNEL_ASSIGNMENT_LEFT_SIDE;
				break;
			case 1:
				pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_eChannelAssignment = TCC_FLAC_CHANNEL_ASSIGNMENT_RIGHT_SIDE;
				break;
			case 2:
				pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_eChannelAssignment = TCC_FLAC_CHANNEL_ASSIGNMENT_MID_SIDE;
				break;
			default:
				iUnparseableFlag = true;
				break;
		}
	}
	else 
	{
		pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiChannels = (TCAS_U32)uiGetData + 1;
		pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_eChannelAssignment = TCC_FLAC_CHANNEL_ASSIGNMENT_INDEPENDENT;
	}

	switch( uiGetData = (TCAS_U32)(TCC_FLAC_CheckCrc8[3] & 0x0e) >> 1 ) 
	{
		case 0:
			if(pstDecoder->m_pstPrivate->m_iHasStreamInfo){
				pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBitsPerSample = pstDecoder->m_pstPrivate->m_stStreamInfo.m_uiBitsPerSample;
			} else {
				iUnparseableFlag = true;
			}
			break;
		case 1:
			pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBitsPerSample = 8;
			break;
		case 2:
			pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBitsPerSample = 12;
			break;
		case 4:
			pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBitsPerSample = 16;
			break;
		case 5:
			pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBitsPerSample = 20;
			break;
		case 6:
			pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBitsPerSample = 24;
			break;
		case 3:
		case 7:
			iUnparseableFlag = true;
			break;
		default:
			TCC_FLAC_ASSERT(0);
			break;
	}
	
	/* check to make sure that reserved bit is 0 */
	if(TCC_FLAC_CheckCrc8[3] & 0x01) {/* MAGIC NUMBER */
		iUnparseableFlag = true;
	}

	/* read the frame's starting sample number (or frame number as the case may be) */
	if(	TCC_FLAC_CheckCrc8[1] & 0x01 ||
		/*@@@ this clause is a concession to the old way of doing variable blocksize; the only known implementation is flake and can probably be removed without inconveniencing anyone */
		( pstDecoder->m_pstPrivate->m_iHasStreamInfo && pstDecoder->m_pstPrivate->m_stStreamInfo.m_uiMinBlockSize != pstDecoder->m_pstPrivate->m_stStreamInfo.m_uiMaxBlockSize )
	) 
	{ /* variable blocksize */
		if( !TCC_FLACDEC_ReadU64UFT8(pstDecoder->m_pstPrivate->m_pstInputBit, &ulX2, TCC_FLAC_CheckCrc8, &uiRawHeaderLen) )
		{
			return false; /* read_callback_ sets the state for us */
		}
		if( ulX2 == TCC_FLAC_U64L(0xffffffffffffffff) ) 
		{
			/* i.e. non-UTF8 code... */
			pstDecoder->m_pstPrivate->m_ucLookAhead = TCC_FLAC_CheckCrc8[uiRawHeaderLen-1]; /* back up as much as we can */
			pstDecoder->m_pstPrivate->m_iCached = true;
			//send_error_to_client_(pstDecoder, TCC_FLAC_STREAM_DECODER_ERROR_STATUS_BAD_HEADER);
			pstDecoder->m_pstProtected->m_eStateProtected = TCC_FLAC_STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
			return true;
		}
		pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_eNumberType = TCC_FLAC_FRAME_NUMBER_TYPE_SAMPLE_NUMBER;
		pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uNumber.m_ulSampleNumber = ulX2;
	}
	else  /* fixed blocksize */
	{
		if( !TCC_FLACDEC_ReadU32UFT8(pstDecoder->m_pstPrivate->m_pstInputBit, &uiGetData, TCC_FLAC_CheckCrc8, &uiRawHeaderLen) )
		{
			return false; /* read_callback_ sets the state for us */
		}
		if( uiGetData == 0xffffffff ) 
		{ /* i.e. non-UTF8 code... */
			pstDecoder->m_pstPrivate->m_ucLookAhead = TCC_FLAC_CheckCrc8[uiRawHeaderLen-1]; /* back up as much as we can */
			pstDecoder->m_pstPrivate->m_iCached = true;
			//send_error_to_client_(pstDecoder, TCC_FLAC_STREAM_DECODER_ERROR_STATUS_BAD_HEADER);
			pstDecoder->m_pstProtected->m_eStateProtected = TCC_FLAC_STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
			return true;
		}

		pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_eNumberType = TCC_FLAC_FRAME_NUMBER_TYPE_FRAME_NUMBER;
		pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uNumber.m_uiFrameNumber = uiGetData;
	}

	if( uiBlockSizeHint ) 
	{
		if( !TCC_FLACDEC_ReadU32(pstDecoder->m_pstPrivate->m_pstInputBit, &uiGetData, 8) )
		{
			return false;
		}

		TCC_FLAC_CheckCrc8[uiRawHeaderLen++] = (TCAS_U8)uiGetData;
		if( uiBlockSizeHint == 7 ) 
		{
			TCAS_U32 _x;
			if(!TCC_FLACDEC_ReadU32(pstDecoder->m_pstPrivate->m_pstInputBit, &_x, 8))
			{
				return false;
			}
			TCC_FLAC_CheckCrc8[uiRawHeaderLen++] = (TCAS_U8)_x;
			uiGetData = (uiGetData << 8) | _x;
		}
		pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBlockSize = uiGetData+1;
	}

	if( uiSampleRateHint ) 
	{
		if( !TCC_FLACDEC_ReadU32(pstDecoder->m_pstPrivate->m_pstInputBit, &uiGetData, 8) )
		{
			return false;
		}
		TCC_FLAC_CheckCrc8[uiRawHeaderLen++] = (TCAS_U8)uiGetData;
		if( uiSampleRateHint != 12 ) 
		{
			TCAS_U32 _x;
			if(!TCC_FLACDEC_ReadU32(pstDecoder->m_pstPrivate->m_pstInputBit, &_x, 8))
			{
				return false;
			}
			TCC_FLAC_CheckCrc8[uiRawHeaderLen++] = (TCAS_U8)_x;
			uiGetData = (uiGetData << 8) | _x;
		}
		if( uiSampleRateHint == 12 )
		{
			pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiSampleRate = uiGetData*1000;
		}
		else if( uiSampleRateHint == 13 )
		{
			pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiSampleRate = uiGetData;
		}
		else
		{
			pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiSampleRate = uiGetData*10;
		}
	}

	/* read the CRC-8 byte */
	if( !TCC_FLACDEC_ReadU32(pstDecoder->m_pstPrivate->m_pstInputBit, &uiGetData, 8) )
	{
		return false;
	}

	ucCRC8 = (TCAS_U8)uiGetData;

	if( TCC_FLAC_crc8(TCC_FLAC_CheckCrc8, uiRawHeaderLen) != ucCRC8 ) 
	{
		//send_error_to_client_(pstDecoder, TCC_FLAC_STREAM_DECODER_ERROR_STATUS_BAD_HEADER);
		pstDecoder->m_pstProtected->m_eStateProtected = TCC_FLAC_STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
		return true;
	}

	/* calculate the sample number from the frame number if needed */
	pstDecoder->m_pstPrivate->m_uiNextFixedBlockSize = 0;
	if(pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_eNumberType == TCC_FLAC_FRAME_NUMBER_TYPE_FRAME_NUMBER )
	{
		ulX2 = pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uNumber.m_uiFrameNumber;
		
		pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_eNumberType = TCC_FLAC_FRAME_NUMBER_TYPE_SAMPLE_NUMBER;
		
		if( pstDecoder->m_pstPrivate->m_uiFixedBlockSize )
		{
			pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uNumber.m_ulSampleNumber = (TCAS_U64)pstDecoder->m_pstPrivate->m_uiFixedBlockSize * (TCAS_U64)ulX2;
		}
		else if(pstDecoder->m_pstPrivate->m_iHasStreamInfo) 
		{
			if( pstDecoder->m_pstPrivate->m_stStreamInfo.m_uiMinBlockSize == pstDecoder->m_pstPrivate->m_stStreamInfo.m_uiMaxBlockSize )
			{
				pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uNumber.m_ulSampleNumber = (TCAS_U64)pstDecoder->m_pstPrivate->m_stStreamInfo.m_uiMinBlockSize * (TCAS_U64)ulX2;
				pstDecoder->m_pstPrivate->m_uiNextFixedBlockSize = pstDecoder->m_pstPrivate->m_stStreamInfo.m_uiMaxBlockSize;
			}
			else {
				iUnparseableFlag = true;
			}
		}
		else if( ulX2 == 0 ) 
		{
			pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uNumber.m_ulSampleNumber = 0;
			pstDecoder->m_pstPrivate->m_uiNextFixedBlockSize = pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBlockSize;
		}
		else 
		{
			/* can only get here if the stream has invalid frame numbering and no STREAMINFO, so assume it's not the last (possibly short) frame */
			pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uNumber.m_ulSampleNumber = (TCAS_U64)pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBlockSize * (TCAS_U64)ulX2;
		}
	}

	if( iUnparseableFlag ) 
	{
		//send_error_to_client_(pstDecoder, TCC_FLAC_STREAM_DECODER_ERROR_STATUS_UNPARSEABLE_STREAM);
		pstDecoder->m_pstProtected->m_eStateProtected = TCC_FLAC_STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
		return true;
	}

	return true;
}

static TCAS_S32 ReadSubFrame( TCC_FLAC_StreamDecoder *pstDecoder, 
							  TCAS_U32 uiChannel, 
							  TCAS_U32 uiBitsPerSample, 
							  TCAS_S32 iFullDecodingFlag
)
{
	TCAS_U32 uiGetData;
	TCAS_S32 iWastedBits;
#ifndef ARM_OPT	
	TCAS_U32 i;
#endif

	if( !TCC_FLACDEC_ReadU32(pstDecoder->m_pstPrivate->m_pstInputBit, &uiGetData, 8) )
	{
		return false;
	}

	iWastedBits = (uiGetData & 1);
	uiGetData &= 0xfe;

	if( iWastedBits ) 
	{
		TCAS_U32 uiUtmp;
		if( !TCC_FLACDEC_ReadUUnary(pstDecoder->m_pstPrivate->m_pstInputBit, &uiUtmp) )
		{
			return false;
		}

		pstDecoder->m_pstPrivate->m_stFrame.m_stSubFrames[uiChannel].m_uiWastedBits = uiUtmp+1;
		uiBitsPerSample -= pstDecoder->m_pstPrivate->m_stFrame.m_stSubFrames[uiChannel].m_uiWastedBits;
	}
	else {
		pstDecoder->m_pstPrivate->m_stFrame.m_stSubFrames[uiChannel].m_uiWastedBits = 0;
	}

	/* Lots of magic numbers here */
	if( uiGetData & 0x80 ) 
	{
		//send_error_to_client_(pstDecoder, TCC_FLAC_STREAM_DECODER_ERROR_STATUS_LOST_SYNC);
		pstDecoder->m_pstProtected->m_eStateProtected = TCC_FLAC_STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
		return true;
	}
	else if( uiGetData == 0 ) 
	{
		if( !ReadSubFrameConstant(pstDecoder, uiChannel, uiBitsPerSample, iFullDecodingFlag) ){
			return false;
		}
	}
	else if( uiGetData == 2 ) 
	{
		if( !ReadSubFrameVerbatim(pstDecoder, uiChannel, uiBitsPerSample, iFullDecodingFlag) ){
			return false;
		}
	}
	else if( uiGetData < 16 ) 
	{
		//send_error_to_client_(pstDecoder, TCC_FLAC_STREAM_DECODER_ERROR_STATUS_UNPARSEABLE_STREAM);
		pstDecoder->m_pstProtected->m_eStateProtected = TCC_FLAC_STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
		return true;
	}
	else if( uiGetData <= 24 ) 
	{
		if( !ReadSubFrameFixed(pstDecoder, uiChannel, uiBitsPerSample, (uiGetData>>1)&7, iFullDecodingFlag) ){
			return false;
		}
		if( pstDecoder->m_pstProtected->m_eStateProtected == TCC_FLAC_STREAM_DECODER_SEARCH_FOR_FRAME_SYNC ) {/* means bad sync or got corruption */
			return true;
		}
	}
	else if( uiGetData < 64 ) 
	{
		//send_error_to_client_(pstDecoder, TCC_FLAC_STREAM_DECODER_ERROR_STATUS_UNPARSEABLE_STREAM);
		pstDecoder->m_pstProtected->m_eStateProtected = TCC_FLAC_STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
		return true;
	}
	else 
	{
		if( !ReadSubFrameLPC(pstDecoder, uiChannel, uiBitsPerSample, ((uiGetData>>1)&31)+1, iFullDecodingFlag) ){
			return false;
		}
		if( pstDecoder->m_pstProtected->m_eStateProtected == TCC_FLAC_STREAM_DECODER_SEARCH_FOR_FRAME_SYNC ){ /* means bad sync or got corruption */
			return true;
		}
	}

	if( iWastedBits && iFullDecodingFlag ) 
	{
		uiGetData = pstDecoder->m_pstPrivate->m_stFrame.m_stSubFrames[uiChannel].m_uiWastedBits;
#ifdef ARM_OPT
		FLAC_MemShiftLeft32(pstDecoder->m_pstPrivate->m_piOutput[uiChannel], uiGetData, pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBlockSize);
#else
		for( i = 0; i < pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBlockSize; i++ )
		{
			pstDecoder->m_pstPrivate->m_piOutput[uiChannel][i] <<= uiGetData;
		}
#endif
	}

	return true;
}

static TCAS_S32 ReadSubFrameConstant( TCC_FLAC_StreamDecoder *pstDecoder, 
									  TCAS_U32 uiChannel, 
									  TCAS_U32 uiBitsPerSample, 
									  TCAS_S32 iFullDecodingFlag
)
{
	TCC_FLAC_Subframe_Constant *pstSubFrameConst = &pstDecoder->m_pstPrivate->m_stFrame.m_stSubFrames[uiChannel].m_unData.m_stConstant;
	TCAS_S32 iGetData;
#ifndef ARM_OPT	
	TCAS_U32 i;
#endif
	TCAS_S32 *piOutPut = pstDecoder->m_pstPrivate->m_piOutput[uiChannel];

	pstDecoder->m_pstPrivate->m_stFrame.m_stSubFrames[uiChannel].m_eType = TCC_FLAC_SUBFRAME_TYPE_CONSTANT;

	if(!TCC_FLACDEC_ReadS32(pstDecoder->m_pstPrivate->m_pstInputBit, &iGetData, uiBitsPerSample))
	{
		return false;
	}

	pstSubFrameConst->m_iValue = iGetData;

	/* decode the subframe */
	if(iFullDecodingFlag) 
	{
#ifdef	ARM_OPT	
		FLAC_MemSet32( piOutPut, iGetData, pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBlockSize );
#else
		for(i = 0; i < pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBlockSize; i++)
		{
			piOutPut[i] = iGetData;
		}
#endif			
	}

	return true;
}

static TCAS_S32 ReadSubFrameFixed( TCC_FLAC_StreamDecoder *pstDecoder, 
								   TCAS_U32 uiChannel, 
								   TCAS_U32 uiBitsPerSample, 
								   const TCAS_U32 uiOrder, 
								   TCAS_S32 iFullDecodingFlag
)
{
	TCC_FLAC_Subframe_Fixed *pstSubFrameFixed = &pstDecoder->m_pstPrivate->m_stFrame.m_stSubFrames[uiChannel].m_unData.m_stFixed;
	TCAS_S32 i32;
	TCAS_U32 u32;
	TCAS_U32 uiUtmp;

	pstDecoder->m_pstPrivate->m_stFrame.m_stSubFrames[uiChannel].m_eType = TCC_FLAC_SUBFRAME_TYPE_FIXED;

	pstSubFrameFixed->m_piResidual = pstDecoder->m_pstPrivate->m_piResidual[uiChannel];
	pstSubFrameFixed->m_uiOrder = uiOrder;

	/* read warm-up samples */
	for( uiUtmp = 0; uiUtmp < uiOrder; uiUtmp++ ) 
	{
		if(!TCC_FLACDEC_ReadS32(pstDecoder->m_pstPrivate->m_pstInputBit, &i32, uiBitsPerSample))
		{
			return false; /* read_callback_ sets the state for us */
		}
		pstSubFrameFixed->m_iWarmUp[uiUtmp] = i32;
	}

	/* read entropy coding method info */
	if(!TCC_FLACDEC_ReadU32(pstDecoder->m_pstPrivate->m_pstInputBit, &u32, TCC_FLAC_ENTROPY_CODING_METHOD_TYPE_LEN))
	{
		return false;
	}

	pstSubFrameFixed->m_stEntropyCodingMethod.m_eType = (TCC_FLAC_EntropyCodingMethodType)u32;

	switch( pstSubFrameFixed->m_stEntropyCodingMethod.m_eType ) 
	{
		case TCC_FLAC_ENTROPY_CODING_METHOD_PARTITIONED_RICE:
		case TCC_FLAC_ENTROPY_CODING_METHOD_PARTITIONED_RICE2:
			if(!TCC_FLACDEC_ReadU32(pstDecoder->m_pstPrivate->m_pstInputBit, &u32, TCC_FLAC_ENTROPY_CODING_METHOD_PARTITIONED_RICE_ORDER_LEN))
			{
				return false;
			}
			pstSubFrameFixed->m_stEntropyCodingMethod.m_unData.m_stPartitionedRice.m_uiOrder = u32;
			
			/* read residual */
			if(!ReadResidualPartitionedRice(pstDecoder, uiOrder, pstSubFrameFixed->m_stEntropyCodingMethod.m_unData.m_stPartitionedRice.m_uiOrder, /*&pstDecoder->m_pstPrivate->partitioned_rice_contents[uiChannel],*/ pstDecoder->m_pstPrivate->m_piResidual[uiChannel], pstSubFrameFixed->m_stEntropyCodingMethod.m_eType == TCC_FLAC_ENTROPY_CODING_METHOD_PARTITIONED_RICE2))
			{
				return false;
			}
			
			break;
		default:
			//send_error_to_client_(pstDecoder, TCC_FLAC_STREAM_DECODER_ERROR_STATUS_UNPARSEABLE_STREAM);
			pstDecoder->m_pstProtected->m_eStateProtected = TCC_FLAC_STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
			return true;
	}
	
	/* decode the subframe */
	if( iFullDecodingFlag ) 
	{
		pstDecoder->m_pCallback->m_pfMemcpy( pstDecoder->m_pstPrivate->m_piOutput[uiChannel], pstSubFrameFixed->m_iWarmUp, sizeof(TCAS_S32) * uiOrder);

		TCC_FLAC_fixed_restore_signal( pstDecoder->m_pstPrivate->m_piResidual[uiChannel], 
					pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBlockSize-uiOrder, 
					uiOrder, 
					pstDecoder->m_pstPrivate->m_piOutput[uiChannel]+uiOrder
#ifndef ARM_OPT
					,pstDecoder->m_pCallback
#endif
					);
	}

	return true;
}

static TCAS_U32 TCC_FLAC_bitmath_ilog2( TCAS_U32 iX )
{
	TCAS_U32 uiLog2 = 0;

	TCC_FLAC_ASSERT( iX > 0 );

	while( iX >>= 1 )
	{
		uiLog2++;
	}

	return uiLog2;
}

static TCAS_S32 ReadSubFrameLPC( TCC_FLAC_StreamDecoder *pstDecoder, 
								 TCAS_U32 uiChannel, 
								 TCAS_U32 uiBitsPerSample, 
								 const TCAS_U32 uiOrder, 
								 TCAS_S32 iFullDecodingFlag 
)
{
	TCC_FLAC_Subframe_LPC *pstSubFrameLPC = &pstDecoder->m_pstPrivate->m_stFrame.m_stSubFrames[uiChannel].m_unData.m_stLPC;
	TCAS_S32 i32;
	TCAS_U32 u32;
	TCAS_U32 uiUtmp;

	pstDecoder->m_pstPrivate->m_stFrame.m_stSubFrames[uiChannel].m_eType = TCC_FLAC_SUBFRAME_TYPE_LPC;

	pstSubFrameLPC->m_piResidual = pstDecoder->m_pstPrivate->m_piResidual[uiChannel];
	pstSubFrameLPC->m_uiOrder = uiOrder;

	/* read warm-up samples */
	for( uiUtmp = 0; uiUtmp < uiOrder; uiUtmp++ ) 
	{
		if(!TCC_FLACDEC_ReadS32(pstDecoder->m_pstPrivate->m_pstInputBit, &i32, uiBitsPerSample))
		{
			return false;
		}
		pstSubFrameLPC->m_iWarmUp[uiUtmp] = i32;
	}

	/* read qlp coeff precision */
	if(!TCC_FLACDEC_ReadU32(pstDecoder->m_pstPrivate->m_pstInputBit, &u32, TCC_FLAC_SUBFRAME_LPC_QLP_COEFF_PRECISION_LEN))
	{
		return false;
	}

	if(u32 == (1u << TCC_FLAC_SUBFRAME_LPC_QLP_COEFF_PRECISION_LEN) - 1) 
	{
		//send_error_to_client_(pstDecoder, TCC_FLAC_STREAM_DECODER_ERROR_STATUS_LOST_SYNC);
		pstDecoder->m_pstProtected->m_eStateProtected = TCC_FLAC_STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
		return true;
	}
	pstSubFrameLPC->m_uiQCoeffPrecision = u32+1;

	/* read qlp shift */
	if(!TCC_FLACDEC_ReadS32(pstDecoder->m_pstPrivate->m_pstInputBit, &i32, TCC_FLAC_SUBFRAME_LPC_QLP_SHIFT_LEN))
	{
		return false;
	}

	pstSubFrameLPC->m_iQuantizationLevel = i32;

	/* read quantized lp coefficiencts */
	for( uiUtmp = 0; uiUtmp < uiOrder; uiUtmp++ ) 
	{
		if(!TCC_FLACDEC_ReadS32(pstDecoder->m_pstPrivate->m_pstInputBit, &i32, pstSubFrameLPC->m_uiQCoeffPrecision))
		{
			return false;
		}
		pstSubFrameLPC->m_iQCoeff[uiUtmp] = i32;
	}

	/* read entropy coding method info */
	if(!TCC_FLACDEC_ReadU32(pstDecoder->m_pstPrivate->m_pstInputBit, &u32, TCC_FLAC_ENTROPY_CODING_METHOD_TYPE_LEN))
	{
		return false;
	}

	pstSubFrameLPC->m_stEntropyCodingMethod.m_eType = (TCC_FLAC_EntropyCodingMethodType)u32;
	switch( pstSubFrameLPC->m_stEntropyCodingMethod.m_eType ) 
	{
		case TCC_FLAC_ENTROPY_CODING_METHOD_PARTITIONED_RICE:
		case TCC_FLAC_ENTROPY_CODING_METHOD_PARTITIONED_RICE2:
			if(!TCC_FLACDEC_ReadU32(pstDecoder->m_pstPrivate->m_pstInputBit, &u32, TCC_FLAC_ENTROPY_CODING_METHOD_PARTITIONED_RICE_ORDER_LEN))
			{
				return false;
			}
			pstSubFrameLPC->m_stEntropyCodingMethod.m_unData.m_stPartitionedRice.m_uiOrder = u32;
			
			/* read residual */
			if(!ReadResidualPartitionedRice(pstDecoder, uiOrder, pstSubFrameLPC->m_stEntropyCodingMethod.m_unData.m_stPartitionedRice.m_uiOrder, pstSubFrameLPC->m_piResidual, pstSubFrameLPC->m_stEntropyCodingMethod.m_eType == TCC_FLAC_ENTROPY_CODING_METHOD_PARTITIONED_RICE2))
			{
				return false;
			}
			break;

		default:
			//send_error_to_client_(pstDecoder, TCC_FLAC_STREAM_DECODER_ERROR_STATUS_UNPARSEABLE_STREAM);
			pstDecoder->m_pstProtected->m_eStateProtected = TCC_FLAC_STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
			return true;
	}

	/* decode the subframe */
	if( iFullDecodingFlag ) 
	{
		pstDecoder->m_pCallback->m_pfMemcpy(pstDecoder->m_pstPrivate->m_piOutput[uiChannel], pstSubFrameLPC->m_iWarmUp, sizeof(TCAS_S32) * uiOrder);
		/*@@@@@@ technically not pessimistic enough, should be more like
		if( (FLAC__uint64)order * ((((FLAC__uint64)1)<<bps)-1) * ((1<<subframe->qlp_coeff_precision)-1) < (((FLAC__uint64)-1) << 32) )
		*/
		if( uiBitsPerSample + pstSubFrameLPC->m_uiQCoeffPrecision + TCC_FLAC_bitmath_ilog2(uiOrder) <= 32 )
		{
			if( uiBitsPerSample <= 16 && pstSubFrameLPC->m_uiQCoeffPrecision <= 16 ) 
			{
				if( uiOrder <= 8 )
				{
					if(uiOrder == 1 || !(uiOrder%2))
					{
						pstDecoder->m_pstPrivate->m_pfLpcRestoreSignal16bitOrder8( pstSubFrameLPC->m_piResidual, 
																				   pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBlockSize-uiOrder, 
																				   pstSubFrameLPC->m_iQCoeff, 
																				   uiOrder, 
																				   pstSubFrameLPC->m_iQuantizationLevel, 
																				   pstDecoder->m_pstPrivate->m_piOutput[uiChannel]+uiOrder );
					}
					else
					{
						pstDecoder->m_pstPrivate->m_pfLpcRestoreSignal_org( pstSubFrameLPC->m_piResidual, 
																				   pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBlockSize-uiOrder, 
																				   pstSubFrameLPC->m_iQCoeff, 
																				   uiOrder, 
																				   pstSubFrameLPC->m_iQuantizationLevel, 
																				   pstDecoder->m_pstPrivate->m_piOutput[uiChannel]+uiOrder );
					}
				}
				else
				{
					if(uiOrder == 1 || !(uiOrder%2))
					{
						pstDecoder->m_pstPrivate->m_pfLpcRestoreSignal16bit( pstSubFrameLPC->m_piResidual, 
																			 pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBlockSize-uiOrder, 
																			 pstSubFrameLPC->m_iQCoeff, 
																			 uiOrder, 
																			 pstSubFrameLPC->m_iQuantizationLevel, 
																			 pstDecoder->m_pstPrivate->m_piOutput[uiChannel]+uiOrder );
					}
					else
					{
						pstDecoder->m_pstPrivate->m_pfLpcRestoreSignal_org( pstSubFrameLPC->m_piResidual, 
																			 pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBlockSize-uiOrder, 
																			 pstSubFrameLPC->m_iQCoeff, 
																			 uiOrder, 
																			 pstSubFrameLPC->m_iQuantizationLevel, 
																			 pstDecoder->m_pstPrivate->m_piOutput[uiChannel]+uiOrder );
					}
				}
			}
			else
			{
				if(uiOrder == 1 || !(uiOrder%2))
				{
					pstDecoder->m_pstPrivate->m_pfLpcRestoreSignal( pstSubFrameLPC->m_piResidual, 
																	pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBlockSize-uiOrder, 
																	pstSubFrameLPC->m_iQCoeff, 
																	uiOrder, 
																	pstSubFrameLPC->m_iQuantizationLevel, 
																	pstDecoder->m_pstPrivate->m_piOutput[uiChannel]+uiOrder );
				}
				else
				{
					pstDecoder->m_pstPrivate->m_pfLpcRestoreSignal_org( pstSubFrameLPC->m_piResidual, 
																	pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBlockSize-uiOrder, 
																	pstSubFrameLPC->m_iQCoeff, 
																	uiOrder, 
																	pstSubFrameLPC->m_iQuantizationLevel, 
																	pstDecoder->m_pstPrivate->m_piOutput[uiChannel]+uiOrder );
				}
			}
		}
		else
		{
			pstDecoder->m_pstPrivate->m_pfLpcRestoreSignal64bit( pstSubFrameLPC->m_piResidual, 
																 pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBlockSize-uiOrder, 
																 pstSubFrameLPC->m_iQCoeff, 
																 uiOrder, 
																 pstSubFrameLPC->m_iQuantizationLevel, 
																 pstDecoder->m_pstPrivate->m_piOutput[uiChannel]+uiOrder );
		}
	}

	return true;
}

static TCAS_S32 ReadSubFrameVerbatim( TCC_FLAC_StreamDecoder *pstDecoder, 
									  TCAS_U32 uiChannel, 
									  TCAS_U32 uiBitsPerSample, 
									  TCAS_S32 iFullDecodingFlag 
)
{
	TCC_FLAC_Subframe_Verbatim *pstSubFrameVerbatim = &pstDecoder->m_pstPrivate->m_stFrame.m_stSubFrames[uiChannel].m_unData.m_stVerbatim;
	TCAS_S32 iGetData, *piResidual = pstDecoder->m_pstPrivate->m_piResidual[uiChannel];
	TCAS_U32 i;

	pstDecoder->m_pstPrivate->m_stFrame.m_stSubFrames[uiChannel].m_eType = TCC_FLAC_SUBFRAME_TYPE_VERBATIM;

	pstSubFrameVerbatim->m_piData = piResidual;

	for( i = 0; i < pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBlockSize; i++ ) 
	{
		if(!TCC_FLACDEC_ReadS32(pstDecoder->m_pstPrivate->m_pstInputBit, &iGetData, uiBitsPerSample))
		{
			return false;
		}
		piResidual[i] = iGetData;
	}

	/* decode the subframe */
	if( iFullDecodingFlag )
	{
		pstDecoder->m_pCallback->m_pfMemcpy(pstDecoder->m_pstPrivate->m_piOutput[uiChannel], pstSubFrameVerbatim->m_piData, sizeof(TCAS_S32) * pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBlockSize);
	}

	return true;
}

static TCAS_S32 ReadResidualPartitionedRice( TCC_FLAC_StreamDecoder *pstDecoder, 
											 TCAS_U32 uiPredictorOrder, 
											 TCAS_U32 uiPartitionOrder, 
											 /*TCC_FLAC_EntropyCodingMethod_PartitionedRiceContents *partitioned_rice_contents,*/ 
											 TCAS_S32 *piResidual,
											 TCAS_S32 iIsExtended
)
{
	TCAS_U32 uiRiceParam;
	TCAS_S32 i;
	TCAS_U32 uiPartition, uiSample, uiUtmp;

	const TCAS_U32 uiPartitions = 1u << uiPartitionOrder;
	const TCAS_U32 uiPartitionSamples = uiPartitionOrder > 0 ? pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBlockSize >> uiPartitionOrder : pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBlockSize - uiPredictorOrder;
	const TCAS_U32 uiPlen = iIsExtended ? TCC_FLAC_ENTROPY_CODING_METHOD_PARTITIONED_RICE2_PARAMETER_LEN : TCC_FLAC_ENTROPY_CODING_METHOD_PARTITIONED_RICE_PARAMETER_LEN;
	const TCAS_U32 uiPesc = iIsExtended ? TCC_FLAC_ENTROPY_CODING_METHOD_PARTITIONED_RICE2_ESCAPE_PARAMETER : TCC_FLAC_ENTROPY_CODING_METHOD_PARTITIONED_RICE_ESCAPE_PARAMETER;


	if( uiPartitionOrder == 0 ) 
	{
		if(pstDecoder->m_pstPrivate->m_stFrame.m_stHeader.m_uiBlockSize < uiPredictorOrder) 
		{
			//send_error_to_client_(pstDecoder, TCC_FLAC_STREAM_DECODER_ERROR_STATUS_LOST_SYNC);
			pstDecoder->m_pstProtected->m_eStateProtected = TCC_FLAC_STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
			return true;
		}
	}
	else 
	{
		if(uiPartitionSamples < uiPredictorOrder) 
		{
			//send_error_to_client_(pstDecoder, TCC_FLAC_STREAM_DECODER_ERROR_STATUS_LOST_SYNC);
			pstDecoder->m_pstProtected->m_eStateProtected = TCC_FLAC_STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
			return true;
		}
	}

	//if(!TCC_FLAC_format_entropy_coding_method_partitioned_rice_contents_ensure_size(&pstDecoder->reallocmem, partitioned_rice_contents, max(6, uiPartitionOrder))) {
	//	pstDecoder->m_pstProtected->m_eStateProtected = TCC_FLAC_STREAM_DECODER_MEMORY_ALLOCATION_ERROR;
	//	return false;
	//}

	uiSample = 0;

	for( uiPartition = 0; uiPartition < uiPartitions; uiPartition++ ) 
	{
		if(!TCC_FLACDEC_ReadU32(pstDecoder->m_pstPrivate->m_pstInputBit, &uiRiceParam, uiPlen))
		{
			return false;
		}
		//partitioned_rice_contents->parameters[uiPartition] = uiRiceParam;
		if( uiRiceParam < uiPesc ) 
		{
			uiUtmp = (uiPartitionOrder == 0 || uiPartition > 0)? uiPartitionSamples : uiPartitionSamples - uiPredictorOrder;
			
			if(!pstDecoder->m_pstPrivate->m_pfReadRiceSignedBlock(pstDecoder->m_pstPrivate->m_pstInputBit, piResidual + uiSample, uiUtmp, uiRiceParam))
			{
				return false;
			}
			uiSample += uiUtmp;
		}
		else 
		{
			if(!TCC_FLACDEC_ReadU32(pstDecoder->m_pstPrivate->m_pstInputBit, &uiRiceParam, TCC_FLAC_ENTROPY_CODING_METHOD_PARTITIONED_RICE_RAW_LEN))
			{
				return false;
			}

			//partitioned_rice_contents->raw_bits[uiPartition] = uiRiceParam;
			for( uiUtmp = (uiPartitionOrder == 0 || uiPartition > 0)? 0 : uiPredictorOrder; uiUtmp < uiPartitionSamples; uiUtmp++, uiSample++ ) 
			{
				if(!TCC_FLACDEC_ReadS32(pstDecoder->m_pstPrivate->m_pstInputBit, &i, uiRiceParam))
				{
					return false;
				}

				piResidual[uiSample] = i;
			}
		}
	}

	return true;
}

static TCAS_S32 ReadZeroPadding( TCC_FLAC_StreamDecoder *pstDecoder )
{
	if( !TCC_FLACDEC_CheckByteAlignedConsumed( pstDecoder->m_pstPrivate->m_pstInputBit ) ) 
	{
		TCAS_U32 zero = 0;
		if(!TCC_FLACDEC_ReadU32( pstDecoder->m_pstPrivate->m_pstInputBit, &zero, TCC_FLACDEC_BitsLeftForByteAlign(pstDecoder->m_pstPrivate->m_pstInputBit)))
		{
			return false;
		}

		if(zero != 0) 
		{
			//send_error_to_client_(pstDecoder, TCC_FLAC_STREAM_DECODER_ERROR_STATUS_LOST_SYNC);
			pstDecoder->m_pstProtected->m_eStateProtected = TCC_FLAC_STREAM_DECODER_SEARCH_FOR_FRAME_SYNC;
		}
	}
	return true;
}


TCAS_S32 TCC_FLAC_Fill_Buff(TCAS_U8 buffer[], TCAS_U32 *uiBytes, TCASVoid *client_data, stBufferInfo *pbuff)
{
	TCC_FLAC_StreamDecoder *pstDecoder = (TCC_FLAC_StreamDecoder *)client_data;

	if((*uiBytes > 0) && (pbuff->m_iBytesLeft <= 0))
	{
		*uiBytes = 0;
		pstDecoder->m_pstProtected->m_eStateProtected = TCC_FLAC_STREAM_DECODER_ABORTED; //TCC_FLAC_STREAM_DECODER_END_OF_STREAM
		return false;
	}

	if((TCAS_S32)*uiBytes > pbuff->m_iBytesLeft){
		*uiBytes = pbuff->m_iBytesLeft;
	}

	if(*uiBytes > 0)
	{
		pstDecoder->m_pCallback->m_pfMemcpy(buffer, pbuff->m_puCurrPtr, *uiBytes);
		pbuff->m_puCurrPtr += *uiBytes;
		pbuff->m_iBytesLeft -= *uiBytes;

		return true;
	}

	return false;
}

static TCC_FLAC_StreamDecoderWriteStatus write_audio_frame_to_client_(TCC_FLAC_StreamDecoder *pstDecoder, const TCC_FLAC_Frame *pstFrame, const TCAS_S32 * const buffer[])
{

#ifdef MD5_CHECKING
	if(!pstDecoder->m_pstPrivate->m_iHasStreamInfo)
	{
		pstDecoder->m_pstPrivate->m_iMd5CheckFlag = false;
	}
	if(pstDecoder->m_pstPrivate->m_iMd5CheckFlag) 
	{
		if(!TCC_FLAC_MD5Accumulate(&pstDecoder->m_pstPrivate->md5context, buffer, pstFrame->m_stHeader.m_uiChannels, pstFrame->m_stHeader.m_uiBlockSize, (pstFrame->m_stHeader.m_uiBitsPerSample+7) / 8))
		{
			return TCC_FLAC_STREAM_DECODER_WRITE_STATUS_ABORT;
		}
	}
#endif
	return pstDecoder->m_pstPrivate->m_pfWriteCallback(pstDecoder, pstFrame, buffer, pstDecoder->m_pstPrivate->m_pvClientDataPrivate );
	//return 0;
}

TCAS_S32 FLAC_Decoder_Setbuffer(TCC_FLAC_StreamDecoder *pstDecoder, flac_callback_t *pCallback)
{
	TCAS_U32 i;

	for( i = 0; i < pstDecoder->m_uiMaxCh; i++ )
	{
		pstDecoder->m_pstPrivate->m_piOutput[i] = pCallback->m_pfMalloc( pstDecoder->m_uiMaxBlock * sizeof(TCAS_S32) );
		if( pstDecoder->m_pstPrivate->m_piOutput[i] == NULL )
		{
			return -1;
		}
	}

	for( i = 0; i < pstDecoder->m_uiMaxCh; i++ ) 
	{
		pstDecoder->m_pstPrivate->m_piResidual[i] = (TCAS_S32 *)pCallback->m_pfMalloc( pstDecoder->m_uiMaxBlock * sizeof(TCAS_S32) );
		if( pstDecoder->m_pstPrivate->m_piResidual[i] == NULL )
		{
			pstDecoder->m_pstProtected->m_eStateProtected = TCC_FLAC_STREAM_DECODER_MEMORY_ALLOCATION_ERROR;
			return -1;
		}

		pstDecoder->m_pstPrivate->m_iResidualUnaligned[i] = pstDecoder->m_pstPrivate->m_piResidual[i];
	}

	pstDecoder->m_pstPrivate->m_uiOutputCapacity = pstDecoder->m_uiMaxBlock;
	pstDecoder->m_pstPrivate->m_uiOutputChannels = pstDecoder->m_uiMaxCh;


#ifdef MD5_CHECKING
	if( pstDecoder->m_pstProtected->m_iMd5CheckingProtected )
	{
		TCC_FLAC_MD5Accumulate_alloc(&pstDecoder->m_pstPrivate->md5context);
	}
#endif

	return 0;
}
