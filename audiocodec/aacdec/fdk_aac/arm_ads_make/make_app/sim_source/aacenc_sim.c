
#include <stdio.h>
#include <stdlib.h>



#if 1

#include "aenc.h"

//cdk version

#define SAVING_BUFFERSIZE (1024*256)
#define AUDIO_ENC_BUF_SIZE 40
#define AUDIO_ENC_OUTBUF_SIZE (1024*256)

//For Audio Only Encoder
int		gsAudioEncHandle;
aenc_input_t 	gsAEncInput;
aenc_output_t gsAEncOutput;
aenc_init_t 	gsAEncInit;

unsigned char gpucPcmBuffer[SAVING_BUFFERSIZE];
unsigned char gpucBinBuffer[AUDIO_ENC_OUTBUF_SIZE];
FILE *infile, *outfile;

// inputfile outputfile profile transport frequency channel vbrmode bitrate
// ex) infilename outfilename 0 2 44100 2 1 10000
int main(int argc, char **pp_argv)
{
 // OPEN
	int ret;
	int channel = 2;
	int frequency = 44100;
	int bitrate = 100000;
	int eAACProfile; //0 : LC, 1 : HE, 2 : HE_PS, 3 : LD, 4 : ELD, 5 : LTP, 6 : SCALABLE 
	int eTransport;  //0: RAW-AAC, 1: ADIF, 2: ADTS, 3: LATM
	int vbrmode = 0; //0: CBR, 1: VBR_1, 2: VBR_2, 3: VBR_3, 4: VBR_4, 5: VBR_5
	int encSampleCnt, n_read, read_num, eof=0;
	int outAvailable = AUDIO_ENC_OUTBUF_SIZE;
	aenc_input_t 	*pAEncInput 	= &gsAEncInput;
	aenc_output_t 	*pAEncOutput 	= &gsAEncOutput;
	aenc_init_t 	*pAEncInit		= &gsAEncInit;
	
	infile = fopen(pp_argv[1], "rb");
	outfile = fopen(pp_argv[2], "wb");

	if( infile == NULL )
	{
		printf( "[file %s] file open failed \n", pp_argv[1] );
		return -1;
	}
	if( outfile == NULL )
	{
		printf( "[file %s] file open failed \n", pp_argv[2] );
		return -1;
	}

	eAACProfile = atoi(pp_argv[3]); //0 : LC, 1 : HE, 2 : HE_PS
	eTransport = atoi(pp_argv[4]);  //0: RAW-AAC, 1: ADIF, 2: ADTS, 3: LATM

	frequency = atoi(pp_argv[5]);
	channel = atoi(pp_argv[6]);
	vbrmode = atoi(pp_argv[7]);
	bitrate = atoi(pp_argv[8]);
 
	// Initialize aenc Structures
	memset( pAEncInput, 	0x00, sizeof(aenc_input_t) );
	memset( pAEncOutput, 	0x00, sizeof(aenc_output_t) );
	memset( pAEncInit,  	0x00, sizeof(aenc_init_t) );
	
	// audio encoder common
	pAEncInput->m_eSampleRate 		= frequency;
	pAEncInput->m_uiNumberOfChannel = channel;
	pAEncInput->m_uiBitsPerSample 	= bitrate;

	// default : 16
	if( pAEncInput->m_uiBitsPerSample == 0 )
	{
		pAEncInput->m_uiBitsPerSample = 16;
	}

	pAEncInput->m_iNchannelOrder[CH_LEFT_FRONT] = 1;	//first channel
	pAEncInput->m_iNchannelOrder[CH_RIGHT_FRONT] = 2;	//second channel

	// use pcm_array
	if (1) //(encodeInfo->m_iAudioEncProcessMode == 0)
	{
		pAEncInput->m_ePcmInterLeaved = TCAS_ENABLE;	// 0 or 1
		pAEncInput->m_iNchannelOffsets[0] = 0;
		pAEncInput->m_iNchannelOffsets[1] = 2;
	}
	// use DAC(mic, aux)
	else
	{
		//not support yet
	}	

	// Audio callback
	pAEncInit->m_sCommonInfo.m_psAudioCallback = malloc(sizeof(aenc_callback_func_t));
	if(pAEncInit->m_sCommonInfo.m_psAudioCallback == NULL)
	{
		printf("m_psAudioCallback allocation fail\r\n");
		return -1;
	}
	pAEncInit->m_sCommonInfo.m_psAudioCallback->m_pfMalloc 	= malloc;
	pAEncInit->m_sCommonInfo.m_psAudioCallback->m_pfRealloc = realloc;
	pAEncInit->m_sCommonInfo.m_psAudioCallback->m_pfFree 	= free;
//	pAEncInit->m_sCommonInfo.m_psAudioCallback->m_pfMemcpy 	=;
//	pAEncInit->m_sCommonInfo.m_psAudioCallback->m_pfMemmove =;
//	pAEncInit->m_sCommonInfo.m_psAudioCallback->m_pfMemset 	=;
	pAEncInit->m_sCommonInfo.m_psAudioEncInput 				= pAEncInput;
	pAEncInit->m_sCommonInfo.m_psAudioEncOutput 			= pAEncOutput;
	

	// Audio encoder output setting
	pAEncOutput->m_uiNumberOfChannel = channel;
	pAEncOutput->m_uiBitRates 		 = bitrate;
	pAEncInit->m_unAudioEncodeParams.m_unAAC.m_iAACProfileType = eAACProfile;
	pAEncInit->m_unAudioEncodeParams.m_unAAC.m_iAACHeaderType = eTransport;
	pAEncInit->m_unAudioEncodeParams.m_unAAC.m_iBitrateMode = vbrmode;

	pAEncInit->m_sCommonInfo.m_iMaxStreamLength = 2048;
	pAEncOutput->m_uiSamplesPerChannel = 2048;			

	pAEncOutput->m_pcStream = (char *)gpucBinBuffer;
	pAEncOutput->m_iStreamLength = 0;

	/* set input pcm buffer */
//	pAEncInit->m_sCommonInfo.m_iPcmLength = pAEncOutput->m_uiNumberOfChannel * pAEncOutput->m_uiSamplesPerChannel * sizeof(short)* AUDIO_ENC_BUF_SIZE;
//	encodeInfo->m_pcPcmBuffer = (unsigned char*)encodeInfo->m_sCallback.m_pfMalloc(pAEncInit->m_sCommonInfo.m_iPcmLength);
//	if(encodeInfo->m_pcPcmBuffer == NULL)
//	{
//		TestPrint("m_pcPcmBuffer allocation fail\r\n");
//		return -1;
//	}
	
	pAEncInput->m_pvChannel[0] = (void *)gpucPcmBuffer;

	ret = TCC_AAC_ENC( AUDIO_INIT, &gsAudioEncHandle, pAEncInit, NULL);
	if( ret < 0 )
	{
		// If AUDIO_INIT failed, close AAC Codec
		TCC_AAC_ENC( AUDIO_CLOSE, &gsAudioEncHandle, NULL, NULL );
		return ret;
	}

	if( (pAEncOutput->m_uiNumberOfChannel==0) || (pAEncOutput->m_eSampleRate==0) || (pAEncOutput->m_uiSamplesPerChannel==0) )
	{
		printf("Invalid Parameter\r\n");
		return -1;
	}
	pAEncInput->m_uiSamplesPerChannel = pAEncOutput->m_uiSamplesPerChannel;
	pAEncInput->m_uiNumberOfChannel	  = pAEncOutput->m_uiNumberOfChannel;
	pAEncInput->m_eSampleRate		  = pAEncOutput->m_eSampleRate;
	
//	if (pAEncOutput->m_iStreamLength > 0)	//copy configdata
//		fwrite(pAEncOutput->m_pcStream,pAEncOutput->m_iStreamLength,1,outfile);		


	while (!eof)
	{
		read_num = pAEncInput->m_uiNumberOfChannel * pAEncInput->m_uiSamplesPerChannel * sizeof(short);
		n_read = fread(gpucPcmBuffer, 1, read_num, infile);
		if(n_read != read_num)
		{
			if( n_read == 0 ) {
				eof = 1;
				break;
			}
			if( n_read < read_num )
			{
				memset( gpucPcmBuffer + n_read, 0, read_num - n_read );
			}
		}

		pAEncInput->m_pvChannel[0] = (void *)gpucPcmBuffer;
		pAEncOutput->m_pcStream = (char *)gpucBinBuffer;
		pAEncOutput->m_iStreamLength = 0;

		ret = TCC_AAC_ENC( AUDIO_ENCODE, &gsAudioEncHandle, pAEncInput, pAEncOutput);
		if( ret < 0 )
		{
			return ret;
		}		

		if (pAEncOutput->m_iStreamLength > 0) {
			fwrite(pAEncOutput->m_pcStream,pAEncOutput->m_iStreamLength,1,outfile);		
		}
  }

  TCC_AAC_ENC(AUDIO_CLOSE, &gsAudioEncHandle, NULL, NULL);

  fclose(infile);
  fclose(outfile);

  return 0;
}
#else

