

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <syslog.h>


#include <functional>
#include <fstream>



#include "logging.h"



/*--------------------------------------------------
		           Definition
--------------------------------------------------*/

#define LOG_LINE_LENGTH 128

typedef enum
{
	DATA_BUF_EMPTY,
	DATA_BUF_HAS_DATA,
	DATA_BUF_FULL,

} DATA_BUFSTS_T;


/*--------------------------------------------------
		     Fucntion declaration
--------------------------------------------------*/
static void formatData(const uint8_t* in, uint32_t inLen, uint8_t* out, uint32_t outMaxLen, uint32_t& outLen);
static bool isTagFilteredOut(const char* tagIn);
static void output_data(uint8_t level, const char* tag, const uint8_t* data, uint32_t len);
static void output_data_line(uint8_t level, const char* tag, const uint8_t* data, uint32_t len);
static void output_line_ringbuf(const uint8_t* data, uint32_t len);
static void output_format(uint8_t level, const char* tag, const char* format, va_list args_list);



/*--------------------------------------------------
		     Variable
--------------------------------------------------*/
static const uint8_t maxSingleCharExpandLength = 5;

static uint8_t log_level;
static uint8_t log_dump_format;
static uint8_t log_out_destination;
static char* logxTag = NULL;

static const char* conrolCharExpandTextList[] = {
"[NUL]",
"[SOH]",
"[STX]",
"[ETX]",
"[EOT]",
"[ENQ]",
"[ACK]",
"[BEL]",
"[BS]",
"[HT]",
"[LF]",
"[VT]",
"[FF]",
"[CR]",
"[SO]",
"[SI]",
"[DLE]",
"[DC1]",
"[DC2]",
"[DC3]",
"[DC4]",
"[NAK]",
"[SYN]",
"[ETB]",
"[CAN]",
"[EM]",
"[SUB]",
"[ESC]",
"[FS]",
"[GS]",
"[RS]",
"[US]"
};

#define LOGX_RINGBUF_LEN 5000


/*--------------------------------------------------
		Function definition
--------------------------------------------------*/

void logx_init(void)
{
	log_level = LOG_ERR;
	log_out_destination = LOGX_DEST_SYSLOG;
	log_dump_format = LOGX_FMT_NONE;
}


void logx_set_level(uint8_t level)
{
	log_level = level;
}

void logx_set_dump_format(uint8_t format)
{
	log_dump_format = format;
}

void logx_set_out_destination(uint8_t dest)
{
	log_out_destination = dest;
}

uint8_t logx_get_dump_format(void)
{
	return log_dump_format;
}

uint8_t logx_get_out_destination(void)
{
	return log_out_destination;
}


void logx_set_tag(const char* tag)
{
	if(tag == NULL) return;

	/* If not change, then return. */
	if(logxTag && tag)
	{
		/* VS2017: stricmp */
		if(strcasecmp(tag, logxTag) == 0) return;
	}

	if(logxTag != NULL)
	{
		free(logxTag);
	}

	int32_t len = strlen(tag) + 1;

	logxTag = (char*)malloc(len);
	if(logxTag)
	{
		strcpy(logxTag, tag);
	}
}

