`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 06/16/2025 04:09:26 PM
// Design Name: 
// Module Name: poweronreset
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


module poweronreset(
        input clk,
        output rst
    );
    
    SRL16 #(.INIT(16'hFFFF)) srl_rst (  //LUT as a shift-register initialized to All ones
        .Q(rst),
        .A0(1'b1),                      //Address pointing to the last element in LUT
        .A1(1'b1),
        .A2(1'b1),
        .A3(1'b1),
        .CLK(clk),
        .D(1'b0)
    );
endmodule
