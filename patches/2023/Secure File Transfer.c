//
//  Secure File Transfer.c
//  Snocomm
//
//  Created by Andres Barbudo on 12/20/23.
//  Copyright © 2023 Snocomm. All rights reserved.
//

#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>
#include "SecureTransport.h"
#include <stdio.h>

int main() {
    SSLContextRef sslContext;
    SSLCreateContext(NULL, kSSLClientSide, kSSLStreamType);
    
    // Configure SSL/TLS context, load certificates, and set up client parameters
    
    // Perform SSL/TLS handshake and secure file transfer using SecureTransport
    
    SSLClose(sslContext);
    
    return 0;
}
