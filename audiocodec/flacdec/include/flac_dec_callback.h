/* -------------------------------------
 * Copyright (C) 2007 Telechips
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FLAC_DEC_CALLBACK_INFO_H__
#define FLAC_DEC_CALLBACK_INFO_H__

#include <stddef.h>

typedef struct flac_callback_t
{
		TCASVoid* (*m_pfMalloc	) ( size_t size);								//!< malloc
		TCASVoid  (*m_pfFree	) ( TCASVoid* ptr);								//!< free
		TCASVoid* (*m_pfMemcpy	) ( TCASVoid* dst, const TCASVoid* src, size_t num);		//!< memcpy
		TCASVoid  (*m_pfMemset	) ( TCASVoid* ptr, TCAS_S32 value, size_t num);			//!< memset
		TCASVoid* (*m_pfMemmove	) ( TCASVoid* dst, const TCASVoid* src, size_t num);		//!< memmove

		TCASVoid* (*m_pfFopen	) ( const TCAS_S8 *filename, const TCAS_S8 *mode);			//!< fopen
		size_t  (*m_pfFread	) ( TCASVoid* ptr, size_t size, size_t count, TCASVoid* stream);	//!< fread
		TCAS_S32  (*m_pfFseek	) ( TCASVoid* stream, long offset, TCAS_S32 origin);				//!< fseek
		long	  (*m_pfFtell	) ( TCASVoid* stream);								//!< ftell
		TCAS_S32  (*m_pfFclose  ) ( TCASVoid* stream);								//!< fclose
}flac_callback_t;

#endif	/* FLAC_DEC_CALLBACK_INFO_H__ */
