/*
 * Copyright (C) 2012-2014 Freescale Semiconductor, Inc.
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

#include "StreamAdapter.h"
#include "RequestManager.h"

static void convertUV(u8 *pbySrcColor, u8 *pbyDstColor, u32 width, u32 height) //StreamBuffer *dst, StreamBuffer *src)
{
 //   FLOGI("convertUV\n");

    unsigned int bufWidth = width;
    unsigned int bufHeight = height;


	unsigned char *pSrcU1Offset = pbySrcColor;
    unsigned char *pSrcU2Offset = pbySrcColor + bufWidth;
    unsigned char *pSrcU3Offset = pbySrcColor + bufWidth * 2;
    unsigned char *pSrcU4Offset = pbySrcColor + bufWidth * 3;

    unsigned char *pSrcV1Offset = pSrcU1Offset + 1;
    unsigned char *pSrcV2Offset = pSrcU2Offset + 1;
    unsigned char *pSrcV3Offset = pSrcU3Offset + 1;
    unsigned char *pSrcV4Offset = pSrcU4Offset + 1;

    unsigned int srcUVStride = bufWidth*3;

	unsigned char *pDstU1Offset = pbyDstColor;
    unsigned char *pDstU2Offset = pbyDstColor + bufWidth;

    unsigned char *pDstV1Offset = pDstU1Offset + 1;
    unsigned char *pDstV2Offset = pDstU2Offset + 1;

    unsigned int dstUVStride = bufWidth;

    unsigned int nw, nh;
    for(nh = 0; nh < (bufHeight >> 2); nh++) {
        for(nw=0; nw < (bufWidth >> 1); nw++) {

            *pDstU1Offset = *pSrcU1Offset;
            *pDstU2Offset = *pSrcU3Offset;
            pDstU1Offset += 2;
            pDstU2Offset += 2;
            pSrcU1Offset += 2;
            pSrcU3Offset += 2;

            *pDstV1Offset = *pSrcV1Offset;
            *pDstV2Offset = *pSrcV3Offset;
            pDstV1Offset += 2;
            pDstV2Offset += 2;
            pSrcV1Offset += 2;
            pSrcV3Offset += 2;
        }

        pSrcU1Offset += srcUVStride;
        pSrcU3Offset += srcUVStride;
        pSrcV1Offset += srcUVStride;
        pSrcV3Offset += srcUVStride;

        pDstU1Offset += dstUVStride;
        pDstU2Offset += dstUVStride;
        pDstV1Offset += dstUVStride;
        pDstV2Offset += dstUVStride;
    }
}

static void YUV422ToYUV420Interleave(u8* pbySrcY, u8* pbySrcC,
            u32 dwSrcYStide, u32 dwSrcCStride, u32 dwSrcHeight,
            u8 *pbyDstY, u8 *pbyDstC,
            u32 dwDstYStide, u32 dwDstCStide, u32 dwDstHeight) {
    //convert Y
    u8 *pInY = pbySrcY;
    u8 *pOutY = pbyDstY;
    u32 nDstLine;

    for(nDstLine = 0; nDstLine < dwDstHeight; nDstLine++) {
        memcpy(pOutY, pInY, dwDstYStide);
        pInY += dwSrcYStide;
        pOutY += dwDstYStide;
    }

    convertUV(pbySrcC, pbyDstC, dwSrcYStide, dwSrcHeight);

    return;
}

StreamAdapter::StreamAdapter(int id)
    : mPrepared(false), mStarted(false), mStreamId(id), mWidth(0), mHeight(0), mFormat(0), mUsage(0),
      mMaxProducerBuffers(0), mNativeWindow(NULL), mStreamState(STREAM_INVALID), mReceiveFrame(true)
{
    g2dHandle = NULL;
    sem_init(&mRespondSem, 0, 0);
}

StreamAdapter::~StreamAdapter()
{
}

int StreamAdapter::initialize(int width, int height, int format, int usage, int bufferNum)
{
	mResize = 0;
	if (width == 3264){
		width = 2592;
		height = 1944;
		mResize = 1;
	}

    mWidth = width;
    mHeight = height;
    mFormat = format;
    mUsage = usage;
    mMaxProducerBuffers = bufferNum;
    return 0;
}

int StreamAdapter::setPreviewWindow(const camera2_stream_ops_t* window)
{
    mNativeWindow = window;
    return 0;
}

void StreamAdapter::setDeviceAdapter(sp<DeviceAdapter>& device)
{
    mDeviceAdapter = device;
}

void StreamAdapter::setMetadaManager(sp<MetadaManager>& metaManager)
{
    mMetadaManager = metaManager;
}

void StreamAdapter::setErrorListener(CameraErrorListener *listener)
{
    mErrorListener = listener;
}

int StreamAdapter::start()
{
    FLOG_TRACE("StreamAdapter %s running", __FUNCTION__);

    mTime1 = mTime2 = 0;
    mTotalFrames = mFps = 0;
    mShowFps = false;
    char prop_value[CAMERA_SENSOR_LENGTH];
    if (property_get("sys.camera.fps", prop_value, "0")) {
        if (strcmp(prop_value, "1") == 0) {
            mShowFps = true;
        }
    }

    mStreamThread = new StreamThread(this);
    mThreadQueue.Clear();
    mThreadQueue.postSyncMessage(new SyncMessage(STREAM_START, 0));

    fAssert(mDeviceAdapter.get() != NULL);
    mDeviceAdapter->addFrameListener(this);
    mStarted = true;
    return NO_ERROR;
}

int StreamAdapter::stop()
{
    FLOG_TRACE("StreamAdapter %s running", __FUNCTION__);
    if (mDeviceAdapter.get() != NULL) {
        mDeviceAdapter->removeFrameListener(this);;
    }

    if (mStreamThread.get() != NULL) {
        mThreadQueue.postSyncMessage(new SyncMessage(STREAM_STOP, 0));
    }
    FLOG_TRACE("StreamAdapter %s end", __FUNCTION__);
    if (mShowFps) {
        if (mStreamId == STREAM_ID_PREVIEW) {
            FLOGI("preview ouput %d frames", mTotalFrames);
        }
        else if (mStreamId == STREAM_ID_RECORD) {
            FLOGI("recorder ouput %d frames", mTotalFrames);
        }
    }

    mStarted = false;
    return NO_ERROR;
}

int StreamAdapter::release()
{
    FLOG_TRACE("StreamAdapter %s running", __FUNCTION__);
    if (mStreamThread.get() == NULL) {
        return NO_ERROR;
    }

    mThreadQueue.postSyncMessage(new SyncMessage(STREAM_EXIT, 0));
    mStreamThread->requestExitAndWait();
    mStreamThread.clear();
    mThreadQueue.Clear();
    mStreamState = STREAM_INVALID;
    mPrepared = false;

    return NO_ERROR;
}

bool StreamAdapter::handleStream()
{
    bool shouldLive = true;
    int ret = 0;
    CameraFrame *frame = NULL;

    sp<CMessage> msg = mThreadQueue.waitMessage(THREAD_WAIT_TIMEOUT);
    if (msg == 0) {
        if (mStreamState == STREAM_STARTED) {
            FLOGI("%s: get invalid message", __FUNCTION__);
            sem_post(&mRespondSem);
        }
        return shouldLive;
    }


    switch (msg->what) {
        case STREAM_FRAME:
            frame = (CameraFrame *)msg->arg0;
            if (!frame || !frame->mBufHandle) {
                FLOGI("%s invalid frame", __FUNCTION__);
                break;
            }

            if (mStreamState == STREAM_STARTED) {
                ret = processFrame(frame);
                if (!ret) {
                    //the frame release from StreamThread.
                    frame->release();
                    break;
                }
            }

            //the frame release from StreamThread.
            frame->release();
            cancelBuffer(frame);
            if (ret != 0) {
                mErrorListener->handleError(ret);
                if (ret <= CAMERA2_MSG_ERROR_DEVICE) {
                    FLOGI("stream thread dead because of error...");
                    mStreamState = STREAM_EXITED;
                }
            }

            break;

        case STREAM_START:
            FLOGI("stream thread received STREAM_START command");
            if (mStreamState == STREAM_EXITED) {
                FLOGI("can't start stream thread, thread dead...");
            }
            else {
                mStreamState = STREAM_STARTED;
            }
            if ((g2dHandle == NULL) && (mDeviceAdapter->UseMJPG() == false)) {
                g2d_open(&g2dHandle);
            }

            break;

        case STREAM_STOP:
            FLOGI("stream thread received STREAM_STOP command");
            if (mStreamState == STREAM_EXITED) {
                FLOGI("can't stop stream thread, thread dead...");
            }
            else {
                mStreamState = STREAM_STOPPED;
            }

            if (g2dHandle != NULL) {
                g2d_close(g2dHandle);
                g2dHandle = NULL;
            }

            break;

        case STREAM_EXIT:
            FLOGI("stream thread exiting...");
            mStreamState = STREAM_EXITED;
            shouldLive = false;
            break;

        default:
            FLOGE("Invalid stream Thread Command 0x%x.", msg->what);
            break;
    } // end switch

    return shouldLive;
}

void StreamAdapter::enableReceiveFrame()
{
    mReceiveFrame = true;
}

void StreamAdapter::handleCameraFrame(CameraFrame *frame)
{
    if (!frame || !frame->mBufHandle) {
        FLOGI("%s invalid frame", __FUNCTION__);
        return;
    }
    //don't need receive camera frame.
    if (!mReceiveFrame) {
        return;
    }
    else if (mStreamId == STREAM_ID_JPEG) {
        //captureStream should reveive one frame every time.
        mReceiveFrame = false;
    }
    //the frame processed in StreamThread.
    frame->addReference();
    mThreadQueue.postMessage(new CMessage(STREAM_FRAME, (int)frame));
}

void StreamAdapter::applyRequest()
{
    struct timespec ts;

    clock_gettime(CLOCK_REALTIME, &ts);
    long msecs = 500000000 + ts.tv_nsec;//500ms
    long secs = msecs / 1000000000;//get seconds.
    ts.tv_sec += secs;
    ts.tv_nsec = msecs % 1000000000;//get milliseconds.
    sem_timedwait(&mRespondSem, &ts);
}

void StreamAdapter::convertNV12toYV12(StreamBuffer* dst, StreamBuffer* src)
{
    uint8_t *Yin, *UVin, *Yout, *Uout, *Vout;
    int srcYSize = 0, srcUVSize = 0;
    int dstYStride = 0, dstUVStride = 0;
    int dstYSize = 0, dstUVSize = 0;

    srcYSize = src->mWidth * src->mHeight;
    srcUVSize = src->mWidth * src->mHeight >> 2;
    Yin = (uint8_t *)src->mVirtAddr;
    UVin = Yin + src->mWidth * src->mHeight;

    dstYStride = (dst->mWidth+15)/16*16;
    dstUVStride = (dst->mWidth/2+15)/16*16;
    dstYSize = dstYStride * dst->mHeight;
    dstUVSize = dstUVStride * dst->mHeight / 2;
    Yout = (uint8_t *)dst->mVirtAddr;
    Vout = Yout + dstYSize;
    Uout = Vout + dstUVSize;

    for (int y = 0; y < dst->mHeight && y < src->mHeight; y++) {
        int width = (dst->mWidth < src->mWidth) ? dst->mWidth : src->mWidth;
        memcpy(Yout, Yin, width);
        Yout += dst->mWidth;
        Yin += src->mWidth;
    }

    int yMax = (dst->mHeight < src->mHeight) ? dst->mHeight : src->mHeight;
    int xMax = (dst->mWidth < src->mWidth) ? (dst->mWidth) : src->mWidth;

    for (int y = 0; y < (yMax+1)/2; y++) {
        for (int x=0; x<(xMax+1)/2; x++) {
            Uout[x] = UVin[2*x];
            Vout[x] = UVin[2*x+1];
        }
        UVin += src->mWidth;
        Uout += dstUVStride;
        Vout += dstUVStride;
    }
}

void StreamAdapter::convertNV12toNV21(StreamBuffer* dst, StreamBuffer* src)
{
    int Ysize = 0, UVsize = 0;
    uint8_t *srcIn, *dstOut;
    uint32_t *UVout;
    struct g2d_buf s_buf, d_buf;
    int size = (src->mSize > dst->mSize) ? dst->mSize : src->mSize;

    Ysize  = src->mWidth * src->mHeight;
    UVsize = src->mWidth *  src->mHeight >> 2;
    srcIn = (uint8_t *)src->mVirtAddr;
    dstOut = (uint8_t *)dst->mVirtAddr;
    UVout = (uint32_t *)(dstOut + Ysize);

    if (g2dHandle != NULL) {
        s_buf.buf_paddr = src->mPhyAddr;
        s_buf.buf_vaddr = src->mVirtAddr;
        d_buf.buf_paddr = dst->mPhyAddr;
        d_buf.buf_vaddr = dst->mVirtAddr;
        g2d_copy(g2dHandle, &d_buf, &s_buf, size);
        g2d_finish(g2dHandle);
    }
    else {
        memcpy(dstOut, srcIn, size);
    }

    for (int k = 0; k < UVsize/2; k++) {
        __asm volatile ("rev16 %0, %0" : "+r"(*UVout));
        UVout += 1;
    }
}

int StreamAdapter::processFrame(CameraFrame *frame)
{
    status_t ret = NO_ERROR;
    int size;

    if (mShowFps) {
        showFps();
    }

    StreamBuffer buffer;
    ret = requestBuffer(&buffer);
    if (ret != NO_ERROR) {
        FLOGE("%s requestBuffer failed", __FUNCTION__);
        goto err_ext;
    }

    size = (frame->mSize > buffer.mSize) ? buffer.mSize : frame->mSize;
    if (mStreamId == STREAM_ID_PRVCB &&
            buffer.mFormat == HAL_PIXEL_FORMAT_YCbCr_420_P) {
        convertNV12toYV12(&buffer, frame);
    }
    else if (mStreamId == STREAM_ID_PRVCB && buffer.mWidth <= 1280 &&
            buffer.mFormat == HAL_PIXEL_FORMAT_YCbCr_420_SP) {
        convertNV12toNV21(&buffer, frame);
    }
    else if (g2dHandle != NULL) {
        struct g2d_buf s_buf, d_buf;
        s_buf.buf_paddr = frame->mPhyAddr;
        s_buf.buf_vaddr = frame->mVirtAddr;
        d_buf.buf_paddr = buffer.mPhyAddr;
        d_buf.buf_vaddr = buffer.mVirtAddr;
        g2d_copy(g2dHandle, &d_buf, &s_buf, size);
        g2d_finish(g2dHandle);

        //when 1080p recording, although g2d_copy is fast, but vpu encode is slower,
        //so slow down g2d_copy to free bus, then vpu encode run faster,
        //It's a balance.
        if( (mDeviceAdapter.get() != NULL) && (mDeviceAdapter->mCpuNum == 2) &&
            (mWidth == 1920) && (mHeight == 1080) ) {
            usleep(33000);
        }
    }
	else if(mDeviceAdapter.get() && mDeviceAdapter->UseMJPG()) {
		YUV422ToYUV420Interleave((unsigned char *)frame->mVirtAddr, (unsigned char *)frame->mVirtAddr + frame->mWidth * frame->mHeight,
            frame->mWidth, frame->mWidth/2, frame->mHeight,
            (unsigned char *)buffer.mVirtAddr, (unsigned char *)buffer.mVirtAddr + buffer.mWidth * buffer.mHeight, buffer.mWidth, buffer.mWidth/4, buffer.mHeight);
	} else {
        memcpy(buffer.mVirtAddr, (void *)frame->mVirtAddr, size);
    }

    buffer.mTimeStamp = frame->mTimeStamp;
    ret = renderBuffer(&buffer);
    if (ret != NO_ERROR) {
        FLOGE("%s renderBuffer failed", __FUNCTION__);
        goto err_ext;
    }

err_ext:
    sem_post(&mRespondSem);

    return ret;
}

void StreamAdapter::showFps()
{
    mTime2 = systemTime();
    mFps ++;
    mTotalFrames ++;
    if ((mTime2 - mTime1 >= 1000000000LL) && (mFps > 1)) {
        if (mStreamId == STREAM_ID_PREVIEW) {
            FLOGI("Preview %s %d fps", __FUNCTION__, mFps);
        }
        else if (mStreamId == STREAM_ID_RECORD) {
            FLOGI("Recorder %s %d fps", __FUNCTION__, mFps);
        }
        mTime1 = mTime2;
        mFps = 0;
    }
}

int StreamAdapter::requestBuffer(StreamBuffer* buffer)
{
    buffer_handle_t *buf;
    int i = 0;
    GraphicBufferMapper& mapper = GraphicBufferMapper::get();
    Rect  bounds;
    void *pVaddr;

    if (NULL == mNativeWindow) {
        FLOGE("mNativeWindow is null");
        return BAD_VALUE;
    }

    int err = mNativeWindow->dequeue_buffer(mNativeWindow, &buf);
    if (err != 0) {
        FLOGE("dequeueBuffer failed: %s (%d)", strerror(-err), -err);
        if (ENODEV == err) {
            FLOGE("Preview surface abandoned!");
            mNativeWindow = NULL;
        }

        return BAD_VALUE;
    }

    bounds.left   = 0;
    bounds.top    = 0;
    bounds.right  = mWidth;
    bounds.bottom = mHeight;

    // lock buffer before sending to FrameProvider for filling
    mapper.lock(*buf, mUsage, bounds, &pVaddr);

    private_handle_t *handle = (private_handle_t *)(*buf);
    buffer->mWidth = mWidth;
    buffer->mHeight = mHeight;
    buffer->mFormat = mFormat;
    buffer->mVirtAddr = pVaddr;
    buffer->mPhyAddr = handle->phys;
    buffer->mSize = handle->size;
    buffer->mBufHandle = *buf;

    return 0;
}

int StreamAdapter::renderBuffer(StreamBuffer *buffer)
{
    status_t ret = NO_ERROR;

    GraphicBufferMapper& mapper = GraphicBufferMapper::get();

    // unlock buffer before sending to stream
    mapper.unlock(buffer->mBufHandle);

    ret = mNativeWindow->enqueue_buffer(mNativeWindow, buffer->mTimeStamp,
                                        &buffer->mBufHandle);
    if (ret != 0) {
        FLOGE("Surface::queueBuffer returned error %d, phyAddr 0x%x, mBufHandle %p, mBindUVCBufIdx %d",
            ret, buffer->mPhyAddr, buffer->mBufHandle, buffer->mBindUVCBufIdx);
    }

    return ret;
}

int StreamAdapter::cancelBuffer(StreamBuffer *buffer)
{
    status_t ret = NO_ERROR;

    GraphicBufferMapper& mapper = GraphicBufferMapper::get();

    mapper.unlock(buffer->mBufHandle);

    ret = mNativeWindow->cancel_buffer(mNativeWindow, &buffer->mBufHandle);
    if (ret != 0) {
        FLOGE("Surface::queueBuffer returned error %d", ret);
    }

    return ret;
}



