/*
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

#ifndef TCAS_TYPEDEF_H__
#define TCAS_TYPEDEF_H__

/*===========================================================================

           A U D I O   S Y S T E M   H E A D E R   F I L E

Copyright (c) 2008 by TELECHIPS, Incorporated.  All Rights Reserved.
===========================================================================*/

typedef signed char 		TCAS_S8;
typedef unsigned char 		TCAS_U8;
typedef signed short int 	TCAS_S16;
typedef unsigned short int 	TCAS_U16;
typedef signed int 			TCAS_S32;
typedef unsigned int 		TCAS_U32;

#if defined (__GNUC__)
typedef signed long long 	TCAS_S64;
typedef unsigned long long 	TCAS_U64;
#else
typedef signed __int64 		TCAS_S64;
typedef unsigned __int64 	TCAS_U64;
#endif

typedef float 				TCAS_F32;
typedef double 				TCAS_F64;

typedef void 				TCASVoid;

#ifndef NULL
#define NULL (0)
#endif

#ifdef true
#undef true
#endif
#ifdef false
#undef false
#endif
#ifndef __cplusplus
#define true (1)
#define false (0)
#endif

#endif	/* TCAS_TYPEDEF_H__ */
