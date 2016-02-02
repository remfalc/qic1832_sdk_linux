/*
--       This software proprietary belong to Quanta computer Inc. and may be used        --
--        only as expressly authorized agreement from          --
--                            Quanta Computer Inc..                             --
--                   (C) COPYRIGHT 2010 Quanta Computer Inc.                    --
--                            ALL RIGHTS RESERVED                             --
--                 The entire notice above must be reproduced                 --
--                  on all copies and should not be removed.                  --
*/

#ifndef _QIC_API_COMMON_DEFINE_H_
#define _QIC_API_COMMON_DEFINE_H_

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sched.h>
#include <assert.h>
#include <time.h>
#include <stddef.h>

#include "qic_cfg.h"
#include "qic_xuctrl.h"

/* debug message type define */
#define DEBUG_ERROR     0x01
#define DEBUG_INFO      0x02
#define DEBUG_FRAME     0x04
#define DEBUG_DETAIL    0x08

/*simulcast stream id define*/
#define MAX_SIMULCAST_STREAM 4
#define STREAM0 0
#define STREAM1 1
#define STREAM2 2
#define STREAM3 3

/* device ID define - dirty */
#define DEV_ID_0 0x01
#define DEV_ID_1 0x02
#define DEV_ID_2 0x04
#define DEV_ID_3 0x08
#define DEV_ID_4 0x10
#define DEV_ID_5 0x20
#define DEV_ID_6 0x40
#define DEV_ID_7 0x80

/* flip define */
#define H_FLIP 0x01
#define V_FLIP 0x02

#define PTZ_PAN_STEP    0xE10
#define PTZ_TILT_STEP   0xE10
#define PTZ_ZOOM_STEP   0x01

#define CLEAR(x) memset (&(x), 0, sizeof (x))
#define FOURCC_FORMAT		"%c%c%c%c"
#define FOURCC_ARGS(c)		(c) & 0xFF, ((c) >> 8) & 0xFF, ((c) >> 16) & 0xFF, ((c) >> 24) & 0xFF
#define TIME_DELAY(ms)		usleep(ms*1000)
#define MAX(a, b) ((a) < (b) ? (b) : (a))


typedef struct {
    long vid;
    long pid;
    long revision;
    unsigned int svn;
    unsigned char fw_api_version;
} version_info_t;

typedef struct {
    char dev_yuv[16];
    char dev_avc[16];
}qic_dev_name_s;

typedef struct {
    void * start;
    size_t length;
}mmap_buffer_s;

typedef struct{
    unsigned short  width;
    unsigned short  height;
    unsigned int frame_interval;
    unsigned int bitrate;
    unsigned short key_frame_interval;
}simulcast_config_t;

typedef struct{
    simulcast_config_t configs[MAX_SIMULCAST_STREAM];
    char config_all;
}simulcast_configs_t;

typedef struct {
    unsigned char bPframe;   		   //is P frame
    unsigned char bencding_stream;   //is encoding stream
    unsigned int stream_id;                //stream id of simulcast streams
    unsigned long timestamp;		   // timestamp
    unsigned int temporal_layer_id;    // temporal layer id of steram
    unsigned int frame_len;                // frame size
    char * frame_data;			   // frame data
#ifdef QIC_MD_API
    md_status_t md_status;
#endif
} out_frame_t;

typedef struct {
    /* device parameters*/
    char* dev_name;
    int fd;
    unsigned int dev_id; /* for dyn-config each device, generated by config_initial */
    unsigned char is_bind; /*for bind check, 2-way video*/

    /* video parameters */
    unsigned int format; /* YUV or MPEG2TS */

    unsigned int bitrate;  /* ~8000000bps */
    unsigned short width; /* ~1920 */
    unsigned short height; /* ~1080 */
    unsigned char framerate; /* fps */
    int gop; /* group of picture */
    unsigned char slicesize; /* 0 -auto, 1or up - slice height, i.e. 640 * (16*slicesize) */
    /* HD resolution force to 0 in Linux */

    unsigned char is_demux; /* on or off (need "much" CPU resource) */

    /* NRI related */
    unsigned char nri_iframe;
    unsigned char nri_sps;
    unsigned char nri_pps;
    unsigned char nri_set;

    /* internal usage */
    unsigned char is_on; /* start / stop capture */
    mmap_buffer_s* buffers; /* mmap buffer */
    unsigned int buffer_start_address; /* __not implement__ */
    unsigned int num_mmap_buffer; /* for kernel UVC buffer */
    unsigned char skype_stream;  /*for SECS2.2 api*/
    unsigned int frame_interval;

    unsigned char is_encoding_video;
    EuExSelectCodec_t codec_type;  /*codec type*/
    unsigned short key_frame_interval;
    simulcast_configs_t simulcast_configs_setting;
    /* raw V4L2 Data Dump int length, char *data */
    void (*raw_dump)(int, char*);
} qic_dev;

typedef struct {
    /* for config initialization */
    unsigned int num_devices; /* max 8 devices */
    qic_dev *cam;

    /* output frame buffer: dev_id, length, data, timestamp*/
    void (*frame_output)(unsigned int, unsigned int, char *, unsigned long);
#if defined(QIC1822)&& defined(QIC_SIMULCAST_API)
    /* output simulcast frame buffer: dev_id, simulcat_frame_t*/
    void (*frame_output2)(unsigned int, out_frame_t);
#endif
    /*debug info print: int level, char* string*/
    void (*debug_print)(int, char*);
    /* debug msg type - ERROR, INFO, FRAME, */
    unsigned int debug_msg_type;

    /* __TEST__ higher prio for less frame lost */
    unsigned char high_prio;
} qic_module;

typedef enum {
    FW_PASS 		= 0,   //FW upgarde success
    FW_FAIL 		= 1,   //FW upgarde fail
    FW_NO_2ND_BL 		= 2,   //no found 2nd Boot rom on flash
    FW_IS_STREAMING 	= 3,   //qic video/audio is streaming
    FW_IS_DOWNLAODING	= 4
} FWDLERRCODE_e;

#ifdef DEBUG_LOG
/* debug output function */
#   define LOG_PRINT(str, level, format, arg...) { \
    memset (str, 0, sizeof(str)); \
    snprintf(str, sizeof(str), format, ##arg); \
    (*dev_pt->debug_print)(level, str); \
    }

#   define LOG_XU_PRINT(str_d, str_x, ret) LOG_PRINT(str_d, !ret ? DEBUG_INFO:DEBUG_ERROR, "[xuctrl] %s\n", str_x)
/* debug level, 0:information, 1:warning, 2:critical, 3:fatal */
/*static char *debug_level[] = {"INFO", "WARN", "CRIT", "FATL", "DEAD"};*/
#else
#   define LOG_PRINT(str, level, format, arg...){}
#   define LOG_XU_PRINT(str_d, str_x, ret){}
#endif

// Common
int xioctl(int fd, int request, void * arg);

#endif