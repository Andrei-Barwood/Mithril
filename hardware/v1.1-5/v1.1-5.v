module secure_increment_tb;
    reg [1023:0] value;      // Large register for big integers
    reg [1023:0] result;     // Result register
    reg [1023:0] modulus;    // Modulus for secure operations
    
    initial begin
        // Initialize test values
        value = "12345678901234567890";
        modulus = "1000000000000000000000";
        
        $display("=== Secure Increment Operations ===");
        
        // Test basic increment
        $display("Original value: %s", value);
        $fmpz_inc(result, value);
        $display("After increment: %s", result);
        
        // Test in-place increment
        $display("\nTesting in-place increment:");
        $display("Before: %s", value);
        $fmpz_inc_inplace(value);
        $display("After: %s", value);
        
        // Test modular increment for IoT security
        $display("\nTesting modular increment:");
        value = "999999999999999999999";  // Near modulus
        $display("Value: %s", value);
        $display("Modulus: %s", modulus);
        $fmpz_inc_mod(result, value, modulus);
        $display("Result (value + 1) mod modulus: %s", result);
        
        $finish;
    end
endmodule
