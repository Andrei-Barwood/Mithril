//
//  HTTPS Connection.c
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
    CFStringRef serverName = CFSTR("www.example.com");
    
    CFReadStreamRef readStream;
    CFWriteStreamRef writeStream;
    
    CFStreamCreatePairWithSocketToHost(NULL, serverName, 443, &readStream, &writeStream);
    
    CFReadStreamOpen(readStream);
    CFWriteStreamOpen(writeStream);
    
    // Perform SSL/TLS handshake and secure communication using SecureTransport
    
    CFReadStreamClose(readStream);
    CFWriteStreamClose(writeStream);
    
    return 0;
}
