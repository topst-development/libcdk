/****************************************************************************
 *   FileName    : aacdec.c
 *   Description : 
 ****************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved 
 
This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited to re-distribution in source or binary form is strictly prohibited.
This source code is provided ??AS IS?? and nothing contained in this source code shall constitute any express or implied warranty of any kind, including without limitation, any warranty of merchantability, fitness for a particular purpose or non-infringement of any patent, copyright or other third party intellectual property right. No warranty is made, express or implied, regarding the information??s accuracy, completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability arising from, out of or in connection with this source code or the use in the source code. 
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement between Telechips and Company.
*
****************************************************************************/

#include "tcc_audio_interface.h"

typedef enum
{	
	// LATM/LOAS (Low Overhead Audio Stream): LATM with sync information
	TF_AAC_LOAS			= 0,	// default value

	// LATM (Low-overhead MPEG-4 Audio Transport Multiplex), without LOAS Sync-Layer, No random access is possible
	TF_AAC_LATM_MCP1	= 1,	// LATM wiht muxConfigPresent = 1
	TF_AAC_LATM_MCP0	= 2,	// LATM wiht muxConfigPresent = 0

	// ADTS (Audio Data Transport Stream)
	TF_AAC_ADTS			= 3,	

	// ADIF (Audio Data Interchange Format)
	TF_AAC_ADIF			= 4,	// not supported

	TF_UNKNOWN			= 0x7FFFFFFF	// Unknown format
}TransportFormat;

void* latm_parser_init(TCAS_U8 *pucPacketDataPtr, 
						TCAS_U32 iDataLength, 
  						TCAS_S32 *piSamplingFrequency, 
  						TCAS_S32 *piChannelNumber, 
  						void *pCallback,
  						TransportFormat eFormat);
TCAS_S32 latm_parser_get_frame(void *pLatmDmxHandle,
							TCAS_U8 *pucPacketDataPtr, TCAS_S32 iStreamLength,
							TCAS_U8 **pucAACRawDataPtr, TCAS_S32 *piAACRawDataLength,
							TCAS_U32 uiInitFlag);
TCAS_S32 latm_parser_close(void *pLatmDmxHandle);
TCAS_S32 latm_parser_get_header_type(void *pLatmDmxHandle);


TCAS_S32 TCC_AAC_DEC ( TCAS_S32 Op, TCAS_SLONG* pHandle, void* pParam1, void* pParam2 );

decoder_func_list_t FucntionList = {
	TCC_AAC_DEC,
	{NULL},
};

