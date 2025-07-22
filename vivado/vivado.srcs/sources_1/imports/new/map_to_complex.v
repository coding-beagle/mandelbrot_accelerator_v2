`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 04/26/2025 09:40:43 AM
// Design Name: 
// Module Name: map_to_complex
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
// This file takes in the current parameters of the view, i.e. the current size
// of the screen in viewWidth + viewHeight, the current zoom level and pan level
// and a pixel on the screenSpace and converts that to its floating point
// representation in the simulation's complex space.
//
// We assume that EVERYTHING is in the twos complement form:
// of Q12.52
// 
// Let's pray that resolution works out to be a power of 2 (in this code we're
// assuming that the draw resolution is 1024 / 512 )
//
// It's gonna get zoomLevelX and zoomLevelY from somewhere else, not sure where
// yet, and we need to make sure that zoomLevelX and zoomLevelY are roughly
// in the ratio of 4:2.25
//////////////////////////////////////////////////////////////////////////////////


module complexMapper(
    input clk,
    input signed [63:0] stepX, // how much 1 pixel means in complex space
    input signed [63:0] stepY,
    input signed [63:0] topLeftX, // complex space coordinates for top left camera (starting point)
    input signed [63:0] topLeftY,
    input [63:0] pixelX, // screen space coordinates
    input [63:0] pixelY,
    output signed [63:0] outRe1,
    output signed [63:0] outIm,
    output signed [63:0] outRe2,
    output signed [63:0] outRe3,
    output signed [63:0] outRe4
    );
        
    wire signed [63:0] intermediateRe;
    wire signed [63:0] intermediateIm; 
    
//    assign intermediateRe = pixelX[63:52] * stepX;
//    assign intermediateIm = pixelY[63:52] * stepY;

    mult_gen_1 intermediateReMult(
        .CLK(clk),
        .A(pixelX),
        .B(stepX),
        .P(intermediateRe)
    );
    
    mult_gen_1 intermediateImMult(
        .CLK(clk),
        .A(pixelY),
        .B(stepY),
        .P(intermediateIm)
    );
    
    assign outRe1 = topLeftX - intermediateRe; 
    assign outIm =  topLeftY - intermediateIm; 
    assign outRe2 = topLeftX - intermediateRe - stepX; 
    assign outRe3 = topLeftX - intermediateRe - stepX - stepX; 
    assign outRe4 = topLeftX - intermediateRe - stepX - stepX - stepX; 
//    assign outRe5 = topLeftX - intermediateRe - stepX - stepX - stepX - stepX; 
//    assign outRe6 = topLeftX - intermediateRe - stepX - stepX - stepX - stepX - stepX; 
//    assign outRe7 = topLeftX - intermediateRe - stepX - stepX - stepX - stepX - stepX - stepX; 
//    assign outRe8 = topLeftX - intermediateRe - stepX - stepX - stepX - stepX - stepX - stepX - stepX; 
        
endmodule
