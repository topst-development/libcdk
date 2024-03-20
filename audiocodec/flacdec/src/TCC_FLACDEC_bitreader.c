/*
   libFLAC - Free Lossless Audio Codec library
   Copyright (C) 2001-2009  Josh Coalson
   Copyright (C) 2011-2014  Xiph.Org Foundation
   Copyright (C) 2007-2017  Telechips
  
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
  
   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
  
   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
  
   - Neither the name of the Xiph.org Foundation nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.
  
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "tcas_typedef.h"
#include "flac_dec_callback.h"

#include "tcc_flac_bitreader.h"
#include "tcc_flac_crc.h"
#include "tcc_flac_assert.h"


#define TCC_FLAC_BYTESPERWORD (4)
#define TCC_FLAC_BITSPERWORD (32)
#define TCC_FLAC_ALL_ONES ((TCAS_U32)0xffffffff)

#if 1
/* counts the # of zero MSBs in a word */
#define ALIGNED_UNARY_BITS(word) ( \
	((word) <= 0xffff) ? \
		( ((word) <= 0xff) ? (bytes2unary[word] + 24) : (bytes2unary[(word) >> 8] + 16) ) : \
		( ((word) <= 0xffffff) ? (bytes2unary[(word) >> 16] + 8) : (bytes2unary[(word) >> 24]) ) \
)

#endif

#define TCC_FLAC_BITBUFFER_SIZE		( 4608u / TCC_FLAC_BITSPERWORD ) /* in words ( at least 1K uiBytes ) */ //65536u / TCC_FLAC_BITSPERWORD

