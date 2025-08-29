//
// iot_secure_multiplication_test.v
// Test secure multiplication for IoT device values
//

module iot_secure_test;
    
    // IoT sensor data simulation
    reg [31:0] sensor_value_a;
    reg [31:0] sensor_value_b;
    reg [63:0] secure_result;
    
    // Clock for IoT timing simulation
    reg clk;
    always #5 clk = ~clk;
    
    initial begin
        $display("IoT Secure Multiplication Test Starting...");
        clk = 0;
        
        // Test Case 1: Small IoT sensor values
        sensor_value_a = 32'd1234;
        sensor_value_b = 32'd5678;
        #10;
        secure_result = $secure_mul(sensor_value_a, sensor_value_b);
        $display("Test 1: %d * %d = %d", sensor_value_a, sensor_value_b, secure_result);
        
        // Test Case 2: Larger IoT device IDs
        sensor_value_a = 32'd123456;
        sensor_value_b = 32'd789012;
        #10;
        secure_result = $secure_mul(sensor_value_a, sensor_value_b);
        $display("Test 2: %d * %d = %d", sensor_value_a, sensor_value_b, secure_result);
        
        // Test Case 3: Zero handling (important for IoT error cases)
        sensor_value_a = 32'd0;
        sensor_value_b = 32'd12345;
        #10;
        secure_result = $secure_mul(sensor_value_a, sensor_value_b);
        $display("Test 3: %d * %d = %d", sensor_value_a, sensor_value_b, secure_result);
        
        // Test Case 4: Maximum values for IoT range
        sensor_value_a = 32'hFFFFFFFF;
        sensor_value_b = 32'h00000002;
        #10;
        secure_result = $secure_mul(sensor_value_a, sensor_value_b);
        $display("Test 4: %h * %h = %h", sensor_value_a, sensor_value_b, secure_result);
        
        $display("IoT Secure Multiplication Test Complete");
        $finish;
    end
    
endmodule
