#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#include "libavutil/avutil.h"

#include "libswscale/swscale.h"


void nv12_resze(uint8_t *srcBuf, int srcW, int srcH, uint8_t *dstBuf, int dstW, int dstH)
{
    enum AVPixelFormat srcFormat = AV_PIX_FMT_NV12;
    enum AVPixelFormat dstFormat = AV_PIX_FMT_NV12;	

	struct SwsContext *sws;

    sws = sws_getContext(srcW, srcH, srcFormat, dstW, dstH,
                         dstFormat, SWS_FAST_BILINEAR, NULL, NULL, NULL);

    const uint8_t * const nv12_src[4] = { srcBuf, srcBuf+srcW*srcH, NULL, NULL };
    int src_stride[4]   = {srcW, srcW, 0, 0};
   
    uint8_t *nv12_dst[4]     = {dstBuf, dstBuf+dstW*dstH, 0, 0 };
    int dst_stride[4]       = { dstW, dstW, 0, 0};

    sws_scale(sws, nv12_src, src_stride, 0, srcH, nv12_dst, dst_stride);
    sws_freeContext(sws);
}
