`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 08/01/2025 09:23:12 PM
// Design Name: 
// Module Name: RESET_DRIVER
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


module RESET_DRIVER(
        input wire clk,
        input wire [31:0] slv_reg,
        output wire RST_1,
        output wire RST_2,
        output wire RST_3,
        output wire RST_4
    );
    
        reg [3:0] RST;
        
        assign RST_1 = RST[0];
        assign RST_2 = RST[1];
        assign RST_3 = RST[2];
        assign RST_4 = RST[3];
        
        always @(posedge clk) begin
            RST <= slv_reg[3:0];
        end
endmodule
