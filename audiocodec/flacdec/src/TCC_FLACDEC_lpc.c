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
#include "tcc_flac_assert.h"
#include "tcc_flac_format.h"
#include "tcc_flac_lpc.h"


TCASVoid TCC_FLAC_lpc_restore_signal( const TCAS_S32 piResidual[], 
									  TCAS_U32 uiDataLength, 
									  const TCAS_S32 piQlpCoeff[], 
									  TCAS_U32 iOrder, 
									  TCAS_S32 iLpQuant, 
									  TCAS_S32 piData[]
)
{
	TCAS_U32 i, j;
	TCAS_S32 iSum;
	const TCAS_S32 *piResi = piResidual, *piHistroy;

	TCC_FLAC_ASSERT(iOrder > 0);

	for( i = 0; i < uiDataLength; i++ ) 
	{
		iSum = 0;
		piHistroy = piData;
		for( j = 0; j < iOrder; j++ ) 
		{
			iSum += piQlpCoeff[j] * (*(--piHistroy));
		}

		*(piData++) = *(piResi++) + (iSum >> iLpQuant);
	}
}
#ifndef ARM_OPT
TCASVoid TCC_FLAC_lpc_restore_signal_wide( const TCAS_S32 piResidual[], 
										   TCAS_U32 uiDataLength, 
										   const TCAS_S32 piQlpCoeff[], 
										   TCAS_U32 iOrder, 
										   TCAS_S32 iLpQuant, 
										   TCAS_S32 piData[]	
)
{
	TCAS_U32 i, j;
	TCAS_S64 iSum;
	const TCAS_S32 *piResi = piResidual, *piHistroy;

	TCC_FLAC_ASSERT(iOrder > 0);

    TCC_FLAC_ASSERT(iLpQuant > 0); //added by jrkim
		
	for( i = 0; i < uiDataLength; i++ ) 
	{
		iSum = 0;
		piHistroy = piData;

		for( j = 0; j < iOrder; j++ )
		{
			iSum += (TCAS_S64)piQlpCoeff[j] * (TCAS_S64)(*(--piHistroy));
		}

		*(piData++) = *(piResi++) + (TCAS_S32)(iSum >> iLpQuant);
	}
}
#endif
