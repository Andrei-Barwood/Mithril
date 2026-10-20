//
// mithril_add_mod.v
// Modular Addition for Curve25519/Ed25519
// Implements: result = (a + b) mod p
// Where p = 2^255 - 19 (Curve25519 prime)
//

`timescale 1ns / 1ps

module mithril_add_mod #(
    parameter WIDTH = 256
) (
    input  wire                 clk,
    input  wire                 rst_n,
    input  wire                 start,
    input  wire [WIDTH-1:0]     operand_a,
    input  wire [WIDTH-1:0]     operand_b,
    input  wire [WIDTH-1:0]     modulus,      // For generic modular arithmetic
    output reg  [WIDTH-1:0]     result,
    output reg                  done,
    output reg                  error
);

    // Curve25519 prime: 2^255 - 19
    localparam [WIDTH-1:0] CURVE25519_P = 256'h7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffed;
    
    // States
    localparam IDLE       = 3'b000;
    localparam ADD        = 3'b001;
    localparam REDUCE     = 3'b010;
    localparam CLEANUP    = 3'b011;
    localparam DONE_ST    = 3'b100;
    
    reg [2:0] state, next_state;
    
    // Internal registers
    reg [WIDTH:0] temp_sum;
    reg [WIDTH-1:0] working_modulus;
    reg [WIDTH-1:0] a_reg, b_reg;
    reg [7:0] operation_timer;
    
    //=======================================================================
    // State Machine
    //=======================================================================
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            state <= IDLE;
        end else begin
            state <= next_state;
        end
    end
    
    always @(*) begin
        next_state = state;
        
        case (state)
            IDLE:    if (start) next_state = ADD;
            ADD:     if (operation_timer >= 8'd3) next_state = REDUCE;
            REDUCE:  if (operation_timer >= 8'd10) next_state = CLEANUP;
            CLEANUP: if (operation_timer >= 8'd15) next_state = DONE_ST;
            DONE_ST: if (!start) next_state = IDLE;
            default: next_state = IDLE;
        endcase
    end
    
    //=======================================================================
    // Datapath - Constant-Time Modular Addition
    //=======================================================================
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            result          <= {WIDTH{1'b0}};
            done            <= 1'b0;
            error           <= 1'b0;
            temp_sum        <= {(WIDTH+1){1'b0}};
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
                        
                        // Check if modulus is zero (error condition)
                        if (modulus == {WIDTH{1'b0}}) begin
                            working_modulus <= CURVE25519_P;  // Use default
                        end else begin
                            working_modulus <= modulus;
                        end
                    end
                end
                
                ADD: begin
                    if (operation_timer == 8'd1) begin
                        // Perform addition
                        temp_sum <= {1'b0, a_reg} + {1'b0, b_reg};
                    end
                    operation_timer <= operation_timer + 1'b1;
                end
                
                REDUCE: begin
                    if (operation_timer == 8'd5) begin
                        // Constant-time modular reduction
                        // If temp_sum >= modulus, subtract modulus
                        if (temp_sum[WIDTH-1:0] >= working_modulus) begin
                            result <= temp_sum[WIDTH-1:0] - working_modulus;
                        end else begin
                            result <= temp_sum[WIDTH-1:0];
                        end
                    end
                    operation_timer <= operation_timer + 1'b1;
                end
                
                CLEANUP: begin
                    // Clear sensitive data
                    if (operation_timer == 8'd12) begin
                        a_reg    <= {WIDTH{1'b0}};
                        b_reg    <= {WIDTH{1'b0}};
                        temp_sum <= {(WIDTH+1){1'b0}};
                    end
                    operation_timer <= operation_timer + 1'b1;
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
