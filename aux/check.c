/*
* ====================================================================
* Copyright 2016 LG Electronics, All Rights Reserved.
* The code is licensed persuant to accompanying the GPLv3 free
* software license.
* ====================================================================
*/

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "fantomas.h"
#include "fantomas16_ctr.h"

static inline uint8_t test_char(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    } else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
  return 0;
}

size_t test_conv(uint8_t *buf, size_t len, const char *str) {
    unsigned int i;

    for (i = 0; *str && i < len; i++, buf++, str += 2) {
        *buf = test_char(str[0]) << 4 | test_char(str[1]);
    }

    return i;
}

#define TEST_VECTOR(VAR, STR)                                               \
    uint8_t VAR[(strlen(STR)) >> 1]  __attribute__ ((aligned (16)));        \
    assert(test_conv(VAR, sizeof(VAR), STR) == (strlen(STR) >> 1));         \



int8_t getPKCC(char *pt, char *ct, char *k, char *c, int it) {
    switch(it){
        case 0:
            memcpy(pt,"00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF",576);
            memcpy(ct,"907FE486A5E77E85D2B89F04FF3E92AD73BBC54811ADF89F7BFB3DFB25196A67CEAF68E79103CFCCDD2C321EB7F99E561D90CAE708149F0B83C9225191E6E404F4AC039FF23ABED7BA6D62834FED7C6BF996E46B4A879578705017484A49EFB0654F1BBC4EED1AF0462202D4129BCB0CD45D7A6A3F074710F4407F2022E3D49D535AE31834D9A5590A6F77F0A547E99E0778C3D38B534E3439E603ABFB89AF348EA5ABC7567308BB209E400FD6CE2128371C90B111DAB15F25871EC1D4AB26B3C010AA262834575DCFA8BF918A9634AC44EFD907DAF90DE0705163E99B55FBD04941CB7443756FF726B468D8B9862F7E99582BF77A3F47B9077BF9D023063D61CBBF1AE507C2991ADFEFF4C06E02B90E8F6815EC18BE3ABB17EF656B7456761D",576);
            memcpy(k ,"0123456789ABCDEF0123456789ABCDEF", 32);
            memcpy(c ,"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF", 32);
            pt[576] = ct[576] = c[32] = k[32] = '\0';
            return 1;
        case 1:
            memcpy(pt,"123321123321123321123321ABCCBAABCC", 34);
            memcpy(ct,"7FC1EB94BE1621219420C882D6686FFE5D", 34);
            memcpy(k ,"0123456789ABCDEF0123456789ABCDEF", 32);
            memcpy(c ,"FFFFFFFFFFFFFFFFFFFFFFFFFFF0FFFF", 32);
            pt[34] = ct[34] = c[32] = k[32] = '\0';
            return 1;
        default:
            return 0;
    }

}


int8_t getPKC(char *pt, char *ct, char *k, int it) {
    switch(it){
        case 0:
            memcpy(pt,"AAAACCCCF0F000FF0000000000000000", 32);
            memcpy(k ,"555566667878807F0080000000000000", 32);
            memcpy(ct,"FE8163D47AB6189A4E550FEBB612E3E2", 32);
            pt[34] = ct[34] = k[32] = '\0';
            return 1;
        case 1:
            memcpy(pt,"000102030405060708090A0B0C0D0E0F", 32);
            memcpy(k ,"0102030405060708090A0B0C0D0E0F10", 32);
            memcpy(ct,"7C56DB0B18CACB391ABAB7C0E416A342", 32);
            pt[34] = ct[34] = k[32] = '\0';
            return 1;
        default:
            return 0;
    }

}


