//
//  Import and Export X-509 Certificate.c
//  Snocomm
//
//  Created by Andres Barbudo on 12/18/23.
//  Copyright © 2023 Snocomm. All rights reserved.
//

#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>
#include <stdio.h>

int main() {
    SecCertificateRef certificate;
    
    // Assume you have obtained a certificate from a previous operation
    // For example, you might load it from a file using SecCertificateCreateWithData
    
    // Export the certificate to a DER-encoded data
    CFDataRef exportedCertificateData = SecCertificateCopyData(certificate);
    // You can now save or transmit the exportedCertificateData as needed
    
    // Import the certificate from the DER-encoded data
    SecCertificateRef importedCertificate = SecCertificateCreateWithData(NULL, exportedCertificateData);
    
    if (importedCertificate != NULL) {
        printf("Certificate imported successfully.\n");
        CFRelease(importedCertificate);
    } else {
        printf("Error importing certificate.\n");
    }
    
    CFRelease(exportedCertificateData);
    
    return 0;
}

