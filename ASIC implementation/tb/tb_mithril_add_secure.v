// ============================================================================
// Mithril Cryptography API - Hardware ASIC Implementation
// Testbench: tb_mithril_add_secure
// Simulator: Icarus Verilog (iverilog) + vvp
// Features: Waveform dump (.vcd), self-checking assertions, timing checks
// ============================================================================

`timescale 1ns / 1ps

module tb_mithril_add_secure;

    parameter WIDTH = 256;

    reg              clk;
    reg              rst_n;
    reg              start;
    reg  [WIDTH-1:0] operand_a;
    reg  [WIDTH-1:0] operand_b;

    wire [WIDTH-1:0] result;
    wire             carry_out;
    wire             done;
    wire             error;

    // Simulation metrics
    integer pass_count = 0;
    integer fail_count = 0;
    integer test_id    = 0;

    // DUT Instantiation
    mithril_add_secure #(WIDTH) dut (
        .clk        (clk),
        .rst_n      (rst_n),
        .start      (start),
        .operand_a  (operand_a),
        .operand_b  (operand_b),
        .result     (result),
        .carry_out  (carry_out),
        .done       (done),
        .error      (error)
    );

    // 100 MHz Clock Generation (10 ns period)
    always #5 clk = ~clk;

    // Task for one single addition verification
    task check_addition;
        input [WIDTH-1:0] a;
        input [WIDTH-1:0] b;
        input [8*32:1]    description;

        reg [WIDTH:0] expected_sum;
        integer start_cycle, elapsed_cycles;
        begin
            test_id = test_id + 1;
            expected_sum = {1'b0, a} + {1'b0, b};

            @(posedge clk);
            operand_a = a;
            operand_b = b;
            start     = 1'b1;
            start_cycle = $time;

            @(posedge clk);
            start = 1'b0;

            // Wait for completion
            while (!done) @(posedge clk);
            elapsed_cycles = ($time - start_cycle) / 10;

            if (result === expected_sum[WIDTH-1:0] && carry_out === expected_sum[WIDTH] && !error) begin
                $display("[PASS] Test %0d (%0s): Latency=%0d cycles", test_id, description, elapsed_cycles);
                $display("       A   = %h", a);
                $display("       B   = %h", b);
                $display("       RES = %h (Carry=%0b)", result, carry_out);
                pass_count = pass_count + 1;
            end else begin
                $display("[FAIL] Test %0d (%0s):", test_id, description);
                $display("       A        = %h", a);
                $display("       B        = %h", b);
                $display("       EXP_RES  = %h (ExpCarry=%0b)", expected_sum[WIDTH-1:0], expected_sum[WIDTH]);
                $display("       ACT_RES  = %h (ActCarry=%0b, Err=%0b)", result, carry_out, error);
                fail_count = fail_count + 1;
            end

            // Wait for return to IDLE
            @(posedge clk);
            #20;
        end
    endtask

    initial begin
        // Configure VCD Waveform Dump for Icarus / GTKWave
        $dumpfile("mithril_add_secure.vcd");
        $dumpvars(0, tb_mithril_add_secure);

        $display("=========================================================");
        $display("   MITHRIL ASIC - 256-bit Secure Adder (Icarus Verilog)  ");
        $display("=========================================================");

        // Initialize signals
        clk       = 1'b0;
        rst_n     = 1'b0;
        start     = 1'b0;
        operand_a = {WIDTH{1'b0}};
        operand_b = {WIDTH{1'b0}};

        // Reset DUT
        #30;
        rst_n = 1'b1;
        #20;

        // Test 1: Simple Addition
        check_addition(
            256'h123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0,
            256'h0FEDCBA9876543210FEDCBA9876543210FEDCBA9876543210FEDCBA987654321,
            "Simple Addition"
        );

        // Test 2: Overflow / Max Value Carry Check
        check_addition(
            256'hFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF,
            256'h0000000000000000000000000000000000000000000000000000000000000001,
            "Max Value + 1 Overflow"
        );

        // Test 3: Zero Operands
        check_addition(
            256'h0000000000000000000000000000000000000000000000000000000000000000,
            256'h0000000000000000000000000000000000000000000000000000000000000000,
            "Zero + Zero"
        );

        // Test 4: Curve25519 Basepoint / Scalar Addition
        check_addition(
            256'h7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffed,
            256'h0000000000000000000000000000000000000000000000000000000000000013,
            "Curve25519 Prime + 19"
        );

        // Test 5: Alternating Bit Pattern
        check_addition(
            256'hAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA,
            256'h5555555555555555555555555555555555555555555555555555555555555555,
            "Alternating Bit Patterns"
        );

        $display("---------------------------------------------------------");
        $display("Summary: %0d Passed, %0d Failed", pass_count, fail_count);
        $display("Waveform saved to: mithril_add_secure.vcd");
        $display("=========================================================");

        if (fail_count == 0)
            $display("[SUCCESS] All 256-bit Secure Adder tests PASSED!");
        else
            $display("[ERROR] Failures detected in 256-bit Secure Adder!");

        #50;
        $finish;
    end

endmodule
