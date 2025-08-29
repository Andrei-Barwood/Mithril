module crypto_testbench;
    reg [1023:0] operand_a_str, operand_b_str, result_str;
    
    initial begin
        // Initialize string variables
        operand_a_str = "123456789012345678901234567890";
        operand_b_str = "987654321098765432109876543210";
        
        $display("Starting cryptographic addition test");
        $display("Operand A: %s", operand_a_str);
        $display("Operand B: %s", operand_b_str);
        
        // Call your PLI big integer addition
        $big_add(operand_a_str, operand_b_str, result_str);
        
        $display("Result: %s", result_str);
        $display("Cryptographic operation completed");
        
        #10 $finish;
    end
endmodule
