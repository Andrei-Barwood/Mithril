// ============================================================================
// Mithril Cryptography API - Hardware ASIC Implementation
// Module: mithril_div_rem
// Description: Non-restoring / constant-cycle division and remainder core
//              with divide-by-zero protection for cryptographic telemetry.
// ============================================================================

`timescale 1ns / 1ps

module mithril_div_rem (
    input  wire                 clk,
    input  wire                 rst_n,
    input  wire                 start,
    input  wire [63:0]          dividend,
    input  wire [63:0]          divisor,
    output reg  [63:0]          quotient,
    output reg  [63:0]          remainder,
    output reg                  done,
    output reg                  div_by_zero,
    output reg                  error
);

    localparam [1:0] IDLE    = 2'b00;
    localparam [1:0] COMPUTE = 2'b01;
    localparam [1:0] DONE_ST = 2'b10;

    reg [1:0] state, next_state;

    reg [6:0]   bit_counter;
    reg [63:0]  rem_reg;
    reg [63:0]  quot_reg;
    reg [63:0]  d_reg;

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n)
            state <= IDLE;
        else
            state <= next_state;
    end

    always @(*) begin
        next_state = state;
        case (state)
            IDLE: begin
                if (start)
                    next_state = COMPUTE;
            end
            COMPUTE: begin
                if (bit_counter == 7'd64)
                    next_state = DONE_ST;
            end
            DONE_ST: begin
                if (!start)
                    next_state = IDLE;
            end
            default: next_state = IDLE;
        endcase
    end

    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            quotient    <= 64'd0;
            remainder   <= 64'd0;
            done        <= 1'b0;
            div_by_zero <= 1'b0;
            error       <= 1'b0;
            bit_counter <= 7'd0;
            rem_reg     <= 64'd0;
            quot_reg    <= 64'd0;
            d_reg       <= 64'd0;
        end else begin
            case (state)
                IDLE: begin
                    done        <= 1'b0;
                    div_by_zero <= 1'b0;
                    error       <= 1'b0;
                    bit_counter <= 7'd0;

                    if (start) begin
                        if (divisor == 64'd0) begin
                            div_by_zero <= 1'b1;
                            error       <= 1'b1;
                            quotient    <= 64'd0;
                            remainder   <= 64'd0;
                        end else begin
                            quot_reg <= dividend;
                            d_reg    <= divisor;
                            rem_reg  <= 64'd0;
                        end
                    end
                end

                COMPUTE: begin
                    if (div_by_zero) begin
                        // Skip compute if division by zero
                        bit_counter <= 7'd64;
                    end else begin
                        // Shift and subtract non-restoring step
                        reg [64:0] next_rem;
                        next_rem = {rem_reg[62:0], quot_reg[63]};
                        if (next_rem >= {1'b0, d_reg}) begin
                            rem_reg  <= next_rem[63:0] - d_reg;
                            quot_reg <= {quot_reg[62:0], 1'b1};
                        end else begin
                            rem_reg  <= next_rem[63:0];
                            quot_reg <= {quot_reg[62:0], 1'b0};
                        end
                        bit_counter <= bit_counter + 7'd1;
                    end
                end

                DONE_ST: begin
                    if (!div_by_zero) begin
                        quotient  <= quot_reg;
                        remainder <= rem_reg;
                    end
                    done <= 1'b1;
                end

                default: begin
                    done  <= 1'b0;
                    error <= 1'b1;
                end
            endcase
        end
    end

endmodule
