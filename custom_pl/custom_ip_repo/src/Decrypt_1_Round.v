


module AES_Decrypt_10_no_key(
    input wire Clock,
    input wire Command, //Begins encryption on rising edge
    input wire [127:0] Key,
    input wire [127:0] Round1,
    input wire [127:0] Round2,
    input wire [127:0] Round3,
    input wire [127:0] Round4,
    input wire [127:0] Round5,
    input wire [127:0] Round6,
    input wire [127:0] Round7,
    input wire [127:0] Round8,
    input wire [127:0] Round9,
    input wire [127:0] Round10,
    input wire [127:0] Data,
    output reg [127:0] o_Data,
    output reg Complete_Flag
    );
    
    parameter Idle = 0;
    parameter Decrypt = 1;
    
    reg State;
    reg Last_Command;
    reg [3:0] round;
    reg [127:0] Data_reg;
    
    reg [127:0] selected_key;
    
        
    //Start
    wire [127:0] Add0;
    AddRoundKey add0 (Data, Round10,Add0); // Start Add Key
    
    
    //Round 1
    wire [127:0] Shift;
    InvShiftRows shift1 (Data_reg, Shift);
    wire [127:0] Sub;
    InvSubBytes sub1 (Shift, Sub);
    wire [127:0] Add;
    AddRoundKey add1 (Sub,selected_key,Add);
    wire [127:0] Mix;
    InvMixColumns mix1 (Add, Mix);
    
    initial begin
        State = Idle;
        round = 0;
        o_Data = 0;
        Data_reg = 0;
        Complete_Flag = 0;
        Last_Command = 0;
    end
    
    always @(posedge Clock)begin
        if ((Last_Command == 0) & (Command == 1))begin
            State = Decrypt;
        end
        Last_Command = Command;
        case (State)
            Decrypt: begin
                case (round)
                    0: selected_key = Round9; //Doesnt take effect until next clock cycle
                    1: selected_key = Round8;
                    2: selected_key = Round7;
                    3: selected_key = Round6;
                    4: selected_key = Round5;
                    5: selected_key = Round4;
                    6: selected_key = Round3;
                    7: selected_key = Round2;
                    8: selected_key = Round1;
                    9: selected_key = Key;
                    default: selected_key = Round9;
                endcase
                if (round == 0) begin
                    //Start
                    Complete_Flag = 0; //Reset Complete_Flag
                    //if ( Key_reg != Key) //If key has changed. Process round keys
                    //    Key_reg = Key; 
                    //else begin
                        Data_reg = Add0; //Add round key
                        round = round + 1;
                    //end
                    end
                else if(round < 10) begin
                    //Normal Round
                    Data_reg = Mix; //Do normal round
                    round = round + 1;
                    end
                else begin
                    //Last Round
                    Complete_Flag = 1;
                    State = Idle;
                    o_Data = Add; //Skip Mix in last round
                    round = 0;
                end
            end
            default: begin 
                round = 0;
            end
        endcase
    end
endmodule
