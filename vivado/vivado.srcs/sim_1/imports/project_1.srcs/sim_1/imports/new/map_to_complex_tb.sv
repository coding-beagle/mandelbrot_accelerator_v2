`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 04/26/2025 10:23:12 AM
// Design Name: 
// Module Name: map_to_complex_tb
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


module map_to_complex_tb(
    );
    
    reg clk;
    reg signed [63:0] stepXVal;
    reg signed [63:0] stepYVal;
    reg signed [63:0] topLeftXVal;
    reg signed [63:0] topLeftYVal;
//    reg [11:0] offset;
    reg signed [63:0] pixel_x; 
    reg signed [63:0] pixel_y;
    wire signed [63:0] out_re;
    wire signed [63:0] out_im;
    
    always #3 clk <= ~clk;
    
    initial begin
       clk <= 0;
	   stepXVal <= 64'h00000a3d70a3d70a;
	   stepYVal <= 64'h00000da740da73ff;
	   topLeftXVal <= 64'hfff0000000000000;
	   topLeftYVal <= 64'h0010000000000000;
       pixel_x <= 64'h0000000000000000;
	   pixel_y <= 64'h2580000000000000;
//	   offset <= 1;
	   #1000 $finish; 
    end
    
    complexMapper DUT(
        .clk(clk),
        .stepX(stepXVal), 
        .stepY(stepYVal), 
        .topLeftX(topLeftXVal), 
        .topLeftY(topLeftYVal),
        .pixelX(pixel_x), 
        .pixelY(pixel_y),
        .outRe1(out_re),
        .outIm1(out_im)
       );
       
endmodule