#include "aacenc_lib.h"
#include "FDK_audio.h"

//simulation version

#define LOGD(...)     {printf("[AACENC:D][%s:%d] ",__func__,__LINE__);printf(__VA_ARGS__);printf("\x1b[0m\n");}
#define LOGW(...)     {printf("[AACENC:W][%s:%d] ",__func__,__LINE__);printf(__VA_ARGS__);printf("\x1b[0m\n");}
#define LOGE(...)     {printf("[AACENC:E][%s:%d] ",__func__,__LINE__);printf(__VA_ARGS__);printf("\x1b[0m\n");}


typedef enum OMX_AUDIO_AACPROFILETYPE{
  OMX_AUDIO_AACObjectNull = 0,      /**< Null, not used */
  OMX_AUDIO_AACObjectMain = 1,      /**< AAC Main object */
  OMX_AUDIO_AACObjectLC,            /**< AAC Low Complexity object (AAC profile) */
  OMX_AUDIO_AACObjectSSR,           /**< AAC Scalable Sample Rate object */
  OMX_AUDIO_AACObjectLTP,           /**< AAC Long Term Prediction object */
  OMX_AUDIO_AACObjectHE,            /**< AAC High Efficiency (object type SBR, HE-AAC profile) */
  OMX_AUDIO_AACObjectScalable,      /**< AAC Scalable object */
  OMX_AUDIO_AACObjectERLC = 17,     /**< ER AAC Low Complexity object (Error Resilient AAC-LC) */
  OMX_AUDIO_AACObjectLD = 23,       /**< AAC Low Delay object (Error Resilient) */
  OMX_AUDIO_AACObjectHE_PS = 29,    /**< AAC High Efficiency with Parametric Stereo coding (HE-AAC v2, object type PS) */
  OMX_AUDIO_AACObjectELD = 39,      /** AAC Enhanced Low Delay. NOTE: Pending Khronos standardization **/
  OMX_AUDIO_AACObjectKhronosExtensions = 0x6F000000, /**< Reserved region for introducing Khronos Standard Extensions */
  OMX_AUDIO_AACObjectVendorStartUnused = 0x7F000000, /**< Reserved region for introducing Vendor Extensions */
  OMX_AUDIO_AACObjectMax = 0x7FFFFFFF
} OMX_AUDIO_AACPROFILETYPE;


