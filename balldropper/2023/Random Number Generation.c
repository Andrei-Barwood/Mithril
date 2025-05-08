//
//  Random Number Generation.c
//  Snocomm
//
//  Created by Andres Barbudo on 12/18/23.
//  Copyright © 2023 Snocomm. All rights reserved.
//

#include <stdio.h>
#include <Security/Security.h>

int main() {
    uint32_t randomValue;
    OSStatus status = SecRandomCopyBytes(kSecRandomDefault, sizeof(randomValue), (uint8_t*)&randomValue);
    
    if (status == errSecSuccess) {
        printf("Random Value: %u\n", randomValue);
    } else {
        printf("Error generating random number. Error: %d\n", (int)status);
    }
    
    return 0;
}
