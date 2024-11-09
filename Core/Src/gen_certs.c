/*
 * gen_certs.c
 *
 *  Created on: Nov 5, 2024
 *      Author: kiro
 */



#include "gen_certs.h"

//#include "mbedtls/x509.h"
//#include "mbedtls/pk.h"
//#include "mbedtls/entropy.h"
//#include "mbedtls/ctr_drbg.h"
//#include "mbedtls/ecp.h"
//#include "mbedtls/oid.h"
//#include "stdio.h"
//#include "mbedtls/x509_crt.h"
//#include "x509_crt.h"
//#include "mbedtls/x509_crt.h"
//#include "entropy.h"
//
//#include <mbedtls/ctr_drbg.h>
//#include <mbedtls/entropy.h>
//#include <mbedtls/pem.h>
//#include <mbedtls/ssl.h>
//#include "rsa.h"

#include "stm32f4xx_hal.h"
//#include "compat-1.3.h"

#include "mbedtls/sha256.h"



#include "mbedtls/x509_crt.h"
#include "mbedtls/pk.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include <string.h>

// Глобальное состояние, которое изменяется при каждом вызове для повышения непредсказуемости
static uint32_t pseudo_random_state = 0xA5A5A5A5;

int my_entropy_source(void *data, unsigned char *output, size_t len, size_t *olen) {
    mbedtls_sha256_context sha_ctx;
    mbedtls_sha256_init(&sha_ctx);
    mbedtls_sha256_starts(&sha_ctx, 0); // Используем SHA-256 (полная версия)

    for (size_t i = 0; i < len; i += 32) {  // Генерируем SHA-256 блоками (по 32 байта)
        uint32_t tick = HAL_GetTick();
        pseudo_random_state ^= tick;  // Изменяем состояние на основе HAL_GetTick

        // Подготавливаем входные данные для SHA (используем HAL_GetTick и случайное состояние)
        unsigned char seed_data[8];
        seed_data[0] = (tick >> 24) & 0xFF;
        seed_data[1] = (tick >> 16) & 0xFF;
        seed_data[2] = (tick >> 8) & 0xFF;
        seed_data[3] = tick & 0xFF;
        seed_data[4] = (pseudo_random_state >> 24) & 0xFF;
        seed_data[5] = (pseudo_random_state >> 16) & 0xFF;
        seed_data[6] = (pseudo_random_state >> 8) & 0xFF;
        seed_data[7] = pseudo_random_state & 0xFF;

        // Обновляем SHA-256 состоянием из `seed_data`
        mbedtls_sha256_update(&sha_ctx, seed_data, sizeof(seed_data));

        // Финализируем SHA-256 и получаем 32 байта случайных данных
        unsigned char hash_output[32];
        mbedtls_sha256_finish(&sha_ctx, hash_output);

        // Копируем часть результата SHA-256 в выходной буфер
        size_t copy_len = (len - i < 32) ? len - i : 32;
        memcpy(output + i, hash_output, copy_len);

        // Инициализируем SHA-256 заново для следующей итерации
        mbedtls_sha256_starts(&sha_ctx, 0);
    }

    *olen = len;
    mbedtls_sha256_free(&sha_ctx);

    return 0;  // Успешное выполнение
}





int verify_key_cert_pair(const unsigned char *cert_buf, size_t cert_len, const unsigned char *key_buf, size_t key_len) {
    int ret;
    mbedtls_x509_crt cert;
    mbedtls_pk_context key;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_entropy_context entropy;
    const char *pers = "key_cert_verification";
    unsigned char test_message[] = "test message";
    unsigned char signature[256];
    size_t sig_len;

    // Инициализация структур
    mbedtls_x509_crt_init(&cert);
    mbedtls_pk_init(&key);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_entropy_init(&entropy);

    if ((ret = mbedtls_entropy_add_source(&entropy, my_entropy_source, NULL, MBEDTLS_ENTROPY_MAX_GATHER, MBEDTLS_ENTROPY_SOURCE_STRONG)) != 0){
    	return ret;
    }


    // Инициализация генератора случайных чисел
    if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char *)pers, strlen(pers))) != 0) {
        printf("Failed in mbedtls_ctr_drbg_seed: -0x%x\n", -ret);
        return ret;
    }

    // Загрузка сертификата
    if ((ret = mbedtls_x509_crt_parse(&cert, cert_buf, cert_len)) != 0) {
        printf("Failed to parse certificate: -0x%x\n", -ret);
        return ret;
    }

    // Загрузка приватного ключа
    if ((ret = mbedtls_pk_parse_key(&key, key_buf, key_len, NULL, 0, NULL, NULL)) != 0) {
        printf("Failed to parse private key: -0x%x\n", -ret);
        return ret;
    }

    // Подпись сообщения приватным ключом
    if ((ret = mbedtls_pk_sign(&key, MBEDTLS_MD_SHA256, test_message, sizeof(test_message),
            signature, sizeof(signature), &sig_len, NULL, NULL)) != 0) {
        printf("Failed to sign message: -0x%x\n", -ret);
        return ret;
    }

    // Проверка подписи с использованием публичного ключа из сертификата
    if ((ret = mbedtls_pk_verify(&cert.pk, MBEDTLS_MD_SHA256, test_message, sizeof(test_message),
                                 signature, sig_len)) != 0) {
        printf("Key and certificate do not match: -0x%x\n", -ret);
        ret = -1;  // Ключ и сертификат не совпадают
    } else {
        printf("Key and certificate match.\n");
        ret = 0;  // Ключ и сертификат совпадают
    }

    // Освобождение ресурсов
    mbedtls_x509_crt_free(&cert);
    mbedtls_pk_free(&key);
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);

    return ret;
}
