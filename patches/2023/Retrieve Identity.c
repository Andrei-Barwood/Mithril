//
//  Retrieve Identity.c
//  Snocomm
//
//  Created by Andres Barbudo on 12/18/23.
//  Copyright © 2023 Snocomm. All rights reserved.
//

#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>
#include <stdio.h>

int main() {
    CFStringRef identityName = CFSTR("MyIdentity");
    SecIdentityRef identity;
    
    OSStatus status = SecIdentityCopyPreferred(identityName, NULL, &identity);
    
    if (status == errSecSuccess) {
        printf("Identity retrieved successfully.\n");
        CFRelease(identity);
    } else {
        printf("Error: %d\n", (int)status);
    }
    
    return 0;
}