static CHANNEL_MODE getChannelMode(unsigned int nChannels) {
    CHANNEL_MODE chMode = MODE_INVALID;
    switch (nChannels) {
        case 1: chMode = MODE_1; break;
        case 2: chMode = MODE_2; break;
        case 3: chMode = MODE_1_2; break;
        case 4: chMode = MODE_1_2_1; break;
        case 5: chMode = MODE_1_2_2; break;
        case 6: chMode = MODE_1_2_2_1; break;
        default: chMode = MODE_INVALID;
    }
    return chMode;
}

static AUDIO_OBJECT_TYPE getAOTFromProfile(unsigned int profile) {
    if (profile == OMX_AUDIO_AACObjectLC) {
        return AOT_AAC_LC;
    } else if (profile == OMX_AUDIO_AACObjectHE) {
        return AOT_SBR;
    } else if (profile == OMX_AUDIO_AACObjectHE_PS) {
        return AOT_PS;
    } else if (profile == OMX_AUDIO_AACObjectLD) {
        return AOT_ER_AAC_LD;
    } else if (profile == OMX_AUDIO_AACObjectELD) {
        return AOT_ER_AAC_ELD;
    } else {
        LOGW("Unsupported AAC profile - defaulting to AAC-LC");
        return AOT_AAC_LC;
    }
}

