//
//  Generate RSA Key Pair.c
//  Snocomm
//
//  Created by Andres Barbudo on 12/18/23.
//  Copyright © 2023 Snocomm. All rights reserved.
//


#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>
#include <stdio.h>

int main() {
    SecKeyRef publicKey, privateKey;
    CFMutableDictionaryRef parameters = CFDictionaryCreateMutable(NULL, 0, NULL, NULL);
    
    CFDictionaryAddValue(parameters, kSecAttrKeyType, kSecAttrKeyTypeRSA);
    CFDictionaryAddValue(parameters, kSecAttrKeySizeInBits, kCFNumberSInt32Type);
    
    OSStatus status = SecKeyGeneratePair(parameters, &publicKey, &privateKey);
    
    if (status == errSecSuccess) {
        printf("RSA key pair generated successfully.\n");
        CFRelease(publicKey);
        CFRelease(privateKey);
    } else {
        printf("Error: %d\n", (int)status);
    }
    
    CFRelease(parameters);
    
    return 0;
}
