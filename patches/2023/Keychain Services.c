//
//  Keychain Services.c
//  Snocomm
//
//  Created by Andres Barbudo on 12/18/23.
//  Copyright © 2023 Snocomm. All rights reserved.
//

#include <stdio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>

int main() {
    CFStringRef serviceName = CFSTR("MyApp");
    CFStringRef accountName = CFSTR("user@example.com");
    CFStringRef password = CFSTR("mySecretPassword");
    
    OSStatus status = SecKeychainAddGenericPassword(
                                                    NULL,                 // Default keychain
                                                    CFStringGetLength(accountName),
                                                    CFStringGetCStringPtr(accountName, kCFStringEncodingUTF8),
                                                    CFStringGetLength(serviceName),
                                                    CFStringGetCStringPtr(serviceName, kCFStringEncodingUTF8),
                                                    CFStringGetLength(password),
                                                    CFStringGetCStringPtr(password, kCFStringEncodingUTF8),
                                                    NULL
                                                    );
    
    if (status == errSecSuccess) {
        printf("Password added to keychain.\n");
    } else {
        printf("Error: %d\n", (int)status);
    }
    
    return 0;
}
