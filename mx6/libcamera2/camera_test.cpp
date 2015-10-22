#include <utils/Log.h>

#include <string.h>
#include <unistd.h>
#include <time.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <utils/threads.h>
#include <utils/RefBase.h>
#include <binder/MemoryBase.h>
#include <binder/MemoryHeapBase.h>
#include <camera/CameraParameters.h>
#include <utils/Vector.h>
#include <utils/KeyedVector.h>
#include <cutils/properties.h>
#include <hardware_legacy/power.h>
#include <ui/GraphicBufferMapper.h>
#include <ui/Rect.h>
#include <ui/PixelFormat.h>
#include <system/window.h>
#include <system/camera.h>
#include "gralloc_priv.h"
#include <linux/videodev2.h>
#include <hardware/camera2.h>

#include <ion/ion.h>

struct StreamBuffer {
	int mIndex;
    int mWidth;
    int mHeight;
    int mFormat;
    void *mVirtAddr;
    int mPhyAddr;
    size_t mSize;
    buffer_handle_t mBufHandle;
    nsecs_t mTimeStamp;

    //for uvc jpeg stream
    int mBindUVCBufIdx;
    void *mpFrameBuf;
};

struct v4l2_capability     mCap;
struct StreamBuffer *mCameraBuffer[3];
int mIonFd = -1;
int mCamFd = -1;

#define DEV_NAME "/dev/video0"

#define FLOG_TRACE(format, ...) ALOGI((format), ## __VA_ARGS__)
#define FLOGI(format, ...) ALOGI((format), ## __VA_ARGS__)
#define FLOGW(format, ...) ALOGW((format), ## __VA_ARGS__)
#define FLOGE(format, ...) ALOGE((format), ## __VA_ARGS__)
#define FLOG_RUNTIME(format, ...) ALOGI((format), ## __VA_ARGS__)

#define fAssert(e) ((e) ? (void)0 : __assert2(__FILE__, __LINE__, __func__, #e))

int convertPixelFormatToV4L2Format(PixelFormat format)
{
    int nFormat = 0;

    switch (format) {
        case HAL_PIXEL_FORMAT_YCbCr_420_SP:
        case HAL_PIXEL_FORMAT_YCrCb_420_SP:
            nFormat = v4l2_fourcc('N', 'V', '1', '2');
            break;

        case HAL_PIXEL_FORMAT_YCbCr_420_P:
            nFormat = v4l2_fourcc('Y', 'U', '1', '2');
            break;

        case HAL_PIXEL_FORMAT_YCbCr_422_I:
            nFormat = v4l2_fourcc('Y', 'U', 'Y', 'V');
            break;

        default:
            FLOGE("Error: format:0x%x not supported!", format);
            break;
    }
    FLOGI("pixel format: 0x%x", nFormat);
    return nFormat;
}

PixelFormat convertV4L2FormatToPixelFormat(unsigned int format)
{
    PixelFormat nFormat = 0;

    switch (format) {
        case v4l2_fourcc('N', 'V', '1', '2'):
            nFormat = HAL_PIXEL_FORMAT_YCbCr_420_SP;
            break;

        case v4l2_fourcc('Y', 'U', '1', '2'):
            nFormat = HAL_PIXEL_FORMAT_YCbCr_420_P;
            break;

        case v4l2_fourcc('Y', 'U', 'Y', 'V'):
            nFormat = HAL_PIXEL_FORMAT_YCbCr_422_I;
            break;

        default:
            FLOGE("Error: format:0x%x not supported!", format);
            break;
    }
    FLOGI("pixel format: 0x%x", nFormat);
    return nFormat;
}

int startDevice()
{
    status_t ret = NO_ERROR;

 
    int state;
    struct v4l2_buffer buf;
    for (int i = 0; i < 3; i++) {
        StreamBuffer* frame = mCameraBuffer[i];

        memset(&buf, 0, sizeof (struct v4l2_buffer));
        buf.index    = i;
        buf.type     = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory   = V4L2_MEMORY_USERPTR;
        buf.m.offset = frame->mPhyAddr;
        ret = ioctl(mCamFd, VIDIOC_QBUF, &buf);
        if (ret < 0) {
            FLOGE("VIDIOC_QBUF Failed");
            return BAD_VALUE;
        }
    }

    enum v4l2_buf_type bufType;
    if (1) {
        bufType = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        ret = ioctl(mCamFd, VIDIOC_STREAMON, &bufType);
        if (ret < 0) {
            FLOGE("VIDIOC_STREAMON failed: %s", strerror(errno));
            return ret;
        }
    }

    FLOGI("Created device thread");
    return ret;
}


