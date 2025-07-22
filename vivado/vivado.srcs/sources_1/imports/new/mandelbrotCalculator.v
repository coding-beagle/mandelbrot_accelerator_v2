`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 04/26/2025 11:46:50 AM
// Design Name: 
// Module Name: mandelbrotCalculator
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
// Turns a screen space coordinate into an iteration count;
// if the iter count == 255 then we assume that it is in the set
// otherwise it isn't
//
//////////////////////////////////////////////////////////////////////////////////


module mandelbrotCalculator(
    input clk,
    input rst,
    input [63:0] complexX, 
    input [63:0] complexY,
    output finishedFlag,
    output [9:0] iterationCount
    );
    
    BUFG mandelclocker(
        .I(clk),
        .O(mandelclock)
    );
    
    reg [9:0] iterCount;
    reg signed [63:0] cRe; // converted screen pixel to complex coordinate
    reg signed [63:0] cIm; // converted screen pixel to complex coordinate
    reg signed [63:0] zIm;
    reg signed [63:0] zRe;
    wire signed [63:0] zReSquared;
    wire signed [63:0] zImSquared;
    
    reg [2:0] calc_state;
    
    localparam  WAIT_FOR_SQUARES = 0,
                WAIT_FOR_TEMP = 1,
                CALC_Z = 2,
                FINAL = 3;
    
    wire signed [63:0] temp;
    reg signed [63:0] temp2;
//    reg [127:0] debug_out;
    
    reg [4:0] multiplication_reg;
    
//    wire signed [63:0] multi_input_1 = (calc_state == WAIT_FOR_TEMP) ? zIm : zRe;

    
    mult_gen_0 ZRE_SQUARER(
        .CLK(mandelclock),
        .A(zRe),
        .B(zRe),
        .P(zReSquared)
    );
    
    
    mult_gen_0 ZIM_SQUARER(
        .CLK(mandelclock),
        .A(zIm),
        .B(zIm),
        .P(zImSquared)
    );
    
    mult_gen_2 ZIM_x_ZRE(
        .CLK(mandelclock),
        .A(zRe),
        .B(zIm),
        .P(temp)
    );
    
    reg finished;
    
    assign finishedFlag = finished;
    assign iterationCount = iterCount;
    
    always @ (posedge mandelclock) begin
        if(rst) begin
            finished <= 0;
            iterCount <= 0;
//            debug_out <= 64'h0000000000001111;
            zIm <= 64'h0000000000000000;
            zRe <= 64'h0000000000000000;
            multiplication_reg <= 0;
            temp2 <= 64'h0000000000000000;
            cRe <= complexX;
            cIm <= complexY;
            calc_state <= 0;
        end else if(!finished) begin
                case(calc_state)
                    WAIT_FOR_SQUARES: begin
                        if(multiplication_reg < 1) // wait for multipliers to work
                            multiplication_reg <= multiplication_reg + 1;
                        else begin
                            calc_state <= WAIT_FOR_TEMP;
                            multiplication_reg <= 0;
                        end
                    end
                    WAIT_FOR_TEMP: begin
                        if(multiplication_reg < 1) // wait for multipliers to work
                            multiplication_reg <= multiplication_reg + 1;
                        else begin
                            calc_state <= CALC_Z;
                            temp2 <= zReSquared + zImSquared; // zRe ^ 2 + zIm ^ 2
                            multiplication_reg <= 0;
                        end
                    end
                    CALC_Z: begin
                        zRe <= zReSquared + (~(zImSquared) + 1) + cRe; // convert to two's compleMENT, i.e zReSquared - zImSquared + cRe
                        zIm <= temp + cIm + temp; // convoluted way of writing 2 * zRe zIm + cIm
                        calc_state <= FINAL;
                    end
                    FINAL: begin                        
                        if ((iterCount == 255) || (temp2 >= $signed(64'h0040000000000000))) begin
                            finished <= 1;
                        end else begin
                            iterCount <= iterCount + 1;
                            calc_state <= WAIT_FOR_SQUARES;
                        end
                    end
                endcase                
                
                
//                OG CODE:
//                zReSquaredIntermediate = (zRe * zRe) >> 52; // zRe ^ 2
//                zImSquaredIntermediate = (zIm * zIm) >> 52; // zIm ^ 2\
//                zReSquared = zReSquaredIntermediate[63:0];
//                zImSquared = zImSquaredIntermediate[63:0];
//                temp = (zRe * zIm) >> 52; // zRe * zIm
//                temp2 = zReSquared + zImSquared; // zRe ^ 2 + zIm ^ 2 
//                // zRe ^ 2 - zIm ^ 2 + cRe
//                zRe <= zReSquared[63:0] + (~(zImSquared[63:0]) + 1) + cRe; // convert to two's compleMENT
//                // 2 * zRe * zIm + cIm = zRe * zIm + zRe * zIm + cIm
//                zIm <= (temp[63:0]) + cIm + (temp[63:0]); // convoluted way of writing 2 * zRe zIm + cIm
////                debug_out <= temp2[63:0];
                
                
                        
           end
    end
    
endmodule
