`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 03/09/2020 10:26:26 PM
// Design Name: 
// Module Name: MemodC
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


module MemodC(
    input wire reset,
    input wire clk_slow,
    input wire clk_fast,
    input wire go,
    input wire [1023:0] M,
    input wire [1023:0] e,
    input wire [1023:0] N,
    output wire [1023:0] out,
    output reg finished
    );
    
    parameter Stop = 0;
    parameter Square = 1;
    parameter Multiply = 2;
    reg [1:0] state;
    reg [1:0] last_state;
    reg last_go;
    
    wire ABmodN_finished; 
    reg ABmodN_go; 
    reg [1023:0] e_holder;
    
    reg [1023:0] C;
    assign out = C;
    
    wire [1023:0] Input_B;
    assign Input_B = (state == Square) ? C : M; // else multiply
    wire [1023:0] ABmodN_out;
    
    
    
    ABmodN uut(
    .reset(reset),
    .clk_slow(clk_slow),
    .clk_fast(clk_fast),
    .go(ABmodN_go),
    .A(C),
    .B(Input_B),
    .N(N),
    .out(ABmodN_out),
    .finished(ABmodN_finished)
    );
    
    //integer i;
    reg [10:0] i;
    
    reg delay;
    
    
    always @(posedge clk_slow) begin//, negedge reset) begin
        if (reset == 0) begin
            state <= Stop;
            last_state <= Stop;
            finished <= 1;
            ABmodN_go <= 0;
            last_go <= 0;
            i <= 0;
            delay <= 1;
            C <= 0;
        end
        else begin
            last_state <= state;
            case (state) 
                Stop: begin 
                    if( go == 1 && last_go == 0) begin
                        if ( e[1023] == 1) begin
                            C <= M; // Do first multiply
                        end
                        else begin
                            C <= 1;
                        end
                        i <= 1023;// - 1;
                        e_holder <= e << 1;
                        finished <= 0;
                        ABmodN_go <= 0;
                        state <= Square;
                        delay <= 1;
                    end
                    last_go <= go;
                end
                Square: begin 
                    // ABmodC( C, C, N)
                    if( last_state != Square ) begin
                        //Do nothing
                        delay <= 1;
                        ABmodN_go <= 1;
                    end
                    else if ( delay == 1) begin
                        delay <= 0;
                    end
                    else begin
                        if (ABmodN_finished == 1) begin
                            C <= ABmodN_out;
                            state <= Multiply;
                            ABmodN_go <= 0;
                        end
                    end
                end
                Multiply: begin 
                    if( last_state != Multiply) begin
                        if( e_holder[1023] == 1) begin
                            //Multiply
                            //if(i>0) begin
                                i <= i-1;
                                e_holder <= e_holder << 1;
                                state <= Multiply; // Stay in Multiply
                                ABmodN_go <= 1; // Start multiplication
                                delay <= 1; //Trigger delay so ABmodN_finished will fall
                            //end 
                            //else begin
                            //    finished <= 1;
                            //    state <= Stop;
                            //end
                        end
                        else begin
                            //Square
                            if(i>1) begin // i>0 or i >1
                                i <= i-1;
                                e_holder <= e_holder << 1;
                                state <= Square;
                            end 
                            else begin
                                finished <= 1;
                                state <= Stop;
                            end
                        end
                    end 
                    else begin
                        // Wait for multiplication to complete  
                        if ( delay == 1) begin
                            delay <= 0;
                        end   
                        else if (ABmodN_finished == 1) begin
                            C <= ABmodN_out;
                            ABmodN_go <= 0;
                            if(i>1) begin
                                state <= Square;
                            end                            
                            else begin
                                finished <= 1;
                                state <= Stop;
                            end
                        end 
                    end
                end
                default: begin state <= Stop; end
            endcase
        end
    end
    
endmodule
