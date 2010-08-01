/*
 * Generated by asn1c-0.9.22 (http://lionet.info/asn1c)
 * From ASN.1 module "POKERTH-PROTOCOL"
 * 	found in "../../../docs/pokerth.asn1"
 * 	`asn1c -fskeletons-copy`
 */

#ifndef	_PlayerResult_H_
#define	_PlayerResult_H_


#include <asn_application.h>

/* Including external dependencies */
#include "NonZeroId.h"
#include "Card.h"
#include <NativeInteger.h>
#include <asn_SEQUENCE_OF.h>
#include <constr_SEQUENCE_OF.h>
#include <constr_SEQUENCE.h>

#ifdef __cplusplus
extern "C" {
#endif

/* PlayerResult */
typedef struct PlayerResult {
	NonZeroId_t	 playerId;
	Card_t	 resultCard1;
	Card_t	 resultCard2;
	struct bestHandPosition {
		A_SEQUENCE_OF(long) list;
		
		/* Context for parsing across buffer boundaries */
		asn_struct_ctx_t _asn_ctx;
	} bestHandPosition;
	long	 cardsValue;
	long	 moneyWon;
	long	 playerMoney;
	/*
	 * This type is extensible,
	 * possible extensions are below.
	 */
	
	/* Context for parsing across buffer boundaries */
	asn_struct_ctx_t _asn_ctx;
} PlayerResult_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_PlayerResult;

#ifdef __cplusplus
}
#endif

#endif	/* _PlayerResult_H_ */