int registerCameraBuffers(StreamBuffer *pBuffer,
                                             int        & num)
{
    status_t ret = NO_ERROR;

    if ((pBuffer == NULL) || (num <= 0)) {
        FLOGE("requestCameraBuffers invalid pBuffer");
        return BAD_VALUE;
    }

    struct v4l2_requestbuffers req;
    memset(&req, 0, sizeof (req));
    req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_USERPTR;
    req.count  = num;

    ret = ioctl(mCamFd, VIDIOC_REQBUFS, &req);
    if (ret < 0) {
        FLOGE("VIDIOC_REQBUFS failed: %s", strerror(errno));
        return ret;
    }

    struct v4l2_buffer buf;
    for (int i = 0; i < num; i++) {
        CameraFrame *buffer = pBuffer + i;
        memset(&buf, 0, sizeof (buf));
        buf.index    = i;
        buf.type     = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory   = V4L2_MEMORY_USERPTR;
        buf.m.offset = buffer->mPhyAddr;
        buf.length   = buffer->mSize;

        ret = ioctl(mCamFd, VIDIOC_QUERYBUF, &buf);
        if (ret < 0) {
            FLOGE("Unable to query buffer (%s)", strerror(errno));
            return ret;
        }

        // Associate each Camera buffer
       // buffer->setObserver(this);
       // mDeviceBufs[i] = buffer;
    }

   // mBufferSize  = pBuffer->mSize;
   // mBufferCount = num;

    return ret;
}


int setDeviceConfig(int         width,
                                        int         height,
                                        PixelFormat format,
                                        int         fps)
{
    if (mCamFd <= 0) {
        FLOGE("setDeviceConfig: DeviceAdapter uninitialized");
        return BAD_VALUE;
    }
    if ((width == 0) || (height == 0)) {
        FLOGE("setDeviceConfig: invalid parameters");
        return BAD_VALUE;
    }

    status_t ret = NO_ERROR;
    int input    = 1;
    ret = ioctl(mCamFd, VIDIOC_S_INPUT, &input);
    if (ret < 0) {
        FLOGE("Open: VIDIOC_S_INPUT Failed: %s", strerror(errno));
        return ret;
    }

    int vformat;
    vformat = convertPixelFormatToV4L2Format(format);

    if ((width > 1920) || (height > 1080)) {
        fps = 15;
    }
    FLOGI("Width * Height %d x %d format 0x%x, fps: %d",
          width, height, vformat, fps);

  //  mVideoInfo->width       = width;
  //  mVideoInfo->height      = height;
 //   mVideoInfo->framesizeIn = (width * height << 1);
 //   mVideoInfo->formatIn    = vformat;

    struct v4l2_streamparm param;
    memset(&param, 0, sizeof(param));
    param.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    param.parm.capture.timeperframe.numerator   = 1;
    param.parm.capture.timeperframe.denominator = fps;
    param.parm.capture.capturemode = 0;//getCaptureMode(width, height);
    ret = ioctl(mCamFd, VIDIOC_S_PARM, &param);
    if (ret < 0) {
        FLOGE("Open: VIDIOC_S_PARM Failed: %s", strerror(errno));
        return ret;
    }

    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type                 = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width        = width & 0xFFFFFFF8;
    fmt.fmt.pix.height       = height & 0xFFFFFFF8;
    fmt.fmt.pix.pixelformat  = vformat;
    fmt.fmt.pix.priv         = 0;
    fmt.fmt.pix.sizeimage    = 0;
    fmt.fmt.pix.bytesperline = 0;

    // Special stride alignment for YU12
    if (vformat == v4l2_fourcc('Y', 'U', '1', '2')){
        // Goolge define the the stride and c_stride for YUV420 format
        // y_size = stride * height
        // c_stride = ALIGN(stride/2, 16)
        // c_size = c_stride * height/2
        // size = y_size + c_size * 2
        // cr_offset = y_size
        // cb_offset = y_size + c_size
        // int stride = (width+15)/16*16;
        // int c_stride = (stride/2+16)/16*16;
        // y_size = stride * height
        // c_stride = ALIGN(stride/2, 16)
        // c_size = c_stride * height/2
        // size = y_size + c_size * 2
        // cr_offset = y_size
        // cb_offset = y_size + c_size

        // GPU and IPU take below stride calculation
        // GPU has the Y stride to be 32 alignment, and UV stride to be
        // 16 alignment.
        // IPU have the Y stride to be 2x of the UV stride alignment
        int stride = (width+31)/32*32;
        int c_stride = (stride/2+15)/16*16;
        fmt.fmt.pix.bytesperline = stride;
        fmt.fmt.pix.sizeimage    = stride*height+c_stride * height;
        FLOGI("Special handling for YV12 on Stride %d, size %d",
            fmt.fmt.pix.bytesperline,
            fmt.fmt.pix.sizeimage);
    }

    ret = ioctl(mCamFd, VIDIOC_S_FMT, &fmt);
    if (ret < 0) {
        FLOGE("Open: VIDIOC_S_FMT Failed: %s", strerror(errno));
        return ret;
    }

    return ret;
}


