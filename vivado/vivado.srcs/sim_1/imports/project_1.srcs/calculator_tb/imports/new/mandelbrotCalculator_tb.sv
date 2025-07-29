`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 04/26/2025 12:50:14 PM
// Design Name: 
// Module Name: mandelbrotCalculator_tb
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module mandelbrotCalculator_tb(
    );
    
    reg clk;
    reg reset;
    reg signed [63:0] pixel_x; 
    reg signed [63:0] pixel_y;
    wire finished_up_bruv;
    wire [7:0] iter_count;
    wire [63:0] debug;
    wire [63:0] zReal;
    wire [63:0] zIm;
    wire [2:0] state;
    
    parameter clock_period=2;
    always 
        #(clock_period/2) clk=~clk;
    
    // Reference function to calculate Mandelbrot iterations
    function automatic integer mandelbrot_reference;
        input signed [63:0] c_real;
        input signed [63:0] c_imag;
        integer max_iter;
        
        reg signed [63:0] z_real, z_imag;
        reg signed [63:0] z_real_sq, z_imag_sq;
        reg signed [63:0] temp;
        reg signed [127:0] mult_result;
        integer iter;
        
        begin
            max_iter = 255;
            z_real = 64'h0000000000000000;  // 0.0 in Q12.52
            z_imag = 64'h0000000000000000;  // 0.0 in Q12.52
            iter = 0;
            
            while (iter < max_iter) begin
                // Calculate z_real^2 (Q12.52 * Q12.52 = Q24.104, shift right 52 to get Q12.52)
                mult_result = z_real * z_real;
                z_real_sq = mult_result >>> 52;
                
                // Calculate z_imag^2
                mult_result = z_imag * z_imag;
                z_imag_sq = mult_result >>> 52;
                
                // Check escape condition: |z|^2 = z_real^2 + z_imag^2 >= 4.0
                // 4.0 in Q12.52 format = 0x0040000000000000
                if ((z_real_sq + z_imag_sq) >= 64'h0040000000000000) begin
                    mandelbrot_reference = iter;
                    return;
                end
                
                // Calculate 2 * z_real * z_imag
                mult_result = z_real * z_imag;
                temp = mult_result >>> 52;
                temp = temp << 1;  // Multiply by 2
                
                // Update z: z = z^2 + c
                z_real = z_real_sq - z_imag_sq + c_real;
                z_imag = temp + c_imag;
                
                iter = iter + 1;
            end
            
            mandelbrot_reference = max_iter;
        end
    endfunction
    
    // Test variables
    integer expected_iter;
    integer test_count;
    
    initial begin
        reset <= 1;
        clk <= 0;
        test_count <= 0;
        
        // Test case 1: Point that should converge quickly
        pixel_x <= 64'hffff222222222222;  // Your original test point
        pixel_y <= 64'h0011bab4eead3bab;
        
        #15 reset <= 0;
        
        // Calculate expected result
        expected_iter = mandelbrot_reference(pixel_x, pixel_y);
        $display("Test 1: Expected iterations = %d", expected_iter);
        
        // Wait for calculation to complete
        wait(finished_up_bruv == 1);
        $display("Test 1: Hardware result = %d, Expected = %d", iter_count, expected_iter);
        if (iter_count == expected_iter) 
            $display("Test 1: PASS");
        else 
            $display("Test 1: FAIL - Mismatch!");
        
        #100;
        
        // Test case 2: Origin (should iterate to max)
        reset <= 1;
        #20;
        pixel_x <= 64'h0000000000000000;  // 0.0
        pixel_y <= 64'h0000000000000000;  // 0.0
        reset <= 0;
        
        expected_iter = mandelbrot_reference(pixel_x, pixel_y);
        $display("Test 2: Expected iterations = %d", expected_iter);
        
        wait(finished_up_bruv == 1);
        $display("Test 2: Hardware result = %d, Expected = %d", iter_count, expected_iter);
        if (iter_count == expected_iter) 
            $display("Test 2: PASS");
        else 
            $display("Test 2: FAIL - Mismatch!");
        
        #100;
        
        // Test case 3: Point outside set (should escape quickly)
        reset <= 1;
        #20;
        pixel_x <= 64'h0020000000000000;  // 2.0 in Q12.52
        pixel_y <= 64'h0020000000000000;  // 2.0 in Q12.52
        reset <= 0;
        
        expected_iter = mandelbrot_reference(pixel_x, pixel_y);
        $display("Test 3: Expected iterations = %d", expected_iter);
        
        wait(finished_up_bruv == 1);
        $display("Test 3: Hardware result = %d, Expected = %d", iter_count, expected_iter);
        if (iter_count == expected_iter) 
            $display("Test 3: PASS");
        else 
            $display("Test 3: FAIL - Mismatch!");
        
        #100;
        
        // Test case 4: Point on boundary (interesting case)
        reset <= 1;
        #20;
        pixel_x <= 64'hffff000000000000;  // -1.0 in Q12.52
        pixel_y <= 64'h0000000000000000;  // 0.0 in Q12.52
        reset <= 0;
        
        expected_iter = mandelbrot_reference(pixel_x, pixel_y);
        $display("Test 4: Expected iterations = %d", expected_iter);
        
        wait(finished_up_bruv == 1);
        $display("Test 4: Hardware result = %d, Expected = %d", iter_count, expected_iter);
        if (iter_count == expected_iter) 
            $display("Test 4: PASS");
        else 
            $display("Test 4: FAIL - Mismatch!");
        
        $display("All tests completed!");
        #100;
        $finish; 
    end
    
    mandelbrotCalculator DUT(
        .clk(clk), 
        .rst(reset), 
        .complexX(pixel_x), 
        .complexY(pixel_y), 
        .finishedFlag(finished_up_bruv), 
        .iterationCount(iter_count)
    );
    
endmodule
