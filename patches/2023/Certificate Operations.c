//
//  Certificate Operations.c
//  Snocomm
//
//  Created by Andres Barbudo on 12/18/23.
//  Copyright © 2023 Snocomm. All rights reserved.
//

#include <stdio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>

int main() {
    CFStringRef certificatePath = CFSTR("/path/to/certificate.cer");
    CFDataRef certificateData = CFDataCreateWithContentsOfURL(
                                                              kCFAllocatorDefault,
                                                              CFURLCreateWithFileSystemPath(kCFAllocatorDefault, certificatePath, kCFURLPOSIXPathStyle, false)
                                                              );
    
    SecCertificateRef certificate = SecCertificateCreateWithData(NULL, certificateData);
    if (certificate != NULL) {
        OSStatus status = SecCertificateIsValid(certificate, NULL);
        if (status == errSecSuccess) {
            printf("Certificate is valid.\n");
        } else {
            printf("Certificate is not valid. Error: %d\n", (int)status);
        }
    } else {
        printf("Error loading certificate.\n");
    }
    
    CFRelease(certificateData);
    CFRelease(certificate);
    
    return 0;
}

