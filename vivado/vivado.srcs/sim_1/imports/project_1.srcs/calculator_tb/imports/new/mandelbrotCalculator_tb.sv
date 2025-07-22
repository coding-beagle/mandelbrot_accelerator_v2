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
    
    parameter clock_period=10;
    always 
        #(clock_period/2) clk=~clk;
    
    initial begin
       reset <= 1;
       clk <= 0;
       pixel_x <= 64'hffff222222222222;
	   pixel_y <= 64'h0011bab4eead3bab;
       #15 reset <= 0;
	   #7000 $finish; 
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
