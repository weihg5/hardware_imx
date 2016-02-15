/*
 * Copyright (C) 2012-2013 Freescale Semiconductor, Inc.
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

#include "Ov5645Mipi.h"


status_t Ov5645Mipi::initSensorInfo(const CameraInfo& info)
{
    if (mCameraHandle < 0) {
        FLOGE("OvDevice: initParameters sensor has not been opened");
        return BAD_VALUE;
    }

    // first read sensor format.
    int ret = 0, index = 0;
    int sensorFormats[MAX_SENSOR_FORMAT];
    memset(mAvailableFormats, 0, sizeof(mAvailableFormats));
    memset(sensorFormats, 0, sizeof(sensorFormats));

    // v4l2 does not support enum format, now hard code here.
    sensorFormats[index++] = v4l2_fourcc('N', 'V', '1', '2');
    sensorFormats[index++] = v4l2_fourcc('Y', 'V', '1', '2');
    sensorFormats[index++] = v4l2_fourcc('B', 'L', 'O', 'B');
    sensorFormats[index++] = v4l2_fourcc('R', 'A', 'W', 'S');
    mAvailableFormatCount = index;
    changeSensorFormats(sensorFormats, index);

    index = 0;
    char TmpStr[20];
    int  previewCnt = 0, pictureCnt = 0, sensorCnt = 0;
    struct v4l2_frmsizeenum vid_frmsize;
    struct v4l2_frmivalenum vid_frmval;
    while (ret == 0) {
        memset(TmpStr, 0, 20);
        memset(&vid_frmsize, 0, sizeof(struct v4l2_frmsizeenum));
        vid_frmsize.index        = index++;
        vid_frmsize.pixel_format = v4l2_fourcc('N', 'V', '1', '2');
        ret = ioctl(mCameraHandle,VIDIOC_ENUM_FRAMESIZES, &vid_frmsize);
        if (ret == 0) {
            FLOG_RUNTIME("enum frame size w:%d, h:%d",vid_frmsize.discrete.width, vid_frmsize.discrete.height);
            memset(&vid_frmval, 0, sizeof(struct v4l2_frmivalenum));
            vid_frmval.index        = 0;
            vid_frmval.pixel_format = vid_frmsize.pixel_format;
            vid_frmval.width        = vid_frmsize.discrete.width;
            vid_frmval.height       = vid_frmsize.discrete.height;

            // v4l2 does not support, now hard code here.
            if ((ret == 0) && (vid_frmval.width!=0 && vid_frmval.height!=0)) /* vid_frmval.width==1280 && vid_frmval.height==960)|| */
				//if((vid_frmval.width==640 && vid_frmval.height==480)||(vid_frmval.width==1280 && vid_frmval.height==720))
				{
                	FLOG_RUNTIME("vid_frmval denominator:%d, numeraton:%d",vid_frmval.discrete.denominator,vid_frmval.discrete.numerator);
                	if ((vid_frmsize.discrete.width > 1920) || (vid_frmsize.discrete.height > 1080)) 
					{
                   	 	//vid_frmval.discrete.denominator = 15;
                    	//vid_frmval.discrete.numerator   = 1;
						mPictureResolutions[pictureCnt++] = 1080;
                		mPictureResolutions[pictureCnt++] = 1920;
                    	mPreviewResolutions[previewCnt++] = 1080;
                    	mPreviewResolutions[previewCnt++] = 1920;
						mSensorPicSize[sensorCnt++] = vid_frmsize.discrete.width;
						mSensorPicSize[sensorCnt++] = vid_frmsize.discrete.height;

						mPictureResolutions[pictureCnt++] = 720;
                		mPictureResolutions[pictureCnt++] = 1280;
                    	mPreviewResolutions[previewCnt++] = 720;
                    	mPreviewResolutions[previewCnt++] = 1280;
						mSensorPicSize[sensorCnt++] = vid_frmsize.discrete.width;
						mSensorPicSize[sensorCnt++] = vid_frmsize.discrete.height;

						mPictureResolutions[pictureCnt++] = 960;
                		mPictureResolutions[pictureCnt++] = 1280;
                    	mPreviewResolutions[previewCnt++] = 960;
                    	mPreviewResolutions[previewCnt++] = 1280;
						mSensorPicSize[sensorCnt++] = vid_frmsize.discrete.width;
						mSensorPicSize[sensorCnt++] = vid_frmsize.discrete.height;
               		}
                	else if ((vid_frmsize.discrete.width == 1920) && (vid_frmsize.discrete.height == 1080)) 
					{
                   	 	//vid_frmval.discrete.denominator = 15;
                    	//vid_frmval.discrete.numerator   = 1;
						mPictureResolutions[pictureCnt++] = 768;
                		mPictureResolutions[pictureCnt++] = 1024;
                    	mPreviewResolutions[previewCnt++] = 768;
                    	mPreviewResolutions[previewCnt++] = 1024;
						mSensorPicSize[sensorCnt++] = vid_frmsize.discrete.width;
						mSensorPicSize[sensorCnt++] = vid_frmsize.discrete.height;

						mPictureResolutions[pictureCnt++] = 480;
						mPictureResolutions[pictureCnt++] = 640;
						mPreviewResolutions[previewCnt++] = 480;
						mPreviewResolutions[previewCnt++] = 640;
						mSensorPicSize[sensorCnt++] = vid_frmsize.discrete.width;
						mSensorPicSize[sensorCnt++] = vid_frmsize.discrete.height;

						mPictureResolutions[pictureCnt++] = 480;
						mPictureResolutions[pictureCnt++] = 720;
						mPreviewResolutions[previewCnt++] = 480;
						mPreviewResolutions[previewCnt++] = 720;
						mSensorPicSize[sensorCnt++] = vid_frmsize.discrete.width;
						mSensorPicSize[sensorCnt++] = vid_frmsize.discrete.height;

                	}
			#if 0
                	else if ((vid_frmsize.discrete.width == 1280) && (vid_frmsize.discrete.height == 720)) 
					{
                   	 	//vid_frmval.discrete.denominator = 15;
                    	//vid_frmval.discrete.numerator   = 1;
						mPictureResolutions[pictureCnt++] = 480;
                		mPictureResolutions[pictureCnt++] = 640;
                    	mPreviewResolutions[previewCnt++] = 480;
                    	mPreviewResolutions[previewCnt++] = 640;
						mSensorPicSize[sensorCnt++] = vid_frmsize.discrete.width;
						mSensorPicSize[sensorCnt++] = vid_frmsize.discrete.height;
						
						mPictureResolutions[pictureCnt++] = 480;
                		mPictureResolutions[pictureCnt++] = 720;
                    	mPreviewResolutions[previewCnt++] = 480;
                    	mPreviewResolutions[previewCnt++] = 720;
						mSensorPicSize[sensorCnt++] = vid_frmsize.discrete.width;
						mSensorPicSize[sensorCnt++] = vid_frmsize.discrete.height;
                	}
					else 
					{
						mPictureResolutions[pictureCnt++] = 0;
                		mPictureResolutions[pictureCnt++] = 0;
                    	mPreviewResolutions[previewCnt++] = 0;
                    	mPreviewResolutions[previewCnt++] = 0;
						mSensorPicSize[sensorCnt++] = vid_frmsize.discrete.width;
						mSensorPicSize[sensorCnt++] = vid_frmsize.discrete.height;						
					}
			#endif
				}
            }
        
    } // end while

    mPreviewResolutionCount = previewCnt;
    mPictureResolutionCount = pictureCnt;

    mMinFrameDuration = 33331760L;
    mMaxFrameDuration = 30000000000L;
    int i;
    for (i=0; i<MAX_RESOLUTION_SIZE  && i<pictureCnt; i+=2) {
        FLOGI("SupportedPictureSizes: %d x %d", mPictureResolutions[i], mPictureResolutions[i+1]);
    }

    adjustPreviewResolutions();
    for (i=0; i<MAX_RESOLUTION_SIZE  && i<previewCnt; i+=2) {
        FLOGI("SupportedPreviewSizes: %d x %d", mPreviewResolutions[i], mPreviewResolutions[i+1]);
    }
    FLOGI("FrameDuration is %lld, %lld", mMinFrameDuration, mMaxFrameDuration);

    i = 0;
    mTargetFpsRange[i++] = 10;
    mTargetFpsRange[i++] = 15;
    mTargetFpsRange[i++] = 25;
    mTargetFpsRange[i++] = 30;

    setMaxPictureResolutions();
    FLOGI("mMaxWidth:%d, mMaxHeight:%d", mMaxWidth, mMaxHeight);
    mFocalLength = 3.42f;
    mPhysicalWidth = 3.673f;
    mPhysicalHeight = 2.738f;

	mCrop = 1;
	
    return NO_ERROR;
}


