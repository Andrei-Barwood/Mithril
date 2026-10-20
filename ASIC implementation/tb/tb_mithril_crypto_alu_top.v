// ============================================================================
// Mithril Cryptography API - Hardware ASIC Implementation
// Testbench: tb_mithril_crypto_alu_top
// Simulator: Icarus Verilog (iverilog) + vvp
// Features: Full cryptographic ALU co-processor testbench with opcode routing
// ============================================================================

`timescale 1ns / 1ps

module tb_mithril_crypto_alu_top;

    parameter WIDTH = 256;

    reg              clk;
    reg              rst_n;
    reg              start;
    reg  [2:0]       opcode;
    reg  [WIDTH-1:0] operand_a;
    reg  [WIDTH-1:0] operand_b;
    reg  [WIDTH-1:0] modulus;

    wire [WIDTH-1:0] result_256;
    wire [63:0]      result_64;
    wire [63:0]      remainder_64;
    wire             flag_carry_underflow;
    wire             flag_div_by_zero;
    wire             done;
    wire             error;

    localparam [2:0] OP_ADD_SECURE = 3'b000;
    localparam [2:0] OP_SUB_SECURE = 3'b001;
    localparam [2:0] OP_ADD_MOD    = 3'b010;
    localparam [2:0] OP_KARATSUBA  = 3'b011;
    localparam [2:0] OP_DIV_REM    = 3'b100;

    localparam [WIDTH-1:0] CURVE25519_P = 256'h7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffed;

    integer pass_count = 0;
    integer fail_count = 0;

    mithril_crypto_alu_top #(WIDTH) dut (
        .clk                 (clk),
        .rst_n               (rst_n),
        .start               (start),
        .opcode              (opcode),
        .operand_a           (operand_a),
        .operand_b           (operand_b),
        .modulus             (modulus),
        .result_256          (result_256),
        .result_64           (result_64),
        .remainder_64        (remainder_64),
        .flag_carry_underflow(flag_carry_underflow),
        .flag_div_by_zero    (flag_div_by_zero),
        .done                (done),
        .error               (error)
    );

    always #5 clk = ~clk;

    initial begin
        $dumpfile("mithril_crypto_alu_top.vcd");
        $dumpvars(0, tb_mithril_crypto_alu_top);

        $display("=========================================================");
        $display("  MITHRIL ASIC - Cryptographic ALU Top Test (Icarus)     ");
        $display("=========================================================");

        clk       = 1'b0;
        rst_n     = 1'b0;
        start     = 1'b0;
        opcode    = 3'b000;
        operand_a = {WIDTH{1'b0}};
        operand_b = {WIDTH{1'b0}};
        modulus   = {WIDTH{1'b0}};

        #30;
        rst_n = 1'b1;
        #20;

        // --------------------------------------------------------------------
        // Test 1: OP_ADD_SECURE (256-bit addition)
        // --------------------------------------------------------------------
        @(posedge clk);
        opcode    = OP_ADD_SECURE;
        operand_a = 256'h1000;
        operand_b = 256'h2500;
        start     = 1'b1;
        @(posedge clk);
        start     = 1'b0;
        while (!done) @(posedge clk);

        if (result_256 === 256'h3500 && !flag_carry_underflow && !error) begin
            $display("[PASS] OP_ADD_SECURE: 0x1000 + 0x2500 = 0x%h", result_256);
            pass_count = pass_count + 1;
        end else begin
            $display("[FAIL] OP_ADD_SECURE mismatch!");
            fail_count = fail_count + 1;
        end
        #20;

        // --------------------------------------------------------------------
        // Test 2: OP_SUB_SECURE (256-bit subtraction)
        // --------------------------------------------------------------------
        @(posedge clk);
        opcode    = OP_SUB_SECURE;
        operand_a = 256'h5000;
        operand_b = 256'h2000;
        start     = 1'b1;
        @(posedge clk);
        start     = 1'b0;
        while (!done) @(posedge clk);

        if (result_256 === 256'h3000 && !flag_carry_underflow && !error) begin
            $display("[PASS] OP_SUB_SECURE: 0x5000 - 0x2000 = 0x%h", result_256);
            pass_count = pass_count + 1;
        end else begin
            $display("[FAIL] OP_SUB_SECURE mismatch!");
            fail_count = fail_count + 1;
        end
        #20;

        // --------------------------------------------------------------------
        // Test 3: OP_ADD_MOD (Curve25519 modular addition)
        // --------------------------------------------------------------------
        @(posedge clk);
        opcode    = OP_ADD_MOD;
        operand_a = CURVE25519_P - 256'd5;
        operand_b = 256'd10;
        modulus   = 256'd0; // default prime
        start     = 1'b1;
        @(posedge clk);
        start     = 1'b0;
        while (!done) @(posedge clk);

        if (result_256 === 256'd5 && !error) begin
            $display("[PASS] OP_ADD_MOD: (P - 5) + 10 = %0d mod P", result_256);
            pass_count = pass_count + 1;
        end else begin
            $display("[FAIL] OP_ADD_MOD mismatch! Res=%h", result_256);
            fail_count = fail_count + 1;
        end
        #20;

        // --------------------------------------------------------------------
        // Test 4: OP_KARATSUBA (32-bit x 32-bit multiplication)
        // --------------------------------------------------------------------
        @(posedge clk);
        opcode    = OP_KARATSUBA;
        operand_a = 256'd12345;
        operand_b = 256'd67890;
        start     = 1'b1;
        @(posedge clk);
        start     = 1'b0;
        while (!done) @(posedge clk);

        if (result_64 === (64'd12345 * 64'd67890) && !error) begin
            $display("[PASS] OP_KARATSUBA: 12345 * 67890 = %0d", result_64);
            pass_count = pass_count + 1;
        end else begin
            $display("[FAIL] OP_KARATSUBA mismatch! Res=%0d", result_64);
            fail_count = fail_count + 1;
        end
        #20;

        // --------------------------------------------------------------------
        // Test 5: OP_DIV_REM (64-bit division and remainder)
        // --------------------------------------------------------------------
        @(posedge clk);
        opcode    = OP_DIV_REM;
        operand_a = 256'd200;
        operand_b = 256'd15;
        start     = 1'b1;
        @(posedge clk);
        start     = 1'b0;
        while (!done) @(posedge clk);

        if (result_64 === 64'd13 && remainder_64 === 64'd5 && !flag_div_by_zero && !error) begin
            $display("[PASS] OP_DIV_REM: 200 / 15 = %0d (rem %0d)", result_64, remainder_64);
            pass_count = pass_count + 1;
        end else begin
            $display("[FAIL] OP_DIV_REM mismatch! Q=%0d, R=%0d", result_64, remainder_64);
            fail_count = fail_count + 1;
        end
        #20;

        $display("---------------------------------------------------------");
        $display("ALU Summary: %0d Passed, %0d Failed", pass_count, fail_count);
        $display("Waveform saved to: mithril_crypto_alu_top.vcd");
        $display("=========================================================");

        if (fail_count == 0)
            $display("[SUCCESS] All Cryptographic ALU Top operations PASSED!");
        else
            $display("[ERROR] Failures detected in Cryptographic ALU Top!");

        #50;
        $finish;
    end

endmodule
