
#ifndef LOGGING_H
#define LOGGING_H

/*--------------------------------------------------
		           Definition
--------------------------------------------------*/


/* Log level definition for syslog */
#if 0
#define LOG_EMERG          0       /* system is unusable */
#define LOG_ALERT          1       /* action must be taken immediately */
#define LOG_CRIT           2       /* critical conditions */
#define LOG_ERR            3       /* error conditions */
#define LOG_WARNING        4       /* warning conditions */
#define LOG_NOTICE         5       /* normal but significant condition */
#define LOG_INFO           6       /* informational */
#define LOG_DEBUG          7       /* debug-level messages */
#endif


/* Log output destination */
enum {
	LOGX_DEST_STDOUT,
	LOGX_DEST_SYSLOG,
	LOGX_DEST_ENVALID,
};



/* Log format for binary data */
enum {
	LOGX_FMT_NONE,
	LOGX_FMT_HEX,
	/* Alphabet + digit + punctuation + space */
	LOGX_FMT_PRINTABLE,
	/* octal codes 0x00 through 0x1F, and 0x7F (DEL) */
	LOGX_FMT_CONTROL,
	LOGX_FMT_ENVALID,
};


/*--------------------------------------------------
		     Fucntion declaration
--------------------------------------------------*/

#define logx_data(level,tag,data,len) _logx_data(level,tag,data,len)
#define logx_format(level,tag,format, ...) _logx_format(level, tag, format, ## __VA_ARGS__)

void _logx_data(uint8_t level, const char* tag, const uint8_t * data, uint32_t len);
void _logx_format(uint8_t level, const char* tag, const char *format, ...);


void logx_init(void);
void logx_set_level(uint8_t level);
void logx_set_dump_format(uint8_t format);
void logx_set_out_destination(uint8_t dest);
uint8_t logx_get_dump_format(void);
uint8_t logx_get_out_destination(void);

void logx_set_tag(const char* tag);


/* User interface mapping */
#define logx_specail(level, tag, format, ...)  logx_format(level, tag, format, ## __VA_ARGS__)



void shrinkN2MBytes(const char* src, char* dest, uint32_t elementSize, uint32_t n, uint32_t m);
void expandN2MBytes(const char* src, char* dest, uint32_t elementSize, uint32_t n, uint32_t m);

void getCompileDateAndTime(char* str, uint32_t maxLen);



#endif
