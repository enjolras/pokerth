/*
 * Generated by asn1c-0.9.22 (http://lionet.info/asn1c)
 * From ASN.1 module "POKERTH-PROTOCOL"
 * 	found in "../../../docs/pokerth.asn1"
 * 	`asn1c -fskeletons-copy`
 */

#ifndef	_TimeoutWarningMessage_H_
#define	_TimeoutWarningMessage_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum timeoutReason {
	timeoutReason_timeoutNoDataReceived	= 0,
	timeoutReason_timeoutInactiveGame	= 1
} e_timeoutReason;

/* TimeoutWarningMessage */
typedef struct TimeoutWarningMessage {
	long	 timeoutReason;
	long	 remainingSeconds;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} TimeoutWarningMessage_t;

/* Implementation */
/* extern asn_TYPE_descriptor_t asn_DEF_timeoutReason_2;	// (Use -fall-defs-global to expose) */
extern asn_TYPE_descriptor_t asn_DEF_TimeoutWarningMessage;

#ifdef __cplusplus
}
#endif

#endif	/* _TimeoutWarningMessage_H_ */
