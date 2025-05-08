//
//  Password-Based Key Derivation with Salt.c
//  Snocomm
//
//  Created by Andres Barbudo on 12/18/23.
//  Copyright © 2023 Snocomm. All rights reserved.
//

#include <CommonCrypto/CommonKeyDerivation.h>
#include <stdio.h>

int main() {
    const char *password = "MySecurePassword";
    const char *salt = "RandomSalt";
    size_t derivedKeyLength = 32;  // 256 bits
    
    uint8_t derivedKey[derivedKeyLength];
    
    CCKeyDerivationPBKDF(kCCPBKDF2, password, strlen(password),
                         (uint8_t *)salt, strlen(salt),
                         kCCPRFHmacAlgSHA256, 10000,
                         derivedKey, derivedKeyLength);
    
    printf("Derived Key with Salt: ");
    for (size_t i = 0; i < derivedKeyLength; i++) {
        printf("%02x", derivedKey[i]);
    }
    printf("\n");
    
    return 0;
}
