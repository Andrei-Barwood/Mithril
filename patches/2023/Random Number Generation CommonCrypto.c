//
//  Random Number Generation CommonCrypto.c
//  Snocomm
//
//  Created by Andres Barbudo on 12/18/23.
//  Copyright © 2023 Snocomm. All rights reserved.
//

#include <CommonCrypto/CommonRandom.h>
#include <stdio.h>

int main() {
    uint32_t randomValue;
    CCRandomGenerateBytes(&randomValue, sizeof(randomValue));
    
    printf("Random Value: %u\n", randomValue);
    
    return 0;
}
