//
// 
// Mithril Cryptography API - Hardware Implementation
// Secure Addition Module with libsodium principles
//
// Created by Andres Barbudo
// 
//

`timescale 1ns / 1ps

module mithril_add_secure #(
    parameter WIDTH = 256  // 256-bit operations (Ed25519/Curve25519 compatible)
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

    // State machine for constant-time operation
    localparam IDLE       = 3'b000;
    localparam VALIDATE   = 3'b001;
    localparam COMPUTE    = 3'b010;
    localparam CLEANUP    = 3'b011;
    localparam DONE       = 3'b100;
    
    reg [2:0] state, next_state;
    
    // Internal registers for computation
    reg [WIDTH:0] temp_sum;      // Extra bit for carry
    reg [WIDTH-1:0] a_reg, b_reg;
    reg [3:0] cleanup_counter;
    
    // Constant-time operation counter
    reg [7:0] operation_timer;
    localparam OPERATION_CYCLES = 8'd10;  // Fixed timing regardless of data
    
    //=======================================================================
    // State Machine - Sequential Logic
    //=======================================================================
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            state <= IDLE;
        end else begin
            state <= next_state;
        end
    end
    
    //=======================================================================
    // State Machine - Combinational Logic
    //=======================================================================
    always @(*) begin
        next_state = state;
        
        case (state)
            IDLE: begin
                if (start) begin
                    next_state = VALIDATE;
                end
            end
            
            VALIDATE: begin
                next_state = COMPUTE;
            end
            
            COMPUTE: begin
                // Always take OPERATION_CYCLES regardless of data
                if (operation_timer >= OPERATION_CYCLES) begin
                    next_state = CLEANUP;
                end
            end
            
            CLEANUP: begin
                if (cleanup_counter >= 4'd5) begin
                    next_state = DONE;
                end
            end
            
            DONE: begin
                if (!start) begin
                    next_state = IDLE;
                end
            end
            
            default: next_state = IDLE;
        endcase
    end
    
    //=======================================================================
    // Datapath - Constant-Time Addition
    //=======================================================================
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
                        // Capture inputs
                        a_reg <= operand_a;
                        b_reg <= operand_b;
                    end
                end
                
                VALIDATE: begin
                    // In hardware, validation is implicit
                    // But we maintain timing consistency
                    operation_timer <= operation_timer + 1'b1;
                end
                
                COMPUTE: begin
                    // CRITICAL: Constant-time addition
                    // This operation ALWAYS takes OPERATION_CYCLES
                    // regardless of input values
                    if (operation_timer == 8'd2) begin
                        // Perform addition with carry
                        temp_sum <= {1'b0, a_reg} + {1'b0, b_reg};
                    end
                    
                    if (operation_timer == 8'd5) begin
                        // Extract result and carry
                        result    <= temp_sum[WIDTH-1:0];
                        carry_out <= temp_sum[WIDTH];
                    end
                    
                    operation_timer <= operation_timer + 1'b1;
                end
                
                CLEANUP: begin
                    // SECURITY CRITICAL: Clear sensitive data
                    // Overwrite internal registers with zeros
                    case (cleanup_counter)
                        4'd0: a_reg <= {WIDTH{1'b0}};
                        4'd1: b_reg <= {WIDTH{1'b0}};
                        4'd2: temp_sum <= {(WIDTH+1){1'b0}};
                        4'd3: operation_timer <= 8'd0;
                        4'd4: cleanup_counter <= cleanup_counter + 1'b1;
                        default: cleanup_counter <= cleanup_counter + 1'b1;
                    endcase
                    
                    if (cleanup_counter < 4'd4) begin
                        cleanup_counter <= cleanup_counter + 1'b1;
                    end
                end
                
                DONE: begin
                    done <= 1'b1;
                    // Result remains stable until next operation
                end
                
                default: begin
                    // Safe default state
                    result    <= {WIDTH{1'b0}};
                    carry_out <= 1'b0;
                    done      <= 1'b0;
                    error     <= 1'b1;
                end
            endcase
        end
    end
    
    //=======================================================================
    // Timing Verification (for side-channel resistance)
    //=======================================================================
    // synthesis translate_off
    reg [31:0] cycle_count;
    
    always @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            cycle_count <= 32'd0;
        end else begin
            if (state == IDLE && start) begin
                cycle_count <= 32'd0;
            end else if (state != IDLE && state != DONE) begin
                cycle_count <= cycle_count + 1'b1;
            end
            
            // Verify constant timing
            if (state == DONE && done) begin
                if (cycle_count != (OPERATION_CYCLES + 32'd8)) begin
                    $display("WARNING: Non-constant timing detected! Cycles: %d", cycle_count);
                end
            end
        end
    end
    // synthesis translate_on

endmodule
