/*
 * Generated by asn1c-0.9.24 (http://lionet.info/asn1c)
 * From ASN.1 module "RRLP-Components"
 * 	found in "rrlp-components.asn"
 * 	`asn1c -gen-PER -fnative-types`
 */

#ifndef	_MsrAssistData_H_
#define	_MsrAssistData_H_


#include <asn_application.h>

/* Including external dependencies */
#include "SeqOfMsrAssistBTS.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* MsrAssistData */
typedef struct MsrAssistData {
	SeqOfMsrAssistBTS_t	 msrAssistList;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} MsrAssistData_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_MsrAssistData;

#ifdef __cplusplus
}
#endif

#endif	/* _MsrAssistData_H_ */
#include <asn_internal.h>
