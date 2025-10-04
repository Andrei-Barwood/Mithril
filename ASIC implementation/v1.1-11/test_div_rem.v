// test_div_rem.v
// Testbench for division with remainder PLI module
// Demonstrates IoT device mathematical operations in secured environment

module test_div_rem;
    reg [63:0] dividend;
    reg [63:0] divisor;
    reg [63:0] quotient;
    reg [63:0] remainder;
    
    initial begin
        $display("=== IoT Secured Division Module Test ===");
        $display("Time\tDividend\tDivisor\tQuotient\tRemainder");
        $display("----\t--------\t-------\t--------\t---------");
        
        // Test case 1: Normal division
        dividend = 100;
        divisor = 7;
        #10 $div_with_rem(dividend, divisor, quotient, remainder);
        $display("%0t\t%0d\t\t%0d\t%0d\t\t%0d", $time, dividend, divisor, quotient, remainder);
        
        // Test case 2: Exact division
        dividend = 64;
        divisor = 8;
        #10 $div_with_rem(dividend, divisor, quotient, remainder);
        $display("%0t\t%0d\t\t%0d\t%0d\t\t%0d", $time, dividend, divisor, quotient, remainder);
        
        // Test case 3: Division by zero (security check)
        dividend = 50;
        divisor = 0;
        #10 $div_with_rem(dividend, divisor, quotient, remainder);
        $display("%0t\t%0d\t\t%0d\t%0d\t\t%0d", $time, dividend, divisor, quotient, remainder);
        
        // Test case 4: Zero dividend
        dividend = 0;
        divisor = 10;
        #10 $div_with_rem(dividend, divisor, quotient, remainder);
        $display("%0t\t%0d\t\t%0d\t%0d\t\t%0d", $time, dividend, divisor, quotient, remainder);
        
        // Test case 5: Large numbers (cryptographic use case)
        dividend = 1024;
        divisor = 17;
        #10 $div_with_rem(dividend, divisor, quotient, remainder);
        $display("%0t\t%0d\t\t%0d\t%0d\t\t%0d", $time, dividend, divisor, quotient, remainder);
        
        #10 $finish;
    end
endmodule
