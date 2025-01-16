//
//  Sign and Verify with RSA.c
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
    SecKeyRef privateKey, publicKey;
    // Assume you have privateKey and publicKey from a previous key generation
    
    const char *dataToSign = "Hello, World!";
    size_t dataToSignLength = strlen(dataToSign);
    
    uint8_t signature[256];  // Assuming RSA-2048 key size
    
    OSStatus status = SecKeyRawSign(
                                    privateKey,
                                    kSecPaddingPKCS1,
                                    (const uint8_t *)dataToSign,
                                    dataToSignLength,
                                    signature,
                                    sizeof(signature)
                                    );
    
    if (status == errSecSuccess) {
        printf("Signature created successfully.\n");
        
        status = SecKeyRawVerify(
                                 publicKey,
                                 kSecPaddingPKCS1,
                                 (const uint8_t *)dataToSign,
                                 dataToSignLength,
                                 signature,
                                 sizeof(signature)
                                 );
        
        if (status == errSecSuccess) {
            printf("Signature verified successfully.\n");
        } else {
            printf("Verification failed. Error: %d\n", (int)status);
        }
    } else {
        printf("Signing failed. Error: %d\n", (int)status);
    }
    
    return 0;
}
