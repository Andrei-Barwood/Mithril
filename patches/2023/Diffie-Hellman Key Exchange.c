//
//  Diffie-Hellman Key Exchange.c
//  Snocomm
//
//  Created by Andres Barbudo on 12/18/23.
//  Copyright © 2023 Snocomm. All rights reserved.
//

#include <CommonCrypto/CommonCryptor.h>
#include <CommonCrypto/CommonDH.h>
#include <stdio.h>

int main() {
    size_t privateKeySize = CC_DH_MAX_KEY_SIZE / 8;
    size_t publicKeySize = CC_DH_MAX_KEY_SIZE / 8;
    size_t sharedKeySize = CC_DH_MAX_KEY_SIZE / 8;
    
    uint8_t privateKey[privateKeySize];
    uint8_t publicKey[publicKeySize];
    uint8_t sharedKey[sharedKeySize];
    
    CCDHParameters parameters;
    CCDHInitializeWithSeed(parameters, kCCDHRFC3526Group5, kCCDHParties, NULL, 0);
    
    CCDHGenerateKeyPairWithSeed(parameters, kCCDHParties, privateKey, publicKey, NULL, 0);
    
    CCDHComputeKey(parameters, kCCDHParties, privateKey, publicKey, sharedKey, NULL, 0);
    
    // Use the shared key for further secure communication
    
    return 0;
}

