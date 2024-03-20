// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (C) Telechips Inc.
 */
/****************************************************************************
 *   FileName    : TCC_FLAC_DEC.h
 *   Description : 
 ****************************************************************************

 ****************************************************************************/

#ifndef TCC_FLAC_DEC_H_
#define TCC_FLAC_DEC_H_

#ifndef NULL
#define NULL (0)
#endif

typedef void* H_FLAC_DEC;

typedef enum
{
	FLAC_CH1_MONO		= 0,	//1 channel: mono
	FLAC_CH2_STEREO		= 1,	//2 channels: left, right
	FLAC_CH2_LEFT_SIDE	= 2,	//left/side stereo: channel 0 is the left channel, channel 1 is the side(difference) channel
	FLAC_CH2_RIGHT_SIDE	= 3,	//right/side stereo: channel 0 is the side(difference) channel, channel 1 is the right channel 
	FLAC_CH2_MID_SIDE	= 4,	//mid/side stereo: channel 0 is the mid(average) channel, channel 1 is the side(difference) channel 
	FLAC_CH3_3F			= 5,	//3 channels: left, right, center 
	FLAC_CH4_2F2R		= 6,	//4 channels: left, right, back left, back right 
	FLAC_CH5_3F2R		= 7,	//5 channels: left, right, center, back/surround left, back/surround right 
	FLAC_CH6_3F_LFE_2R	= 8,	//6 channels: left, right, center, LFE, back/surround left, back/surround right
	FLAC_CH7			= 9,	//7 channels: not defined
	FLAC_CH8			= 10,	//8 channels: not defined 
	FLAC_CH_UNKNOWN		= 0x7fffffff
}TCC_FLAC_Channel_Type;

typedef struct {
	TCAS_U32 min_block_size;	/* <16> The minimum block size (in samples) used in the stream. */
	TCAS_U32 max_block_size;	/* <16> The maximum block size (in samples) used in the stream. */
	TCAS_U32 min_frame_size;	/* <24> The minimum frame size (in uiBytes) used in the stream. */
	TCAS_U32 max_frame_size;	/* <24> The maximum frame size (in uiBytes) used in the stream. */
	TCAS_U32 sample_rate;		/* <20> Sample rate in Hz. */
	TCAS_U32 num_channels;		/* <3>  (number of channels)-1. FLAC supports from 1 to 8 channels */
	TCAS_U32 bits_per_sample;	/* <5>  (bits per sample)-1. */  
	TCAS_S64 total_samples;		/* <36>  Total samples in stream. */
	TCAS_U8 MD5_signature[128/8];	/* <128>  MD5 signature of the unencoded audio data. */ 
	
} FLACMetaData_StreamInfo;

typedef enum {
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
} TCC_FLAC_DecoderState;


#define	FLAC_DEC_SEEK	(5)

#define FILE_DECODING		(0)
#define BUFFER_DECODING		(1)

TCAS_ERROR_TYPE
TCC_FLAC_DEC( TCAS_U32 Op, H_FLAC_DEC* pHandle, TCASVoid* pParam1, TCASVoid* pParam2 );

#endif	//TCC_FLAC_DEC_H_

/* uiEnd of file */
