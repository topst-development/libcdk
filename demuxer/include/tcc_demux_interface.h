/****************************************************************************
 *   FileName    : tcc_demux_interface.h
 *   Description : demuxer
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
 *
 * \file
 *		tcc_demux_interface.h
 * \date
 *		
 * \author
 *		TCW01230@telechips.com
 * \brief
 *		demux common interface
 * \version
 *		- 0.0.1 : 
 *
 ***********************************************************************
 */
#ifndef _TCC_DEMUX_INTERFACE_H_
#define _TCC_DEMUX_INTERFACE_H_

typedef int av_sint32;
typedef unsigned long av_ulong;

typedef av_sint32 fnCdkDemux (av_sint32 iOpCode, av_ulong* pHandle, void* pParam1, void* pParam2);

#ifndef NULL
#define NULL 0
#endif

typedef struct demux_func_list_t
{
	/** demux main function */
	fnCdkDemux *pfMainFunction;

	/** demux function pointers */
	void *pfDemuxDependent[8];
} demux_func_list_t;

extern demux_func_list_t FucntionList;
#endif //_TCC_DEMUX_INTERFACE_H_

