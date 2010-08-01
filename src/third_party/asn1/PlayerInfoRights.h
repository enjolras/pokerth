/*
 * Generated by asn1c-0.9.22 (http://lionet.info/asn1c)
 * From ASN.1 module "POKERTH-PROTOCOL"
 * 	found in "../../../docs/pokerth.asn1"
 * 	`asn1c -fskeletons-copy`
 */

#ifndef	_PlayerInfoRights_H_
#define	_PlayerInfoRights_H_


#include <asn_application.h>

/* Including external dependencies */
#include <NativeEnumerated.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Dependencies */
typedef enum PlayerInfoRights {
	PlayerInfoRights_playerRightsGuest	= 1,
	PlayerInfoRights_playerRightsNormal	= 2,
	PlayerInfoRights_playerRightsAdmin	= 3
} e_PlayerInfoRights;

/* PlayerInfoRights */
typedef long	 PlayerInfoRights_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_PlayerInfoRights;
asn_struct_free_f PlayerInfoRights_free;
asn_struct_print_f PlayerInfoRights_print;
asn_constr_check_f PlayerInfoRights_constraint;
ber_type_decoder_f PlayerInfoRights_decode_ber;
der_type_encoder_f PlayerInfoRights_encode_der;
xer_type_decoder_f PlayerInfoRights_decode_xer;
xer_type_encoder_f PlayerInfoRights_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _PlayerInfoRights_H_ */
