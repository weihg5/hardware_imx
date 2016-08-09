#undef LOG_TAG
#define LOG_TAG "IPU_RESIZE"
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

#include <linux/ipu.h>

static int resize_crop (int inx, int iny, int incropw, int incroph, int inw, int inh, void *inbuf,
				 int outx, int outy, int outcropw, int outcroph, int outw, int outh, void *outbuf)
{
	struct ipu_task task;
	int fd_ipu;
	int ret, i;
	
	// for time
	struct timeval start1, end1, start2, end2;

	gettimeofday(&start1, NULL);

	// Clear &task	
	memset(&task, 0, sizeof(task));
 
	// Input image size and format
	task.input.width    = inw;
	task.input.height   = inh;
	task.input.format   = IPU_PIX_FMT_NV12;
	
	task.input.crop.pos.x = inx;
	task.input.crop.pos.y = iny;
	task.input.crop.w = incropw;
	task.input.crop.h = incroph;
	task.input.paddr = (int)inbuf;
	// Output image size and format
	task.output.width   = outw;
	task.output.height  = outh;
	task.output.format  = IPU_PIX_FMT_NV12;

	task.output.crop.pos.x = outx;
	task.output.crop.pos.y = outy;
	task.output.crop.w = outcropw;
	task.output.crop.h = outcroph;
 	task.output.paddr = (int)outbuf;
	// Open IPU device
	fd_ipu = open("/dev/mxc_ipu", O_RDWR, 0);
	if (fd_ipu < 0) {
		ALOGE("open ipu dev fail\n");
		ret = -1;
		goto done;
	}

	// Perform cropping 
	ret = ioctl(fd_ipu, IPU_QUEUE_TASK, &task);
	if (ret < 0)
	{
		ALOGE("ioct IPU_QUEUE_TASK fail %x\n", ret);
		goto done;
	}
	
	gettimeofday(&end1, NULL);

	ALOGE("ipu crop resize time=%ldms", (end1.tv_sec-start1.tv_sec)*1000 + (end1.tv_usec-start1.tv_usec)/1000);
	
	
done:
	if (fd_ipu)
		close(fd_ipu);

	return ret;
}


int ipu_resize(int inw, int inh, void *inbuf,
				int outw, int outh, void *outbuf){

	int inx, iny, incropw, incroph; 
	int outx, outy, outcropw, outcroph;

	
	struct timeval start1, end1, start2, end2;
	gettimeofday(&start1, NULL); 
	inx = 0;
	iny = 0;
	incropw = inw >> 1;
	incroph = inh >> 1;

	outx = 0;
	outy = 0;
	outcropw = outw >> 1;
	outcroph = outh >> 1;

	resize_crop(inx, iny, incropw, incroph, inw, inh, inbuf,
				outx, outy, outcropw, outcroph, outw, outh, outbuf);

	inx = inw >> 1;
	iny = 0;
	outx = outw >> 1;
	outy = 0;
	resize_crop(inx, iny, incropw, incroph, inw, inh, inbuf,
				outx, outy, outcropw, outcroph, outw, outh, outbuf);

	inx = 0;
	iny = inh >> 1;
	outx = 0;
	outy = outh >> 1;
	resize_crop(inx, iny, incropw, incroph, inw, inh, inbuf,
				outx, outy, outcropw, outcroph, outw, outh, outbuf);

	inx = inw >> 1;
	iny = inh >> 1;
	outx = outw >> 1;
	outy = outh >> 1;

	resize_crop(inx, iny, incropw, incroph, inw, inh, inbuf,
				outx, outy, outcropw, outcroph, outw, outh, outbuf);

	gettimeofday(&end1, NULL);

	ALOGE("ipu resize time=%ldms", (end1.tv_sec-start1.tv_sec)*1000 + (end1.tv_usec-start1.tv_usec)/1000);
	return 0;

}

