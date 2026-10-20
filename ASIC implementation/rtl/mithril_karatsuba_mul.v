// ============================================================================
// Mithril Cryptography API - Hardware ASIC Implementation
// Module: mithril_karatsuba_mul
// Description: Pipelined Karatsuba multiplier for IoT / ASIC cryptography.
//              Takes two 32-bit operands and computes 64-bit result using
//              Karatsuba decomposition:
//              Z0 = Al * Bl
//              Z2 = Ah * Bh
//              Z1 = (Ah + Al) * (Bh + Bl) - Z2 - Z0
//              Result = (Z2 << 32) + (Z1 << 16) + Z0
// ============================================================================

`timescale 1ns / 1ps

module mithril_karatsuba_mul (
    input  wire                 clk,
    input  wire                 rst_n,
    input  wire                 start,
    input  wire [31:0]          operand_a,
    input  wire [31:0]          operand_b,
    output reg  [63:0]          result,
    output reg                  done,
    output reg                  error
);

    localparam [2:0] IDLE     = 3'b000;
    localparam [2:0] SPLIT    = 3'b001;
    localparam [2:0] MULT     = 3'b010;
    localparam [2:0] COMBINE  = 3'b011;
    localparam [2:0] DONE_ST  = 3'b100;

    reg [2:0] state, next_state;

    // Split components (16-bit halves)
    reg [15:0] a_high, a_low;
    reg [15:0] b_high, b_low;
    reg [16:0] a_sum, b_sum;

    // Partial products
    reg [31:0] z0;       // a_low * b_low
    reg [31:0] z2;       // a_high * b_high
    reg [33:0] z1_raw;   // a_sum * b_sum
    reg [33:0] z1;       // z1_raw - z2 - z0

    reg [3:0] step_timer;

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n)
            state <= IDLE;
        else
            state <= next_state;
    end

    always @(*) begin
        next_state = state;
        case (state)
            IDLE:    if (start) next_state = SPLIT;
            SPLIT:   next_state = MULT;
            MULT:    if (step_timer >= 4'd4) next_state = COMBINE;
            COMBINE: if (step_timer >= 4'd8) next_state = DONE_ST;
            DONE_ST: if (!start) next_state = IDLE;
            default: next_state = IDLE;
        endcase
    end

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            result     <= 64'd0;
            done       <= 1'b0;
            error      <= 1'b0;
            step_timer <= 4'd0;
            a_high     <= 16'd0;
            a_low      <= 16'd0;
            b_high     <= 16'd0;
            b_low      <= 16'd0;
            a_sum      <= 17'd0;
            b_sum      <= 17'd0;
            z0         <= 32'd0;
            z2         <= 32'd0;
            z1_raw     <= 34'd0;
            z1         <= 34'd0;
        end else begin
            case (state)
                IDLE: begin
                    done       <= 1'b0;
                    error      <= 1'b0;
                    step_timer <= 4'd0;
                    if (start) begin
                        a_high <= operand_a[31:16];
                        a_low  <= operand_a[15:0];
                        b_high <= operand_b[31:16];
                        b_low  <= operand_b[15:0];
                    end
                end

                SPLIT: begin
                    a_sum <= {1'b0, a_high} + {1'b0, a_low};
                    b_sum <= {1'b0, b_high} + {1'b0, b_low};
                    step_timer <= 4'd0;
                end

                MULT: begin
                    if (step_timer == 4'd1) begin
                        z0     <= a_low * b_low;
                        z2     <= a_high * b_high;
                        z1_raw <= a_sum * b_sum;
                    end
                    if (step_timer == 4'd3) begin
                        z1 <= z1_raw - {2'b0, z2} - {2'b0, z0};
                    end
                    step_timer <= step_timer + 4'd1;
                end

                COMBINE: begin
                    if (step_timer == 4'd6) begin
                        result <= ({32'b0, z2} << 32) + ({30'b0, z1} << 16) + {32'b0, z0};
                    end
                    step_timer <= step_timer + 4'd1;
                end

                DONE_ST: begin
                    done <= 1'b1;
                end

                default: begin
                    result <= 64'd0;
                    done   <= 1'b0;
                    error  <= 1'b1;
                end
            endcase
        end
    end

endmodule
