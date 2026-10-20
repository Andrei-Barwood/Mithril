// ============================================================================
// Mithril Cryptography API - Hardware ASIC Implementation
// Testbench: tb_mithril_div_rem
// Simulator: Icarus Verilog (iverilog) + vvp
// Features: 64-bit Division with Remainder and zero-divisor detection
// ============================================================================

`timescale 1ns / 1ps

module tb_mithril_div_rem;

    reg        clk;
    reg        rst_n;
    reg        start;
    reg [63:0] dividend;
    reg [63:0] divisor;

    wire [63:0] quotient;
    wire [63:0] remainder;
    wire        done;
    wire        div_by_zero;
    wire        error;

    integer pass_count = 0;
    integer fail_count = 0;
    integer test_id    = 0;

    mithril_div_rem dut (
        .clk        (clk),
        .rst_n      (rst_n),
        .start      (start),
        .dividend   (dividend),
        .divisor    (divisor),
        .quotient   (quotient),
        .remainder  (remainder),
        .done       (done),
        .div_by_zero(div_by_zero),
        .error      (error)
    );

    always #5 clk = ~clk;

    task check_div;
        input [63:0] n;
        input [63:0] d;
        input [63:0] exp_q;
        input [63:0] exp_r;
        input        exp_zero;
        input [8*32:1] description;
        begin
            test_id = test_id + 1;

            @(posedge clk);
            dividend = n;
            divisor  = d;
            start    = 1'b1;

            @(posedge clk);
            start    = 1'b0;

            while (!done) @(posedge clk);

            if (exp_zero) begin
                if (div_by_zero && error) begin
                    $display("[PASS] Test %0d (%0s): Divide-by-zero properly trapped!", test_id, description);
                    pass_count = pass_count + 1;
                end else begin
                    $display("[FAIL] Test %0d (%0s): Divide-by-zero NOT flagged! (zero=%0b, err=%0b)",
                             test_id, description, div_by_zero, error);
                    fail_count = fail_count + 1;
                end
            end else begin
                if (quotient === exp_q && remainder === exp_r && !error && !div_by_zero) begin
                    $display("[PASS] Test %0d (%0s): %0d / %0d = %0d (rem %0d)",
                             test_id, description, n, d, quotient, remainder);
                    pass_count = pass_count + 1;
                end else begin
                    $display("[FAIL] Test %0d (%0s): %0d / %0d", test_id, description, n, d);
                    $display("       EXP_Q = %0d, EXP_R = %0d", exp_q, exp_r);
                    $display("       ACT_Q = %0d, ACT_R = %0d (Err=%0b)", quotient, remainder, error);
                    fail_count = fail_count + 1;
                end
            end

            @(posedge clk);
            #20;
        end
    endtask

    initial begin
        $dumpfile("mithril_div_rem.vcd");
        $dumpvars(0, tb_mithril_div_rem);

        $display("=========================================================");
        $display("  MITHRIL ASIC - 64-bit Division & Remainder (Icarus)    ");
        $display("=========================================================");

        clk      = 1'b0;
        rst_n    = 1'b0;
        start    = 1'b0;
        dividend = 64'd0;
        divisor  = 64'd0;

        #30;
        rst_n = 1'b1;
        #20;

        // Test 1: Standard Division with Remainder (100 / 7 = 14 rem 2)
        check_div(64'd100, 64'd7, 64'd14, 64'd2, 1'b0, "100 / 7");

        // Test 2: Exact Division (64 / 8 = 8 rem 0)
        check_div(64'd64, 64'd8, 64'd8, 64'd0, 1'b0, "64 / 8 (Exact)");

        // Test 3: Divide by Zero Protection (Trap assertion)
        check_div(64'd50, 64'd0, 64'd0, 64'd0, 1'b1, "Divide by Zero");

        // Test 4: Zero Dividend (0 / 10 = 0 rem 0)
        check_div(64'd0, 64'd10, 64'd0, 64'd0, 1'b0, "0 / 10");

        // Test 5: Cryptographic Scale Numbers (1024 / 17 = 60 rem 4)
        check_div(64'd1024, 64'd17, 64'd60, 64'd4, 1'b0, "1024 / 17");

        $display("---------------------------------------------------------");
        $display("Summary: %0d Passed, %0d Failed", pass_count, fail_count);
        $display("Waveform saved to: mithril_div_rem.vcd");
        $display("=========================================================");

        if (fail_count == 0)
            $display("[SUCCESS] All Division/Remainder tests PASSED!");
        else
            $display("[ERROR] Failures detected in Division/Remainder!");

        #50;
        $finish;
    end

endmodule
