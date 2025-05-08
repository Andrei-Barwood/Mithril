//
//  Check if Password Meets Policy.c
//  Snocomm
//
//  Created by Andres Barbudo on 12/18/23.
//  Copyright © 2023 Snocomm. All rights reserved.
//

#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>
#include <stdio.h>

int main() {
    CFStringRef password = CFSTR("SecureP@ssword123");
    
    CFDictionaryRef policy = CFDictionaryCreate(
                                                kCFAllocatorDefault,
                                                (const void **)&kSecPolicyLabel,
                                                (const void **)&kSecPolicyLabelPasswordQuality,
                                                1,
                                                NULL,
                                                NULL
                                                );
    
    OSStatus status = SecCheckPrivateKeyPassword(policy, password, NULL);
    
    if (status == errSecSuccess) {
        printf("Password meets policy requirements.\n");
    } else {
        printf("Password does not meet policy requirements. Error: %d\n", (int)status);
    }
    
    CFRelease(policy);
    
    return 0;
}

