//
//  Encrypt and Decrypt with RSA Key.c
//  Snocomm
//
//  Created by Andres Barbudo on 12/18/23.
//  Copyright © 2023 Snocomm. All rights reserved.
//

#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>
#include <CommonCrypto/CommonCrypto.h>
#include <stdio.h>

int main() {
    SecKeyRef publicKey, privateKey;
    // Assume you have privateKey and publicKey from a previous key generation
    
    const char *plaintext = "Hello, World!";
    size_t plaintextLength = strlen(plaintext);
    
    size_t cipherBufferSize = SecKeyGetBlockSize(publicKey);
    uint8_t *cipherBuffer = malloc(cipherBufferSize);
    
    OSStatus status = SecKeyEncrypt(
                                    publicKey,
                                    kSecPaddingPKCS1,
                                    (const uint8_t *)plaintext,
                                    plaintextLength,
                                    cipherBuffer,
                                    &cipherBufferSize
                                    );
    
    if (status == errSecSuccess) {
        printf("Encryption successful.\n");
        
        size_t decryptedBufferSize = SecKeyGetBlockSize(privateKey);
        uint8_t *decryptedBuffer = malloc(decryptedBufferSize);
        
        status = SecKeyDecrypt(
                               privateKey,
                               kSecPaddingPKCS1,
                               cipherBuffer,
                               cipherBufferSize,
                               decryptedBuffer,
                               &decryptedBufferSize
                               );
        
        if (status == errSecSuccess) {
            printf("Decryption successful. Decrypted Text: %s\n", (char *)decryptedBuffer);
        } else {
            printf("Decryption failed. Error: %d\n", (int)status);
        }
        
        free(decryptedBuffer);
    } else {
        printf("Encryption failed. Error: %d\n", (int)status);
    }
    
    free(cipherBuffer);
    
    return 0;
}

