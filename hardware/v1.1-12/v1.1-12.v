module test_rem_mod;
    reg [63:0] x;        // Input value
    reg [5:0] k;         // Power of 2 (k <= 63)
    reg [63:0] result;   // Output result
    
    initial begin
        $display("=== IoT Secure Modulo 2^k Division Test ===");
        
        // Test Case 1: 25 mod 2^3 = 25 mod 8 = 1
        x = 64'd25;
        k = 6'd3;
        $rem_mod_pow_of_2(x, k, result);
        $display("Test 1: %0d mod 2^%0d = %0d", x, k, result);
        
        // Test Case 2: 1000 mod 2^10 = 1000 mod 1024 = 1000
        x = 64'd1000;
        k = 6'd10;
        $rem_mod_pow_of_2(x, k, result);
        $display("Test 2: %0d mod 2^%0d = %0d", x, k, result);
        
        // Test Case 3: 12345 mod 2^8 = 12345 mod 256 = 57
        x = 64'd12345;
        k = 6'd8;
        $rem_mod_pow_of_2(x, k, result);
        $display("Test 3: %0d mod 2^%0d = %0d", x, k, result);
        
        // Test Case 4: Security boundary test (2^16 - 1)
        x = 64'd65535;
        k = 6'd16;
        $rem_mod_pow_of_2(x, k, result);
        $display("Test 4: %0d mod 2^%0d = %0d", x, k, result);
        
        $display("=== All tests completed ===");
        $finish;
    end
endmodule
