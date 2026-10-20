//
// tb_mithril_add.v
// Testbench for Mithril Secure Addition
// Verifies hardware against libsodium reference
//

`timescale 1ns / 1ps

module tb_mithril_add;

    // Clock and reset
    reg clk;
    reg rst_n;
    
    // Signals for mithril_add_secure
    reg start;
    reg [255:0] operand_a;
    reg [255:0] operand_b;
    wire [255:0] result_hw;
    wire carry_out_hw;
    wire done_hw;
    wire error_hw;
    
    // Signals for libsodium reference
    reg [255:0] result_ref;
    
    //=======================================================================
    // DUT Instantiation
    //=======================================================================
    mithril_add_secure #(
        .WIDTH(256)
    ) dut (
        .clk        (clk),
        .rst_n      (rst_n),
        .start      (start),
        .operand_a  (operand_a),
        .operand_b  (operand_b),
        .result     (result_hw),
        .carry_out  (carry_out_hw),
        .done       (done_hw),
        .error      (error_hw)
    );
    
    //=======================================================================
    // Clock Generation
    //=======================================================================
    initial begin
        clk = 0;
        forever #5 clk = ~clk;  // 100MHz clock
    end
    
    //=======================================================================
    // Test Stimulus
    //=======================================================================
    initial begin
        // Initialize
        rst_n = 0;
        start = 0;
        operand_a = 256'h0;
        operand_b = 256'h0;
        
        // Reset pulse
        #20;
        rst_n = 1;
        #20;
        
        $display("=== Mithril Secure Addition Test ===");
        
        // Test 1: Simple addition
        $display("\n[Test 1] Simple Addition");
        operand_a = 256'h123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0;
        operand_b = 256'h0FEDCBA9876543210FEDCBA9876543210FEDCBA9876543210FEDCBA987654321;
        
        start = 1;
        #10;
        start = 0;
        
        wait(done_hw);
        
        // Call libsodium reference via PLI
        $mithril_add_scalar(operand_a, operand_b, result_ref);
        
        // Compare results
        if (result_hw === result_ref) begin
            $display("PASS: Hardware matches libsodium reference");
        end else begin
            $display("FAIL: Mismatch!");
            $display("  HW:  %h", result_hw);
            $display("  REF: %h", result_ref);
        end
        
        #100;
        
        // Test 2: Maximum values (overflow test)
        $display("\n[Test 2] Overflow Test");
        operand_a = 256'hFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF;
        operand_b = 256'h0000000000000000000000000000000000000000000000000000000000000001;
        
        start = 1;
        #10;
        start = 0;
        
        wait(done_hw);
        
        if (carry_out_hw) begin
            $display("PASS: Overflow detected correctly");
        end else begin
            $display("FAIL: Overflow not detected");
        end
        
        #100;
        
        // Test 3: Zero operands
        $display("\n[Test 3] Zero Operands");
        operand_a = 256'h0;
        operand_b = 256'h0;
        
        start = 1;
        #10;
        start = 0;
        
        wait(done_hw);
        
        if (result_hw == 256'h0) begin
            $display("PASS: Zero addition correct");
        end else begin
            $display("FAIL: Zero addition incorrect");
        end
        
        #100;
        
        // Test 4: Timing consistency check
        $display("\n[Test 4] Constant-Time Verification");
        test_timing_consistency();
        
        #100;
        $display("\n=== All Tests Complete ===");
        $finish;
    end
    
    //=======================================================================
    // Timing Consistency Test
    //=======================================================================
    task test_timing_consistency;
        integer cycles_test1, cycles_test2;
        integer start_time, end_time;
        
        begin
            // Test with different data patterns
            operand_a = 256'h1111111111111111111111111111111111111111111111111111111111111111;
            operand_b = 256'h2222222222222222222222222222222222222222222222222222222222222222;
            
            start_time = $time;
            start = 1;
            #10;
            start = 0;
            wait(done_hw);
            end_time = $time;
            cycles_test1 = (end_time - start_time) / 10;
            
            #50;
            
            operand_a = 256'hAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA;
            operand_b = 256'h5555555555555555555555555555555555555555555555555555555555555555;
            
            start_time = $time;
            start = 1;
            #10;
            start = 0;
            wait(done_hw);
            end_time = $time;
            cycles_test2 = (end_time - start_time) / 10;
            
            if (cycles_test1 == cycles_test2) begin
                $display("PASS: Constant timing verified (%0d cycles)", cycles_test1);
            end else begin
                $display("FAIL: Timing varies! Test1=%0d, Test2=%0d", cycles_test1, cycles_test2);
            end
        end
    endtask

endmodule
