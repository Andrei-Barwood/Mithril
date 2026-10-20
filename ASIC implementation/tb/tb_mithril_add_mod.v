// ============================================================================
// Mithril Cryptography API - Hardware ASIC Implementation
// Testbench: tb_mithril_add_mod
// Simulator: Icarus Verilog (iverilog) + vvp
// Features: Curve25519 Modular Addition validation, GTKWave trace
// ============================================================================

`timescale 1ns / 1ps

module tb_mithril_add_mod;

    parameter WIDTH = 256;

    reg              clk;
    reg              rst_n;
    reg              start;
    reg  [WIDTH-1:0] operand_a;
    reg  [WIDTH-1:0] operand_b;
    reg  [WIDTH-1:0] modulus;

    wire [WIDTH-1:0] result;
    wire             done;
    wire             error;

    localparam [WIDTH-1:0] CURVE25519_P = 256'h7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffed;

    integer pass_count = 0;
    integer fail_count = 0;
    integer test_id    = 0;

    mithril_add_mod #(WIDTH) dut (
        .clk        (clk),
        .rst_n      (rst_n),
        .start      (start),
        .operand_a  (operand_a),
        .operand_b  (operand_b),
        .modulus    (modulus),
        .result     (result),
        .done       (done),
        .error      (error)
    );

    always #5 clk = ~clk;

    task check_mod_add;
        input [WIDTH-1:0] a;
        input [WIDTH-1:0] b;
        input [WIDTH-1:0] mod_val;
        input [WIDTH-1:0] expected_res;
        input [8*32:1]    description;

        integer start_cycle, elapsed_cycles;
        begin
            test_id = test_id + 1;

            @(posedge clk);
            operand_a = a;
            operand_b = b;
            modulus   = mod_val;
            start     = 1'b1;
            start_cycle = $time;

            @(posedge clk);
            start = 1'b0;

            while (!done) @(posedge clk);
            elapsed_cycles = ($time - start_cycle) / 10;

            if (result === expected_res && !error) begin
                $display("[PASS] Test %0d (%0s): Latency=%0d cycles", test_id, description, elapsed_cycles);
                $display("       A   = %h", a);
                $display("       B   = %h", b);
                $display("       RES = %h", result);
                pass_count = pass_count + 1;
            end else begin
                $display("[FAIL] Test %0d (%0s):", test_id, description);
                $display("       A       = %h", a);
                $display("       B       = %h", b);
                $display("       EXP_RES = %h", expected_res);
                $display("       ACT_RES = %h (Err=%0b)", result, error);
                fail_count = fail_count + 1;
            end

            @(posedge clk);
            #20;
        end
    endtask

    initial begin
        $dumpfile("mithril_add_mod.vcd");
        $dumpvars(0, tb_mithril_add_mod);

        $display("=========================================================");
        $display("  MITHRIL ASIC - Curve25519 Modular Adder (Icarus)       ");
        $display("=========================================================");

        clk       = 1'b0;
        rst_n     = 1'b0;
        start     = 1'b0;
        operand_a = {WIDTH{1'b0}};
        operand_b = {WIDTH{1'b0}};
        modulus   = {WIDTH{1'b0}}; // Default prime

        #30;
        rst_n = 1'b1;
        #20;

        // Test 1: Small numbers (Sum < p -> no reduction)
        check_mod_add(
            256'd100,
            256'd250,
            256'd0,
            256'd350,
            "Small Numbers (< p)"
        );

        // Test 2: Exact boundary reduction ((p - 1) + 1 = 0 mod p)
        check_mod_add(
            CURVE25519_P - 256'd1,
            256'd1,
            256'd0,
            256'd0,
            "(p - 1) + 1 = 0 mod p"
        );

        // Test 3: Exceeding prime ((p - 5) + 10 = 5 mod p)
        check_mod_add(
            CURVE25519_P - 256'd5,
            256'd10,
            256'd0,
            256'd5,
            "(p - 5) + 10 = 5 mod p"
        );

        // Test 4: Maximum sum reduction ((p - 1) + (p - 1) = p - 2 mod p)
        check_mod_add(
            CURVE25519_P - 256'd1,
            CURVE25519_P - 256'd1,
            256'd0,
            CURVE25519_P - 256'd2,
            "(p - 1) + (p - 1) = p - 2 mod p"
        );

        $display("---------------------------------------------------------");
        $display("Summary: %0d Passed, %0d Failed", pass_count, fail_count);
        $display("Waveform saved to: mithril_add_mod.vcd");
        $display("=========================================================");

        if (fail_count == 0)
            $display("[SUCCESS] All Curve25519 Modular Adder tests PASSED!");
        else
            $display("[ERROR] Failures detected in Modular Adder!");

        #50;
        $finish;
    end

endmodule
