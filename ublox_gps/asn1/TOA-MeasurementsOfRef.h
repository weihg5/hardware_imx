/*
 * Generated by asn1c-0.9.24 (http://lionet.info/asn1c)
 * From ASN.1 module "RRLP-Components"
 * 	found in "rrlp-components.asn"
 * 	`asn1c -gen-PER -fnative-types`
 */

#ifndef	_TOA_MeasurementsOfRef_H_
#define	_TOA_MeasurementsOfRef_H_


#include <asn_application.h>

/* Including external dependencies */
#include "RefQuality.h"
#include "NumOfMeasurements.h"
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* TOA-MeasurementsOfRef */
typedef struct TOA_MeasurementsOfRef {
	RefQuality_t	 refQuality;
	NumOfMeasurements_t	 numOfMeasurements;
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} TOA_MeasurementsOfRef_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_TOA_MeasurementsOfRef;

#ifdef __cplusplus
}
#endif

#endif	/* _TOA_MeasurementsOfRef_H_ */
#include <asn_internal.h>
