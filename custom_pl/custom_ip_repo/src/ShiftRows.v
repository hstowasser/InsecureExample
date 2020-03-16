`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 11/26/2019 01:29:08 PM
// Design Name: 
// Module Name: ShiftRows
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


module ShiftRows(
    input [127:0] state,
    output [127:0] o_state
    );
    parameter COL_0 = 96;
    parameter COL_1 = 64;
    parameter COL_2 = 32;
    parameter COL_3 = 0;
    parameter ROW_0 = 24;
    parameter ROW_1 = 16;
    parameter ROW_2 = 8;
    parameter ROW_3 = 0;
    
    //Row 0
    assign o_state[COL_0+ROW_0+:8] = state[COL_0+ROW_0+:8];
    assign o_state[COL_1+ROW_0+:8] = state[COL_1+ROW_0+:8];
    assign o_state[COL_2+ROW_0+:8] = state[COL_2+ROW_0+:8];
    assign o_state[COL_3+ROW_0+:8] = state[COL_3+ROW_0+:8];
    //Row 1
    assign o_state[COL_0+ROW_1+:8] = state[COL_1+ROW_1+:8];
    assign o_state[COL_1+ROW_1+:8] = state[COL_2+ROW_1+:8];
    assign o_state[COL_2+ROW_1+:8] = state[COL_3+ROW_1+:8];
    assign o_state[COL_3+ROW_1+:8] = state[COL_0+ROW_1+:8];
    //Row 2
    assign o_state[COL_0+ROW_2+:8] = state[COL_2+ROW_2+:8];
    assign o_state[COL_1+ROW_2+:8] = state[COL_3+ROW_2+:8];
    assign o_state[COL_2+ROW_2+:8] = state[COL_0+ROW_2+:8];
    assign o_state[COL_3+ROW_2+:8] = state[COL_1+ROW_2+:8];
    //Row 3
    assign o_state[COL_0+ROW_3+:8] = state[COL_3+ROW_3+:8];
    assign o_state[COL_1+ROW_3+:8] = state[COL_0+ROW_3+:8];
    assign o_state[COL_2+ROW_3+:8] = state[COL_1+ROW_3+:8];
    assign o_state[COL_3+ROW_3+:8] = state[COL_2+ROW_3+:8];
endmodule


module InvShiftRows(
    input [127:0] state,
    output [127:0] o_state
    );
    
    parameter COL_0 = 96;
    parameter COL_1 = 64;
    parameter COL_2 = 32;
    parameter COL_3 = 0;
    parameter ROW_0 = 24;
    parameter ROW_1 = 16;
    parameter ROW_2 = 8;
    parameter ROW_3 = 0;
    
    //Row 1
    assign o_state[COL_0+ROW_0+:8] = state[COL_0+ROW_0+:8];
    assign o_state[COL_1+ROW_0+:8] = state[COL_1+ROW_0+:8];
    assign o_state[COL_2+ROW_0+:8] = state[COL_2+ROW_0+:8];
    assign o_state[COL_3+ROW_0+:8] = state[COL_3+ROW_0+:8];
    //Row 2
    assign o_state[COL_0+ROW_1+:8] = state[COL_3+ROW_1+:8];
    assign o_state[COL_1+ROW_1+:8] = state[COL_0+ROW_1+:8];
    assign o_state[COL_2+ROW_1+:8] = state[COL_1+ROW_1+:8];
    assign o_state[COL_3+ROW_1+:8] = state[COL_2+ROW_1+:8];
    //Row 3
    assign o_state[COL_0+ROW_2+:8] = state[COL_2+ROW_2+:8];
    assign o_state[COL_1+ROW_2+:8] = state[COL_3+ROW_2+:8];
    assign o_state[COL_2+ROW_2+:8] = state[COL_0+ROW_2+:8];
    assign o_state[COL_3+ROW_2+:8] = state[COL_1+ROW_2+:8];
    //Row 4
    assign o_state[COL_0+ROW_3+:8] = state[COL_1+ROW_3+:8];
    assign o_state[COL_1+ROW_3+:8] = state[COL_2+ROW_3+:8];
    assign o_state[COL_2+ROW_3+:8] = state[COL_3+ROW_3+:8];
    assign o_state[COL_3+ROW_3+:8] = state[COL_0+ROW_3+:8];
endmodule