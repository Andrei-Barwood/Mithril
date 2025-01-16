//
//  Cipher Suite Configuration.c
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
    
    // Configure SSL/TLS context, set preferred cipher suites, and other parameters
    
    // Perform SSL/TLS handshake and secure communication using SecureTransport
    
    SSLClose(sslContext);
    
    return 0;
}
