// secure_math_test.v
// IoT Device Secure Mathematics Test

module secure_math_test;
    reg [31:0] operand_a, operand_b;
    reg [31:0] result;
    reg underflow_flag;
    
    initial begin
        $display("=== IoT Secure Subtraction Test ===");
        
        // Test case 1: Normal subtraction
        operand_a = 100;
        operand_b = 30;
        $secure_subtract(operand_a, operand_b, result, underflow_flag);
        $display("Test 1: %d - %d = %d, Underflow: %b", 
                 operand_a, operand_b, result, underflow_flag);
        
        // Test case 2: Underflow condition
        operand_a = 20;
        operand_b = 50;
        $secure_subtract(operand_a, operand_b, result, underflow_flag);
        $display("Test 2: %d - %d = %d, Underflow: %b", 
                 operand_a, operand_b, result, underflow_flag);
        
        // Test case 3: Equal values
        operand_a = 42;
        operand_b = 42;
        $secure_subtract(operand_a, operand_b, result, underflow_flag);
        $display("Test 3: %d - %d = %d, Underflow: %b", 
                 operand_a, operand_b, result, underflow_flag);
        
        #100 $finish;
    end
endmodule
