//
//  Retreive Passwords from Keychain.c
//  Snocomm
//
//  Created by Andres Barbudo on 12/18/23.
//  Copyright © 2023 Snocomm. All rights reserved.
//

#include <stdio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>
#include <stdio.h>

int main() {
    CFStringRef serviceName = CFSTR("MyApp");
    CFStringRef accountName = CFSTR("user@example.com");
    CFStringRef password;
    
    OSStatus status = SecKeychainFindGenericPassword(
                                                     NULL,
                                                     CFStringGetLength(accountName),
                                                     CFStringGetCStringPtr(accountName, kCFStringEncodingUTF8),
                                                     CFStringGetLength(serviceName),
                                                     CFStringGetCStringPtr(serviceName, kCFStringEncodingUTF8),
                                                     NULL,
                                                     NULL,
                                                     &password
                                                     );
    
    if (status == errSecSuccess) {
        printf("Retrieved password from keychain: %s\n", CFStringGetCStringPtr(password, kCFStringEncodingUTF8));
        CFRelease(password);
    } else {
        printf("Error: %d\n", (int)status);
    }
    
    return 0;
}
