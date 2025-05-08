//
//  AES Encryption and Decryption.c
//  Snocomm
//
//  Created by Andres Barbudo on 12/18/23.
//  Copyright © 2023 Snocomm. All rights reserved.
//

#include <CommonCrypto/CommonCryptor.h>
#include <stdio.h>

void encryptAES(const char *key, const char *iv, const char *plaintext) {
    size_t dataLength = strlen(plaintext);
    size_t bufferSize = dataLength + kCCBlockSizeAES128;
    char ciphertext[bufferSize];
    
    CCCrypt(kCCEncrypt, kCCAlgorithmAES, kCCOptionPKCS7Padding,
            key, kCCKeySizeAES256, iv,
            plaintext, dataLength,
            ciphertext, bufferSize, &dataLength);
    
    printf("AES Encrypted Text: %s\n", ciphertext);
}

void decryptAES(const char *key, const char *iv, const char *ciphertext) {
    size_t dataLength = strlen(ciphertext);
    size_t bufferSize = dataLength + kCCBlockSizeAES128;
    char decryptedText[bufferSize];
    
    CCCrypt(kCCDecrypt, kCCAlgorithmAES, kCCOptionPKCS7Padding,
            key, kCCKeySizeAES256, iv,
            ciphertext, dataLength,
            decryptedText, bufferSize, &dataLength);
    
    decryptedText[dataLength] = '\0';  // Null-terminate the decrypted text
    
    printf("AES Decrypted Text: %s\n", decryptedText);
}

int main() {
    const char *key = "SecretKey12345678";
    const char *iv = "InitializationVe";
    const char *plaintext = "Hello, AES!";
    
    encryptAES(key, iv, plaintext);
    decryptAES(key, iv, "EncryptedTextFromAbove");
    
    return 0;
}

