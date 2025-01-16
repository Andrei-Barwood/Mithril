//
//  Cryptographic Operations.c
//  Snocomm
//
//  Created by Andres Barbudo on 12/18/23.
//  Copyright © 2023 Snocomm. All rights reserved.
//

#include <stdio.h>
#include <CommonCrypto/CommonDigest.h>

int main() {
    const char *inputData = "Hello, World!";
    size_t inputDataSize = strlen(inputData);
    
    unsigned char hash[CC_SHA256_DIGEST_LENGTH];
    CC_SHA256(inputData, (CC_LONG)inputDataSize, hash);
    
    printf("SHA-256 Hash: ");
    for (size_t i = 0; i < CC_SHA256_DIGEST_LENGTH; i++) {
        printf("%02x", hash[i]);
    }
    printf("\n");
    
    return 0;
}
