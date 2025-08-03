`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 08/01/2025 09:06:12 PM
// Design Name: 
// Module Name: COORDINATE_CONVERTER
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


module COORDINATE_CONVERTER(
        input wire clk,
        input wire [31:0] STEP_X_HIGH,
        input wire [31:0] STEP_X_LOW,
        input wire [31:0] STEP_Y_HIGH,
        input wire [31:0] STEP_Y_LOW,
        input wire [31:0] TOP_LEFT_X_HIGH,
        input wire [31:0] TOP_LEFT_X_LOW,
        input wire [31:0] TOP_LEFT_Y_HIGH,
        input wire [31:0] TOP_LEFT_Y_LOW,
        input wire [11:0] PIXEL_X,
        input wire [11:0] PIXEL_Y,
        output wire [63:0] C_RE_OUT,
        output wire [63:0] C_IM_OUT
    );
    
    wire [63:0] intermediate_x, intermediate_y;
    
    mult_gen_0 MULTIPLIER_X(
        .A(PIXEL_X),
        .B({STEP_X_HIGH, STEP_X_LOW}),
        .P(intermediate_x)
    );
    
    mult_gen_0 MULTIPLIER_Y(
        .A(PIXEL_Y),
        .B({STEP_Y_HIGH, STEP_Y_LOW}),
        .P(intermediate_y)
    );
    
    reg [63:0] C_RE_REG, C_IM_REG;
    
    assign C_RE_OUT = C_RE_REG;
    assign C_IM_OUT = C_IM_REG;
    
    always @ (posedge clk) begin
        C_RE_REG <= {TOP_LEFT_X_HIGH, TOP_LEFT_X_LOW} - intermediate_x;
        C_IM_REG <= {TOP_LEFT_Y_HIGH, TOP_LEFT_Y_LOW} - intermediate_y;
    end
    
endmodule
