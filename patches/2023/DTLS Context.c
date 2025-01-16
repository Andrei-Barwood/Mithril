//
//  DTLS Context.c
//  Snocomm
//
//  Created by Andres Barbudo on 12/20/23.
//  Copyright © 2023 Snocomm. All rights reserved.
//

#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>
#include <SecureTransport/SecureTransport.h>
#include <stdio.h>

#define SERVER_NAME "www.example.com"
#define PORT 443

int main() {
    // Step 1: Create a TCP socket and connect to the server
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("Error creating socket");
        return -1;
    }
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_NAME, &server_addr.sin_addr) != 1) {
        perror("Error converting server address");
        close(sockfd);
        return -1;
    }
    
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error connecting to server");
        close(sockfd);
        return -1;
    }
    
    // Step 2: Create a DTLS context
    SSLContextRef sslContext;
    OSStatus status = SSLCreateContext(NULL, kSSLClientSide, kSSLDatagramType, &sslContext);
    if (status != noErr) {
        fprintf(stderr, "SSLCreateContext failed with error %d\n", (int)status);
        close(sockfd);
        return -1;
    }
    
    // Step 3: Load certificates and private key
    CFStringRef certPath = CFStringCreateWithCString(NULL, "/path/to/client.crt", kCFStringEncodingUTF8);
    CFStringRef keyPath = CFStringCreateWithCString(NULL, "/path/to/client.key", kCFStringEncodingUTF8);
    
    status = SSLSetCertificate(sslContext, CFArrayCreate(NULL, (const void**)&certPath, 1, NULL));
    if (status != noErr) {
        fprintf(stderr, "SSLSetCertificate failed with error %d\n", (int)status);
        CFRelease(certPath);
        CFRelease(keyPath);
        SSLClose(sslContext);
        close(sockfd);
        return -1;
    }
    
    status = SSLSetPrivateKey(sslContext, CFArrayCreate(NULL, (const void**)&keyPath, 1, NULL));
    if (status != noErr) {
        fprintf(stderr, "SSLSetPrivateKey failed with error %d\n", (int)status);
        CFRelease(certPath);
        CFRelease(keyPath);
        SSLClose(sslContext);
        close(sockfd);
        return -1;
    }
    
    CFRelease(certPath);
    CFRelease(keyPath);
    
    // Step 4: Set up the SSL connection
    status = SSLSetIOFuncs(sslContext, SocketRead, SocketWrite);
    if (status != noErr) {
        fprintf(stderr, "SSLSetIOFuncs failed with error %d\n", (int)status);
        SSLClose(sslContext);
        close(sockfd);
        return -1;
    }
    
    status = SSLSetConnection(sslContext, (SSLConnectionRef)sockfd);
    if (status != noErr) {
        fprintf(stderr, "SSLSetConnection failed with error %d\n", (int)status);
        SSLClose(sslContext);
        close(sockfd);
        return -1;
    }
    
    // Step 5: Perform DTLS handshake
    status = SSLHandshake(sslContext);
    if (status != noErr) {
        fprintf(stderr, "SSLHandshake failed with error %d\n", (int)status);
        SSLClose(sslContext);
        close(sockfd);
        return -1;
    }
    
    // Step 6: Secure communication
    const char *request = "GET / HTTP/1.1\r\nHost: www.example.com\r\n\r\n";
    char buffer[4096];
    
    status = SSLWrite(sslContext, request, strlen(request), NULL);
    if (status != noErr) {
        fprintf(stderr, "SSLWrite failed with error %d\n", (int)status);
        SSLClose(sslContext);
        close(sockfd);
        return -1;
    }
    
    status = SSLRead(sslContext, buffer, sizeof(buffer), NULL);
    if (status != noErr) {
        fprintf(stderr, "SSLRead failed with error %d\n", (int)status);
        SSLClose(sslContext);
        close(sockfd);
        return -1;
    }
    
    printf("Received response:\n%s\n", buffer);
    
    // Step 7: Close the SSL connection and socket
    SSLClose(sslContext);
    close(sockfd);
    
    return 0;
}
