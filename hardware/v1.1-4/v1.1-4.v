// testbench_mixed_subtraction.v
module testbench;
    reg [511:0] large_integer;
    reg [15:0] small_value;
    wire [511:0] result;
    
    initial begin
        $dumpfile("mixed_subtraction.vcd");
        $dumpvars(0, testbench);
        
        // Initialize values
        large_integer = 512'h1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF1234567890ABCDEF;
        small_value = 16'h1234;
        
        // Perform mixed subtraction
        #10;
        result = $mixed_subtract(large_integer, small_value);
        
        $display("Large Integer: %h", large_integer);
        $display("Small Value: %h", small_value);
        $display("Result: %h", result);
        
        #100;
        $finish;
    end
endmodule