void test_fantomas_sse( ) {
    char hkey[33], hpt[33], hct[33];
    uint8_t output[16] __attribute__ ((aligned (16)));
    int it = 0;

    printf("Testing if Fantomas SSE satisfies the test vectors...\r\n");

    int nerror = 0;
    int error = 0;
    int n_test = 0;
    while(getPKC(hpt, hct, hkey, it++)) {
        n_test++;
        error = 0;
        TEST_VECTOR(key, hkey);
        TEST_VECTOR(plain_text, hpt);
        TEST_VECTOR(cipher_text, hct);

        enc_fantomas(output, plain_text, key);

        for(int i = 0; i < 16; i++) {
            if(output[i] != cipher_text[i]) {
                printf("[FAIL]\r\n");
                printf("Encryption error in test #%d:\r\n", n_test);
                printf("    real ciphertext: ");
                for(int j=0; j<16; j++) {
                    printf("%02X",cipher_text[j]);
                }
                printf("\r\n             output: ");
                for(int j=0; j<16; j++) {
                    printf("%02X",output[j]);
                }
                printf("\r\n");
                nerror++;
                error = 1;
                break;
            }
        }
        if(!error) {
            printf("[PASS]\r\n");
            printf("    real ciphertext: ");
            for(int j=0; j<16; j++) {
                printf("%02X",cipher_text[j]);
            }
            printf("\r\n             output: ");
            for(int j=0; j<16; j++) {
                printf("%02X",output[j]);
            }
            printf("\r\n");
        }
        error = 0;

        dec_fantomas(output, cipher_text, key);

        for(int i = 0; i < 16; i++) {
            if(output[i] != plain_text[i]) {
                printf("[FAIL]\r\n");
                printf("Decryption error in test #%d:\r\n", n_test);
                printf("     real plaintext: ");
                for(int j=0; j<16; j++) {
                    printf("%02X",plain_text[j]);
                }
                printf("\r\n             output: ");
                for(int j=0; j<16; j++) {
                    printf("%02X",output[j]);
                }
                printf("\r\n");
                nerror++;
                error = 1;
                break;
            }
        }
        if(!error) {
            printf("[PASS]\r\n");
            printf("     real plaintext: ");
            for(int j=0; j<16; j++) {
                printf("%02X",plain_text[j]);
            }
            printf("\r\n             output: ");
            for(int j=0; j<16; j++) {
                printf("%02X",output[j]);
            }
            printf("\r\n");
        }
    }
    
    printf("Total errors: %d\r\n\r\n",nerror);
}

void test_fantomas16_ctr_sse( ) {
    char hkey[33], hpt[600], hct[600], hcr[33];
    int it = 0;

    printf("Testing if Fantomas_CTR (16 blocks) SSE satisfies the test vectors...\r\n");

    int nerror = 0;
    int error = 0;
    int n_test = 0;
    while(getPKCC(hpt, hct, hkey, hcr, it++)) {
        n_test++;
        error = 0;
        TEST_VECTOR(key, hkey);
        TEST_VECTOR(cnt, hcr);
        TEST_VECTOR(plain_text, hpt);
        TEST_VECTOR(cipher_text, hct);
        uint32_t l_io = strlen(hpt) >> 1;
        TEST_VECTOR(io0, hpt);

        ctr_fantomas16(io0,l_io, key, cnt);

        for(int i = 0; i < 16; i++) {
            if(io0[i] != cipher_text[i]) {
                printf("[FAIL]\r\n");
                printf("Encryption error in test #%d:\r\n", n_test);
                printf("    real ciphertext: ");
                for(uint32_t j=0; j<l_io; j++) {
                    printf("%02X",cipher_text[j]);
                }
                printf("\r\n             output: ");
                for(uint32_t j=0; j<l_io; j++) {
                    printf("%02X",io0[j]);
                }
                printf("\r\n");
                nerror++;
                error = 1;
                break;
            }
        }
        if(!error) {
            printf("    real ciphertext: ");
            for(uint32_t j=0; j<l_io; j++) {
                printf("%02X",cipher_text[j]);
            }
            printf("\r\n             output: ");
            for(uint32_t j=0; j<l_io; j++) {
                printf("%02X",io0[j]);
            }
            printf("\r\n");
        }
        error = 0;

        TEST_VECTOR(io1, hct);

        ctr_fantomas16(io1,l_io, key, cnt);

        for(int i = 0; i < 16; i++) {
            if(io1[i] != plain_text[i]) {
                printf("[FAIL]\r\n");
                printf("Decryption error in test #%d:\r\n", n_test);
                printf("     real plaintext: ");
                for(uint32_t j=0; j<l_io; j++) {
                    printf("%02X",cipher_text[j]);
                }
                printf("\r\n             output: ");
                for(uint32_t j=0; j<l_io; j++) {
                    printf("%02X",io1[j]);
                }
                printf("\r\n");
                nerror++;
                error = 1;
                break;
            }
        }
        if(!error) {
            printf("[PASS]\r\n");
            printf("     real plaintext: ");
            for(uint32_t j=0; j<l_io; j++) {
                printf("%02X",cipher_text[j]);
            }
            printf("\r\n             output: ");
            for(uint32_t j=0; j<l_io; j++) {
                printf("%02X",io1[j]);
            }
            printf("\r\n");
        }
    }
    
    printf("Total errors: %d\r\n\r\n",nerror);
}


int main() {
    test_fantomas_sse( ) ;
    test_fantomas16_ctr_sse( ) ;

    return 0;
}