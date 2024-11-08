/*
 * gen_certs.h
 *
 *  Created on: Nov 5, 2024
 *      Author: kiro
 */

#ifndef INC_GEN_CERTS_H_
#define INC_GEN_CERTS_H_

#include "stdio.h"

int verify_key_cert_pair(const unsigned char *cert_buf, size_t cert_len, const unsigned char *key_buf, size_t key_len);

#endif /* INC_GEN_CERTS_H_ */
