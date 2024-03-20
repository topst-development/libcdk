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

#ifndef TCC_FLAC_PT_STREAM_DECODER_H__
#define TCC_FLAC_PT_STREAM_DECODER_H__

#include "tcc_flac_streamdec.h"

typedef struct TCC_FLAC_StreamDecoderProtected 
{
	TCC_FLAC_StreamDecoderState		m_eStateProtected;
	TCAS_U32						m_uiChannelsProtected;
	TCC_FLAC_ChannelAssignment		m_eChannelAssignmentProtected;
	TCAS_U32						m_uiBitsPerSampleProtected;
	TCAS_U32						m_uiSampleRateProtected;		/* in Hz */
	TCAS_U32						m_uiBlockSizeProtected;			/* in samples (per channel) */

#ifdef MD5_CHECKING
	TCAS_S32						m_iMd5CheckingProtected;		/* if true, generate MD5 signature of decoded data and compare against signature in the STREAMINFO metadata block */
#endif

} TCC_FLAC_StreamDecoderProtected;


#endif	/* TCC_FLAC_PT_STREAM_DECODER_H__ */
