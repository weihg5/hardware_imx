/*
 * Generated by asn1c-0.9.24 (http://lionet.info/asn1c)
 * From ASN.1 module "ULP-Components"
 * 	found in "supl.asn"
 * 	`asn1c -gen-PER -fnative-types`
 */

#ifndef	_PositionEstimate_H_
#define	_PositionEstimate_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum latitudeSign {
	latitudeSign_north	= 0,
	latitudeSign_south	= 1
} e_latitudeSign;

/* Forward declarations */
struct AltitudeInfo;

/* PositionEstimate */
typedef struct PositionEstimate {
	long	 latitudeSign;
	long	 latitude;
	long	 longitude;
	struct uncertainty {
		long	 uncertaintySemiMajor;
		long	 uncertaintySemiMinor;
		long	 orientationMajorAxis;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} *uncertainty;
	long	*confidence	/* OPTIONAL */;
	struct AltitudeInfo	*altitudeInfo	/* OPTIONAL */;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} PositionEstimate_t;

/* Implementation */
/* extern asn_TYPE_descriptor_t asn_DEF_latitudeSign_2;	// (Use -fall-defs-global to expose) */
extern asn_TYPE_descriptor_t asn_DEF_PositionEstimate;

#ifdef __cplusplus
}
#endif

/* Referred external types */
#include "AltitudeInfo.h"

#endif	/* _PositionEstimate_H_ */
#include <asn_internal.h>
