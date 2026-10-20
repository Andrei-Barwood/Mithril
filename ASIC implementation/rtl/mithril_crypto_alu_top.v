// ============================================================================
// Mithril Cryptography API - Hardware ASIC Implementation
// Module: mithril_crypto_alu_top
// Description: Top-level ASIC Cryptographic Arithmetic Logic Unit.
//              Integrates constant-time 256-bit addition, subtraction,
//              Curve25519 modular addition, Karatsuba multiplication,
//              and secured division with remainder.
// ============================================================================

`timescale 1ns / 1ps

module mithril_crypto_alu_top #(
    parameter WIDTH = 256
) (
    input  wire                 clk,
    input  wire                 rst_n,
    input  wire                 start,
    input  wire [2:0]           opcode,
    input  wire [WIDTH-1:0]     operand_a,
    input  wire [WIDTH-1:0]     operand_b,
    input  wire [WIDTH-1:0]     modulus,

    output reg  [WIDTH-1:0]     result_256,
    output reg  [63:0]          result_64,
    output reg  [63:0]          remainder_64,
    output reg                  flag_carry_underflow,
    output reg                  flag_div_by_zero,
    output reg                  done,
    output reg                  error
);

    // Opcodes
    localparam [2:0] OP_ADD_SECURE = 3'b000;
    localparam [2:0] OP_SUB_SECURE = 3'b001;
    localparam [2:0] OP_ADD_MOD    = 3'b010;
    localparam [2:0] OP_KARATSUBA  = 3'b011;
    localparam [2:0] OP_DIV_REM    = 3'b100;

    // Submodule controls
    wire start_add = (start && opcode == OP_ADD_SECURE);
    wire start_sub = (start && opcode == OP_SUB_SECURE);
    wire start_mod = (start && opcode == OP_ADD_MOD);
    wire start_mul = (start && opcode == OP_KARATSUBA);
    wire start_div = (start && opcode == OP_DIV_REM);

    // Submodule outputs
    wire [WIDTH-1:0] add_res;
    wire             add_carry, add_done, add_err;

    wire [WIDTH-1:0] sub_res;
    wire             sub_underflow, sub_done, sub_err;

    wire [WIDTH-1:0] mod_res;
    wire             mod_done, mod_err;

    wire [63:0]      mul_res;
    wire             mul_done, mul_err;

    wire [63:0]      div_quot, div_rem;
    wire             div_done, div_zero, div_err;

    // ------------------------------------------------------------------------
    // Submodule Instances
    // ------------------------------------------------------------------------
    mithril_add_secure #(WIDTH) u_add (
        .clk        (clk),
        .rst_n      (rst_n),
        .start      (start_add),
        .operand_a  (operand_a),
        .operand_b  (operand_b),
        .result     (add_res),
        .carry_out  (add_carry),
        .done       (add_done),
        .error      (add_err)
    );

    mithril_sub_secure #(WIDTH) u_sub (
        .clk        (clk),
        .rst_n      (rst_n),
        .start      (start_sub),
        .operand_a  (operand_a),
        .operand_b  (operand_b),
        .result     (sub_res),
        .underflow  (sub_underflow),
        .done       (sub_done),
        .error      (sub_err)
    );

    mithril_add_mod #(WIDTH) u_mod (
        .clk        (clk),
        .rst_n      (rst_n),
        .start      (start_mod),
        .operand_a  (operand_a),
        .operand_b  (operand_b),
        .modulus    (modulus),
        .result     (mod_res),
        .done       (mod_done),
        .error      (mod_err)
    );

    mithril_karatsuba_mul u_mul (
        .clk        (clk),
        .rst_n      (rst_n),
        .start      (start_mul),
        .operand_a  (operand_a[31:0]),
        .operand_b  (operand_b[31:0]),
        .result     (mul_res),
        .done       (mul_done),
        .error      (mul_err)
    );

    mithril_div_rem u_div (
        .clk        (clk),
        .rst_n      (rst_n),
        .start      (start_div),
        .dividend   (operand_a[63:0]),
        .divisor    (operand_b[63:0]),
        .quotient   (div_quot),
        .remainder  (div_rem),
        .done       (div_done),
        .div_by_zero(div_zero),
        .error      (div_err)
    );

    // ------------------------------------------------------------------------
    // Output Multiplexing
    // ------------------------------------------------------------------------
    always @(*) begin
        result_256           = {WIDTH{1'b0}};
        result_64            = 64'd0;
        remainder_64         = 64'd0;
        flag_carry_underflow = 1'b0;
        flag_div_by_zero     = 1'b0;
        done                 = 1'b0;
        error                = 1'b0;

        case (opcode)
            OP_ADD_SECURE: begin
                result_256           = add_res;
                flag_carry_underflow = add_carry;
                done                 = add_done;
                error                = add_err;
            end

            OP_SUB_SECURE: begin
                result_256           = sub_res;
                flag_carry_underflow = sub_underflow;
                done                 = sub_done;
                error                = sub_err;
            end

            OP_ADD_MOD: begin
                result_256           = mod_res;
                done                 = mod_done;
                error                = mod_err;
            end

            OP_KARATSUBA: begin
                result_64            = mul_res;
                done                 = mul_done;
                error                = mul_err;
            end

            OP_DIV_REM: begin
                result_64            = div_quot;
                remainder_64         = div_rem;
                flag_div_by_zero     = div_zero;
                done                 = div_done;
                error                = div_err;
            end

            default: begin
                error                = 1'b1;
            end
        endcase
    end

endmodule
