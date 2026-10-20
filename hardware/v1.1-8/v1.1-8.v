// test_mixed_mult.v
// Test module for mixed multiplication PLI

module test_mixed_mult;
    reg [255:0] a;           // Large integer
    reg [31:0] b;            // Standard 32-bit multiplier
    reg [255:0] modulus;     // Optional modulus
    reg [255:0] result;      // Result storage
    integer status;          // Return status
    
    initial begin
        // Test 1: Basic multiplication without modulus
        a = 256'h123456789ABCDEF0123456789ABCDEF0;
        b = 32'h1000;
        
        status = $mixed_mult(result, a, b);
        $display("Test 1 - Basic multiplication:");
        $display("  a = %h", a);
        $display("  b = %h", b);
        $display("  result = %h", result);
        $display("  status = %d", status);
        
        // Test 2: Multiplication with modular reduction
        a = 256'hFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF0;
        b = 32'h1000;
        modulus = 256'h100000000000000000000000000000000;
        
        status = $mixed_mult(result, a, b, modulus);
        $display("\nTest 2 - Modular multiplication:");
        $display("  a = %h", a);
        $display("  b = %h", b);
        $display("  modulus = %h", modulus);
        $display("  result = %h", result);
        $display("  status = %d (1=overflow/reduction applied)", status);
        
        // Test 3: Zero multiplication
        a = 0;
        b = 32'h12345678;
        
        status = $mixed_mult(result, a, b);
        $display("\nTest 3 - Zero multiplication:");
        $display("  result = %h", result);
        $display("  status = %d", status);
        
        $finish;
    end
    
endmodule
