
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

#ifndef TCC_FLAC_FORMAT_H__
#define TCC_FLAC_FORMAT_H__

#ifdef __cplusplus
extern "C" {
#endif

#define TCC_FLAC_MAX_BLOCK_SIZE (65535u)	/* maximum block size, in samples, permitted by the format */
#define TCC_FLAC_SUBSET_MAX_BLOCK_SIZE_48000HZ (4608u)	/* maximum block size, in samples, permitted by the FLAC subset for sample rates up to 48kHz. */
#define TCC_FLAC_MAX_CHANNELS (8u)	/* maximum number of channels permitted by the format */
//#define TCC_FLAC_MAX_CHANNELS (2u)

#define TCC_FLAC_MAX_LPC_ORDER (32u)					/* maximum LPC order permitted by the format */
#define TCC_FLAC_SUBSET_MAX_LPC_ORDER_48000HZ (12u)		/* maximum LPC order permitted by the FLAC subset for sample rates up to 48kHz */
#define TCC_FLAC_MIN_QLP_COEFF_PRECISION (5u)			/* minimum quantized linear predictor coefficient precision permitted by the format */
#define TCC_FLAC_MAX_QLP_COEFF_PRECISION (15u)			/* The maximum quantized linear predictor coefficient precision permitted by the format */
#define TCC_FLAC_MAX_FIXED_ORDER (4u)					/* maximum order of the fixed predictors permitted by the format */
#define TCC_FLAC_MAX_RICE_PARTITION_ORDER (15u)			/* maximum Rice partition order permitted by the format */
#define TCC_FLAC_SUBSET_MAX_RICE_PARTITION_ORDER (8u)	/* maximum Rice partition order permitted by the FLAC Subset */
#define TCC_FLAC_STREAM_SYNC_LENGTH (4u)				/* length of the FLAC signature in uiBytes */


/*****************************************************************************
 * Subframe structures
 *****************************************************************************/

/*****************************************************************************/

/* entropy coding methods. */
typedef enum 
{
	TCC_FLAC_ENTROPY_CODING_METHOD_PARTITIONED_RICE = 0,
	TCC_FLAC_ENTROPY_CODING_METHOD_PARTITIONED_RICE2 = 1
} TCC_FLAC_EntropyCodingMethodType;


/* Header for a Rice partitioned residual */
typedef struct 
{

	TCAS_U32	m_uiOrder; /* partition order, i.e. # of contexts = 2 ^ \a order. */

} TCC_FLAC_EntropyCodingMethodPartitionedRice;

/* Header for the entropy coding method */
typedef struct 
{
	TCC_FLAC_EntropyCodingMethodType				m_eType;
	union 
	{
		TCC_FLAC_EntropyCodingMethodPartitionedRice	m_stPartitionedRice;
	} m_unData;

} TCC_FLAC_EntropyCodingMethod;

extern const TCAS_U32 TCC_FLAC_ENTROPY_CODING_METHOD_TYPE_LEN; /* == 2 (bits) */

/*****************************************************************************/

/* subframe types. */
typedef enum 
{
	TCC_FLAC_SUBFRAME_TYPE_CONSTANT = 0, /* constant signal */
	TCC_FLAC_SUBFRAME_TYPE_VERBATIM = 1, /* uncompressed signal */
	TCC_FLAC_SUBFRAME_TYPE_FIXED = 2, /* fixed polynomial prediction */
	TCC_FLAC_SUBFRAME_TYPE_LPC = 3 /* linear prediction */
} TCC_FLAC_SubframeType;

/* CONSTANT subframe */
typedef struct 
{
	TCAS_S32	m_iValue; /* constant signal value */
} TCC_FLAC_Subframe_Constant;


/* VERBATIM subframe */
typedef struct 
{
	const TCAS_S32 *m_piData; /* pointer to verbatim signal */
} TCC_FLAC_Subframe_Verbatim;


/* FIXED subframe */
typedef struct 
{
	TCC_FLAC_EntropyCodingMethod m_stEntropyCodingMethod;	/* residual coding method */

	TCAS_U32		m_uiOrder;	/* polynomial order */

	TCAS_S32		m_iWarmUp[TCC_FLAC_MAX_FIXED_ORDER];	/* Warmup samples to prime the predictor, length == order */

	const TCAS_S32	*m_piResidual;	/* residual signal, length == (blocksize minus order) samples */
} TCC_FLAC_Subframe_Fixed;


/* LPC subframe */
typedef struct 
{
	TCC_FLAC_EntropyCodingMethod m_stEntropyCodingMethod;	/* residual coding method */
	
	TCAS_U32 m_uiOrder;				/* FIR order */
	TCAS_U32 m_uiQCoeffPrecision;	/* Quantized FIR filter coefficient precision in bits */
	TCAS_S32 m_iQuantizationLevel;	/* qlp coeff shift needed. */
	
	TCAS_S32 m_iQCoeff[TCC_FLAC_MAX_LPC_ORDER];	/* FIR filter coefficients */
	TCAS_S32 m_iWarmUp[TCC_FLAC_MAX_LPC_ORDER];	/* Warmup samples to prime the predictor, length == order */

	const TCAS_S32 *m_piResidual;	/* residual signal, length == (blocksize minus order) samples */

} TCC_FLAC_Subframe_LPC;

/* FLAC subframe structure */
typedef struct 
{
	TCC_FLAC_SubframeType			m_eType;
	union 
	{
		TCC_FLAC_Subframe_Constant	m_stConstant;
		TCC_FLAC_Subframe_Fixed		m_stFixed;
		TCC_FLAC_Subframe_LPC		m_stLPC;
		TCC_FLAC_Subframe_Verbatim	m_stVerbatim;
	} m_unData;

	TCAS_U32						m_uiWastedBits;
} TCC_FLAC_Subframe;

/*****************************************************************************/


/*****************************************************************************
 * Frame structures
 *****************************************************************************/

/* available channel assignments */
typedef enum 
{
	TCC_FLAC_CHANNEL_ASSIGNMENT_INDEPENDENT = 0, /**< independent channels */
	TCC_FLAC_CHANNEL_ASSIGNMENT_LEFT_SIDE = 1, /**< left+side stereo */
	TCC_FLAC_CHANNEL_ASSIGNMENT_RIGHT_SIDE = 2, /**< right+side stereo */
	TCC_FLAC_CHANNEL_ASSIGNMENT_MID_SIDE = 3 /**< mid+side stereo */
} TCC_FLAC_ChannelAssignment;


/* possible frame numbering methods */
typedef enum 
{
	TCC_FLAC_FRAME_NUMBER_TYPE_FRAME_NUMBER, /* number contains the frame number */
	TCC_FLAC_FRAME_NUMBER_TYPE_SAMPLE_NUMBER /* number contains the sample number of first sample in frame */
} TCC_FLAC_FrameNumberType;

/* FLAC frame header structure. */
typedef struct 
{
	TCAS_U32					m_uiBlockSize;			/* number of samples per subframe */
	TCAS_U32					m_uiSampleRate;			/* sample rate in Hz */
	TCAS_U32					m_uiChannels;			/* number of channels (== number of subframes) */
	TCC_FLAC_ChannelAssignment	m_eChannelAssignment;	/* channel assignment for the frame */
	TCAS_U32					m_uiBitsPerSample;		/* sample resolution */
	TCC_FLAC_FrameNumberType	m_eNumberType;			/* numbering scheme used for the frame */

	union 
	{
		TCAS_U32				m_uiFrameNumber;
		TCAS_U64				m_ulSampleNumber;
	} m_uNumber;

	TCAS_U8						m_ucCRC;

} TCC_FLAC_FrameHeader;


/* FLAC frame footer structure. */
typedef struct 
{
	TCAS_U16					m_usCRC;
} TCC_FLAC_FrameFooter;

/* FLAC frame structure. */
typedef struct 
{
	TCC_FLAC_FrameHeader		m_stHeader;
	TCC_FLAC_Subframe			m_stSubFrames[TCC_FLAC_MAX_CHANNELS];
//	TCC_FLAC_FrameFooter m_stFooter;
} TCC_FLAC_Frame;

/* FLAC STREAMINFO structure. */
typedef struct 
{
	TCAS_U32 m_uiMinBlockSize;
	TCAS_U32 m_uiMaxBlockSize;
	TCAS_U32 m_uiMinFrameSize;
	TCAS_U32 m_uiMaxFrameSize;
	TCAS_U32 m_uiSampleRate;
	TCAS_U32 m_uiChannels;
	TCAS_U32 m_uiBitsPerSample;
	TCAS_U64 m_ulTotalSamples;
	TCAS_U8  m_ucMd5Sum[16];
} TCC_FLAC_StreamInfo;

#define TCC_FLAC_STREAM_METADATA_STREAMINFO_LENGTH (34u)	/* total stream length of the STREAMINFO block in bytes. */

/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif	/* TCC_FLAC_FORMAT_H__ */