#if 1
static const TCAS_U8 bytes2unary[] = 
{
	0x08, 0x07, 0x06, 0x06, 0x05, 0x05, 0x05, 0x05, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
#endif 


TCAS_U16 TCC_FLAC_Crc16Table[256];

void CRC16_TableGeneration(TCAS_U16 *puiTable)
{
	TCAS_S32 iDiv;
	TCAS_S32 iBit;
	TCAS_U16 uiRem;

	if(puiTable == NULL) {
		return;
	}
	/* CRC-16 Table Generation */
	for (iDiv = 0; iDiv < 256; iDiv++)
	{
		/* initial value = 0 */
		uiRem = iDiv << 8;

		/* poly = x^16 + x^15 + x^2 + x^0 */
		for (iBit = 8; iBit > 0; iBit--)
		{
			if (uiRem & MSB16)
			{
				uiRem = (uiRem << 1) ^ CRC16_POLYNOMIAL;
			}
			else
			{
				uiRem = (uiRem << 1);
			}  
		}
		puiTable[iDiv] = uiRem; /* save table */
	}
}

/* adjust for compilers that can't understand using LLU suffix for uint64_t literals */
#ifdef _MSC_VER
#define TCC_FLAC_U64L(x) (x)
#else
#define TCC_FLAC_U64L(x) (x##LLU)
#endif

struct TCC_FLACDEC_BitParser 
{
	TCAS_U32		*m_puiBitBuffer;
	TCAS_U32		m_uiCapacity;			/* in words */
	TCAS_U32		m_uiCompletedWords;		/* # of completed words in buffer */
	TCAS_U32		m_uiIncompletedBytes;	/* # of uiBytes in incomplete word at buffer[words] */
	TCAS_U32		m_uiConsumedWords;
	TCAS_U32		m_uiConsumedBits;		/* #words+(#uiNumBits of head word) already consumed from the front of buffer */
	TCAS_U32		m_uiFrameCrc16_BP;		/* the running frame CRC */
	TCAS_U32		m_uiCrc16Align;			/* the number of uiNumBits in the current consumed word that should not be CRC'd */

	TCASVoid		*m_pvClientData;

	stBufferInfo	m_stBuffInfo;

	flac_callback_t	*m_pCallback;

};

/* added by jr-kim START >>>>> */
#ifdef ARM_ADS
static __inline TCAS_S32 TCC_FLAC_CLZ_NZ( TCAS_U32 uiX)
{
	register TCAS_S32 numZeros,tmp;
	
	__asm {
		/* ARM9 */
		mov		numZeros,#1
		movs	tmp,uiX,lsr #16
		addeq	numZeros,numZeros,#16
		moveq	uiX,uiX,lsl #16
		movs	tmp,uiX,lsr #24
		addeq	numZeros,numZeros,#8
		moveq	uiX,uiX,lsl #8
		movs	tmp,uiX,lsr #28
		addeq	numZeros,numZeros,#4
		moveq	uiX,uiX,lsl #4
		movs	tmp,uiX,lsr #30
		addeq	numZeros,numZeros,#2
		moveq	uiX,uiX,lsl #2
		sub		numZeros,numZeros,uiX,lsr #31
		
		/* ARM9E */
		//clz		numZeros,uiX
   	}
   	return numZeros;
}
#else

static __inline TCAS_S32 TCC_FLAC_CLZ_NZ(TCAS_S32 uiX)
{
	register TCAS_S32 iNumZeros;

	/* count leading zeros with binary search (function should be 17 ARM instructions total) */
	iNumZeros = 1;
	if (!((TCAS_U32)uiX >> 16))	{ iNumZeros += 16; uiX <<= 16; }
	if (!((TCAS_U32)uiX >> 24))	{ iNumZeros +=  8; uiX <<=  8; }
	if (!((TCAS_U32)uiX >> 28))	{ iNumZeros +=  4; uiX <<=  4; }
	if (!((TCAS_U32)uiX >> 30))	{ iNumZeros +=  2; uiX <<=  2; }

	iNumZeros -= ((TCAS_U32)uiX >> 31);

	return iNumZeros;
}
#endif /* ARM_ADS */
/*<<<<< added by jr-kim END */

static __inline TCAS_U32 SwapWord32( TCAS_U32 uiData )
{
	uiData = ( ( ( uiData << 8 ) & 0xFF00FF00 ) | ( ( uiData >> 8 ) & 0x00FF00FF ) );
	return ( ( uiData >> 16 ) | ( uiData << 16 ) );
}

static TCASVoid UpdateCRC16( TCC_FLACDEC_BitParser *pstBitParser, TCAS_U32 uiWord )
{
	register TCAS_U32 uiCrc = pstBitParser->m_uiFrameCrc16_BP;

	switch( pstBitParser->m_uiCrc16Align ) 
	{
		case  0: uiCrc = TCC_FLAC_CRC16UPDATE((TCAS_U32)(uiWord >> 24), uiCrc);
		case  8: uiCrc = TCC_FLAC_CRC16UPDATE((TCAS_U32)((uiWord >> 16) & 0xff), uiCrc);
		case 16: uiCrc = TCC_FLAC_CRC16UPDATE((TCAS_U32)((uiWord >> 8) & 0xff), uiCrc);
		case 24: pstBitParser->m_uiFrameCrc16_BP = TCC_FLAC_CRC16UPDATE((TCAS_U32)(uiWord & 0xff), uiCrc);
	}

	pstBitParser->m_uiCrc16Align = 0;
}

TCAS_S32 TCC_FLAC_Fill_Buff(TCAS_U8 buffer[], TCAS_U32 *uiBytes, TCASVoid *client_data, stBufferInfo *pbuff);
extern TCASVoid Swap_Word(TCAS_S32 *bitbuffer, TCAS_U32 uiStart, TCAS_U32 uiEnd);

TCAS_S32 TCC_FLACDEC_FillBitStream( TCC_FLACDEC_BitParser *pstBitParser )
{
	TCAS_U32 uiStart, uiEnd;
	TCAS_U32 uiBytes;
	TCAS_U8 *pucTarget;

	/* first shift the unconsumed buffer data toward the front as much as possible */
	if( pstBitParser->m_uiConsumedWords > 0 ) 
	{
		uiStart = pstBitParser->m_uiConsumedWords;
		uiEnd = pstBitParser->m_uiCompletedWords + ( pstBitParser->m_uiIncompletedBytes ? 1 : 0 );
		pstBitParser->m_pCallback->m_pfMemmove( pstBitParser->m_puiBitBuffer, pstBitParser->m_puiBitBuffer+uiStart, TCC_FLAC_BYTESPERWORD * (uiEnd - uiStart) );

		pstBitParser->m_uiCompletedWords -= uiStart;
		pstBitParser->m_uiConsumedWords = 0;
	}

	 /* set the pucTarget for reading, taking into account word alignment and endianness  */
	uiBytes = ( pstBitParser->m_uiCapacity - pstBitParser->m_uiCompletedWords ) * TCC_FLAC_BYTESPERWORD - pstBitParser->m_uiIncompletedBytes;
	if( uiBytes == 0 )
	{
		return false; /* no space left, buffer is too small; see note for TCC_FLAC_BITBUFFER_SIZE  */
	}

	pucTarget = ((TCAS_U8*)(pstBitParser->m_puiBitBuffer+pstBitParser->m_uiCompletedWords)) + pstBitParser->m_uiIncompletedBytes;

	if( pstBitParser->m_uiIncompletedBytes )
	{
		pstBitParser->m_puiBitBuffer[pstBitParser->m_uiCompletedWords] = SwapWord32(pstBitParser->m_puiBitBuffer[pstBitParser->m_uiCompletedWords]);
	}

	/* read in the data; note that the callback may return a smaller number of uiBytes */
	if( !TCC_FLAC_Fill_Buff( pucTarget, &uiBytes, pstBitParser->m_pvClientData, &pstBitParser->m_stBuffInfo ) )
	{
		return false;
	}

	uiEnd = (pstBitParser->m_uiCompletedWords*TCC_FLAC_BYTESPERWORD + pstBitParser->m_uiIncompletedBytes + uiBytes + (TCC_FLAC_BYTESPERWORD-1)) / TCC_FLAC_BYTESPERWORD;

#ifdef	ARM_OPT
	Swap_Word(pstBitParser->m_puiBitBuffer, pstBitParser->m_uiCompletedWords, uiEnd);
#else	
	for( uiStart = pstBitParser->m_uiCompletedWords; uiStart < uiEnd; uiStart++ )
	{
		pstBitParser->m_puiBitBuffer[uiStart] = SwapWord32( pstBitParser->m_puiBitBuffer[uiStart] );
	}
#endif		

	uiEnd = pstBitParser->m_uiCompletedWords*TCC_FLAC_BYTESPERWORD + pstBitParser->m_uiIncompletedBytes + uiBytes;
	pstBitParser->m_uiCompletedWords = uiEnd / TCC_FLAC_BYTESPERWORD;
	pstBitParser->m_uiIncompletedBytes = uiEnd % TCC_FLAC_BYTESPERWORD;
	
	return true;
}

TCC_FLACDEC_BitParser *TCC_FLACDEC_CreateBitParser( flac_callback_t *pCallback )
{

	TCC_FLACDEC_BitParser *pstBitParser = (TCC_FLACDEC_BitParser*)pCallback->m_pfMalloc(sizeof(TCC_FLACDEC_BitParser));
    if (pstBitParser == 0) {
        return pstBitParser;
    }

	pCallback->m_pfMemset(pstBitParser, 0, sizeof(TCC_FLACDEC_BitParser));
	pstBitParser->m_puiBitBuffer = 0;
	pstBitParser->m_uiCapacity = 0;
	pstBitParser->m_uiCompletedWords = pstBitParser->m_uiIncompletedBytes = 0;
	pstBitParser->m_uiConsumedWords = pstBitParser->m_uiConsumedBits = 0;
	pstBitParser->m_pvClientData = 0;

	pstBitParser->m_puiBitBuffer = (TCAS_U32*)pCallback->m_pfMalloc(sizeof(TCAS_U32) * TCC_FLAC_BITBUFFER_SIZE);
    if (pstBitParser->m_puiBitBuffer == 0) {
        pCallback->m_pfFree(pstBitParser);
        pstBitParser = 0;
    } else {
	    pstBitParser->m_pCallback = pCallback;

	    CRC16_TableGeneration(TCC_FLAC_Crc16Table);
    }

	return pstBitParser;
}

TCASVoid TCC_FLACDEC_DestroyBitParser( TCC_FLACDEC_BitParser *pstBitParser, flac_callback_t *pCallback )
{
	//TCC_FLAC_ASSERT(0 != pstBitParser);
    if (pstBitParser == 0)
    {
        return;
    }

	if(pstBitParser->m_puiBitBuffer)
	{
		pCallback->m_pfFree(pstBitParser->m_puiBitBuffer);
	}

    pCallback->m_pfFree(pstBitParser);
}

TCAS_S32 TCC_FLACDEC_InitBitParser( TCC_FLACDEC_BitParser *pstBitParser, TCASVoid *cd )
{
	TCC_FLAC_ASSERT(0 != pstBitParser);

	pstBitParser->m_uiCompletedWords = pstBitParser->m_uiIncompletedBytes = 0;
	pstBitParser->m_uiConsumedWords = pstBitParser->m_uiConsumedBits = 0;
	pstBitParser->m_uiCapacity = TCC_FLAC_BITBUFFER_SIZE;

	if(pstBitParser->m_puiBitBuffer == 0)
	{
		return false;
	}
	//pstBitParser->read_callback = rcb;
	pstBitParser->m_pvClientData = cd;

	pstBitParser->m_uiFrameCrc16_BP = 0;
	pstBitParser->m_uiCrc16Align = 0;

	return true;
}

TCAS_S32 TCC_FLACDEC_SetBitBuffer( TCC_FLACDEC_BitParser *pstBitParser, TCAS_S8 **ppucBuff, TCAS_S32 *piLength )
{
	if( pstBitParser == 0 )
	{
		return -1;	// TCAS_ERROR_NULL_INSTANCE
	}

	pstBitParser->m_stBuffInfo.m_puCurrPtr = *ppucBuff;
	pstBitParser->m_stBuffInfo.m_iBytesLeft = *piLength;

	pstBitParser->m_uiCompletedWords = pstBitParser->m_uiIncompletedBytes = 0;
	pstBitParser->m_uiConsumedWords = pstBitParser->m_uiConsumedBits = 0;
	pstBitParser->m_uiCapacity = TCC_FLAC_BITBUFFER_SIZE;

	if(pstBitParser->m_puiBitBuffer == 0)
	{
		return false;
	}

	pstBitParser->m_uiFrameCrc16_BP = 0;
	pstBitParser->m_uiCrc16Align = 0;

	return 0;	// TCAS_SUCCESS
}

TCAS_S32 TCC_FLACDEC_GetBitBufferState( TCC_FLACDEC_BitParser *pstBitParser, TCAS_S8 **ppucBuff, TCAS_S32 *piLength )
{
	TCAS_S32 iBytesLeft;
	TCC_FLAC_ASSERT(0 != pstBitParser);

	if(!TCC_FLACDEC_CheckByteAlignedConsumed(pstBitParser))
	{
		iBytesLeft = 0;
	}

	*ppucBuff = pstBitParser->m_stBuffInfo.m_puCurrPtr;
	*piLength = pstBitParser->m_stBuffInfo.m_iBytesLeft;// + (TCC_FLACDEC_GetUnconsumedBits(pstBitParser) / 8);

	if( pstBitParser->m_stBuffInfo.m_iBytesLeft )
	{
		iBytesLeft = 0;
	}

	iBytesLeft = ( TCC_FLACDEC_GetUnconsumedBits(pstBitParser) / 8 );

//	if( iBytesLeft )
//	{
//		 iBytesLeft+=0;
//	}

	return true;
}


TCAS_S32 TCC_FLACDEC_ResetBitParser( TCC_FLACDEC_BitParser *pstBitParser )
{
	pstBitParser->m_uiCompletedWords = pstBitParser->m_uiIncompletedBytes = 0;
	pstBitParser->m_uiConsumedWords = pstBitParser->m_uiConsumedBits = 0;
	return true;
}

TCASVoid TCC_FLACDEC_ResetReadCRC16( TCC_FLACDEC_BitParser *pstBitParser, TCAS_U16 usSeed )
{
	TCC_FLAC_ASSERT(0 != pstBitParser);
	TCC_FLAC_ASSERT(0 != pstBitParser->m_puiBitBuffer);
	TCC_FLAC_ASSERT((pstBitParser->m_uiConsumedBits & 7) == 0);

	pstBitParser->m_uiFrameCrc16_BP = (TCAS_U32)usSeed;
	pstBitParser->m_uiCrc16Align = pstBitParser->m_uiConsumedBits;
}

TCAS_U16 TCC_FLACDEC_GetReadCRC16( TCC_FLACDEC_BitParser *pstBitParser )
{
	TCC_FLAC_ASSERT(0 != pstBitParser);
	TCC_FLAC_ASSERT(0 != pstBitParser->m_puiBitBuffer);
	TCC_FLAC_ASSERT((pstBitParser->m_uiConsumedBits & 7) == 0);
	TCC_FLAC_ASSERT(pstBitParser->m_uiCrc16Align <= pstBitParser->m_uiConsumedBits);

	/* CRC any tail uiBytes in a partially-consumed word */
	if(pstBitParser->m_uiConsumedBits) {
		const TCAS_U32 tail = pstBitParser->m_puiBitBuffer[pstBitParser->m_uiConsumedWords];
		for( ; pstBitParser->m_uiCrc16Align < pstBitParser->m_uiConsumedBits; pstBitParser->m_uiCrc16Align += 8) {
			pstBitParser->m_uiFrameCrc16_BP = TCC_FLAC_CRC16UPDATE((TCAS_U32)((tail >> (TCC_FLAC_BITSPERWORD-8-pstBitParser->m_uiCrc16Align)) & 0xff), pstBitParser->m_uiFrameCrc16_BP);
		}
	}
	return pstBitParser->m_uiFrameCrc16_BP;
}

TCAS_S32 TCC_FLACDEC_CheckByteAlignedConsumed( const TCC_FLACDEC_BitParser *pstBitParser )
{
	return ((pstBitParser->m_uiConsumedBits & 7) == 0);
}

TCAS_U32 TCC_FLACDEC_BitsLeftForByteAlign( const TCC_FLACDEC_BitParser *pstBitParser )
{
	return 8 - (pstBitParser->m_uiConsumedBits & 7);
}

TCAS_U32 TCC_FLACDEC_GetUnconsumedBits( const TCC_FLACDEC_BitParser *pstBitParser )
{
	return (pstBitParser->m_uiCompletedWords-pstBitParser->m_uiConsumedWords)*TCC_FLAC_BITSPERWORD + pstBitParser->m_uiIncompletedBytes*8 - pstBitParser->m_uiConsumedBits;
}

TCAS_S32 TCC_FLACDEC_ReadU32( TCC_FLACDEC_BitParser *pstBitParser, TCAS_U32 *puiVal, TCAS_U32 uiNumBits )
{
	TCC_FLAC_ASSERT(0 != pstBitParser);
	TCC_FLAC_ASSERT(0 != pstBitParser->m_puiBitBuffer);

	TCC_FLAC_ASSERT(uiNumBits <= 32);
	TCC_FLAC_ASSERT((pstBitParser->m_uiCapacity*TCC_FLAC_BITSPERWORD) * 2 >= uiNumBits);
	TCC_FLAC_ASSERT(pstBitParser->m_uiConsumedWords <= pstBitParser->m_uiCompletedWords);
	TCC_FLAC_ASSERT(TCC_FLAC_BITSPERWORD >= 32);

	if( uiNumBits == 0 )
	{
		*puiVal = 0;
		return true;
	}

	while((pstBitParser->m_uiCompletedWords-pstBitParser->m_uiConsumedWords)*TCC_FLAC_BITSPERWORD + pstBitParser->m_uiIncompletedBytes*8 - pstBitParser->m_uiConsumedBits < uiNumBits) 
	{
		if( !TCC_FLACDEC_FillBitStream(pstBitParser) )
		{
			return false;
		}
	}

	if(pstBitParser->m_uiConsumedWords < pstBitParser->m_uiCompletedWords)
	{
		if(pstBitParser->m_uiConsumedBits)
		{
			/* this also works when consumed_bits==0, it's just a little slower than necessary for that case */
			const TCAS_U32 n = TCC_FLAC_BITSPERWORD - pstBitParser->m_uiConsumedBits;
			const TCAS_U32 word = pstBitParser->m_puiBitBuffer[pstBitParser->m_uiConsumedWords];
			if(uiNumBits < n)
			{
				*puiVal = (word & (TCC_FLAC_ALL_ONES >> pstBitParser->m_uiConsumedBits)) >> (n-uiNumBits);
				pstBitParser->m_uiConsumedBits += uiNumBits;
				return true;
			}
			*puiVal = word & (TCC_FLAC_ALL_ONES >> pstBitParser->m_uiConsumedBits);
			uiNumBits -= n;
			UpdateCRC16(pstBitParser, word);
			pstBitParser->m_uiConsumedWords++;
			pstBitParser->m_uiConsumedBits = 0;
			if(uiNumBits)
			{ /* if there are still uiNumBits left to read, there have to be less than 32 so they will all be in the next word */
				*puiVal <<= uiNumBits;
				*puiVal |= (pstBitParser->m_puiBitBuffer[pstBitParser->m_uiConsumedWords] >> (TCC_FLAC_BITSPERWORD-uiNumBits));
				pstBitParser->m_uiConsumedBits = uiNumBits;
			}
			return true;
		}
		else 
		{
			const TCAS_U32 word = pstBitParser->m_puiBitBuffer[pstBitParser->m_uiConsumedWords];
			if(uiNumBits < TCC_FLAC_BITSPERWORD)
			{
				*puiVal = word >> (TCC_FLAC_BITSPERWORD-uiNumBits);
				pstBitParser->m_uiConsumedBits = uiNumBits;
				return true;
			}
			/* at this point 'uiNumBits' must be == TCC_FLAC_BITSPERWORD; because of previous assertions, it can't be larger */
			*puiVal = word;
			UpdateCRC16(pstBitParser, word);
			pstBitParser->m_uiConsumedWords++;
			return true;
		}
	}
	else 
	{
		/* in this case we're starting our read at a partial tail word;
		 * the reader has guaranteed that we have at least 'uiNumBits' uiNumBits
		 * available to read, which makes this case simpler.
		 */
		/* OPT: taking out the consumed_bits==0 "else" case below might make things faster if less code allows the compiler to inline this function */
		if(pstBitParser->m_uiConsumedBits)
		{
			/* this also works when consumed_bits==0, it's just a little slower than necessary for that case */
			TCC_FLAC_ASSERT(pstBitParser->m_uiConsumedBits + uiNumBits <= pstBitParser->m_uiIncompletedBytes*8);
			*puiVal = (pstBitParser->m_puiBitBuffer[pstBitParser->m_uiConsumedWords] & (TCC_FLAC_ALL_ONES >> pstBitParser->m_uiConsumedBits)) >> (TCC_FLAC_BITSPERWORD-pstBitParser->m_uiConsumedBits-uiNumBits);
			pstBitParser->m_uiConsumedBits += uiNumBits;
			return true;
		}
		else
		{
			*puiVal = pstBitParser->m_puiBitBuffer[pstBitParser->m_uiConsumedWords] >> (TCC_FLAC_BITSPERWORD-uiNumBits);
			pstBitParser->m_uiConsumedBits += uiNumBits;
			return true;
		}
	}
}

TCAS_S32 TCC_FLACDEC_ReadS32(TCC_FLACDEC_BitParser *pstBitParser, TCAS_S32 *piVal, TCAS_U32 uiNumBits)
{
	/* OPT: inline pucRaw uint32 code here, or make into a macro if possible in the .h file */
	if(!TCC_FLACDEC_ReadU32(pstBitParser, (TCAS_U32*)piVal, uiNumBits))
	{
		return false;
	}

	/* sign-extend: */
	*piVal <<= ( 32 - uiNumBits );
	*piVal >>= ( 32 - uiNumBits );
	return true;
}

TCAS_S32 TCC_FLACDEC_ReadU64(TCC_FLACDEC_BitParser *pstBitParser, TCAS_U64 *pulVal, TCAS_U32 uiNumBits)
{
	TCAS_U32 uiHi, uiLo;

	if( uiNumBits > 32 )
	{
		if(!TCC_FLACDEC_ReadU32(pstBitParser, &uiHi, uiNumBits-32))
		{
			return false;
		}
		if(!TCC_FLACDEC_ReadU32(pstBitParser, &uiLo, 32))
		{
			return false;
		}
		*pulVal = uiHi;
		*pulVal <<= 32;
		*pulVal |= uiLo;
	}
	else
	{
		if(!TCC_FLACDEC_ReadU32(pstBitParser, &uiLo, uiNumBits))
		{
			return false;
		}

		*pulVal = uiLo;
	}
	return true;
}

TCAS_S32 TCC_FLACDEC_ReadUUnary( TCC_FLACDEC_BitParser *pstBitParser, TCAS_U32 *puiVal )
{
	TCAS_U32 i;

	TCC_FLAC_ASSERT(0 != pstBitParser);
	TCC_FLAC_ASSERT(0 != pstBitParser->m_puiBitBuffer);

	*puiVal = 0;

	while(1)
	{
		while( pstBitParser->m_uiConsumedWords < pstBitParser->m_uiCompletedWords )
		{ /* if we've not consumed up to a partial tail word... */
			TCAS_U32 uiWordBits = pstBitParser->m_puiBitBuffer[pstBitParser->m_uiConsumedWords] << pstBitParser->m_uiConsumedBits;
			if( uiWordBits )
			{

				i = ALIGNED_UNARY_BITS(uiWordBits);
				//i = TCC_FLAC_CLZ_NZ(uiWordBits);
				*puiVal += i;
				i++;
				pstBitParser->m_uiConsumedBits += i;
				if(pstBitParser->m_uiConsumedBits == TCC_FLAC_BITSPERWORD)
				{
					UpdateCRC16(pstBitParser, pstBitParser->m_puiBitBuffer[pstBitParser->m_uiConsumedWords]);
					pstBitParser->m_uiConsumedWords++;
					pstBitParser->m_uiConsumedBits = 0;
				}
				return true;
			}
			else
			{
				*puiVal += TCC_FLAC_BITSPERWORD - pstBitParser->m_uiConsumedBits;
				UpdateCRC16(pstBitParser, pstBitParser->m_puiBitBuffer[pstBitParser->m_uiConsumedWords]);
				pstBitParser->m_uiConsumedWords++;
				pstBitParser->m_uiConsumedBits = 0;
				/* didn't find stop bit yet, have to keep going... */
			}
		}
		/* at this point we've eaten up all the whole words; have to try
		 * reading through any tail uiBytes before calling the read callback.
		 * this is a repeat of the above logic adjusted for the fact we
		 * don't have a whole word.  note though if the client is feeding
		 * us data a byte at a time (unlikely), pstBitParser->m_uiConsumedBits may not
		 * be zero.
		 */
		if( pstBitParser->m_uiIncompletedBytes )
		{
			const TCAS_U32 uiEnd = pstBitParser->m_uiIncompletedBytes * 8;
			TCAS_U32 uiWordBits = (pstBitParser->m_puiBitBuffer[pstBitParser->m_uiConsumedWords] & (TCC_FLAC_ALL_ONES << (TCC_FLAC_BITSPERWORD-uiEnd))) << pstBitParser->m_uiConsumedBits;
			if(uiWordBits)
			{
				i = ALIGNED_UNARY_BITS(uiWordBits);
				//i = TCC_FLAC_CLZ_NZ(uiWordBits);

				*puiVal += i;
				i++;
				pstBitParser->m_uiConsumedBits += i;
				TCC_FLAC_ASSERT(pstBitParser->m_uiConsumedBits < TCC_FLAC_BITSPERWORD);
				return true;
			}
			else
			{
				*puiVal += uiEnd - pstBitParser->m_uiConsumedBits;
				pstBitParser->m_uiConsumedBits += uiEnd;
				TCC_FLAC_ASSERT(pstBitParser->m_uiConsumedBits < TCC_FLAC_BITSPERWORD);
				/* didn't find stop bit yet, have to keep going... */
			}
		}
		if( !TCC_FLACDEC_FillBitStream(pstBitParser) )
		{
			return false;
		}
	}
}

/* this is by far the most heavily used reader call.  it ain't pretty but it's fast */
/* a lot of the logic is copied, then adapted, from TCC_FLACDEC_ReadUUnary() and TCC_FLACDEC_ReadU32() */
TCAS_S32 TCC_FLACDEC_ReadRiceSignedBlock( TCC_FLACDEC_BitParser *pstBitParser, TCAS_S32 piValue[], TCAS_U32 uiNumValues, TCAS_U32 uiParameter )
{
	TCAS_U32 i;
	TCAS_U32 uiValue = 0;
	TCAS_U32 uiNumBits; /* the # of binary LSBs left to read to finish a rice codeword */

	/* try and get pstBitParser->m_uiConsumedWords and pstBitParser->m_uiConsumedBits into register;
	 * must remember to flush them back to *pstBitParser before calling other
	 * bitwriter functions that use them, and before returning */
	register TCAS_U32 uiConsumedWords;
	register TCAS_U32 uiConsumedBits;

	TCC_FLAC_ASSERT(0 != pstBitParser);
	TCC_FLAC_ASSERT(0 != pstBitParser->m_puiBitBuffer);
	/* WATCHOUT: code does not work with <32bit words; we can make things much faster with this assertion */
	TCC_FLAC_ASSERT(TCC_FLAC_BITSPERWORD >= 32);
	TCC_FLAC_ASSERT(uiParameter < 32);
	/* the above two asserts also guarantee that the binary part never straddles more that 2 words, so we don't have to loop to read it */

	if( uiNumValues == 0 )
	{
		return true;
	}

	uiConsumedBits = pstBitParser->m_uiConsumedBits;
	uiConsumedWords = pstBitParser->m_uiConsumedWords;

	while(1)
	{
		/* read unary part */
		while(1)
		{
			while(uiConsumedWords < pstBitParser->m_uiCompletedWords)
			{ /* if we've not consumed up to a partial tail word... */
				TCAS_U32 uiWordBits = pstBitParser->m_puiBitBuffer[uiConsumedWords] << uiConsumedBits;
				if(uiWordBits) 
				{
					i = ALIGNED_UNARY_BITS(uiWordBits);
					//TCC_FLAC_CLZ_NZ(uiWordBits);

					uiValue += i;
					uiNumBits = uiParameter;
					i++;
					uiConsumedBits += i;

					if(uiConsumedBits == TCC_FLAC_BITSPERWORD) 
					{
						UpdateCRC16(pstBitParser, pstBitParser->m_puiBitBuffer[uiConsumedWords]);
						uiConsumedWords++;
						uiConsumedBits = 0;
					}
					goto break1;
				}
				else 
				{
					uiValue += TCC_FLAC_BITSPERWORD - uiConsumedBits;
					UpdateCRC16(pstBitParser, pstBitParser->m_puiBitBuffer[uiConsumedWords]);
					uiConsumedWords++;
					uiConsumedBits = 0;
					/* didn't find stop bit yet, have to keep going... */
				}
			}
			/* at this point we've eaten up all the whole words; have to try
			 * reading through any tail uiBytes before calling the read callback.
			 * this is a repeat of the above logic adjusted for the fact we
			 * don't have a whole word.  note though if the client is feeding
			 * us data a byte at a time (unlikely), pstBitParser->m_uiConsumedBits may not
			 * be zero.
			 */
			if( pstBitParser->m_uiIncompletedBytes ) 
			{
				const TCAS_U32 uiEnd = pstBitParser->m_uiIncompletedBytes * 8;
				TCAS_U32 uiWordBits = (pstBitParser->m_puiBitBuffer[uiConsumedWords] & (TCC_FLAC_ALL_ONES << (TCC_FLAC_BITSPERWORD-uiEnd))) << uiConsumedBits;
				if(uiWordBits) 
				{
					i = ALIGNED_UNARY_BITS(uiWordBits);
					//i = TCC_FLAC_CLZ_NZ(uiWordBits);

					uiValue += i;
					uiNumBits = uiParameter;
					i++;
					uiConsumedBits += i;
					TCC_FLAC_ASSERT(uiConsumedBits < TCC_FLAC_BITSPERWORD);
					goto break1;
				}
				else 
				{
					uiValue += uiEnd - uiConsumedBits;
					uiConsumedBits += uiEnd;
					TCC_FLAC_ASSERT(uiConsumedBits < TCC_FLAC_BITSPERWORD);
					/* didn't find stop bit yet, have to keep going... */
				}
			}
			/* flush registers and read; bitreader_read_from_client_() does
			 * not touch pstBitParser->m_uiConsumedBits at all but we still need to set
			 * it in case it fails and we have to return false.
			 */
			pstBitParser->m_uiConsumedBits = uiConsumedBits;
			pstBitParser->m_uiConsumedWords = uiConsumedWords;
			if(!TCC_FLACDEC_FillBitStream(pstBitParser))
			{
				return false;
			}
			uiConsumedWords = pstBitParser->m_uiConsumedWords;
		}
break1:
		/* read binary part */
		TCC_FLAC_ASSERT(uiConsumedWords <= pstBitParser->m_uiCompletedWords);

		if(uiNumBits) 
		{
			while((pstBitParser->m_uiCompletedWords-uiConsumedWords)*TCC_FLAC_BITSPERWORD + pstBitParser->m_uiIncompletedBytes*8 - uiConsumedBits < uiNumBits) 
			{
				/* flush registers and read; bitreader_read_from_client_() does
				 * not touch pstBitParser->m_uiConsumedBits at all but we still need to set
				 * it in case it fails and we have to return false.
				 */
				pstBitParser->m_uiConsumedBits = uiConsumedBits;
				pstBitParser->m_uiConsumedWords = uiConsumedWords;
				if(!TCC_FLACDEC_FillBitStream(pstBitParser))
					return false;
				uiConsumedWords = pstBitParser->m_uiConsumedWords;
			}
			if(uiConsumedWords < pstBitParser->m_uiCompletedWords) 
			{ /* if we've not consumed up to a partial tail word... */
				if(uiConsumedBits) 
				{
					/* this also works when consumed_bits==0, it's just a little slower than necessary for that case */
					const TCAS_U32 n = TCC_FLAC_BITSPERWORD - uiConsumedBits;
					const TCAS_U32 word = pstBitParser->m_puiBitBuffer[uiConsumedWords];
					if(uiNumBits < n) 
					{
						uiValue <<= uiNumBits;
						uiValue |= (word & (TCC_FLAC_ALL_ONES >> uiConsumedBits)) >> (n-uiNumBits);
						uiConsumedBits += uiNumBits;
						goto break2;
					}
					uiValue <<= n;
					uiValue |= word & (TCC_FLAC_ALL_ONES >> uiConsumedBits);
					uiNumBits -= n;
					UpdateCRC16(pstBitParser, word);
					uiConsumedWords++;
					uiConsumedBits = 0;
					if(uiNumBits) 
					{ /* if there are still uiNumBits left to read, there have to be less than 32 so they will all be in the next word */
						uiValue <<= uiNumBits;
						uiValue |= (pstBitParser->m_puiBitBuffer[uiConsumedWords] >> (TCC_FLAC_BITSPERWORD-uiNumBits));
						uiConsumedBits = uiNumBits;
					}
					goto break2;
				}
				else 
				{
					TCC_FLAC_ASSERT(uiNumBits < TCC_FLAC_BITSPERWORD);
					uiValue <<= uiNumBits;
					uiValue |= pstBitParser->m_puiBitBuffer[uiConsumedWords] >> (TCC_FLAC_BITSPERWORD-uiNumBits);
					uiConsumedBits = uiNumBits;
					goto break2;
				}
			}
			else 
			{
				/* in this case we're starting our read at a partial tail word;
				 * the reader has guaranteed that we have at least 'uiNumBits' uiNumBits
				 * available to read, which makes this case simpler.
				 */
				uiValue <<= uiNumBits;
				if(uiConsumedBits) 
				{
					/* this also works when consumed_bits==0, it's just a little slower than necessary for that case */
					TCC_FLAC_ASSERT(uiConsumedBits + uiNumBits <= pstBitParser->m_uiIncompletedBytes*8);
					uiValue |= (pstBitParser->m_puiBitBuffer[uiConsumedWords] & (TCC_FLAC_ALL_ONES >> uiConsumedBits)) >> (TCC_FLAC_BITSPERWORD-uiConsumedBits-uiNumBits);
					uiConsumedBits += uiNumBits;
					goto break2;
				}
				else 
				{
					uiValue |= pstBitParser->m_puiBitBuffer[uiConsumedWords] >> (TCC_FLAC_BITSPERWORD-uiNumBits);
					uiConsumedBits += uiNumBits;
					goto break2;
				}
			}
		}
break2:
		/* compose the value */
		*piValue = (TCAS_S32)(uiValue >> 1 ^ -(TCAS_S32)(uiValue & 1));

		/* are we done? */
		--uiNumValues;
		if( uiNumValues == 0 ) 
		{
			pstBitParser->m_uiConsumedBits = uiConsumedBits;
			pstBitParser->m_uiConsumedWords = uiConsumedWords;
			return true;
		}

		uiValue = 0;
		++piValue;

	}
}

/* on return, if *val == 0xffffffff then the utf-8 sequence was invalid, but the return value will be true */
TCAS_S32 TCC_FLACDEC_ReadU32UFT8( TCC_FLACDEC_BitParser *pstBitParser, TCAS_U32 *puiValue, TCAS_U8 *pucRaw, TCAS_U32 *puiRawLength )
{
	TCAS_U32 uiValue = 0;
	TCAS_U32 uiX;
	TCAS_U32 i;

	if(!TCC_FLACDEC_ReadU32(pstBitParser, &uiX, 8))
	{
		return false;
	}

	if( pucRaw )
	{
		pucRaw[(*puiRawLength)++] = (TCAS_U8)uiX;
	}

	if( !(uiX & 0x80) ) 
	{ /* 0xxxxxxx */
		uiValue = uiX;
		i = 0;
	}
	else if( uiX & 0xC0 && !(uiX & 0x20) ) 
	{ /* 110xxxxx */
		uiValue = uiX & 0x1F;
		i = 1;
	}
	else if( uiX & 0xE0 && !(uiX & 0x10) ) 
	{ /* 1110xxxx */
		uiValue = uiX & 0x0F;
		i = 2;
	}
	else if( uiX & 0xF0 && !(uiX & 0x08) ) 
	{ /* 11110xxx */
		uiValue = uiX & 0x07;
		i = 3;
	}
	else if( uiX & 0xF8 && !(uiX & 0x04) ) 
	{ /* 111110xx */
		uiValue = uiX & 0x03;
		i = 4;
	}
	else if( uiX & 0xFC && !(uiX & 0x02) ) 
	{ /* 1111110x */
		uiValue = uiX & 0x01;
		i = 5;
	}
	else 
	{
		*puiValue = 0xffffffff;
		return true;
	}

	for( ; i; i-- ) 
	{
		if(!TCC_FLACDEC_ReadU32(pstBitParser, &uiX, 8))
		{
			return false;
		}
		if( pucRaw )
		{
			pucRaw[(*puiRawLength)++] = (TCAS_U8)uiX;
		}

		if( !(uiX & 0x80) || (uiX & 0x40) ) 
		{ /* 10xxxxxx */
			*puiValue = 0xffffffff;
			return true;
		}
		uiValue <<= 6;
		uiValue |= (uiX & 0x3F);
	}

	*puiValue = uiValue;
	return true;
}

/* on return, if *val == 0xffffffffffffffff then the utf-8 sequence was invalid, but the return value will be true */
TCAS_S32 TCC_FLACDEC_ReadU64UFT8( TCC_FLACDEC_BitParser *pstBitParser, TCAS_U64 *pulValue, TCAS_U8 *pucRaw, TCAS_U32 *puiRawLength )
{
	TCAS_U64 ulValue = 0;
	TCAS_U32 uiX;
	TCAS_U32 i;

	if( !TCC_FLACDEC_ReadU32( pstBitParser, &uiX, 8 ) )
	{
		return false;
	}

	if (pucRaw )
	{
		pucRaw[(*puiRawLength)++] = (TCAS_U8)uiX;
	}

	if( !(uiX & 0x80) ) 
	{ /* 0xxxxxxx */
		ulValue = uiX;
		i = 0;
	}
	else if( uiX & 0xC0 && !(uiX & 0x20) ) 
	{ /* 110xxxxx */
		ulValue = uiX & 0x1F;
		i = 1;
	}
	else if( uiX & 0xE0 && !(uiX & 0x10) ) 
	{ /* 1110xxxx */
		ulValue = uiX & 0x0F;
		i = 2;
	}
	else if( uiX & 0xF0 && !(uiX & 0x08) ) 
	{ /* 11110xxx */
		ulValue = uiX & 0x07;
		i = 3;
	}
	else if( uiX & 0xF8 && !(uiX & 0x04) ) 
	{ /* 111110xx */
		ulValue = uiX & 0x03;
		i = 4;
	}
	else if( uiX & 0xFC && !(uiX & 0x02) ) 
	{ /* 1111110x */
		ulValue = uiX & 0x01;
		i = 5;
	}
	else if( uiX & 0xFE && !(uiX & 0x01) ) 
	{ /* 11111110 */
		ulValue = 0;
		i = 6;
	}
	else 
	{
		*pulValue = TCC_FLAC_U64L(0xffffffffffffffff);
		return true;
	}

	for( ; i; i-- ) 
	{
		if(!TCC_FLACDEC_ReadU32(pstBitParser, &uiX, 8))
		{
			return false;
		}

		if( pucRaw )
		{
			pucRaw[(*puiRawLength)++] = (TCAS_U8)uiX;
		}

		if( !(uiX & 0x80) || (uiX & 0x40) ) 
		{ /* 10xxxxxx */
			*pulValue = TCC_FLAC_U64L(0xffffffffffffffff);
			return true;
		}

		ulValue <<= 6;
		ulValue |= (uiX & 0x3F);
	}

	*pulValue = ulValue;
	return true;
}
