#pragma once

/*! \defgroup auth GSM/GPRS/3G Authentication
 *  @{
 * \file auth.h */

#include <stdint.h>

#include <osmocom/core/linuxlist.h>
#include <osmocom/core/utils.h>

#define OSMO_A5_MAX_KEY_LEN_BYTES (128/8)
#define OSMO_MILENAGE_IND_BITLEN_MAX 28

/*! Authentication Type (GSM/UMTS) */
enum osmo_sub_auth_type {
	OSMO_AUTH_TYPE_NONE	= 0x00,
	OSMO_AUTH_TYPE_GSM	= 0x01,
	OSMO_AUTH_TYPE_UMTS	= 0x02,
};

extern const struct value_string osmo_sub_auth_type_names[];
static inline const char *osmo_sub_auth_type_name(enum osmo_sub_auth_type val)
{ return get_value_string(osmo_sub_auth_type_names, val); }

/*! Authentication Algorithm.
 * See also osmo_auth_alg_name() and osmo_auth_alg_parse(). */
enum osmo_auth_algo {
	OSMO_AUTH_ALG_NONE,
	OSMO_AUTH_ALG_COMP128v1,
	OSMO_AUTH_ALG_COMP128v2,
	OSMO_AUTH_ALG_COMP128v3,
	OSMO_AUTH_ALG_XOR,
	OSMO_AUTH_ALG_MILENAGE,
	_OSMO_AUTH_ALG_NUM,
};

/*! permanent (secret) subscriber auth data */
struct osmo_sub_auth_data {
	enum osmo_sub_auth_type type;
	enum osmo_auth_algo algo;
	union {
		struct {
			uint8_t opc[16]; /*!< operator invariant value */
			uint8_t k[OSMO_A5_MAX_KEY_LEN_BYTES];	/*!< secret key of the subscriber */
			uint8_t amf[2];
			uint64_t sqn;	/*!< sequence number (in: prev sqn; out: used sqn) */
			int opc_is_op;	/*!< is the OPC field OPC (0) or OP (1) ? */
			unsigned int ind_bitlen; /*!< nr of bits not in SEQ, only SQN */
			unsigned int ind; /*!< which IND slot to use an SQN from */
			uint64_t sqn_ms; /*!< sqn from AUTS (output value only) */
		} umts;
		struct {
			uint8_t ki[OSMO_A5_MAX_KEY_LEN_BYTES];	/*!< secret key */
		} gsm;
	} u;
};

/* data structure describing a computed auth vector, generated by AuC */
struct osmo_auth_vector {
	uint8_t rand[16];	/*!< random challenge */
	uint8_t autn[16];	/*!< authentication nonce */
	uint8_t ck[OSMO_A5_MAX_KEY_LEN_BYTES];		/*!< ciphering key */
	uint8_t ik[OSMO_A5_MAX_KEY_LEN_BYTES];		/*!< integrity key */
	uint8_t res[16];	/*!< authentication result */
	uint8_t res_len;	/*!< length (in bytes) of res */
	uint8_t kc[8];		/*!< Kc for GSM encryption (A5) */
	uint8_t sres[4];	/*!< authentication result for GSM */
	uint32_t auth_types;	/*!< bitmask of OSMO_AUTH_TYPE_* */
};

/* An implementation of an authentication algorithm */
struct osmo_auth_impl {
	struct llist_head list;
	enum osmo_auth_algo algo; /*!< algorithm we implement */
	const char *name;	/*!< name of the implementation */
	unsigned int priority;	/*!< priority value (resp. othe implementations */

	/*! callback for generate authentication vectors */
	int (*gen_vec)(struct osmo_auth_vector *vec,
			struct osmo_sub_auth_data *aud,
			const uint8_t *_rand);

	/* callback for generationg auth vectors + re-sync */
	int (*gen_vec_auts)(struct osmo_auth_vector *vec,
			    struct osmo_sub_auth_data *aud,
			    const uint8_t *auts, const uint8_t *rand_auts,
			    const uint8_t *_rand);
};

int osmo_auth_gen_vec(struct osmo_auth_vector *vec,
		      struct osmo_sub_auth_data *aud, const uint8_t *_rand);

int osmo_auth_gen_vec_auts(struct osmo_auth_vector *vec,
			   struct osmo_sub_auth_data *aud,
			   const uint8_t *auts, const uint8_t *rand_auts,
			   const uint8_t *_rand);

int osmo_auth_register(struct osmo_auth_impl *impl);

int osmo_auth_load(const char *path);

int osmo_auth_supported(enum osmo_auth_algo algo);
void osmo_c4(uint8_t *ck, const uint8_t *kc);
const char *osmo_auth_alg_name(enum osmo_auth_algo alg);
enum osmo_auth_algo osmo_auth_alg_parse(const char *name);

void osmo_auth_c3(uint8_t kc[], const uint8_t ck[], const uint8_t ik[]);

/* @} */
