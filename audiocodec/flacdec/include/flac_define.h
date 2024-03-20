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

#ifndef TCC_FLAC_DEFINE_H__
#define TCC_FLAC_DEFINE_H__

/* VERSION should come from configure */
#define TCC_FLAC_STREAM_METADATA_STREAMINFO_MIN_BLOCK_SIZE_LEN		16 /* bits */
#define TCC_FLAC_STREAM_METADATA_STREAMINFO_MAX_BLOCK_SIZE_LEN		16 /* bits */
#define TCC_FLAC_STREAM_METADATA_STREAMINFO_MIN_FRAME_SIZE_LEN		24 /* bits */
#define TCC_FLAC_STREAM_METADATA_STREAMINFO_MAX_FRAME_SIZE_LEN		24 /* bits */
#define TCC_FLAC_STREAM_METADATA_STREAMINFO_SAMPLE_RATE_LEN		20 /* bits */
#define TCC_FLAC_STREAM_METADATA_STREAMINFO_CHANNELS_LEN			3 /* bits */
#define TCC_FLAC_STREAM_METADATA_STREAMINFO_BITS_PER_SAMPLE_LEN	5 /* bits */
#define TCC_FLAC_STREAM_METADATA_STREAMINFO_TOTAL_SAMPLES_LEN		36 /* bits */
#define TCC_FLAC_STREAM_METADATA_STREAMINFO_MD5SUM_LEN				128 /* bits */

#define TCC_FLAC_FRAME_HEADER_SYNC  0x3ffe
#define TCC_FLAC_FRAME_HEADER_SYNC_LEN  14 /* bits */
#define TCC_FLAC_FRAME_HEADER_RESERVED_LEN  2 /* bits */
#define TCC_FLAC_FRAME_HEADER_BLOCK_SIZE_LEN  4 /* bits */
#define TCC_FLAC_FRAME_HEADER_SAMPLE_RATE_LEN  4 /* bits */
#define TCC_FLAC_FRAME_HEADER_CHANNEL_ASSIGNMENT_LEN  4 /* bits */
#define TCC_FLAC_FRAME_HEADER_BITS_PER_SAMPLE_LEN  3 /* bits */
#define TCC_FLAC_FRAME_HEADER_ZERO_PAD_LEN  1 /* bits */
#define TCC_FLAC_FRAME_HEADER_CRC_LEN  8 /* bits */

#define TCC_FLAC_FRAME_FOOTER_CRC_LEN  16 /* bits */

#define TCC_FLAC_ENTROPY_CODING_METHOD_TYPE_LEN  2 /* bits */
#define TCC_FLAC_ENTROPY_CODING_METHOD_PARTITIONED_RICE_ORDER_LEN  4 /* bits */
#define TCC_FLAC_ENTROPY_CODING_METHOD_PARTITIONED_RICE_PARAMETER_LEN  4 /* bits */
#define TCC_FLAC_ENTROPY_CODING_METHOD_PARTITIONED_RICE2_PARAMETER_LEN 5 /* bits */
#define TCC_FLAC_ENTROPY_CODING_METHOD_PARTITIONED_RICE_RAW_LEN  5 /* bits */

#define TCC_FLAC_ENTROPY_CODING_METHOD_PARTITIONED_RICE_ESCAPE_PARAMETER  15 /* == (1<<TCC_FLAC_ENTROPY_CODING_METHOD_PARTITIONED_RICE_PARAMETER_LEN)-1 */
#define TCC_FLAC_ENTROPY_CODING_METHOD_PARTITIONED_RICE2_ESCAPE_PARAMETER 31 /* == (1<<FLAC__ENTROPY_CODING_METHOD_PARTITIONED_RICE2_PARAMETER_LEN)-1 */

#define TCC_FLAC_SUBFRAME_LPC_QLP_COEFF_PRECISION_LEN  4 /* bits */
#define TCC_FLAC_SUBFRAME_LPC_QLP_SHIFT_LEN  5 /* bits */


#endif	/* TCC_FLAC_DEFINE_H__ */
