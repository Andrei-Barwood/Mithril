//
//  Generate and Verify HMAC.c
//  Snocomm
//
//  Created by Andres Barbudo on 12/18/23.
//  Copyright © 2023 Snocomm. All rights reserved.
//

#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>
#include <CommonCrypto/CommonHMAC.h>
#include <stdio.h>

int main() {
    const char *key = "MySecretKey";
    const char *data = "Hello, World!";
    
    CCHmacContext ctx;
    CCHmacInit(&ctx, kCCHmacAlgSHA256, key, strlen(key));
    CCHmacUpdate(&ctx, data, strlen(data));
    
    unsigned char mac[CC_SHA256_DIGEST_LENGTH];
    CCHmacFinal(&ctx, mac);
    
    // Print or use the generated HMAC in your application
    
    return 0;
}

