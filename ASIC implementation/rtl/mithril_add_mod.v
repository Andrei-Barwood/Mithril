// ============================================================================
// Mithril Cryptography API - Hardware ASIC Implementation
// Module: mithril_add_mod
// Description: Constant-time 256-bit modular addition for Curve25519/Ed25519:
//              result = (operand_a + operand_b) mod p
//              Default p = 2^255 - 19
// ============================================================================

`timescale 1ns / 1ps

module mithril_add_mod #(
    parameter WIDTH = 256
) (
    input  wire                 clk,
    input  wire                 rst_n,
    input  wire                 start,
    input  wire [WIDTH-1:0]     operand_a,
    input  wire [WIDTH-1:0]     operand_b,
    input  wire [WIDTH-1:0]     modulus,      // If 0, uses Curve25519 prime
    output reg  [WIDTH-1:0]     result,
    output reg                  done,
    output reg                  error
);

    // Curve25519 prime: 2^255 - 19
    localparam [WIDTH-1:0] CURVE25519_P = 256'h7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffed;

    localparam [2:0] IDLE     = 3'b000;
    localparam [2:0] ADD_ST   = 3'b001;
    localparam [2:0] REDUCE   = 3'b010;
    localparam [2:0] CLEANUP  = 3'b011;
    localparam [2:0] DONE_ST  = 3'b100;

    reg [2:0] state, next_state;

    reg [WIDTH:0]   temp_sum;
    reg [WIDTH:0]   reduced_sum;
    reg [WIDTH-1:0] working_modulus;
    reg [WIDTH-1:0] a_reg, b_reg;
    reg [7:0]       operation_timer;

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n)
            state <= IDLE;
        else
            state <= next_state;
    end

    always @(*) begin
        next_state = state;
        case (state)
            IDLE:    if (start) next_state = ADD_ST;
            ADD_ST:  if (operation_timer >= 8'd4) next_state = REDUCE;
            REDUCE:  if (operation_timer >= 8'd10) next_state = CLEANUP;
            CLEANUP: if (operation_timer >= 8'd15) next_state = DONE_ST;
            DONE_ST: if (!start) next_state = IDLE;
            default: next_state = IDLE;
        endcase
    end

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            result          <= {WIDTH{1'b0}};
            done            <= 1'b0;
            error           <= 1'b0;
            temp_sum        <= {(WIDTH+1){1'b0}};
            reduced_sum     <= {(WIDTH+1){1'b0}};
            a_reg           <= {WIDTH{1'b0}};
            b_reg           <= {WIDTH{1'b0}};
            working_modulus <= {WIDTH{1'b0}};
            operation_timer <= 8'd0;
        end else begin
            case (state)
                IDLE: begin
                    done            <= 1'b0;
                    error           <= 1'b0;
                    operation_timer <= 8'd0;

                    if (start) begin
                        a_reg <= operand_a;
                        b_reg <= operand_b;
                        if (modulus == {WIDTH{1'b0}})
                            working_modulus <= CURVE25519_P;
                        else
                            working_modulus <= modulus;
                    end
                end

                ADD_ST: begin
                    if (operation_timer == 8'd1) begin
                        temp_sum <= {1'b0, a_reg} + {1'b0, b_reg};
                    end
                    operation_timer <= operation_timer + 8'd1;
                end

                REDUCE: begin
                    if (operation_timer == 8'd5) begin
                        // Constant-time candidate reduction
                        reduced_sum <= temp_sum - {1'b0, working_modulus};
                    end

                    if (operation_timer == 8'd8) begin
                        // If no borrow occurred in reduced_sum (reduced_sum[WIDTH] == 0),
                        // temp_sum >= working_modulus, so result = reduced_sum
                        if (reduced_sum[WIDTH] == 1'b0)
                            result <= reduced_sum[WIDTH-1:0];
                        else
                            result <= temp_sum[WIDTH-1:0];
                    end
                    operation_timer <= operation_timer + 8'd1;
                end

                CLEANUP: begin
                    if (operation_timer == 8'd12) begin
                        a_reg       <= {WIDTH{1'b0}};
                        b_reg       <= {WIDTH{1'b0}};
                        temp_sum    <= {(WIDTH+1){1'b0}};
                        reduced_sum <= {(WIDTH+1){1'b0}};
                    end
                    operation_timer <= operation_timer + 8'd1;
                end

                DONE_ST: begin
                    done <= 1'b1;
                end

                default: begin
                    result <= {WIDTH{1'b0}};
                    done   <= 1'b0;
                    error  <= 1'b1;
                end
            endcase
        end
    end

endmodule