int initSensorInfo()
{
    if (mCamFd < 0) {
        FLOGE("OvDevice: initParameters sensor has not been opened");
        return BAD_VALUE;
    }

    // first read sensor format.
    int ret = 0, index = 0;
    int sensorFormats[MAX_SENSOR_FORMAT];
 //   memset(mAvailableFormats, 0, sizeof(mAvailableFormats));
    memset(sensorFormats, 0, sizeof(sensorFormats));

    // v4l2 does not support enum format, now hard code here.
    sensorFormats[index++] = v4l2_fourcc('N', 'V', '1', '2');
    sensorFormats[index++] = v4l2_fourcc('Y', 'V', '1', '2');
    sensorFormats[index++] = v4l2_fourcc('B', 'L', 'O', 'B');
    sensorFormats[index++] = v4l2_fourcc('R', 'A', 'W', 'S');
    //mAvailableFormats[2] = v4l2_fourcc('Y', 'U', 'Y', 'V');
    //mAvailableFormatCount = index;
    //changeSensorFormats(sensorFormats, index);

    index = 0;
    char TmpStr[20];
    int  previewCnt = 0, pictureCnt = 0;
    struct v4l2_frmsizeenum vid_frmsize;
    struct v4l2_frmivalenum vid_frmval;
    while (ret == 0) {
        memset(TmpStr, 0, 20);
        memset(&vid_frmsize, 0, sizeof(struct v4l2_frmsizeenum));
        vid_frmsize.index        = index++;
        vid_frmsize.pixel_format = v4l2_fourcc('N', 'V', '1', '2');
        ret = ioctl(mCamFd,
                    VIDIOC_ENUM_FRAMESIZES, &vid_frmsize);
        if (ret == 0) {
            FLOG_RUNTIME("enum frame size w:%d, h:%d",
                         vid_frmsize.discrete.width, vid_frmsize.discrete.height);
            memset(&vid_frmval, 0, sizeof(struct v4l2_frmivalenum));
            vid_frmval.index        = 0;
            vid_frmval.pixel_format = vid_frmsize.pixel_format;
            vid_frmval.width        = vid_frmsize.discrete.width;
            vid_frmval.height       = vid_frmsize.discrete.height;

            // ret = ioctl(mCameraHandle, VIDIOC_ENUM_FRAMEINTERVALS,
            // &vid_frmval);
            // v4l2 does not support, now hard code here.
            if (ret == 0) {
                FLOG_RUNTIME("vid_frmval denominator:%d, numeraton:%d",
                             vid_frmval.discrete.denominator,
                             vid_frmval.discrete.numerator);
                if ((vid_frmsize.discrete.width > 1280) ||
                    (vid_frmsize.discrete.height > 800)) {
                    vid_frmval.discrete.denominator = 15;
                    vid_frmval.discrete.numerator   = 1;
                }
                else if ((vid_frmsize.discrete.width == 1024) ||
                    (vid_frmsize.discrete.height == 768)) {
                    // Max fps for ov5640 csi xga cannot reach to 30fps
                    vid_frmval.discrete.denominator = 15;
                    vid_frmval.discrete.numerator   = 1;

                }
                else {
                    vid_frmval.discrete.denominator = 30;
                    vid_frmval.discrete.numerator   = 1;
                }

            }
        }
    } // end while

    return NO_ERROR;
}