void _logx_data(uint8_t level, const char* tag, const uint8_t * data, uint32_t len)
{
	if(len == 0) return;


	if(level > log_level) return;

	if(log_out_destination >= LOGX_DEST_ENVALID) return;


	bool showTag = true;
	/* If tag is not empty */
	if(tag != NULL && tag[0] != '\0')
	{
		if(isTagFilteredOut(tag))	return;
	}
	else
	{
		/* If tag is empty, then don't show tag. */
		showTag = false;
	}

	/* if LOGX_FORMAT is LOGX_FMT_NONE, also process it, just copy it, and add a '\0' terminator. */
	uint8_t*  formatedData;
	uint32_t  formatedDataLen = 0;
	uint32_t  maxExpandDataLen;

	if(showTag)
	{
		int32_t tagLength = strlen(tag)+2;
		/* 1 for '\0', 2 for ": " */
		maxExpandDataLen = maxSingleCharExpandLength*len+tagLength+1;
		formatedData = (uint8_t*)malloc(maxExpandDataLen);
		if(formatedData == NULL) return;

		formatData(data, len, &formatedData[tagLength], maxExpandDataLen-tagLength, formatedDataLen);
		memcpy(formatedData, tag, tagLength-2);
		memcpy(&formatedData[tagLength-2], ": ", 2);
		formatedDataLen += tagLength;
		output_data(level, tag, formatedData, formatedDataLen);
		free(formatedData);
	}
	else
	{
		maxExpandDataLen = maxSingleCharExpandLength*len+1;
		formatedData = (uint8_t*)malloc(maxExpandDataLen);
		if(formatedData == NULL) return;
		formatData(data, len, formatedData, maxExpandDataLen, formatedDataLen);
		output_data(level, tag, formatedData, formatedDataLen);
		free(formatedData);
	}
}

void _logx_format(uint8_t level, const char* tag, const char *format, ...)
{
/* Please use the foramt: printf("%s", "This is the print string."); */
#define MAX_FORMAT_STRING_LEN 256
	if(level > log_level) return;

	if(log_out_destination >= LOGX_DEST_ENVALID) return;

	bool showTag = true;
	/* If tag is not empty */
	if(tag != NULL && tag[0] != '\0')
	{
		if(isTagFilteredOut(tag))	return;
	}
	else
	{
		/* If tag is empty, then don't show tag. */
		showTag = false;
	}

	std::string tag_format(tag);
	std::string new_format(format);
	if(showTag) {
		tag_format+=": ";
		new_format=tag_format+new_format;
	}

    va_list args_list;;
    va_start(args_list, format);
	output_format(level, tag, new_format.c_str(), args_list);

    va_end(args_list);

}


void formatData(const uint8_t* in, uint32_t inLen, uint8_t* out, uint32_t outMaxLen, uint32_t& outLen)
{
	int32_t i, writeOffset, len;
	uint8_t data;
	char    tmpText[10];
	uint8_t fmt;
	const char* controlCharText;

	writeOffset = 0;
	fmt = log_dump_format;

	if(outMaxLen == 0) return;

	for(i=0;i<inLen;i++)
	{
		/* Leave one character for string terminate '\0' */
		if(writeOffset + maxSingleCharExpandLength >= outMaxLen)
		{
			break;
		}
		data = in[i];

		if(fmt == LOGX_FMT_NONE || fmt >= LOGX_FMT_ENVALID)
		{
			out[writeOffset++] = data;
		}
		else if(fmt == LOGX_FMT_HEX)
		{
			sprintf(tmpText, "\\x%02X", data);
			strcpy((char*)&out[writeOffset], tmpText);
			writeOffset += 4;
		}
		else if(fmt == LOGX_FMT_PRINTABLE)
		{
			if(data >= 0x20 && data <= 0x7E)
			{
				out[writeOffset++] = data;
			}
			else
			{
				sprintf(tmpText, "\\x%02X", data);
				strcpy((char*)&out[writeOffset], tmpText);
				writeOffset += 4;
			}
		}
		else if(fmt == LOGX_FMT_CONTROL)
		{
			if(data >= 0x20 && data <= 0x7E)
			{
				out[writeOffset++] = data;
			}
			else if(data == 0x7E)
			{
				strcpy((char*)&out[writeOffset], "[DEL]");
				writeOffset += 5;
			}
			else if(data < 0x20)
			{
				controlCharText = conrolCharExpandTextList[data];
				strcpy((char*)&out[writeOffset], controlCharText);
				len = strlen(controlCharText);
				writeOffset += len;
			}
			else
			{
				sprintf(tmpText, "\\x%02X", data);
				strcpy((char*)&out[writeOffset], tmpText);
				writeOffset += 4;
			}
		}
		else
		{
			/* Nothing to do */
		}


	}

	outLen = writeOffset;

	/* Add a string terminator */
	out[writeOffset++] = '\0';
}


