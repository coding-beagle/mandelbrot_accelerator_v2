`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 07/22/2025 03:54:29 PM
// Design Name: 
// Module Name: clk_to_bufg
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


module clk_to_bufg(
    input clk,
    output clk_bfg
    );
    
    BUFG global_clk(
        .I(clk),
        .O(clk_bfg)
    );
endmodule
