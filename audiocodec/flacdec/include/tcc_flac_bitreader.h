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

#ifndef TCC_FLAC_BITREADER_H__
#define TCC_FLAC_BITREADER_H__

struct TCC_FLACDEC_BitParser;
typedef struct TCC_FLACDEC_BitParser TCC_FLACDEC_BitParser;

typedef TCAS_S32 (*TCC_FLAC_BitReaderReadCallback)(TCAS_U8 buffer[], TCAS_U32 *uiBytes, TCASVoid *client_data);

TCC_FLACDEC_BitParser *TCC_FLACDEC_CreateBitParser(flac_callback_t *pCallback);
TCASVoid TCC_FLACDEC_DestroyBitParser(TCC_FLACDEC_BitParser *pstBitParser, flac_callback_t *pCallback);
TCAS_S32 TCC_FLACDEC_InitBitParser(TCC_FLACDEC_BitParser *pstBitParser, TCASVoid *cd );
TCAS_S32 TCC_FLACDEC_ResetBitParser(TCC_FLACDEC_BitParser *pstBitParser);

/* CRC functions */
TCASVoid TCC_FLACDEC_ResetReadCRC16(TCC_FLACDEC_BitParser *pstBitParser, TCAS_U16 usSeed);
TCAS_U16 TCC_FLACDEC_GetReadCRC16(TCC_FLACDEC_BitParser *pstBitParser);

/* info functions */
TCAS_S32 TCC_FLACDEC_CheckByteAlignedConsumed(const TCC_FLACDEC_BitParser *pstBitParser);
TCAS_U32 TCC_FLACDEC_BitsLeftForByteAlign(const TCC_FLACDEC_BitParser *pstBitParser);
TCAS_U32 TCC_FLACDEC_GetUnconsumedBits(const TCC_FLACDEC_BitParser *pstBitParser);

/* read functions */
TCAS_S32 TCC_FLACDEC_ReadU32(TCC_FLACDEC_BitParser *pstBitParser, TCAS_U32 *val, TCAS_U32 bits);
TCAS_S32 TCC_FLACDEC_ReadS32(TCC_FLACDEC_BitParser *pstBitParser, TCAS_S32 *val, TCAS_U32 bits);
TCAS_S32 TCC_FLACDEC_ReadU64(TCC_FLACDEC_BitParser *pstBitParser, TCAS_U64 *val, TCAS_U32 bits);
TCAS_S32 TCC_FLACDEC_ReadUUnary(TCC_FLACDEC_BitParser *pstBitParser, TCAS_U32 *val);
TCAS_S32 TCC_FLACDEC_ReadRiceSignedBlock(TCC_FLACDEC_BitParser *pstBitParser, TCAS_S32 vals[], TCAS_U32 uiNumValues, TCAS_U32 parameter);
TCAS_S32 TCC_FLACDEC_ReadU32UFT8(TCC_FLACDEC_BitParser *pstBitParser, TCAS_U32 *val, TCAS_U8 *raw, TCAS_U32 *rawlen);
TCAS_S32 TCC_FLACDEC_ReadU64UFT8(TCC_FLACDEC_BitParser *pstBitParser, TCAS_U64 *val, TCAS_U8 *raw, TCAS_U32 *rawlen);
TCAS_S32 TCC_FLACDEC_SetBitBuffer(TCC_FLACDEC_BitParser *pstBitParser, TCAS_S8 **pbuff, TCAS_S32 *length);
TCAS_S32 TCC_FLACDEC_GetBitBufferState(TCC_FLACDEC_BitParser *pstBitParser, TCAS_S8 **pbuff, TCAS_S32 *length);

typedef struct 
{
	TCAS_U8 *m_puCurrPtr;
	TCAS_S32 m_iBytesLeft;
}stBufferInfo;

#endif	/* TCC_FLAC_BITREADER_H__ */
