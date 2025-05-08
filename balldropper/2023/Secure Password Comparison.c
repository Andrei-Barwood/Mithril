//
//  Secure Password Comparison.c
//  Snocomm
//
//  Created by Andres Barbudo on 12/18/23.
//  Copyright © 2023 Snocomm. All rights reserved.
//

#include <CommonCrypto/CommonCrypto.h>
#include <stdio.h>

int compareSecureStrings(const char *password1, const char *password2) {
    size_t len1 = strlen(password1);
    size_t len2 = strlen(password2);
    
    if (len1 != len2) {
        return 0;  // Passwords are not equal
    }
    
    return CCRSACryptorFieldsEqual(password1, password2, len1);
}

int main() {
    const char *storedPassword = "SecureP@ssword123";
    const char *inputPassword = "SecureP@ssword123";
    
    if (compareSecureStrings(storedPassword, inputPassword)) {
        printf("Passwords match.\n");
    } else {
        printf("Passwords do not match.\n");
    }
    
    return 0;
}
