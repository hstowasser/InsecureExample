`timescale 1ns / 1ps

/*
module Comparator(
    input wire reset,
    input wire clk,
    input wire go,
    input wire [1023:0] temp, //Note that temp is smaller than ABmodN temp
    input wire [1023:0] N,
    output reg temp_less_than_N,
    output reg finished
    );
    
    parameter chunk_sz = 128; // Must divide 1024
    parameter chunk_ct = 1024/chunk_sz; // 1024/16
    
    reg state;
    parameter Stop = 0;
    parameter Compare = 1;

    reg [chunk_sz-1:0] temp_Chunk;
    reg [chunk_sz-1:0] N_Chunk;

    integer i;
    //reg [10:0] i;
    reg last_go;
    
    always @(posedge clk) begin //, negedge reset) begin    
        if (reset == 0) begin
            state <= Stop;
            last_go <= 0;
            temp_Chunk <= 0;
            N_Chunk <= 0;
            finished <= 0;
            temp_less_than_N <= 0;
        end
        else begin
            case (state) 
                Stop: begin 
                    //i <= 1;
                    if( go == 1 && last_go == 0) begin
                        state <= Compare;
                        finished <= 0;
                        temp_Chunk <= temp[(chunk_ct - 1)*chunk_sz +: chunk_sz];
                        N_Chunk <= N[(chunk_ct - 1)*chunk_sz +: chunk_sz];
                        i <= 2;
                    end
                    last_go <= go;
                end
                Compare: begin
                    if( temp_Chunk < N_Chunk ) begin
                        //temp < N true
                        finished <= 1;
                        temp_less_than_N <= 1;
                        state <= Stop;
                    end
                    else if( temp_Chunk == N_Chunk ) begin
                        //temp < N unknown
                        if( i > chunk_ct ) begin
                            // if last chunk is also equal. temp == N. return false
                            finished <= 1;
                            temp_less_than_N <= 0;
                            state <= Stop;
                        end
                        else begin
                            // else check next chunk
                            temp_Chunk <= temp[(chunk_ct - i)*chunk_sz +: chunk_sz];
                            N_Chunk <= N[(chunk_ct - i)*chunk_sz +: chunk_sz];
                            i <= i + 1;
                        end
                    end
                    else begin
                        // temp < N false
                        finished <= 1;
                        temp_less_than_N <= 0;
                        state <= Stop;
                    end
                end
            endcase
        end
    end
endmodule
*/

module Comparator(
    input wire reset,
    input wire clk,
    input wire go,
    input wire [1023:0] temp, //Note that temp is smaller than ABmodN temp
    input wire [1023:0] N,
    output reg temp_less_than_N,
    output reg finished
    );
    
    parameter chunk_sz = 128; // Must divide 1024
    parameter chunk_ct = 1024/chunk_sz; // 1024/16
    
    reg [10:0] state;
    parameter Stop = 0;
    parameter Compare = 1;

    reg [chunk_sz-1:0] temp_Chunk;
    reg [chunk_sz-1:0] N_Chunk;

    integer i;
    reg last_go;
    
    always @(posedge clk) begin //, negedge reset) begin    
        if (reset == 0) begin
            state <= Stop;
            last_go <= 0;
            temp_Chunk <= 0;
            N_Chunk <= 0;
            finished <= 0;
            temp_less_than_N <= 0;
        end
        else begin
            case (state) 
                0: begin 
                    if( go == 1 && last_go == 0) begin
                        if( temp[(chunk_ct - 1)*chunk_sz +: chunk_sz] < N[(chunk_ct - 1)*chunk_sz +: chunk_sz] ) begin
                            finished <= 1;
                            temp_less_than_N <= 1;
                            state <= 0;
                        end
                        else if( temp[(chunk_ct - 1)*chunk_sz +: chunk_sz] == N[(chunk_ct - 1)*chunk_sz +: chunk_sz] ) begin
                            finished <= 0;
                            state <= 1;
                        end 
                        else begin  
                            finished <= 1;
                            temp_less_than_N <= 0;
                            state = 0;
                        end
                    end
                    last_go <= go;
                end
                1: begin
                    if( temp[(chunk_ct - 2)*chunk_sz +: chunk_sz] < N[(chunk_ct - 2)*chunk_sz +: chunk_sz] ) begin
                        finished <= 1;
                        temp_less_than_N <= 1;
                        state <= 0;
                    end
                    else if( temp[(chunk_ct - 2)*chunk_sz +: chunk_sz] == N[(chunk_ct - 2)*chunk_sz +: chunk_sz] ) begin                        
                        finished <= 0;
                        state = 2;
                    end 
                    else begin  
                        finished <= 1;
                        temp_less_than_N <= 0;
                        state <= 0;
                    end
                end
                2: begin
                    if( temp[(chunk_ct - 3)*chunk_sz +: chunk_sz] < N[(chunk_ct - 3)*chunk_sz +: chunk_sz] ) begin
                        finished <= 1;
                        temp_less_than_N <= 1;
                        state <= 0;
                    end
                    else if( temp[(chunk_ct - 3)*chunk_sz +: chunk_sz] == N[(chunk_ct - 3)*chunk_sz +: chunk_sz] ) begin
                         finished <= 0;
                         state = 3;
                    end 
                    else begin  
                        finished <= 1;
                        temp_less_than_N <= 0;
                        state <= 0;
                    end
                end
                3: begin
                    if( temp[(chunk_ct - 4)*chunk_sz +: chunk_sz] < N[(chunk_ct - 4)*chunk_sz +: chunk_sz] ) begin
                        finished <= 1;
                        temp_less_than_N <= 1;
                        state <= 0;
                    end
                    else if( temp[(chunk_ct - 4)*chunk_sz +: chunk_sz] == N[(chunk_ct - 4)*chunk_sz +: chunk_sz] ) begin
                        finished <= 0;
                        state <= 4;
                    end 
                    else begin  
                        finished <= 1;
                        temp_less_than_N <= 0;
                        state <= 0;
                    end
                end
                4: begin
                    if( temp[(chunk_ct - 5)*chunk_sz +: chunk_sz] < N[(chunk_ct - 5)*chunk_sz +: chunk_sz] ) begin
                        finished <= 1;
                        temp_less_than_N <= 1;
                        state <= 0;
                    end
                    else if( temp[(chunk_ct - 5)*chunk_sz +: chunk_sz] == N[(chunk_ct - 5)*chunk_sz +: chunk_sz] ) begin
                        finished <= 0;
                        state <= 5;
                    end 
                    else begin  
                        finished <= 1;
                        temp_less_than_N <= 0;
                        state <= 0;
                    end
                end
                5: begin
                    if( temp[(chunk_ct - 6)*chunk_sz +: chunk_sz] < N[(chunk_ct - 6)*chunk_sz +: chunk_sz] ) begin
                        finished <= 1;
                        temp_less_than_N <= 1;
                        state <= 0;
                    end
                    else if( temp[(chunk_ct - 6)*chunk_sz +: chunk_sz] == N[(chunk_ct - 6)*chunk_sz +: chunk_sz] ) begin
                        finished <= 0;
                        state <= 6;
                    end 
                    else begin  
                        finished <= 1;
                        temp_less_than_N <= 0;
                        state <= 0;
                    end
                end
                6: begin
                    if( temp[(chunk_ct - 7)*chunk_sz +: chunk_sz] < N[(chunk_ct - 7)*chunk_sz +: chunk_sz] ) begin
                        finished <= 1;
                        temp_less_than_N <= 1;
                        state <= 0;
                    end
                    else if( temp[(chunk_ct - 7)*chunk_sz +: chunk_sz] == N[(chunk_ct - 7)*chunk_sz +: chunk_sz] ) begin
                        finished <= 0;
                        state <= 7;
                    end 
                    else begin  
                        finished <= 1;
                        temp_less_than_N <= 0;
                        state <= 0;
                    end
                end
                7: begin
                    if( temp[(chunk_ct - 8)*chunk_sz +: chunk_sz] < N[(chunk_ct - 8)*chunk_sz +: chunk_sz] ) begin
                        finished <= 1;
                        temp_less_than_N <= 1;
                        state <= 0;
                    end
                    else if( temp[(chunk_ct - 8)*chunk_sz +: chunk_sz] == N[(chunk_ct - 8)*chunk_sz +: chunk_sz] ) begin
                        finished <= 1;
                        temp_less_than_N <= 0;
                        state <= 0;
                    end 
                    else begin  
                        finished <= 1;
                        temp_less_than_N <= 0;
                        state <= 0;
                    end
                end
            endcase
        end
    end
endmodule



module ABmodN(
    input wire reset,
    input wire clk_slow,
    input wire clk_fast,
    input wire go,
    input wire [1023:0] A,
    input wire [1023:0] B,
    input wire [1023:0] N,
    output wire [1023:0] out,
    output reg finished
    //,
    //output wire [2:0] debug_state
    );
    
    reg last_go;
    //wire [1023:0] temp_squared;
    reg [1023+2:0] temp;
    reg [1023:0] A_hold;
    
    reg [2:0] state; // 0 = stop, 1 = go
    //assign debug_state = state;
    //reg [1:0] last_state = 0;
    assign out = temp[1023:0];
    //assign temp_squared = temp << 1;
    
    wire [1023+2:0] add_out;
    wire Cout_unused;
    reg enable_adder;
    wire adder_complete;

    parameter Stop = 0;
    parameter Run = 1;
    parameter Add = 2;
    parameter Sub = 3;
    parameter Cmp = 4; //Compare
    

     
    wire [1023:0] input_B;
    assign input_B = (state == Add) ? B : N; // input_B must be N also when state == cmp
    
    reg add_state; // 0 = subtract, 1 = add 
    //assign add_state = (state == Sub) ? 0 : 1;
    
    
    
    adder_subtractor_multi_clock add_sub(
        .reset(reset),
        .clk(clk_slow),
        .A(temp),
        .B(input_B),
        .S(add_out),
        .add(add_state),
        .Go(enable_adder),
        .Complete(adder_complete),
        .Ci(0),
        .Co(Cout_unused)
    );
//    add_sub adder_subtractor(
//        .reset(reset),
//        .clk(clk_fast),
//        .A(temp),
//        .B(input_B),
//        .S(add_out),
//        .add(add_state),
//        .Go(enable_adder),
//        .Complete(adder_complete)
//    );
    
    reg run_comparator;
    wire temp_less_than_N;
    wire comparator_finished;
    
    Comparator compare(
        .reset(reset),
        .clk(clk_slow),
        .go(run_comparator),
        .temp(temp[1023:0]), //Note that temp is smaller than ABmodN temp
        .N(N),
        .temp_less_than_N(temp_less_than_N),
        .finished(comparator_finished)
        );
    
    //integer i = 0;
    reg [10:0] i;
    reg delay;
    
    always @(posedge clk_slow ) begin//, negedge reset) begin
        if (reset == 0) begin
            enable_adder <= 0;
            state <= Stop;
            finished <= 1;
            temp <= 0;
            last_go <= 0;
            add_state <= 1; //Add
            run_comparator <= 0;
            delay <= 0;
        end
        else begin
            case (state) 
                Stop: begin 
                    enable_adder <= 0;
                    i <= 0;
                    if( go == 1 && last_go == 0) begin
                        temp <= 0;
                        A_hold <= A;
                        state <= Run;
                        finished <= 0;
                        run_comparator <= 0;
                    end
                    last_go <= go;
                end
                Run: begin
                    if( A_hold[1023] == 1)
                    begin
                        state <= Add; 
                        enable_adder <= 1;  
                        add_state <= 1; //Add           
                    end
                    else begin 
                        //state <= Sub;
                        if( temp[1025:1024] > 0) begin // If these bits are set temp is definetly greater than N
                            state <= Sub; //Do subtraction
                            run_comparator <= 0;
                            enable_adder <= 1; // Check if this does anything
                        end 
                        else begin
                            state <= Cmp;
                            enable_adder <= 1;
                        end
                        add_state <= 0; //Subtract 
                    end
                end
                Add: begin 
                    if ( adder_complete == 1) begin
                        enable_adder <= 0;
                        temp <= add_out;
                        //state <= Sub;
                        add_state <= 0; //Subtract 
                        state <= Cmp;
                        run_comparator <= 1;
                    end
                end
                Sub: begin
                    if ( adder_complete == 1) begin
                        temp <= add_out;
                        enable_adder <= 0;
                        // Go back to Cmp because we may have to subtract multiple times
                        state <= Cmp;
                        run_comparator <= 1;
                        
                    end else begin
                        enable_adder <= 1;
                    end
                end
                Cmp: begin
                    if ( enable_adder != 1) begin
                        enable_adder <= 1; //Begin subtraction
                    end else begin
                        if ( temp[1025:1024] > 0 ) begin // If these bits are set temp is definetly greater than N
                            state <= Sub; //Do subtraction
                            run_comparator <= 0;
                        end 
                        else if ( run_comparator == 0 ) begin
                            run_comparator <= 1;
                            delay <= 1;
                        end 
                        else if ( delay == 1) begin
                            delay <= 0;
                        end
                        else if ( comparator_finished == 1) begin
                            run_comparator <= 0;
                            if ( temp_less_than_N == 1) begin
                                //Cancel Subtraction
                                if( i >= 1023) begin
                                    state <= Stop;
                                    finished <= 1;
                                end
                                else begin
                                    // Cancel Subtraction
                                    i <= i + 1;
                                    A_hold <= A_hold << 1;
                                    temp = temp << 1;
                                    state <= Run;
                                    
                                    //Reset the delay and comparison trigger                        
                                    enable_adder <= 0;
                                end
                            end
                            else begin
                                //Complete Subtraction
                                state <= Sub;
                            end
                        end
                    end
                end
                default: begin state <= Stop; end
            endcase
        end
    end
    
endmodule

/*
module ABmodN(
    input wire clk_slow,
    input wire clk_fast,
    input wire go,
    input wire [1023:0] A,
    input wire [1023:0] B,
    input wire [1023:0] N,
    output wire [1023:0] out,
    output reg finished
    );
    
    reg half_clock = 0;
    reg last_go = 0;
    //wire [1023:0] temp_squared;
    reg [1023+2:0] temp;
    reg [1023:0] A_hold;
    
    reg [1:0] state = 0; // 0 = stop, 1 = go
    //reg [1:0] last_state = 0;
    assign out = temp[1023:0];
    //assign temp_squared = temp << 1;
    
    wire [1023+2:0] add_out;
    wire Cout_unused;
    reg enable_adder = 0;
    wire adder_complete;

    parameter Stop = 0;
    parameter Run = 1;
    parameter Add = 2;
    parameter Sub = 3;
    

    //reg [1023:0] input_B;
    wire [1023:0] input_B;
    assign input_B = (state == Sub) ? N : B;
    wire add_state; // 0 = subtract, 1 = add 
    assign add_state = (state == Sub) ? 0 : 1;
    
    
    reg [1:0] delay = 0;

    reg temp_gr_eq_N;
    //assign temp_gr_eq_N = (temp >= N);
    // wire temp_lsh_gr_eq_N;
    // assign temp_lsh_gr_eq_N = ((stupid_holder << 1) >= N);

    adder_subtractor_multi_clock add_sub(
        .clk(clk_fast),
        .A(temp),
        .B(input_B),
        .S(add_out),
        .add(add_state),
        .Go(enable_adder),
        .Complete(adder_complete),
        .Ci(0),
        .Co(Cout_unused)
    );
    
    integer i = 0;
    
    initial begin
        enable_adder <= 0;
        state <= Stop;
        finished <= 1;
        temp <= 0;
    end
    
    //always @(posedge clk_slow) begin
        //half_clock = ~half_clock;
    //end
    
    always @(posedge half_clock) begin
        temp_gr_eq_N = (temp[1023+2:0] >= N[1023:0]);
    end
    
    always @(posedge clk_slow) begin
        case (state) 
            Stop: begin 
                enable_adder <= 0;
                i <= 0;
                delay <= 3;
                half_clock <= 0;
                if( go == 1 && last_go == 0) begin
                    temp <= 0;
                    A_hold <= A;
                    state <= Run;
                    finished <= 0;
                end
                last_go <= go;
            end
            Run: begin
                if( A_hold[1023] == 1)
                begin
                    state <= Add; 
                    enable_adder <= 1;   
                    //input_B <= B;                
                end
                else begin 
                    state <= Sub;
                    //input_B <= N;
                end
                
                half_clock <= 0; 
            end
            Add: begin 
                if ( adder_complete == 1) begin
                    enable_adder <= 0;
                    temp <= add_out;
                    state <= Sub;
                    //input_B <= N;
                end
            end
            Sub: begin
                if (delay == 3) begin
                    half_clock <= 1; //Trigger greater than check
                    delay <= 2;
                    enable_adder <= 1; //Begin subtraction
                end 
                else if (delay == 2) begin
                    delay <= 1;
                end
                else if (delay == 1) begin
                    delay <= 0;
                end
                else if( temp_gr_eq_N == 0 ) begin // Check if subtraction is necessary
                    if( i >= 1023) begin
                        state <= Stop;
                        finished <= 1;
                    end 
                    else begin
                        // Cancel Subtraction
                        i <= i + 1;
                        A_hold <= A_hold << 1;
                        temp = temp << 1;
                        state <= Run;
                        
                        //Reset the delay and comparison trigger
                        half_clock <= 0;
                        delay <= 3;
                        enable_adder <= 0;
                    end
                end               
                else if ( adder_complete == 1) begin
                    temp <= add_out;
                    enable_adder <= 0;
                    delay <= 3;
                    half_clock <= 0; 
                    // Don't change states because we may have to subtract multiple times
                end
            end
            default: begin state <= Stop; end
        endcase
    end
    
endmodule
*/