//
//  Check Password Strength.c
//  Snocomm
//
//  Created by Andres Barbudo on 12/18/23.
//  Copyright © 2023 Snocomm. All rights reserved.
//

#include <CommonCrypto/CommonPassword.h>
#include <stdio.h>

int main() {
    const char *password = "SecureP@ssword123";
    
    CCPWStatus pwStatus;
    CCPasswordIsPasswordWeak(password, &pwStatus);
    
    if (pwStatus == kCCPWStatusPassWeak) {
        printf("Password is weak.\n");
    } else {
        printf("Password is strong.\n");
    }
    
    return 0;
}
