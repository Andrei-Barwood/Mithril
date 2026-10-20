//
// test_secure_math.v
// Test module for secure mathematical operations
//

module test_secure_math;
    
    // String registers for arbitrary precision numbers
    reg [8*32-1:0] input_value;
    reg [8*64-1:0] result_value;
    
    initial begin
        $display("=== Secure Mathematical Operations Test ===");
        
        // Test Case 1: Small number squaring
        input_value = "5";
        #10;
        $secure_square(result_value, input_value);
        #10;
        $display("Test 1 - Input: %s, Result: %s", input_value, result_value);
        
        // Test Case 2: Larger number squaring  
        input_value = "123";
        #10;
        $secure_square(result_value, input_value);
        #10;
        $display("Test 2 - Input: %s, Result: %s", input_value, result_value);
        
        // Test Case 3: Zero case
        input_value = "0";
        #10;
        $secure_square(result_value, input_value);
        #10;
        $display("Test 3 - Input: %s, Result: %s", input_value, result_value);
        
        // Test Case 4: IoT sensor data simulation
        input_value = "256";  // 8-bit sensor max value
        #10;
        $secure_square(result_value, input_value);
        #10;
        $display("Test 4 (IoT) - Input: %s, Result: %s", input_value, result_value);
        
        #50;
        $display("=== All tests completed ===");
        $finish;
    end
    
endmodule
