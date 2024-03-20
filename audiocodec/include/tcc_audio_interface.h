/****************************************************************************
 *   FileName    : adec.h
 *   Description : audio decoder
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved 
 
This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited to re-distribution in source or binary form is strictly prohibited.
This source code is provided ¢®¡ÆAS IS¢®¡¾ and nothing contained in this source code shall constitute any express or implied warranty of any kind, including without limitation, any warranty of merchantability, fitness for a particular purpose or non-infringement of any patent, copyright or other third party intellectual property right. No warranty is made, express or implied, regarding the information¢®?s accuracy, completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability arising from, out of or in connection with this source code or the use in the source code. 
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement between Telechips and Company.
*
****************************************************************************/
/*!
 ***********************************************************************
 \par Copyright
 \verbatim
  ________  _____           _____   _____           ____  ____   ____		
     /     /       /       /       /       /     /   /    /   \ /			
    /     /___    /       /___    /       /____ /   /    /____/ \___			
   /     /       /       /       /       /     /   /    /           \		
  /     /_____  /_____  /_____  /_____  /     / _ /_  _/_      _____/ 		
   																				
  Copyright (c) 2009 Telechips Inc.
  Korad Bldg, 1000-12 Daechi-dong, Kangnam-Ku, Seoul, Korea					
 \endverbatim
 ***********************************************************************
 */
/*!
 ***********************************************************************
 *
 * \file
 *		tcc_audio_interface.h
 * \date
 *		
 * \author
 *		AValgorithm@telechips.com
 * \brief
 *		audio decoder common interface
 * \version
 *		- 0.0.1 : 
 *
 ***********************************************************************
 */
#ifndef TCC_AUDIO_INTERFACE_H_
#define TCC_AUDIO_INTERFACE_H_

//#include <stdlib.h>
//#include <string.h>

#include "audio_common.h"

typedef TCAS_S32 fnCdkAudioDec (TCAS_S32 iOpCode, TCAS_SLONG* pHandle, void* pParam1, void* pParam2);

#ifndef NULL
#define NULL (0)
#endif

typedef struct decoder_func_list_t
{
	/** codec main function */
	fnCdkAudioDec *pfMainFunction;

	/** codec dependent function pointers */
	fnCdkAudioDec *pfCodecDependent[8];
} decoder_func_list_t;

extern decoder_func_list_t FucntionList;
#endif //TCC_AUDIO_INTERFACE_H_