int initV4lDevice()
{
	mCamFd = open(DEV_NAME, O_RDWR);
	if (mCamFd < 0)
		return -1;
    int ret = NO_ERROR;
    ret = ioctl(mCamFd, VIDIOC_QUERYCAP, &mCap);
    if (ret < 0) {
        close(mCamFd);

        FLOGE("query v4l2 capability failed");
        return BAD_VALUE;
    }
    if ((mCap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0)
    {
        close(mCamFd);
        FLOGE("v4l2 capability does not support capture");
        return BAD_VALUE;
    }
	initSensorInfo();
}

int allocateBuffers(int width,int height,
                                   int format, int numBufs)
{
    memset(mCameraBuffer, 0, sizeof(mCameraBuffer));
    mIonFd = ion_open();

    if (mIonFd <= 0) {
        FLOGE("try to allocate buffer from ion in preview or ion invalid");
        return BAD_VALUE;
    }

    int size = 0;
    if ((width == 0) || (height == 0)) {
        FLOGE("allocateBufferFromIon: width or height = 0");
        return BAD_VALUE;
    }
    switch (format) {
        case HAL_PIXEL_FORMAT_YCbCr_420_SP:
            size = width * ((height + 16) & (~15)) * 3 / 2;
            break;

        case HAL_PIXEL_FORMAT_YCbCr_420_P:
            size = width * height * 3 / 2;
            break;

        case HAL_PIXEL_FORMAT_YCbCr_422_I:
            size = width * height * 2;
            break;

        default:
            FLOGE("Error: format not supported int ion alloc");
            return BAD_VALUE;
    }

    unsigned char *ptr = NULL;
    int sharedFd;
    int phyAddr;
    ion_user_handle_t ionHandle;
    size = (size + PAGE_SIZE) & (~(PAGE_SIZE - 1));

    FLOGI("allocateBufferFromIon buffer num:%d", numBufs);
    for (int i = 0; i < numBufs; i++) {
        ionHandle = -1;
        int err = ion_alloc(mIonFd, size, 8, 1, 0, &ionHandle);
        if (err) {
            FLOGE("ion_alloc failed.");
            return BAD_VALUE;
        }

        err = ion_map(mIonFd,
                      ionHandle,
                      size,
                      PROT_READ | PROT_WRITE,
                      MAP_SHARED,
                      0,
                      &ptr,
                      &sharedFd);
        if (err) {
            FLOGE("ion_map failed.");
            return BAD_VALUE;
        }
        phyAddr = ion_phys(mIonFd, ionHandle);
        if (phyAddr == 0) {
            FLOGE("ion_phys failed.");
            return BAD_VALUE;
        }
        FLOG_RUNTIME("phyalloc ptr:0x%x, phy:0x%x, size:%d",
                     (int)ptr,
                     phyAddr,
                     size);

        mCameraBuffer[i].mIndex     = i;
        mCameraBuffer[i].mWidth     = width;
        mCameraBuffer[i].mHeight    = height;
        mCameraBuffer[i].mFormat    = format;
        mCameraBuffer[i].mVirtAddr  = ptr;
        mCameraBuffer[i].mPhyAddr   = phyAddr;
        mCameraBuffer[i].mSize      =  size;
        mCameraBuffer[i].mBufHandle = (buffer_handle_t)ionHandle;
        mCameraBuffer[i].mpFrameBuf  = NULL;
        mCameraBuffer[i].mBindUVCBufIdx = -1;
  
        close(sharedFd);

        FLOGI("PhysMemAdapter::allocateBuffers, i %d, phyAddr 0x%x, mBufHandle %p", i, phyAddr, mCameraBuffer[i].mBufHandle);
    }

    return NO_ERROR;
}

int main()
{
	initV4lDevice();
	allocateBuffers(1920, 1080, HAL_PIXEL_FORMAT_YCbCr_420_SP, 3);
	registerCameraBuffers(mCameraBuffer, 3);
	setDeviceConfig(1280, 720, HAL_PIXEL_FORMAT_YCbCr_420_SP, 30);
	
	startDevice();
}


