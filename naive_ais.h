#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include "rtl-sdr.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
typedef unsigned char uchar;
typedef unsigned short ushort;

extern int dongle;
extern ushort maglut[256*256];
extern ushort modbuf[16*1024]; /* 512 bits (440+) -> 512*26=13312 samples */

#define SpS	26	/* Sample per Symbol */
#define SIG_THRESHOLD 6000  /* minimum signal magnitude */

