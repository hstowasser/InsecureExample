`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 11/30/2019 10:45:01 PM
// Design Name: 
// Module Name: Add_Round_Key
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


`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 11/26/2019 11:17:38 PM
// Design Name: 
// Module Name: AddRoundKey
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


module AddRoundKey(
    input wire [0:127] state,
    input wire [0:127] round_key,
    output reg [0:127] o_state
    );
    

	integer i;

    always@(state or round_key)
        begin

	   for ( i=0; i<=15; i=i+1)
	       o_state[i*8  +:  8] <= round_key[i*8  +:  8] ^ state[i*8  +:  8];


	   end 

    
endmodule
