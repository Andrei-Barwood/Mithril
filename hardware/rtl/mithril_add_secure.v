// ============================================================================
// Mithril Cryptography API - Hardware ASIC Implementation
// Module: mithril_add_secure
// Description: Constant-time 256-bit secure adder with side-channel resistance
//              and automated register sanitization.
// ============================================================================

`timescale 1ns / 1ps

module mithril_add_secure #(
    parameter WIDTH = 256  // 256-bit operations (Curve25519 / Ed25519)
) (
    input  wire                 clk,
    input  wire                 rst_n,
    input  wire                 start,
    input  wire [WIDTH-1:0]     operand_a,
    input  wire [WIDTH-1:0]     operand_b,
    output reg  [WIDTH-1:0]     result,
    output reg                  carry_out,
    output reg                  done,
    output reg                  error
);

    // State encoding (one-hot or binary)
    localparam [2:0] IDLE     = 3'b000;
    localparam [2:0] VALIDATE = 3'b001;
    localparam [2:0] COMPUTE  = 3'b010;
    localparam [2:0] CLEANUP  = 3'b011;
    localparam [2:0] DONE_ST  = 3'b100;

    reg [2:0] state, next_state;

    // Internal datapath registers
    reg [WIDTH:0]   temp_sum;
    reg [WIDTH-1:0] a_reg, b_reg;
    reg [7:0]       operation_timer;
    reg [3:0]       cleanup_counter;

    localparam [7:0] OPERATION_CYCLES = 8'd10;  // Fixed cycles for constant-time

    // ------------------------------------------------------------------------
    // State Register (Sequential Logic)
    // ------------------------------------------------------------------------
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            state <= IDLE;
        end else begin
            state <= next_state;
        end
    end

    // ------------------------------------------------------------------------
    // Next State Logic (Combinational)
    // ------------------------------------------------------------------------
    always @(*) begin
        next_state = state;
        case (state)
            IDLE: begin
                if (start)
                    next_state = VALIDATE;
            end

            VALIDATE: begin
                next_state = COMPUTE;
            end

            COMPUTE: begin
                if (operation_timer >= OPERATION_CYCLES)
                    next_state = CLEANUP;
            end

            CLEANUP: begin
                if (cleanup_counter >= 4'd4)
                    next_state = DONE_ST;
            end

            DONE_ST: begin
                if (!start)
                    next_state = IDLE;
            end

            default: next_state = IDLE;
        endcase
    end

    // ------------------------------------------------------------------------
    // Datapath & Output Registers (Sequential)
    // ------------------------------------------------------------------------
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            result          <= {WIDTH{1'b0}};
            carry_out       <= 1'b0;
            done            <= 1'b0;
            error           <= 1'b0;
            temp_sum        <= {(WIDTH+1){1'b0}};
            a_reg           <= {WIDTH{1'b0}};
            b_reg           <= {WIDTH{1'b0}};
            operation_timer <= 8'd0;
            cleanup_counter <= 4'd0;
        end else begin
            case (state)
                IDLE: begin
                    done            <= 1'b0;
                    error           <= 1'b0;
                    operation_timer <= 8'd0;
                    cleanup_counter <= 4'd0;

                    if (start) begin
                        a_reg <= operand_a;
                        b_reg <= operand_b;
                    end
                end

                VALIDATE: begin
                    operation_timer <= operation_timer + 8'd1;
                end

                COMPUTE: begin
                    if (operation_timer == 8'd2) begin
                        temp_sum <= {1'b0, a_reg} + {1'b0, b_reg};
                    end

                    if (operation_timer == 8'd5) begin
                        result    <= temp_sum[WIDTH-1:0];
                        carry_out <= temp_sum[WIDTH];
                    end

                    operation_timer <= operation_timer + 8'd1;
                end

                CLEANUP: begin
                    // Zeroize sensitive internal state
                    case (cleanup_counter)
                        4'd0: a_reg    <= {WIDTH{1'b0}};
                        4'd1: b_reg    <= {WIDTH{1'b0}};
                        4'd2: temp_sum <= {(WIDTH+1){1'b0}};
                        4'd3: operation_timer <= 8'd0;
                        default: ;
                    endcase
                    cleanup_counter <= cleanup_counter + 4'd1;
                end

                DONE_ST: begin
                    done <= 1'b1;
                end

                default: begin
                    result    <= {WIDTH{1'b0}};
                    carry_out <= 1'b0;
                    done      <= 1'b0;
                    error     <= 1'b1;
                end
            endcase
        end
    end

endmodule
