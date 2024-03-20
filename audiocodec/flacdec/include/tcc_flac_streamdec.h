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


#ifndef TCC_FLAC_STREAM_DECODER_H__
#define TCC_FLAC_STREAM_DECODER_H__

#include "tcc_flac_format.h"

#ifdef __cplusplus
extern "C" {
#endif

/* State values for a TCC_FLAC_StreamDecoder The decoder's state can be obtained by calling TCC_FLAC_stream_decoder_get_state(). */
typedef enum 
{
	TCC_FLAC_STREAM_DECODER_SEARCH_FOR_METADATA = 0,	/* decoder is ready to search for metadata */
	TCC_FLAC_STREAM_DECODER_READ_METADATA,				/* decoder is ready to or is in the process of reading metadata */
	TCC_FLAC_STREAM_DECODER_SEARCH_FOR_FRAME_SYNC,		/* decoder is ready to or is in the process of searching for the frame sync code */

	TCC_FLAC_STREAM_DECODER_READ_FRAME,					/* decoder is ready to or is in the process of reading a frame */
	TCC_FLAC_STREAM_DECODER_END_OF_STREAM,				/* decoder has reached the uiEnd of the stream */
	TCC_FLAC_STREAM_DECODER_OGG_ERROR,					/* error occurred in the underlying Ogg layer */
	TCC_FLAC_STREAM_DECODER_SEEK_ERROR,					/* error occurred while seeking */
	TCC_FLAC_STREAM_DECODER_ABORTED,					/* decoder was aborted by the read callback */

	TCC_FLAC_STREAM_DECODER_MEMORY_ALLOCATION_ERROR,	/* error occurred allocating memory */
	TCC_FLAC_STREAM_DECODER_UNINITIALIZED				/* decoder is in the uninitialized state */
} TCC_FLAC_StreamDecoderState;

/* return values for the TCC_FLAC_stream_decoder_init_*() */
typedef enum 
{
	TCC_FLAC_STREAM_DECODER_INIT_STATUS_OK = 0,					/* Initialization was successful */
	TCC_FLAC_STREAM_DECODER_INIT_STATUS_UNSUPPORTED_CONTAINER,	/* library was not compiled with support for the given container format */
	TCC_FLAC_STREAM_DECODER_INIT_STATUS_INVALID_CALLBACKS,		/* required callback was not supplied */
	TCC_FLAC_STREAM_DECODER_INIT_STATUS_MEMORY_ALLOCATION_ERROR,/* error occurred allocating memory */
	TCC_FLAC_STREAM_DECODER_INIT_STATUS_ALREADY_INITIALIZED		/* already initialized */
} TCC_FLAC_StreamDecoderInitStatus;

/* Return values for the TCC_FLAC_StreamDecoder read callback. */
typedef enum 
{
	TCC_FLAC_STREAM_DECODER_READ_STATUS_CONTINUE,	/* read was OK and decoding can continue */
	TCC_FLAC_STREAM_DECODER_READ_STATUS_END_OF_STREAM,	/* read was attempted while at the uiEnd of the stream */
	TCC_FLAC_STREAM_DECODER_READ_STATUS_ABORT	/* unrecoverable error occurred.  The decoder will return from the process call */
} TCC_FLAC_StreamDecoderReadStatus;


/* Return values for the TCC_FLAC_StreamDecoder seek callback */
typedef enum 
{
	TCC_FLAC_STREAM_DECODER_SEEK_STATUS_OK,	/* seek was OK and decoding can continue */
	TCC_FLAC_STREAM_DECODER_SEEK_STATUS_ERROR,	/* unrecoverable error occurred.  The decoder will return from the process call */
	TCC_FLAC_STREAM_DECODER_SEEK_STATUS_UNSUPPORTED	/* Client does not support seeking */
} TCC_FLAC_StreamDecoderSeekStatus;


/* Return values for the TCC_FLAC_StreamDecoder tell callback */
typedef enum 
{
	TCC_FLAC_STREAM_DECODER_TELL_STATUS_OK,	/* tell was OK and decoding can continue */
	TCC_FLAC_STREAM_DECODER_TELL_STATUS_ERROR,	/* unrecoverable error occurred.  The decoder will return from the process call. */
	TCC_FLAC_STREAM_DECODER_TELL_STATUS_UNSUPPORTED	/* Client does not support telling the position. */
} TCC_FLAC_StreamDecoderTellStatus;


/* Return values for the TCC_FLAC_StreamDecoder length callback */
typedef enum 
{
	TCC_FLAC_STREAM_DECODER_LENGTH_STATUS_OK,	/* length call was OK and decoding can continue */
	TCC_FLAC_STREAM_DECODER_LENGTH_STATUS_ERROR,	/* unrecoverable error occurred.  The decoder will return from the process call */
	TCC_FLAC_STREAM_DECODER_LENGTH_STATUS_UNSUPPORTED	/* Client does not support reporting the length */
} TCC_FLAC_StreamDecoderLengthStatus;

/* Return values for the TCC_FLAC_StreamDecoder write callback. */
typedef enum 
{
	TCC_FLAC_STREAM_DECODER_WRITE_STATUS_CONTINUE,	/* write was OK and decoding can continue */
	TCC_FLAC_STREAM_DECODER_WRITE_STATUS_ABORT	/* unrecoverable error occurred.  The decoder will return from the process call */
} TCC_FLAC_StreamDecoderWriteStatus;


typedef enum 
{
	TCC_FLAC_STREAM_DECODER_ERROR_STATUS_LOST_SYNC,	/* error in the stream caused the decoder to lose synchronization */
	TCC_FLAC_STREAM_DECODER_ERROR_STATUS_BAD_HEADER,	/* decoder encountered a corrupted frame header */
	TCC_FLAC_STREAM_DECODER_ERROR_STATUS_FRAME_CRC_MISMATCH, /* frame's data did not match the CRC in the footer */
	TCC_FLAC_STREAM_DECODER_ERROR_STATUS_UNPARSEABLE_STREAM	/* decoder encountered reserved fields in use in the stream */
} TCC_FLAC_StreamDecoderErrorStatus;


struct TCC_FLAC_StreamDecoderProtected;
struct TCC_FLAC_StreamDecoderPrivate;

typedef struct 
{
	struct TCC_FLAC_StreamDecoderProtected *m_pstProtected; /* avoid the C++ keyword 'protected' */
	struct TCC_FLAC_StreamDecoderPrivate *m_pstPrivate;		/* avoid the C++ keyword 'private' */
	flac_callback_t *m_pCallback;
	TCAS_U32		m_uiTCCxxxx;	//protection
	TCAS_U32		m_uiMaxCh;
	TCAS_U32		m_uiMaxBlock;

	TCAS_U32		m_uiBitsPerSample;
	TCAS_U32		m_uiChannels;
	TCAS_U32		m_uiSampleRate;

} TCC_FLAC_StreamDecoder;


typedef TCC_FLAC_StreamDecoderWriteStatus (*TCC_FLAC_StreamDecoderWriteCallback)(const TCC_FLAC_StreamDecoder *pstDecoder, const TCC_FLAC_Frame *pstFrame, TCAS_S32 * buffer[], TCASVoid *client_data );

TCC_FLAC_StreamDecoder *TCC_FLAC_stream_decoder_new(flac_callback_t *pCallback);
TCASVoid TCC_FLAC_stream_decoder_delete(TCC_FLAC_StreamDecoder *pstDecoder, flac_callback_t *pCallback);
TCAS_S32 TCC_FLAC_stream_decoder_parse_streaminfo(TCC_FLAC_StreamDecoder *pstDecoder, TCAS_S8 *pExtraData, TCAS_S32 length);
TCC_FLAC_StreamDecoderState TCC_FLAC_stream_decoder_get_state(const TCC_FLAC_StreamDecoder *pstDecoder);
#ifdef MD5_CHECKING
TCAS_S32 TCC_FLAC_stream_decoder_set_md5_checking(TCC_FLAC_StreamDecoder *pstDecoder, TCAS_S32 value);
TCAS_S32 TCC_FLAC_stream_decoder_get_md5_checking(const TCC_FLAC_StreamDecoder *pstDecoder);
#endif
TCAS_U64 TCC_FLAC_stream_decoder_get_total_samples(const TCC_FLAC_StreamDecoder *pstDecoder);

#if 0
TCAS_U32 TCC_FLAC_stream_decoder_get_channels(const TCC_FLAC_StreamDecoder *pstDecoder);
TCC_FLAC_ChannelAssignment TCC_FLAC_stream_decoder_get_channel_assignment(const TCC_FLAC_StreamDecoder *pstDecoder);
TCAS_U32 TCC_FLAC_stream_decoder_get_bits_per_sample(const TCC_FLAC_StreamDecoder *pstDecoder);
TCAS_U32 TCC_FLAC_stream_decoder_get_sample_rate(const TCC_FLAC_StreamDecoder *pstDecoder);
TCAS_U32 TCC_FLAC_stream_decoder_get_blocksize(const TCC_FLAC_StreamDecoder *pstDecoder);
#endif

TCC_FLAC_StreamDecoderInitStatus TCC_FLAC_stream_decoder_init(
	TCC_FLAC_StreamDecoder *pstDecoder,
	TCC_FLAC_StreamDecoderWriteCallback write_callback,
	TCASVoid *client_data
);

TCAS_S32 TCC_FLAC_stream_decoder_flush(TCC_FLAC_StreamDecoder *pstDecoder);
TCAS_S32 TCC_FLAC_stream_decoder_reset(TCC_FLAC_StreamDecoder *pstDecoder);
TCAS_S32 TCC_FLAC_stream_decoder_process_single(TCC_FLAC_StreamDecoder *pstDecoder);


#ifdef __cplusplus
}
#endif

#endif	/* TCC_FLAC_STREAM_DECODER_H__ */
