// v1.1-10.v
// Test module demonstrating secure Karatsuba multiplication for IoT

module karats_iot_test;
    
    reg [31:0] operand_a, operand_b;
    wire [31:0] result;
    integer i;
    
    // IoT device simulation parameters
    parameter NUM_TESTS = 10;
    parameter MAX_VALUE = 32'hFFFF; // 16-bit operands for demo
    
    initial begin
        $display("=== IoT Secure Karatsuba Multiplier Test ===");
        $display("Testing multiplication security for IoT devices");
        
        // Test cases for IoT scenarios
        for (i = 0; i < NUM_TESTS; i = i + 1) begin
            // Generate pseudo-random operands (simulate sensor data)
            operand_a = ($random & MAX_VALUE) | 1; // Ensure non-zero
            operand_b = ($random & MAX_VALUE) | 1;
            
            #10; // Simulation delay
            
            // Call secure Karatsuba multiplication
            $display("Test %0d: Multiplying %0d × %0d", i+1, operand_a, operand_b);
            
            // The PLI function will be called and print secure operation log
            // This demonstrates how IoT devices can perform secured math operations
            $karats_mul(operand_a, operand_b);
            
            #10;
        end
        
        // Specific test cases for validation
        $display("\n=== Validation Tests ===");
        
        operand_a = 32'd12345;
        operand_b = 32'd67890;
        #10;
        $display("Standard test: %0d × %0d", operand_a, operand_b);
        $karats_mul(operand_a, operand_b);
        
        #10;
        $display("\n=== IoT Security Test Complete ===");
        $finish;
    end
    
endmodule
