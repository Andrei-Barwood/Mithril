//
// test_secure_decrement.v
// Test module for secure decrement PLI function
//

module test_secure_decrement;
    reg [7:0] counter;
    integer error_code;
    
    initial begin
        $display("=== Secure Decrement Test for IoT Devices ===");
        
        // Test normal decrement
        counter = 8'h05;
        $display("Initial value: %d", counter);
        error_code = $secure_decrement(counter);
        $display("After decrement: %d, Error code: %d", counter, error_code);
        
        // Test underflow protection
        counter = 8'h00;
        $display("Testing underflow protection...");
        $display("Initial value: %d", counter);
        error_code = $secure_decrement(counter);
        $display("After decrement: %d, Error code: %d", counter, error_code);
        
        // Test multiple decrements
        counter = 8'h03;
        $display("Testing multiple decrements...");
        repeat(5) begin
            $display("Value: %d", counter);
            error_code = $secure_decrement(counter);
            if (error_code != 0) 
                $display("Underflow detected!");
        end
        
        $finish;
    end
endmodule
