/*
 * This is the header file for the MD5 message-digest algorithm.
 * The algorithm is due to Ron Rivest.  This code was
 * written by Colin Plumb in 1993, no copyright is claimed.
 * This code is in the public domain; do with it what you wish.
 *
 * Equivalent code is available from RSA Data Security, Inc.
 * This code has been tested against that, and is equivalent,
 * except that you don't need to include two pages of legalese
 * with every copy.
 *
 * To compute the message digest of a chunk of bytes, declare an
 * MD5Context structure, pass it to MD5Init, call MD5Update as
 * needed on buffers full of bytes, and then call MD5Final, which
 * will fill a supplied 16-byte array with the digest.
 *
 * Changed so as no longer to depend on Colin Plumb's `usual.h'
 * header definitions; now uses stuff from dpkg's config.h
 *  - Ian Jackson <ijackson@nyx.cs.du.edu>.
 * Still in the public domain.
 *
 * Josh Coalson: made some changes to integrate with libFLAC.
 * Still in the public domain, with no warranty.
 * 
 * Telechips : made some changes to fit the coding style.
 * Still in the public domain, with no warranty.
 */

#ifndef TCC_FLAC_MD5_H__
#define TCC_FLAC_MD5_H__

#define md5byte TCAS_U8

struct TCC_FLAC_MD5Context 
{
	TCAS_U32 pucBuf[4];
	TCAS_U32 uiBytes[2];
	TCAS_U32 in[16];
	TCAS_U8 *internal_buf;
	TCAS_U32 capacity;
};

TCASVoid TCC_FLAC_MD5Init(struct TCC_FLAC_MD5Context *pstContext);
TCASVoid TCC_FLAC_MD5Update(struct TCC_FLAC_MD5Context *pstContext, md5byte const *pucBuf, TCAS_U32 uiLength);
TCASVoid TCC_FLAC_MD5Final(md5byte digest[16], struct TCC_FLAC_MD5Context *pstContext);
TCASVoid TCC_FLAC_MD5Transform(TCAS_U32 pucBuf[4], TCAS_U32 const puiIn[16]);

TCAS_S32 TCC_FLAC_MD5Accumulate(struct TCC_FLAC_MD5Context *pstContext, const TCAS_S32 * const piSignal[], TCAS_U32 uiChannels, TCAS_U32 uiSamples, TCAS_U32 uiBytesPerSample);

#endif /* TCC_FLAC_MD5_H__ */