#define MAX_PCM_BUFFER 0x10000
#define MAX_BIN_BUFFER 0x2000

unsigned char gpucPcmBuffer[MAX_PCM_BUFFER];
unsigned char gpucBinBuffer[MAX_BIN_BUFFER];
FILE *infile, *outfile;

int main(int argc, char **pp_argv)
{
 // OPEN
	int ret;
	HANDLE_AACENCODER mAACEncoder;  
	unsigned int channel = 2;
	unsigned int frequency = 44100;
	unsigned int bitrate = 29000;
	OMX_AUDIO_AACPROFILETYPE eAACProfile = OMX_AUDIO_AACObjectHE_PS;
	AUDIO_OBJECT_TYPE aot;
	int encSampleCnt, n_read, read_num, eof=0;
	int outAvailable = MAX_BIN_BUFFER;
	
	AACENC_InArgs inargs;
	AACENC_OutArgs outargs;
	
	infile = fopen(pp_argv[1], "rb");
	outfile = fopen(pp_argv[2], "wb");

	if( infile == NULL )
	{
		printf( "[file %s] file open failed \n", pp_argv[1] );
		return -1;
	}
	
	aot = getAOTFromProfile(eAACProfile);		

	/* Open aac encoder */
	LOGD("aacEncOpen Start");
	if (aacEncOpen(&mAACEncoder, 0, channel) != AACENC_OK)
	{
		LOGE("Failed to open AAC encoder");
		return -1;
	}
	if (aacEncoder_SetParam(mAACEncoder, AACENC_AOT, aot) != AACENC_OK) 
	{
		LOGE("Failed to set AAC encoder AACENC_AOT parameters");
		return 1;
	}
	if (aacEncoder_SetParam(mAACEncoder, AACENC_SAMPLERATE, frequency) != AACENC_OK) 
	{
		LOGE("Failed to set AAC encoder AACENC_SAMPLERATE parameters");
		return 1;
	}
	if (aacEncoder_SetParam(mAACEncoder, AACENC_CHANNELORDER, 1) != AACENC_OK) 
	{
		LOGE("Unable to set the wav channel order");
		return -1;
	}
	if (aacEncoder_SetParam(mAACEncoder, AACENC_BITRATE, bitrate) != AACENC_OK) 
	{
		LOGE("Failed to set AAC encoder AACENC_BITRATE parameters\n");
		return -1;
	}
	if (aacEncoder_SetParam(mAACEncoder, AACENC_CHANNELMODE, getChannelMode(channel)) != AACENC_OK) 
	{
		LOGE("Failed to set AAC encoder AACENC_CHANNELMODE parameters\n");
		return -1;
	}
	/* Initilize aac encoder */
	LOGD("aacEncEncode - initialize Start");
	if (AACENC_OK !=  aacEncEncode(mAACEncoder, NULL, NULL, NULL, NULL))
	{
		LOGE("Unable to initialize encoder for profile / sample-rate / bit-rate / channels");
		return -1;
	}
	LOGD("aacEncOpen Done");

	unsigned int actualBitRate  = aacEncoder_GetParam(mAACEncoder, AACENC_BITRATE);
	if (bitrate != actualBitRate) {
		LOGW("Requested bitrate %u unsupported, using %u", bitrate, actualBitRate);
	}

	AACENC_InfoStruct encInfo;
	if (AACENC_OK != aacEncInfo(mAACEncoder, &encInfo)) {
		LOGE("Failed to get AAC encoder info");
		return;
	}
	
	fwrite(encInfo.confBuf,encInfo.confSize,1,outfile);		

	// Limit input size
	switch (eAACProfile)
	{
		case OMX_AUDIO_AACObjectHE:
		case OMX_AUDIO_AACObjectHE_PS:
			encSampleCnt = 2048 * channel;
			break;	
		case OMX_AUDIO_AACObjectLD:
		case OMX_AUDIO_AACObjectELD:
			encSampleCnt = 512 * channel;
			break;
		case OMX_AUDIO_AACObjectLC:
		default:
			encSampleCnt = 1024 * channel;
			break;
	}

	
		while (!eof)
		{

			read_num = encSampleCnt * sizeof(short);
			n_read = fread(gpucPcmBuffer, 1, read_num, infile);
			if(n_read != read_num)
			{
				if( n_read == 0 ) {
					eof = 1;
					break;
				}
				if( n_read < read_num )
				{
					memset( gpucPcmBuffer + n_read, 0, read_num - n_read );
				}
			}

		/*********************************************************/
		///////////////////////ENCODER Setting/////////////////////
		/*********************************************************/
		memset(&inargs, 0, sizeof(inargs));
		memset(&outargs, 0, sizeof(outargs));
		INT_PCM *mInputFrame = gpucPcmBuffer;
		inargs.numInSamples = read_num / sizeof(INT_PCM);

		//void *inPtr = (INT_PCM *)omx_private->remainBuf;
		int   inBufferIds       = IN_AUDIO_DATA;
		int   inBufferSize      = read_num;
		int   inBufferElSize = sizeof(INT_PCM);

		AACENC_BufDesc inBufDesc = { 0 };
		inBufDesc.numBufs           = 1;
		inBufDesc.bufs              = &mInputFrame; //omx_private->remainBuf;
		inBufDesc.bufferIdentifiers = &inBufferIds;
		inBufDesc.bufSizes          = &inBufferSize;
		inBufDesc.bufElSizes        = &inBufferElSize;
        
		void *outPtr = gpucBinBuffer;
		int   outBufferIds   = OUT_BITSTREAM_DATA;
		int   outBufferSize  = outAvailable;
		int   outBufferElSize= sizeof(UCHAR);
        
		AACENC_BufDesc outBufDesc = { 0 };
		outBufDesc.numBufs           = 1;
		outBufDesc.bufs              = &outPtr;
		outBufDesc.bufferIdentifiers = &outBufferIds;
		outBufDesc.bufSizes          = &outBufferSize;
		outBufDesc.bufElSizes        = &outBufferElSize;
		///////////////////////////////////////////////////

		/* Encode aac */
    ret = aacEncEncode(mAACEncoder,
                               &inBufDesc,
                               &outBufDesc,
                               &inargs,
                               &outargs);
            
		if (ret == 0)
		{			
        fwrite(outPtr,outargs.numOutBytes,1,outfile);		
		}
  }

  aacEncClose(&mAACEncoder);

  fclose(infile);
  fclose(outfile);

  return 0;
}


#endif
