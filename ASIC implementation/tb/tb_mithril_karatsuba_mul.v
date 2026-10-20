// ============================================================================
// Mithril Cryptography API - Hardware ASIC Implementation
// Testbench: tb_mithril_karatsuba_mul
// Simulator: Icarus Verilog (iverilog) + vvp
// Features: Karatsuba 32x32 -> 64-bit hardware multiplication check
// ============================================================================

`timescale 1ns / 1ps

module tb_mithril_karatsuba_mul;

    reg         clk;
    reg         rst_n;
    reg         start;
    reg  [31:0] operand_a;
    reg  [31:0] operand_b;

    wire [63:0] result;
    wire        done;
    wire        error;

    integer pass_count = 0;
    integer fail_count = 0;
    integer test_id    = 0;

    mithril_karatsuba_mul dut (
        .clk        (clk),
        .rst_n      (rst_n),
        .start      (start),
        .operand_a  (operand_a),
        .operand_b  (operand_b),
        .result     (result),
        .done       (done),
        .error      (error)
    );

    always #5 clk = ~clk;

    task check_mul;
        input [31:0] a;
        input [31:0] b;
        input [8*32:1] description;

        reg [63:0] expected_product;
        begin
            test_id = test_id + 1;
            expected_product = 64'd0 + (64'd0 + a) * (64'd0 + b);

            @(posedge clk);
            operand_a = a;
            operand_b = b;
            start     = 1'b1;

            @(posedge clk);
            start = 1'b0;

            while (!done) @(posedge clk);

            if (result === expected_product && !error) begin
                $display("[PASS] Test %0d (%0s): %0d * %0d = %0d (0x%h)",
                         test_id, description, a, b, result, result);
                pass_count = pass_count + 1;
            end else begin
                $display("[FAIL] Test %0d (%0s): %0d * %0d", test_id, description, a, b);
                $display("       EXP = %0d (0x%h)", expected_product, expected_product);
                $display("       ACT = %0d (0x%h, Err=%0b)", result, result, error);
                fail_count = fail_count + 1;
            end

            @(posedge clk);
            #20;
        end
    endtask

    initial begin
        $dumpfile("mithril_karatsuba_mul.vcd");
        $dumpvars(0, tb_mithril_karatsuba_mul);

        $display("=========================================================");
        $display("  MITHRIL ASIC - Karatsuba Multiplier Test (Icarus)      ");
        $display("=========================================================");

        clk       = 1'b0;
        rst_n     = 1'b0;
        start     = 1'b0;
        operand_a = 32'd0;
        operand_b = 32'd0;

        #30;
        rst_n = 1'b1;
        #20;

        // Test 1: Small numbers
        check_mul(32'd1234, 32'd5678, "Small Numbers");

        // Test 2: Powers of 2
        check_mul(32'h00010000, 32'h00020000, "Powers of Two");

        // Test 3: Large 16-bit / 32-bit mix
        check_mul(32'hFFFF, 32'hFFFF, "16-bit Max Squares");

        // Test 4: Maximum 32-bit values
        check_mul(32'hFFFFFFFF, 32'hFFFFFFFF, "32-bit Max Values");

        // Test 5: Zero operand
        check_mul(32'd0, 32'h12345678, "Zero Multiplication");

        $display("---------------------------------------------------------");
        $display("Summary: %0d Passed, %0d Failed", pass_count, fail_count);
        $display("Waveform saved to: mithril_karatsuba_mul.vcd");
        $display("=========================================================");

        if (fail_count == 0)
            $display("[SUCCESS] All Karatsuba Multiplier tests PASSED!");
        else
            $display("[ERROR] Failures detected in Karatsuba Multiplier!");

        #50;
        $finish;
    end

endmodule
