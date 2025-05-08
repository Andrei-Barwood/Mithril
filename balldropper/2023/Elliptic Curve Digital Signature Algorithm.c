//
//  Elliptic Curve Digital Signature Algorithm.c
//  Snocomm
//
//  Created by Andres Barbudo on 12/18/23.
//  Copyright © 2023 Snocomm. All rights reserved.
//

#include <CommonCrypto/CommonCryptoError.h>
#include <CommonCrypto/CommonDigestSPI.h>
#include <CommonCrypto/CommonECDSAUtils.h>
#include <stdio.h>

int main() {
    const char *privateKeyHex = "4579F075...";
    const char *message = "Hello, ECDSA!";
    const char *signatureHex = "30460221008D...";
    
    size_t keySizeInBytes = CC_X963ImportImportEphemeralSize(256, 1);
    
    uint8_t keyData[keySizeInBytes];
    size_t keyDataSize = keySizeInBytes;
    
    CC_X963ImportEphemeral(keySizeInBytes, 1, privateKeyHex, keySizeInBytes, keyData, &keyDataSize);
    
    uint8_t signatureData[1024];
    size_t signatureDataSize = sizeof(signatureData);
    
    CCCryptorStatus status = CC_ECDSAVerify(kCC_ECDSA_256, CC_ECDSA_SIGNATURE_RAW, message, strlen(message),
                                            keyData, keyDataSize,
                                            signatureHex, strlen(signatureHex),
                                            signatureData, &signatureDataSize);
    
    if (status == kCCSuccess) {
        printf("ECDSA Signature is valid.\n");
    } else {
        printf("ECDSA Signature is not valid. Error: %d\n", (int)status);
    }
    
    return 0;
}

