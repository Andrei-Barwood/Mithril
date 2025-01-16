//
//  Secure Transport with Non-Blocking I-O.c
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
    // Use non-blocking I/O for secure communication
    
    // Perform SSL/TLS handshake and secure communication using SecureTransport
    
    SSLClose(sslContext);
    
    return 0;
}
