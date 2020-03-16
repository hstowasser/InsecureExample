`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 12/01/2019 11:20:03 AM
// Design Name: 
// Module Name: MixColumns
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


module MixColumns(
    input [127:0] state,
    output [127:0] o_state
    );
    
    //Multiply by 2
    function [7 : 0] gm2(input [7 : 0] op);
    begin
        gm2 = {op[6 : 0], 1'b0} ^ (8'h1b & {8{op[7]}});
    end
    endfunction // gm2
    
    //Multiply by 3
    function [7 : 0] gm3(input [7 : 0] op);
    begin
        gm3 = gm2(op) ^ op;
    end
    endfunction // gm3 
    
    function [32 : 0] mix_word(input [32:0] word);
    begin
        mix_word[3*8+:8] = gm2(word[3*8+:8]) ^ gm3(word[2*8+:8]) ^ word[1*8+:8] ^ word[0+:8];
        mix_word[2*8+:8] = word[3*8+:8] ^ gm2(word[2*8+:8]) ^ gm3(word[1*8+:8]) ^ word[0+:8];
        mix_word[1*8+:8] = word[3*8+:8] ^ word[2*8+:8] ^ gm2(word[1*8+:8]) ^ gm3(word[0+:8]);
        mix_word[0+:8] = gm3(word[3*8+:8]) ^ word[2*8+:8] ^ word[1*8+:8] ^ gm2(word[0+:8]);
    end
    endfunction
    
    assign o_state[3*32+:32] = mix_word(state[3*32+:32]);
    assign o_state[2*32+:32] = mix_word(state[2*32+:32]);
    assign o_state[1*32+:32] = mix_word(state[1*32+:32]);
    assign o_state[0+:32] = mix_word(state[0+:32]);
endmodule

module InvMixColumns(
    input [127:0] state,
    output [127:0] o_state
    );

    //----------------------------------------------------------------
    // Gaolis multiplication functions for Inverse MixColumn.
    //----------------------------------------------------------------
    function [7 : 0] gm2(input [7 : 0] op);
    begin
        gm2 = {op[6 : 0], 1'b0} ^ (8'h1b & {8{op[7]}});
    end
    endfunction // gm2
    
    function [7 : 0] gm3(input [7 : 0] op);
    begin
        gm3 = gm2(op) ^ op;
    end
    endfunction // gm3
    
    function [7 : 0] gm4(input [7 : 0] op);
    begin
        gm4 = gm2(gm2(op));
    end
    endfunction // gm4
    
    function [7 : 0] gm8(input [7 : 0] op);
    begin
        gm8 = gm2(gm4(op));
    end
    endfunction // gm8
    
    function [7 : 0] gm9(input [7 : 0] op);
    begin
        gm9 = gm8(op) ^ op;
    end
    endfunction // gm9
    
    function [7 : 0] gm11(input [7 : 0] op);
    begin
        gm11 = gm8(op) ^ gm2(op) ^ op;
    end
    endfunction // gm11
    
    function [7 : 0] gm13(input [7 : 0] op);
    begin
        gm13 = gm8(op) ^ gm4(op) ^ op;
    end
    endfunction // gm13
    
    function [7 : 0] gm14(input [7 : 0] op);
    begin
        gm14 = gm8(op) ^ gm4(op) ^ gm2(op);
    end
    endfunction // gm14
    
    function [32 : 0] inv_mix_word(input [32:0] word);
    begin
        inv_mix_word[3*8+:8] = gm14(word[3*8+:8]) ^ gm11(word[2*8+:8]) ^ gm13(word[1*8+:8]) ^ gm9(word[0+:8]);
        inv_mix_word[2*8+:8] = gm9(word[3*8+:8]) ^ gm14(word[2*8+:8]) ^ gm11(word[1*8+:8]) ^ gm13(word[0+:8]);
        inv_mix_word[1*8+:8] = gm13(word[3*8+:8]) ^ gm9(word[2*8+:8]) ^ gm14(word[1*8+:8]) ^ gm11(word[0+:8]);
        inv_mix_word[0+:8] = gm11(word[3*8+:8]) ^ gm13(word[2*8+:8]) ^ gm9(word[1*8+:8]) ^ gm14(word[0+:8]);
    end
    endfunction
    
    assign o_state[3*32+:32] = inv_mix_word(state[3*32+:32]);
    assign o_state[2*32+:32] = inv_mix_word(state[2*32+:32]);
    assign o_state[1*32+:32] = inv_mix_word(state[1*32+:32]);
    assign o_state[0+:32] = inv_mix_word(state[0+:32]);
endmodule