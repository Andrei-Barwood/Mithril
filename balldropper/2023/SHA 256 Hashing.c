//
//  SHA 256 Hashing.c
//  Snocomm
//
//  Created by Andres Barbudo on 12/18/23.
//  Copyright © 2023 Snocomm. All rights reserved.
//

#include <CommonCrypto/CommonDigest.h>
#include <stdio.h>

int main() {
    const char *data = "Hello, World!";
    size_t dataLength = strlen(data);
    
    unsigned char hash[CC_SHA256_DIGEST_LENGTH];
    
    CC_SHA256(data, (CC_LONG)dataLength, hash);
    
    printf("SHA-256 Hash: ");
    for (size_t i = 0; i < CC_SHA256_DIGEST_LENGTH; i++) {
        printf("%02x", hash[i]);
    }
    printf("\n");
    
    return 0;
}

