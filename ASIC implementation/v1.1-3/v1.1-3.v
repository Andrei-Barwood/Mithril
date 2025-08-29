//
// iot_crypto_test.v
// IoT Device Cryptographic Operations Test
//
// Created by Andres Barbudo on 28-08-25.
//

module iot_crypto_test;
    
    // Registers to hold large numbers as strings
    reg [8*50:1] large_number_str;  // 50-character string buffer
    reg [8*50:1] result_str;        // Result string buffer
    reg [31:0] small_number;        // 32-bit small number
    
    initial begin
        // Display module information
        $display("=== IoT Cryptographic Mixed Addition Test ===");
        $display("Using FLINT library for secure big integer operations");
        
        // Test Case 1: Basic mixed addition
        large_number_str = "123456789012345678901234567890";
        small_number = 42;
        
        $display("\nTest Case 1: Basic Mixed Addition");
        $display("Large number: %s", large_number_str);
        $display("Small number: %d", small_number);
        
        // Call PLI function
        $mixed_addition(result_str, large_number_str, small_number);
        $display("Result: %s", result_str);
        
        // Test Case 2: Cryptographic key addition
        large_number_str = "987654321098765432109876543210987654321";
        small_number = 256;  // Common key increment
        
        $display("\nTest Case 2: Cryptographic Key Increment");
        $display("Base key: %s", large_number_str);
        $display("Increment: %d", small_number);
        
        $mixed_addition(result_str, large_number_str, small_number);
        $display("New key: %s", result_str);
        
        // Test Case 3: IoT device ID calculation
        large_number_str = "999999999999999999999999999999999";
        small_number = 1024; // Device offset
        
        $display("\nTest Case 3: IoT Device ID Generation");
        $display("Base ID: %s", large_number_str);
        $display("Device offset: %d", small_number);
        
        $mixed_addition(result_str, large_number_str, small_number);
        $display("Device ID: %s", result_str);
        
        $display("\n=== Test Complete ===");
        $finish;
    end
    
endmodule
