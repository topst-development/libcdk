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
#include "tcc_flac_fixed.h"
#include "tcc_flac_assert.h"

#ifndef ARM_OPT
TCASVoid TCC_FLAC_fixed_restore_signal ( const TCAS_S32 piResidual[], 
										 TCAS_U32 uiDataLength, 
										 TCAS_U32 uiOrder, 
										 TCAS_S32 piData[],
										 flac_callback_t *m_pCallback
)
{
	TCAS_S32 i, iDataLen = (TCAS_S32)uiDataLength;

	switch( uiOrder ) 
	{
		case 0:
			TCC_FLAC_ASSERT(sizeof(piResidual[0]) == sizeof(piData[0]));
			m_pCallback->m_pfMemcpy(piData, piResidual, sizeof(piResidual[0])*uiDataLength);
			break;

		case 1:
			for( i = 0; i < iDataLen; i++ )
			{
				piData[i] = piResidual[i] + piData[i-1];
			}
			break;

		case 2:
			for( i = 0; i < iDataLen; i++ )
			{
				piData[i] = piResidual[i] + (piData[i-1]<<1) - piData[i-2];
			}
			break;

		case 3:
			for( i = 0; i < iDataLen; i++ )
			{
				piData[i] = piResidual[i] + (((piData[i-1]-piData[i-2])<<1) + (piData[i-1]-piData[i-2])) + piData[i-3];
			}
			break;

		case 4:
			for( i = 0; i < iDataLen; i++ )
			{
				piData[i] = piResidual[i] + ((piData[i-1]+piData[i-3])<<2) - ((piData[i-2]<<2) + (piData[i-2]<<1)) - piData[i-4];
			}
			break;

		default:
			TCC_FLAC_ASSERT(0);

	}
}
#endif