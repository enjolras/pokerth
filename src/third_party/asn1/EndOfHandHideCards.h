/*
 * Generated by asn1c-0.9.22 (http://lionet.info/asn1c)
 * From ASN.1 module "POKERTH-PROTOCOL"
 * 	found in "../../../docs/pokerth.asn1"
 * 	`asn1c -fskeletons-copy`
 */

#ifndef	_EndOfHandHideCards_H_
#define	_EndOfHandHideCards_H_


#include <asn_application.h>

/* Including external dependencies */
#include "NonZeroId.h"
#include <NativeInteger.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* EndOfHandHideCards */
typedef struct EndOfHandHideCards {
	NonZeroId_t	 playerId;
	long	 moneyWon;
	long	 playerMoney;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} EndOfHandHideCards_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_EndOfHandHideCards;

#ifdef __cplusplus
}
#endif

#endif	/* _EndOfHandHideCards_H_ */
