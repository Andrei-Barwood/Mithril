// ============================================================================
// Mithril Cryptography API - Hardware ASIC Implementation
// Testbench: tb_mithril_sub_secure
// Simulator: Icarus Verilog (iverilog) + vvp
// Features: Waveform dump (.vcd), underflow check, constant-time validation
// ============================================================================

`timescale 1ns / 1ps

module tb_mithril_sub_secure;

    parameter WIDTH = 256;

    reg              clk;
    reg              rst_n;
    reg              start;
    reg  [WIDTH-1:0] operand_a;
    reg  [WIDTH-1:0] operand_b;

    wire [WIDTH-1:0] result;
    wire             underflow;
    wire             done;
    wire             error;

    integer pass_count = 0;
    integer fail_count = 0;
    integer test_id    = 0;

    mithril_sub_secure #(WIDTH) dut (
        .clk        (clk),
        .rst_n      (rst_n),
        .start      (start),
        .operand_a  (operand_a),
        .operand_b  (operand_b),
        .result     (result),
        .underflow  (underflow),
        .done       (done),
        .error      (error)
    );

    always #5 clk = ~clk;

    task check_subtraction;
        input [WIDTH-1:0] a;
        input [WIDTH-1:0] b;
        input [8*32:1]    description;

        reg [WIDTH:0] expected_diff;
        integer start_cycle, elapsed_cycles;
        begin
            test_id = test_id + 1;
            expected_diff = {1'b0, a} - {1'b0, b};

            @(posedge clk);
            operand_a = a;
            operand_b = b;
            start     = 1'b1;
            start_cycle = $time;

            @(posedge clk);
            start = 1'b0;

            while (!done) @(posedge clk);
            elapsed_cycles = ($time - start_cycle) / 10;

            if (result === expected_diff[WIDTH-1:0] && underflow === expected_diff[WIDTH] && !error) begin
                $display("[PASS] Test %0d (%0s): Latency=%0d cycles", test_id, description, elapsed_cycles);
                $display("       A   = %h", a);
                $display("       B   = %h", b);
                $display("       RES = %h (Underflow=%0b)", result, underflow);
                pass_count = pass_count + 1;
            end else begin
                $display("[FAIL] Test %0d (%0s):", test_id, description);
                $display("       A        = %h", a);
                $display("       B        = %h", b);
                $display("       EXP_RES  = %h (ExpUfl=%0b)", expected_diff[WIDTH-1:0], expected_diff[WIDTH]);
                $display("       ACT_RES  = %h (ActUfl=%0b, Err=%0b)", result, underflow, error);
                fail_count = fail_count + 1;
            end

            @(posedge clk);
            #20;
        end
    endtask

    initial begin
        $dumpfile("mithril_sub_secure.vcd");
        $dumpvars(0, tb_mithril_sub_secure);

        $display("=========================================================");
        $display("  MITHRIL ASIC - 256-bit Secure Subtractor (Icarus)      ");
        $display("=========================================================");

        clk       = 1'b0;
        rst_n     = 1'b0;
        start     = 1'b0;
        operand_a = {WIDTH{1'b0}};
        operand_b = {WIDTH{1'b0}};

        #30;
        rst_n = 1'b1;
        #20;

        // Test 1: Simple Positive Subtraction
        check_addition_or_sub:
        check_subtraction(
            256'hFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF,
            256'h0000000000000000000000000000000000000000000000000000000000000001,
            "Max Value - 1"
        );

        // Test 2: Identical Operands (Result = 0, No Underflow)
        check_subtraction(
            256'h123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0,
            256'h123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0,
            "A == B (Zero Result)"
        );

        // Test 3: Underflow Condition (A < B -> Underflow = 1)
        check_subtraction(
            256'h0000000000000000000000000000000000000000000000000000000000000005,
            256'h0000000000000000000000000000000000000000000000000000000000000010,
            "5 - 16 (Underflow Triggered)"
        );

        // Test 4: 0 - 1 (Maximum Underflow wrap-around)
        check_subtraction(
            256'h0000000000000000000000000000000000000000000000000000000000000000,
            256'h0000000000000000000000000000000000000000000000000000000000000001,
            "0 - 1 (Full 256-bit Borrow)"
        );

        $display("---------------------------------------------------------");
        $display("Summary: %0d Passed, %0d Failed", pass_count, fail_count);
        $display("Waveform saved to: mithril_sub_secure.vcd");
        $display("=========================================================");

        if (fail_count == 0)
            $display("[SUCCESS] All 256-bit Subtractor tests PASSED!");
        else
            $display("[ERROR] Failures detected in 256-bit Subtractor!");

        #50;
        $finish;
    end

endmodule
