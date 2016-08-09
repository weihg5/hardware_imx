#ifndef __IPU_RESIZE_H__
#define __IPU_RESIZE_H__
#ifdef __cplusplus
extern "C" {
#endif

int ipu_resize(int inw, int inh, void *inbuf,
				int outw, int outh, void *outbuf);

#ifdef __cplusplus
}
#endif

#endif