bool isTagFilteredOut(const char* tagIn)
{
	#define TOKEN_SEPARATORS "+"

	/* Check the parameters, if logxTag is empty, then passed. */
	if(logxTag == NULL || logxTag[0] == '\0')
	{
		return false;
	}

	/* If logxTag exists, but tagIn is empty, then filtered. */
	if(tagIn == NULL || tagIn[0] == '\0')
	{
		return false;
	}


	/* Allocate a new string to get the token strings */
	int32_t len = strlen(logxTag) + 1;
	char * str = (char*)malloc(len);
	if(str)
	{
		strcpy(str, logxTag);
	}
	else
	{
		return true;
	}

	/* Loop for the token sequences, to find the speicific input tag. */
	bool    isTagExists = false;
	char* tagToken;

#define REENTRANT

#ifdef 	REENTRANT
	char*   rest = str;

	/* Here the strtok_r is used for reentrant, avoid the data race. */
	while(tagToken = strtok_r(rest, TOKEN_SEPARATORS, &rest))
	{
		/* VS2017: stricmp */
		if(strcasecmp(tagToken, tagIn)==0)
		{
			isTagExists = true;
			break;
		}
	}

#else
	tagToken = strtok(str, TOKEN_SEPARATORS);
	while(tagToken != NULL)
	{
		if(strcasecmp(tagToken, tagIn) == 0 )
		{
			isTagExists = true;
			break;
		}
		tagToken = strtok(NULL, TOKEN_SEPARATORS);
	}
#endif

	free(str);

	if(isTagExists)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void output_format(uint8_t level, const char* tag, const char* format, va_list args_list)
{
	uint8_t dest;

	dest = log_out_destination;

	if(dest == LOGX_DEST_STDOUT)
	{
		vprintf(format, args_list);
	}
	else if(dest == LOGX_DEST_SYSLOG)
	{
		vsyslog(level, format, args_list);
	}
	else
	{
		/* Nothing to do */
	}

}


void output_data(uint8_t level, const char* tag, const uint8_t* data, uint32_t len)
{
	uint32_t spiltLen;
	int32_t starPos = 0;

	spiltLen = LOG_LINE_LENGTH;

	while(len>spiltLen)
	{
		output_data_line(level, tag, &data[starPos], spiltLen);
		len -= spiltLen;
		starPos += spiltLen;
	}

	output_data_line(level, tag, &data[starPos], len);
}

void output_data_line(uint8_t level, const char* tag, const uint8_t* data, uint32_t len)
{
	uint8_t dest;

	dest = log_out_destination;

	if(dest == LOGX_DEST_STDOUT)
	{
		printf("%s\n", data);
	}
	else if(dest == LOGX_DEST_SYSLOG)
	{
		syslog(level, "%s", (char*)data);
	}
	else
	{
		/* Nothing to do */
	}

}

// little endian
void shrinkN2MBytes(const char* src, char* dest, uint32_t elementSize, uint32_t n, uint32_t m)
{
	int32_t i, j;
	for(i = 0; i < elementSize; i++)
	{
		for(j = 0; j < m; j++)
		{
			dest[m*i+j] = src[n*i+j];
		}

	}
	/* Doesn't include the terminate part. */
}

void expandN2MBytes(const char* src, char* dest, uint32_t elementSize, uint32_t n, uint32_t m)
{
	int32_t i, j;
	for(i = 0; i < elementSize; i++)
	{
		for(j = 0; j < n; j++)
		{
			dest[m*i+j] = src[n*i+j];
		}
		for(j=n; j<m; j++)
		{
			dest[m*i+j] = 0;
		}
	}
}


void getCompileDateAndTime(char* str, uint32_t maxLen)
{


}


