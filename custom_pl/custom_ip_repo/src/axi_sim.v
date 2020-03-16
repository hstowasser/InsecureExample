`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 02/26/2020 08:04:33 AM
// Design Name: 
// Module Name: axi_test
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


module axi_test();
//clock and reset_n signals
	reg aclk = 1'b1;
	reg bclk = 1'b1;
	reg cclk = 1'b1;
	reg arstn = 1'b0;

	
	//Write Address channel (AW)
	reg [31:0] write_addr =32'd0;	//Master write address
	reg [2:0] write_prot = 3'd0;	//type of write(leave at 0)
	reg write_addr_valid = 1'b0;	//master indicating address is valid
	wire write_addr_ready;		//slave ready to receive address

	//Write Data Channel (W)
	reg [31:0] write_data = 32'd0;	//Master write data
	reg [3:0] write_strb = 4'd0;	//Master byte-wise write strobe
	reg write_data_valid = 1'b0;	//Master indicating write data is valid
	wire write_data_ready;		//slave ready to receive data

	//Write Response Channel (WR)
	reg write_resp_ready = 1'b0;	//Master ready to receive write response
	wire [1:0] write_resp;		//slave write response
	wire write_resp_valid;		//slave response valid
	
	//Read Address channel (AR)
	reg [31:0] read_addr = 32'd0;	//Master read address
	reg [2:0] read_prot =3'd0;	//type of read(leave at 0)
	reg read_addr_valid = 1'b0;	//Master indicating address is valid
	wire read_addr_ready;		//slave ready to receive address

	//Read Data Channel (R)
	reg read_data_ready = 1'b0;	//Master indicating ready to receive data
	wire [31:0] read_data;		//slave read data
	wire [1:0] read_resp;		//slave read response
	wire read_data_valid;		//slave indicating data in channel is valid
	

	//Instantiation of LED IP
	FrankenCrypt_v1_0_S00_AXI # (
		.C_S_AXI_DATA_WIDTH(32),
		.C_S_AXI_ADDR_WIDTH(5)
	) FrankenCrypt_v1_0_S00_AXI_inst (
		.S_AXI_ACLK(aclk),
		.S_AXI_ARESETN(arstn),

		.S_AXI_AWADDR(write_addr),
		.S_AXI_AWPROT(write_prot),
		.S_AXI_AWVALID(write_addr_valid),
		.S_AXI_AWREADY(write_addr_ready),

		.S_AXI_WDATA(write_data),
		.S_AXI_WSTRB(write_strb),
		.S_AXI_WVALID(write_data_valid),
		.S_AXI_WREADY(write_data_ready),

		.S_AXI_BRESP(write_resp),
		.S_AXI_BVALID(write_resp_valid),
		.S_AXI_BREADY(write_resp_ready),

		.S_AXI_ARADDR(read_addr),
		.S_AXI_ARPROT(read_prot),
		.S_AXI_ARVALID(read_addr_valid),
		.S_AXI_ARREADY(read_addr_ready),

		.S_AXI_RDATA(read_data),
		.S_AXI_RRESP(read_resp),
		.S_AXI_RVALID(read_data_valid),
		.S_AXI_RREADY(read_data_ready),
		.clk_50Mhz(bclk),
		.clk_200Mhz(cclk)
	);
	
	reg [31:0] data_out;
	//clock signal
    always
        #5 aclk <=~aclk; // 100 MHz
    always
        #10 bclk <=~bclk; // 50 MHz
    always
        #2.5 cclk <=~cclk; // 200 MHz
    
    integer i = 10;	
    integer ct;
    reg [511 : 0] tc1;
    reg [255 : 0] res1;
    reg [1024-1:0] A;
    reg [1024-1:0] B;
    reg [1024-1:0] N;
    initial
    begin

        arstn = 0;
        i=0;
        #20 arstn=1;
        #20
        
//        tc1 = 512'h61626380000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000018;
//        res1 = 256'hBA7816BF8F01CFEA414140DE5DAE2223B00361A396177A9CB410FF61F20015AD;
        
//        #20 axi_write(8,0); //Setup Address
//        for( i = 15; i >= 0; i=i-1)
//            #20 axi_write(12, tc1[i*32+:32] );    //write to Data
//        #20 axi_write(4,2147483649); //Trigger SHA init
        
//        #2000
//        #20 axi_write(8,44); //Setup Address
//        for( i = 15; i >= 0; i=i-1)
//            #20 axi_read(12);    //write to Data
        
//        #100
        
        
        
        
        //#20 axi_write(0,1);    //write 1 to slv_reg0
        
        //#20 axi_write(4,2);    //write 2 to slv_reg1
        //#20 axi_read(4);    //read 2 to slv_reg1
        
//        #20 axi_write(8,7);    //write 3 to Address
//        #20 axi_write(12,2);    //write 2 to Data
//        #20 axi_write(12,3);    //write 2 to Data
        
//        #20 axi_write(8,7);    //write 3 to Address
//        #20 axi_read(12);    //read from Data
//        #20 axi_read(12);    //read from Data
        
//        #20 axi_write(4,1073741824); //Trigger AES


            // RSA TEST VECTOR
//          #20 axi_write(8,31);    //write to Address
//          #20 axi_write(12,10);    //write to Data
          
//          #20 axi_write(8,63);    //write to Address
//          #20 axi_write(12,89);    //write to Data
          
//          #20 axi_write(8,95);    //write to Address
//          #20 axi_write(12,31);    //write to Data

//          #20 axi_write(8,0);    //write to Address
//          #20 axi_write(12,10);    //write to Data
          
//          #20 axi_write(8,32);    //write to Address
//          #20 axi_write(12,89);    //write to Data
          
//          #20 axi_write(8,65);    //write to Address
//          #20 axi_write(12,31);    //write to Data
    
        //Generate Signature
//        A = 1024'h00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000831ce81db1d7c2929d10f0de2fca1ee5fea3374cd17194841a36f5ea8ce29184e786be771e55746190c33a3d290beccbecd6e4fc08abacfe2c95793592276238;
//        B = 1024'hb4ec3dd043e232f92ad5be80397fc17e4c2c5de6254bf13b02c241b60aec60443bedb128151a98f4d598022d2fa1add387d88e259c36c3c90cb0f94f59ca7818e62cc19643e108cee7248d47445e13e2b7d07d5a22ea44579fc7d0ca1c5564601ca9d856cf48c5600243cf8e4517b70b2bfea0356e03563b9da1d53db8ae2701;
//        A = 1024'h30928553adaa0ee60d78ffacf0793aa4da4f896877cd8692ea9fdba4883859cf612d60e6d28e62e2fffa9cfb0931b6429c9d06a5316f197d9f5dc8fdd0a70901b5ea259f2908182010e7392ddd5a1a43f010048d8bc63932c26571f32b2c976c67f3343c01c28e20208ff165dbf214069b30a1c45b487084a9c40f81803d99fe;
       
       
        //A = 1024'h5527730f9803ff41739051aefb47583bc802aa78163da1a17f9e9c3b6142e4cd6629fdf14adb1855640ba098012d94f22c34f859002f68e2d2d56947e75458c8e58a43e70005bd2fc9fd8e99e8e29ec44c09c4d0c787725c1c296e6cd11931a801e8a7db4ad770724112686ba59a943f808d84c747f87c9ca675e7f6585b606c;
        A = 1024'h330a757f49b407c576b0e7b8f64cbc15eb002cc2002aad58228d1170c61022f7b91ccdbeca30adc1e08e7bd3c6a422c3b07414a54ba8c166abefb314db68a3d33b788ec7c96e32fd3a2ca0222cf5cd61eab77793c3862a8af3301631635d2735e79cbaf8d9c6409393b9fa319a2b502e554ea1c01642b643eefd9a2bb31c2c8e;
        B = 1024'h0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000010001;
        N = 1024'ha3acd56a4ba3634cac7e13ed5dd2728d2b37d851ab5486c6eb848789e17b6e6f5ab0b848e5fe914accee925c0e854cfc91bb78ee3b88590eba7856a39f1de0fe23f4a1ad339b46678988db847e39fcbcc87c7607422226a08de083f9509687edff03568f7bbac574a3573bd850c48ab70f7db4582fbdc9c1a51d0f60a7090c13;


//        N = 1024'hbd7e2e2cf3fe8190ff06a32864835727cf709e8da49a3ba76684d808b99ed4866c06391cceaee01b4a6b2e4e243c42eff4fc9dd4f209c72c13a13907639fe5932931e7e9cbc77752c1ee741b80a391bb1a5d518dba8538a7fee374311d521dad2c67c9abb52d7d7f910558998012f44a8734b059d0397d4d059d6c7cd2f3a941;
//        A = 50;
//        B = 11;
//        N = 12;

        //Test Case compare equal temp N
//        A = 1024'h00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000831ce81db1d7c2929d10f0de2fca1ee5fea3374cd17194841a36f5ea8ce29184e786be771e55746190c33a3d290beccbecd6e4fc08abacfe2c95793592276238;
//        B = 1024'hbd7e2e2cf3fe8190ff06a32864835727cf709e8da49a3ba76684d808b99ed4866c06391cceaee01b4a6b2e4e243c42eff4fc9dd4f209c72c13a13907639fe5932931e7e9cbc77752c1ee741b80a391bb1a5d518dba8538a7fee374311d521dad2c67c9abb52d7d7f910558998012f44a8734b059d0397d4d059d6c7cd2f3a941;
//        N = 1024'hbd7e2e2cf3fe8190ff06a32864835727cf709e8da49a3ba76684d808b99ed4866c06391cceaee01b4a6b2e4e243c42eff4fc9dd4f209c72c13a13907639fe5932931e7e9cbc77752c1ee741b80a391bb1a5d518dba8538a7fee374311d521dad2c67c9abb52d7d7f910558998012f44a8734b059d0397d4d059d6c7cd2f3a941;
    
        #20 
        #20 axi_write(8,0); //Setup Address
        for( i = 31; i >= 0; i=i-1)
            #20 axi_write(12, B[i*32+:32] );    //write to Data
        for( i = 31; i >= 0; i=i-1)
            #20 axi_write(12, A[i*32+:32] );    //write to Data
        for( i = 31; i >= 0; i=i-1)
            #20 axi_write(12, N[i*32+:32] );    //write to Data
        #20 axi_write(4,3221225473); //Trigger RSA
        
        //#20 axi_write(8,127);    //write 3 to Address
        //#20 axi_read(12);    //read from Data
        

        #2000;
       // $finish;
    end
    
    task axi_write;
        input [31:0] addr;
        input [31:0] data;
        begin
            #3 write_addr <= addr;    //Put write address on bus
            write_data <= data;    //put write data on bus
            write_addr_valid <= 1'b1;    //indicate address is valid
            write_data_valid <= 1'b1;    //indicate data is valid
            write_resp_ready <= 1'b1;    //indicate ready for a response
            write_strb <= 4'hF;        //writing all 4 bytes
    
            //wait for one slave ready signal or the other
            wait(write_data_ready || write_addr_ready);
                
            @(posedge aclk); //one or both signals and a positive edge
            if(write_data_ready&&write_addr_ready)//received both ready signals
            begin
                write_addr_valid<=0;
                write_data_valid<=0;
            end
            else    //wait for the other signal and a positive edge
            begin
                if(write_data_ready)    //case data handshake completed
                begin
                    write_data_valid<=0;
                    wait(write_addr_ready); //wait for address address ready
                end
                        else if(write_addr_ready)   //case address handshake completed
                        begin
                    write_addr_valid<=0;
                            wait(write_data_ready); //wait for data ready
                        end 
                @ (posedge aclk);// complete the second handshake
                write_addr_valid<=0; //make sure both valid signals are deasserted
                write_data_valid<=0;
            end
                
            //both handshakes have occured
            //deassert strobe
            write_strb<=0;
    
            //wait for valid response
            wait(write_resp_valid);
            
            //both handshake signals and rising edge
            @(posedge aclk);
    
            //deassert ready for response
            write_resp_ready<=0;
    
    
            //end of write transaction
        end
        endtask
        
        
        task axi_read;
                input [31:0] addr;
                begin
                    //#3 write_addr <= addr;    //Put write address on bus
                    #3 read_addr <= addr;    //Put read address on bus
                    //write_data <= data;    //put write data on bus
                    read_addr_valid <= 1'b1;    //indicate address is valid
                    //write_data_valid <= 1'b1;    //indicate data is valid
                    read_data_ready <= 1'b1;    //indicate ready for a response
                    //write_strb <= 4'hF;        //writing all 4 bytes
            
                    //wait for one slave ready signal or the other
                    //wait(write_data_ready || write_addr_ready);
                    wait(read_addr_ready);  
                        
                    @(posedge aclk); //one or both signals and a positive edge
                    if(read_addr_ready)//received both ready signals
                    begin
                        read_addr_valid<=0;
                        //read_data_ready<=0;
                    end
                    
                    @(posedge aclk);
                    if(read_data_valid)    //case data handshake completed
                    begin
                        data_out <= read_data;
                    end
                    
                    //both handshake signals and rising edge
                    @(posedge aclk);
                    read_data_ready<=0; //deassert ready for response
            
            
                    //end of write transaction
                end
                endtask
endmodule
