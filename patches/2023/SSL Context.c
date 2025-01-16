//
//  SSL Context.c
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
    // (Note: Error handling is omitted for brevity)
    
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_NAME, &server_addr.sin_addr);
    
    connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    
    // Step 2: Create an SSL context
    SSLContextRef sslContext;
    OSStatus status = SSLCreateContext(NULL, kSSLClientSide, kSSLStreamType, &sslContext);
    if (status != noErr) {
        fprintf(stderr, "SSLCreateContext failed with error %d\n", (int)status);
        close(sockfd);
        return -1;
    }
    
    // Step 3: Set up the SSL connection
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
    
    // Step 4: Perform SSL/TLS handshake
    status = SSLHandshake(sslContext);
    if (status != noErr) {
        fprintf(stderr, "SSLHandshake failed with error %d\n", (int)status);
        SSLClose(sslContext);
        close(sockfd);
        return -1;
    }
    
    // Step 5: Secure communication
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
    
    // Step 6: Close the SSL connection and socket
    SSLClose(sslContext);
    close(sockfd);
    
    return 0;
}
