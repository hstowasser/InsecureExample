
`timescale 1 ns / 1 ps

	module FrankenCrypt_v1_0_S00_AXI #
	(
		// Users to add parameters here

		// User parameters ends
		// Do not modify the parameters beyond this line

		// Width of S_AXI data bus
		parameter integer C_S_AXI_DATA_WIDTH	= 32,
		// Width of S_AXI address bus
		parameter integer C_S_AXI_ADDR_WIDTH	= 5
	)
	(
		// Users to add ports here
        input wire clk_50Mhz,
        input wire clk_200Mhz,
		// User ports ends
		// Do not modify the ports beyond this line

		// Global Clock Signal
		input wire  S_AXI_ACLK,
		// Global Reset Signal. This Signal is Active LOW
		input wire  S_AXI_ARESETN,
		// Write address (issued by master, acceped by Slave)
		input wire [C_S_AXI_ADDR_WIDTH-1 : 0] S_AXI_AWADDR,
		// Write channel Protection type. This signal indicates the
    		// privilege and security level of the transaction, and whether
    		// the transaction is a data access or an instruction access.
		input wire [2 : 0] S_AXI_AWPROT,
		// Write address valid. This signal indicates that the master signaling
    		// valid write address and control information.
		input wire  S_AXI_AWVALID,
		// Write address ready. This signal indicates that the slave is ready
    		// to accept an address and associated control signals.
		output wire  S_AXI_AWREADY,
		// Write data (issued by master, acceped by Slave) 
		input wire [C_S_AXI_DATA_WIDTH-1 : 0] S_AXI_WDATA,
		// Write strobes. This signal indicates which byte lanes hold
    		// valid data. There is one write strobe bit for each eight
    		// bits of the write data bus.    
		input wire [(C_S_AXI_DATA_WIDTH/8)-1 : 0] S_AXI_WSTRB,
		// Write valid. This signal indicates that valid write
    		// data and strobes are available.
		input wire  S_AXI_WVALID,
		// Write ready. This signal indicates that the slave
    		// can accept the write data.
		output wire  S_AXI_WREADY,
		// Write response. This signal indicates the status
    		// of the write transaction.
		output wire [1 : 0] S_AXI_BRESP,
		// Write response valid. This signal indicates that the channel
    		// is signaling a valid write response.
		output wire  S_AXI_BVALID,
		// Response ready. This signal indicates that the master
    		// can accept a write response.
		input wire  S_AXI_BREADY,
		// Read address (issued by master, acceped by Slave)
		input wire [C_S_AXI_ADDR_WIDTH-1 : 0] S_AXI_ARADDR,
		// Protection type. This signal indicates the privilege
    		// and security level of the transaction, and whether the
    		// transaction is a data access or an instruction access.
		input wire [2 : 0] S_AXI_ARPROT,
		// Read address valid. This signal indicates that the channel
    		// is signaling valid read address and control information.
		input wire  S_AXI_ARVALID,
		// Read address ready. This signal indicates that the slave is
    		// ready to accept an address and associated control signals.
		output wire  S_AXI_ARREADY,
		// Read data (issued by slave)
		output wire [C_S_AXI_DATA_WIDTH-1 : 0] S_AXI_RDATA,
		// Read response. This signal indicates the status of the
    		// read transfer.
		output wire [1 : 0] S_AXI_RRESP,
		// Read valid. This signal indicates that the channel is
    		// signaling the required read data.
		output wire  S_AXI_RVALID,
		// Read ready. This signal indicates that the master can
    		// accept the read data and response information.
		input wire  S_AXI_RREADY
	);

	// AXI4LITE signals
	reg [C_S_AXI_ADDR_WIDTH-1 : 0] 	axi_awaddr;
	reg  	axi_awready;
	reg  	axi_wready;
	reg [1 : 0] 	axi_bresp;
	reg  	axi_bvalid;
	reg [C_S_AXI_ADDR_WIDTH-1 : 0] 	axi_araddr;
	reg  	axi_arready;
	reg [C_S_AXI_DATA_WIDTH-1 : 0] 	axi_rdata;
	reg [1 : 0] 	axi_rresp;
	reg  	axi_rvalid;

	// Example-specific design signals
	// local parameter for addressing 32 bit / 64 bit C_S_AXI_DATA_WIDTH
	// ADDR_LSB is used for addressing 32/64 bit registers/memories
	// ADDR_LSB = 2 for 32 bits (n downto 2)
	// ADDR_LSB = 3 for 64 bits (n downto 3)
	localparam integer ADDR_LSB = (C_S_AXI_DATA_WIDTH/32) + 1;
	localparam integer OPT_MEM_ADDR_BITS = 2;
	//----------------------------------------------
	//-- Signals for user logic register space example
	//------------------------------------------------
    reg [1023:0] A = 0;
    reg [1023:0] B = 0;
    reg [1023:0] N = 0;
    wire [1023:0] P;
	reg Command_Write = 0;
	reg Address_Write = 0;
	reg Data_Read = 0;
	reg Data_Write = 0;
	
    //reg [127:0] DataIn; 
    
    //reg [127:0] DataOutHolder;
    reg [255:0] DataOutHolder;
    
    wire [127:0] DataOut; 
	
	
	reg [C_S_AXI_DATA_WIDTH-1:0] slv_reg_data_out;
	reg [C_S_AXI_DATA_WIDTH-1:0] Address;
	//-- Number of Slave Registers 5
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg0;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg1;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg2;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg3;
	reg [C_S_AXI_DATA_WIDTH-1:0]	slv_reg4;
	wire	 slv_reg_rden;
	wire	 slv_reg_wren;
	reg [C_S_AXI_DATA_WIDTH-1:0]	 reg_data_out;
	integer	 byte_index;
	reg	 aw_en;

	// I/O Connections assignments

	assign S_AXI_AWREADY	= axi_awready;
	assign S_AXI_WREADY	= axi_wready;
	assign S_AXI_BRESP	= axi_bresp;
	assign S_AXI_BVALID	= axi_bvalid;
	assign S_AXI_ARREADY	= axi_arready;
	assign S_AXI_RDATA	= axi_rdata;
	assign S_AXI_RRESP	= axi_rresp;
	assign S_AXI_RVALID	= axi_rvalid;
	// Implement axi_awready generation
	// axi_awready is asserted for one S_AXI_ACLK clock cycle when both
	// S_AXI_AWVALID and S_AXI_WVALID are asserted. axi_awready is
	// de-asserted when reset is low.

	always @( posedge S_AXI_ACLK )
	begin
	  if ( S_AXI_ARESETN == 1'b0 )
	    begin
	      axi_awready <= 1'b0;
	      aw_en <= 1'b1;
	    end 
	  else
	    begin    
	      if (~axi_awready && S_AXI_AWVALID && S_AXI_WVALID && aw_en)
	        begin
	          // slave is ready to accept write address when 
	          // there is a valid write address and write data
	          // on the write address and data bus. This design 
	          // expects no outstanding transactions. 
	          axi_awready <= 1'b1;
	          aw_en <= 1'b0;
	        end
	        else if (S_AXI_BREADY && axi_bvalid)
	            begin
	              aw_en <= 1'b1;
	              axi_awready <= 1'b0;
	            end
	      else           
	        begin
	          axi_awready <= 1'b0;
	        end
	    end 
	end       

	// Implement axi_awaddr latching
	// This process is used to latch the address when both 
	// S_AXI_AWVALID and S_AXI_WVALID are valid. 

	always @( posedge S_AXI_ACLK )
	begin
	  if ( S_AXI_ARESETN == 1'b0 )
	    begin
	      axi_awaddr <= 0;
	    end 
	  else
	    begin    
	      if (~axi_awready && S_AXI_AWVALID && S_AXI_WVALID && aw_en)
	        begin
	          // Write Address latching 
	          axi_awaddr <= S_AXI_AWADDR;
	        end
	    end 
	end       

	// Implement axi_wready generation
	// axi_wready is asserted for one S_AXI_ACLK clock cycle when both
	// S_AXI_AWVALID and S_AXI_WVALID are asserted. axi_wready is 
	// de-asserted when reset is low. 

	always @( posedge S_AXI_ACLK )
	begin
	  if ( S_AXI_ARESETN == 1'b0 )
	    begin
	      axi_wready <= 1'b0;
	    end 
	  else
	    begin    
	      if (~axi_wready && S_AXI_WVALID && S_AXI_AWVALID && aw_en )
	        begin
	          // slave is ready to accept write data when 
	          // there is a valid write address and write data
	          // on the write address and data bus. This design 
	          // expects no outstanding transactions. 
	          axi_wready <= 1'b1;
	        end
	      else
	        begin
	          axi_wready <= 1'b0;
	        end
	    end 
	end       

	// Implement memory mapped register select and write logic generation
	// The write data is accepted and written to memory mapped registers when
	// axi_awready, S_AXI_WVALID, axi_wready and S_AXI_WVALID are asserted. Write strobes are used to
	// select byte enables of slave registers while writing.
	// These registers are cleared when reset (active low) is applied.
	// Slave register write enable is asserted when valid address and data are available
	// and the slave is ready to accept the write address and write data.
	assign slv_reg_wren = axi_wready && S_AXI_WVALID && axi_awready && S_AXI_AWVALID;

	always @( posedge S_AXI_ACLK )
	begin
	  if ( S_AXI_ARESETN == 1'b0 )
	    begin
	      //slv_reg0 <= 0;
	      Command_Write <= 0;
	      Address_Write <= 0;
	      slv_reg1 <= 0;
	      slv_reg2 <= 0;
	      slv_reg3 <= 0;
	      Data_Write = 0;
	      slv_reg4 <= 0;
	    end 
	  else begin
	    if (slv_reg_wren)
	      begin
	        case ( axi_awaddr[ADDR_LSB+OPT_MEM_ADDR_BITS:ADDR_LSB] )
	          3'h0: begin end
//	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
//	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
//	                // Respective byte enables are asserted as per write strobes 
//	                // Slave register 0
//	                //slv_reg0[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
//	              end  
	          3'h1:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 1
	                slv_reg1[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	                Command_Write <= 1;
	              end  
	          3'h2:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 2
	                slv_reg2[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	                Address_Write <= 1;
	              end  
	          3'h3:begin
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 3
	                slv_reg3[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
                    Data_Write <= 1;
	              end  
	              case(Address)
                         0 : begin A[31*32+:32] <= S_AXI_WDATA;    end
                         1 : begin A[30*32+:32] <= S_AXI_WDATA;    end
                         2 : begin A[29*32+:32] <= S_AXI_WDATA;    end
                         3 : begin A[28*32+:32] <= S_AXI_WDATA;    end
                        
                         4 : begin A[27*32+:32] <= S_AXI_WDATA; end
                         5 : begin A[26*32+:32] <= S_AXI_WDATA; end
                         6 : begin A[25*32+:32] <= S_AXI_WDATA; end
                         7 : begin A[24*32+:32] <= S_AXI_WDATA; end
                        
                         8 : begin A[23*32+:32] <= S_AXI_WDATA; end
                         9 : begin A[22*32+:32] <= S_AXI_WDATA; end
                         10: begin A[21*32+:32] <= S_AXI_WDATA; end
                         11: begin A[20*32+:32] <= S_AXI_WDATA; end
                        
                         12: begin A[19*32+:32] <= S_AXI_WDATA; end
                         13: begin A[18*32+:32] <= S_AXI_WDATA; end
                         14: begin A[17*32+:32] <= S_AXI_WDATA; end
                         15: begin A[16*32+:32] <= S_AXI_WDATA; end
                        
                         16: begin A[15*32+:32] <= S_AXI_WDATA; end
                         17: begin A[14*32+:32] <= S_AXI_WDATA; end
                         18: begin A[13*32+:32] <= S_AXI_WDATA; end
                         19: begin A[12*32+:32] <= S_AXI_WDATA; end
                        
                         20: begin A[11*32+:32] <= S_AXI_WDATA; end
                         21: begin A[10*32+:32] <= S_AXI_WDATA; end
                         22: begin A[9*32+:32] <= S_AXI_WDATA; end
                         23: begin A[8*32+:32] <= S_AXI_WDATA; end
                        
                         24: begin A[7*32+:32] <= S_AXI_WDATA; end
                         25: begin A[6*32+:32] <= S_AXI_WDATA; end
                         26: begin A[5*32+:32] <= S_AXI_WDATA; end
                         27: begin A[4*32+:32] <= S_AXI_WDATA; end
                        
                         28: begin A[3*32+:32] <= S_AXI_WDATA; end
                         29: begin A[2*32+:32] <= S_AXI_WDATA; end
                         30: begin A[1*32+:32] <= S_AXI_WDATA; end
                         31: begin A[0*32+:32] <= S_AXI_WDATA; end
                        
                         32: begin B[31*32+:32] <= S_AXI_WDATA; end
                         33: begin B[30*32+:32] <= S_AXI_WDATA; end
                         34: begin B[29*32+:32] <= S_AXI_WDATA; end
                         35: begin B[28*32+:32] <= S_AXI_WDATA; end
                        
                         36: begin B[27*32+:32] <= S_AXI_WDATA; end
                         37: begin B[26*32+:32] <= S_AXI_WDATA; end
                         38: begin B[25*32+:32] <= S_AXI_WDATA; end
                         39: begin B[24*32+:32] <= S_AXI_WDATA; end
                        
                         40: begin B[23*32+:32] <= S_AXI_WDATA; end
                         41: begin B[22*32+:32] <= S_AXI_WDATA; end
                         42: begin B[21*32+:32] <= S_AXI_WDATA; end
                         43: begin B[20*32+:32] <= S_AXI_WDATA; end
                         
                         
                         44: begin B[19*32+:32] <= S_AXI_WDATA; end
                         45: begin B[18*32+:32] <= S_AXI_WDATA; end
                         46: begin B[17*32+:32] <= S_AXI_WDATA; end
                         47: begin B[16*32+:32] <= S_AXI_WDATA; end
                         
                         48: begin B[15*32+:32] <= S_AXI_WDATA; end
                         49: begin B[14*32+:32] <= S_AXI_WDATA; end
                         50: begin B[13*32+:32] <= S_AXI_WDATA; end
                         51: begin B[12*32+:32] <= S_AXI_WDATA; end
                         
                         52: begin B[11*32+:32] <= S_AXI_WDATA; end
                         53: begin B[10*32+:32] <= S_AXI_WDATA; end
                         54: begin B[9*32+:32] <= S_AXI_WDATA; end
                         55: begin B[8*32+:32] <= S_AXI_WDATA; end
                         
                         56: begin B[7*32+:32] <= S_AXI_WDATA; end
                         57: begin B[6*32+:32] <= S_AXI_WDATA; end
                         58: begin B[5*32+:32] <= S_AXI_WDATA; end
                         59: begin B[4*32+:32] <= S_AXI_WDATA; end
                         
                         60: begin B[3*32+:32] <= S_AXI_WDATA; end
                         61: begin B[2*32+:32] <= S_AXI_WDATA; end
                         62: begin B[1*32+:32] <= S_AXI_WDATA; end
                         63: begin B[0*32+:32] <= S_AXI_WDATA; end
                         
                         64: begin N[31*32+:32] <= S_AXI_WDATA; end
                         65: begin N[30*32+:32] <= S_AXI_WDATA; end
                         66: begin N[29*32+:32] <= S_AXI_WDATA; end
                         67: begin N[28*32+:32] <= S_AXI_WDATA; end
                                 
                         68: begin N[27*32+:32] <= S_AXI_WDATA; end
                         69: begin N[26*32+:32] <= S_AXI_WDATA; end
                         70: begin N[25*32+:32] <= S_AXI_WDATA; end
                         71: begin N[24*32+:32] <= S_AXI_WDATA; end
                                 
                         72: begin N[23*32+:32] <= S_AXI_WDATA; end
                         73: begin N[22*32+:32] <= S_AXI_WDATA; end
                         74: begin N[21*32+:32] <= S_AXI_WDATA; end
                         75: begin N[20*32+:32] <= S_AXI_WDATA; end
                                 
                                 
                         76: begin N[19*32+:32] <= S_AXI_WDATA; end
                         77: begin N[18*32+:32] <= S_AXI_WDATA; end
                         78: begin N[17*32+:32] <= S_AXI_WDATA; end
                         79: begin N[16*32+:32] <= S_AXI_WDATA; end
                                 
                         80: begin N[15*32+:32] <= S_AXI_WDATA; end
                         81: begin N[14*32+:32] <= S_AXI_WDATA; end
                         82: begin N[13*32+:32] <= S_AXI_WDATA; end
                         83: begin N[12*32+:32] <= S_AXI_WDATA; end
                                 
                         84: begin N[11*32+:32] <= S_AXI_WDATA; end
                         85: begin N[10*32+:32] <= S_AXI_WDATA; end
                         86: begin N[9*32+:32] <= S_AXI_WDATA; end
                         87: begin N[8*32+:32] <= S_AXI_WDATA; end
                                 
                         88: begin N[7*32+:32] <= S_AXI_WDATA; end
                         89: begin N[6*32+:32] <= S_AXI_WDATA; end
                         90: begin N[5*32+:32] <= S_AXI_WDATA; end
                         91: begin N[4*32+:32] <= S_AXI_WDATA; end
                                 
                         92: begin N[3*32+:32] <= S_AXI_WDATA; end
                         93: begin N[2*32+:32] <= S_AXI_WDATA; end
                         94: begin N[1*32+:32] <= S_AXI_WDATA; end
                         95: begin N[0*32+:32] <= S_AXI_WDATA; end
                         default: begin end
                     endcase
                 end
	          3'h4:
	            for ( byte_index = 0; byte_index <= (C_S_AXI_DATA_WIDTH/8)-1; byte_index = byte_index+1 )
	              if ( S_AXI_WSTRB[byte_index] == 1 ) begin
	                // Respective byte enables are asserted as per write strobes 
	                // Slave register 4
	                slv_reg4[(byte_index*8) +: 8] <= S_AXI_WDATA[(byte_index*8) +: 8];
	              end  
	          default : begin
	                      //slv_reg0 <= slv_reg0;
	                      Command_Write <= 0;
	                      Address_Write <= 0;	                      
	                      slv_reg1 <= slv_reg1;
	                      slv_reg2 <= slv_reg2;
	                      slv_reg3 <= slv_reg3;
	                      Data_Write <= 0;
	                      slv_reg4 <= slv_reg4;
	                    end
	        endcase
	      end
	      else begin
	           Command_Write <= 0;
	           Address_Write <= 0;
	           Data_Write <= 0;
	      end
	  end
	end    

	// Implement write response logic generation
	// The write response and response valid signals are asserted by the slave 
	// when axi_wready, S_AXI_WVALID, axi_wready and S_AXI_WVALID are asserted.  
	// This marks the acceptance of address and indicates the status of 
	// write transaction.

	always @( posedge S_AXI_ACLK )
	begin
	  if ( S_AXI_ARESETN == 1'b0 )
	    begin
	      axi_bvalid  <= 0;
	      axi_bresp   <= 2'b0;
	    end 
	  else
	    begin    
	      if (axi_awready && S_AXI_AWVALID && ~axi_bvalid && axi_wready && S_AXI_WVALID)
	        begin
	          // indicates a valid write response is available
	          axi_bvalid <= 1'b1;
	          axi_bresp  <= 2'b0; // 'OKAY' response 
	        end                   // work error responses in future
	      else
	        begin
	          if (S_AXI_BREADY && axi_bvalid) 
	            //check if bready is asserted while bvalid is high) 
	            //(there is a possibility that bready is always asserted high)   
	            begin
	              axi_bvalid <= 1'b0; 
	            end  
	        end
	    end
	end   

	// Implement axi_arready generation
	// axi_arready is asserted for one S_AXI_ACLK clock cycle when
	// S_AXI_ARVALID is asserted. axi_awready is 
	// de-asserted when reset (active low) is asserted. 
	// The read address is also latched when S_AXI_ARVALID is 
	// asserted. axi_araddr is reset to zero on reset assertion.

	always @( posedge S_AXI_ACLK )
	begin
	  if ( S_AXI_ARESETN == 1'b0 )
	    begin
	      axi_arready <= 1'b0;
	      axi_araddr  <= 32'b0;
	    end 
	  else
	    begin    
	      if (~axi_arready && S_AXI_ARVALID)
	        begin
	          // indicates that the slave has acceped the valid read address
	          axi_arready <= 1'b1;
	          // Read address latching
	          axi_araddr  <= S_AXI_ARADDR;
	        end
	      else
	        begin
	          axi_arready <= 1'b0;
	        end
	    end 
	end       

	// Implement axi_arvalid generation
	// axi_rvalid is asserted for one S_AXI_ACLK clock cycle when both 
	// S_AXI_ARVALID and axi_arready are asserted. The slave registers 
	// data are available on the axi_rdata bus at this instance. The 
	// assertion of axi_rvalid marks the validity of read data on the 
	// bus and axi_rresp indicates the status of read transaction.axi_rvalid 
	// is deasserted on reset (active low). axi_rresp and axi_rdata are 
	// cleared to zero on reset (active low).  
	always @( posedge S_AXI_ACLK )
	begin
	  if ( S_AXI_ARESETN == 1'b0 )
	    begin
	      axi_rvalid <= 0;
	      axi_rresp  <= 0;
	    end 
	  else
	    begin    
	      if (axi_arready && S_AXI_ARVALID && ~axi_rvalid)
	        begin
	          // Valid read data is available at the read data bus
	          axi_rvalid <= 1'b1;
	          axi_rresp  <= 2'b0; // 'OKAY' response
	        end   
	      else if (axi_rvalid && S_AXI_RREADY)
	        begin
	          // Read data is accepted by the master
	          axi_rvalid <= 1'b0;
	        end                
	    end
	end    

	// Implement memory mapped register select and read logic generation
	// Slave register read enable is asserted when valid address is available
	// and the slave is ready to accept the read address.
	assign slv_reg_rden = axi_arready & S_AXI_ARVALID & ~axi_rvalid;
	always @(*)
	begin
	      // Address decoding for reading registers
	      case ( axi_araddr[ADDR_LSB+OPT_MEM_ADDR_BITS:ADDR_LSB] )
	        3'h0   : reg_data_out <= slv_reg0; // Status -out
	        3'h1   : reg_data_out <= slv_reg1; // Command
	        3'h2   : reg_data_out <= Address; // Address
	        3'h3   : // reg_data_out <= slv_reg_data_out; // Data
	           begin
	           case (Address) //When address changes automatically update data_out
                           44: begin reg_data_out <= DataOutHolder[7*32+:32]; end  //Start of SHA256 out (MSB)
                           45: begin reg_data_out <= DataOutHolder[6*32+:32]; end
                           46: begin reg_data_out <= DataOutHolder[5*32+:32]; end
                           47: begin reg_data_out <= DataOutHolder[4*32+:32]; end
                           
                           48: begin reg_data_out <= DataOutHolder[3*32+:32]; end //Start of AES out
                           49: begin reg_data_out <= DataOutHolder[2*32+:32]; end
                           50: begin reg_data_out <= DataOutHolder[1*32+:32]; end
                           51: begin reg_data_out <= DataOutHolder[0*32+:32]; end

               
                           96: begin reg_data_out <= P[31*32+:32]; end
                           97: begin reg_data_out <= P[30*32+:32]; end
                           98: begin reg_data_out <= P[29*32+:32]; end
                           99: begin reg_data_out <= P[28*32+:32]; end
                           
                           100: begin reg_data_out <= P[27*32+:32]; end
                           101: begin reg_data_out <= P[26*32+:32]; end
                           102: begin reg_data_out <= P[25*32+:32]; end
                           103: begin reg_data_out <= P[24*32+:32]; end
                           
                           104: begin reg_data_out <= P[23*32+:32]; end
                           105: begin reg_data_out <= P[22*32+:32]; end
                           106: begin reg_data_out <= P[21*32+:32]; end
                           107: begin reg_data_out <= P[20*32+:32]; end
                           
                           108: begin reg_data_out <= P[19*32+:32]; end
                           109: begin reg_data_out <= P[18*32+:32]; end
                           110: begin reg_data_out <= P[17*32+:32]; end
                           111: begin reg_data_out <= P[16*32+:32]; end
                           
                           112: begin reg_data_out <= P[15*32+:32]; end
                           113: begin reg_data_out <= P[14*32+:32]; end
                           114: begin reg_data_out <= P[13*32+:32]; end
                           115: begin reg_data_out <= P[12*32+:32]; end
                           
                           116: begin reg_data_out <= P[11*32+:32]; end
                           117: begin reg_data_out <= P[10*32+:32]; end
                           118: begin reg_data_out <= P[9*32+:32]; end
                           119: begin reg_data_out <= P[8*32+:32]; end
                           
                           120: begin reg_data_out <= P[7*32+:32]; end
                           121: begin reg_data_out <= P[6*32+:32]; end
                           122: begin reg_data_out <= P[5*32+:32]; end
                           123: begin reg_data_out <= P[4*32+:32]; end
                           
                           124: begin reg_data_out <= P[3*32+:32]; end
                           125: begin reg_data_out <= P[2*32+:32]; end
                           126: begin reg_data_out <= P[1*32+:32]; end
                           127: begin reg_data_out <= P[0*32+:32]; end           
                           default: begin end
                      endcase
	           end
	        3'h4   : reg_data_out <= slv_reg4; 
	        default : reg_data_out <= 0;
	      endcase
              
//          if( axi_araddr[ADDR_LSB+OPT_MEM_ADDR_BITS:ADDR_LSB] == 3'h3 )
//                Data_Read = 1;
//            else
//                Data_Read = 0;
	end

	// Output register or memory read data
	always @( posedge S_AXI_ACLK )
	begin
	  if ( S_AXI_ARESETN == 1'b0 )
	    begin
	      axi_rdata  <= 0;
	    end 
	  else
	    begin    
	      // When there is a valid read address (S_AXI_ARVALID) with 
	      // acceptance of read address by the slave (axi_arready), 
	      // output the read dada 
	      if (slv_reg_rden)
	        begin
	          axi_rdata <= reg_data_out;     // register read data
	          if( axi_araddr[ADDR_LSB+OPT_MEM_ADDR_BITS:ADDR_LSB] == 3'h3 )begin
                      Data_Read <= 1;
              end 
              else begin
                Data_Read <= 0;
              end
	        end   
	        else begin
	           Data_Read <= 0;
	        end
	    end
	end    
	
	// MSB = 0

    
    wire finished;
    reg rstn = 0; //Set to 1 to start
    
//    wire [1023+2:0] add_in;
//    assign add_in[1023:0] = B;
//    assign add_in[1025:1024] = 0;
//    wire Cout_unused;
//    adder_subtractor_multi_clock add_sub(
//        .reset(S_AXI_ARESETN),
//        .clk(clk_200Mhz),
//        .A(add_in),
//        .B(A),
//        .S(P),
//        .add(slv_reg1[1]),
//        .Go(rstn),
//        .Complete(finished),
//        .Ci(0),
//        .Co(Cout_unused)
//    );
    
    MemodC uut(
        .reset(S_AXI_ARESETN),
        .clk_slow(S_AXI_ACLK),
        .clk_fast(S_AXI_ACLK),
        .go(rstn),
        .M(B), //Switched. e is stored at beginning of computation so it can be over written
        .e(A),
        .N(N),
        .out(P),
        .finished(finished)
    );
        
//      wire [2:0] debug_state;
//      ABmodN uut(
//       .reset(S_AXI_ARESETN),
//       .clk_slow(S_AXI_ACLK),
//       .clk_fast(clk_200Mhz),
//       .go(rstn),
//       .A(B), //Switched. e is stored at beginning of computation so it can be over written
//       .B(A),
//       .N(N),
//       .out(P),
//       .finished(finished),
//       .debug_state(debug_state)
//       ); 


	// Add user logic here
    reg TriggerAES; // rising edge
    //assign State = slv_reg1[0];
    wire [127:0] Key;
    assign Key = A[28*32+:128];
    wire [127:0] Round1;
    assign Round1 = A[24*32+:128];
    wire [127:0] Round2;
    assign Round2 = A[20*32+:128];  
    wire [127:0] Round3;  
    assign Round3 = A[16*32+:128];
    wire [127:0] Round4;  
    assign Round4 = A[12*32+:128];
    wire [127:0] Round5; 
    assign Round5 = A[8*32+:128]; 
    wire [127:0] Round6;  
    assign Round6 = A[4*32+:128];
    wire [127:0] Round7;  
    assign Round7 = A[0*32+:128];
    wire [127:0] Round8;  
    assign Round8 = B[28*32+:128];
    wire [127:0] Round9;  
    assign Round9 = B[24*32+:128];
    wire [127:0] Round10; 
    assign Round10 = B[20*32+:128];
    
    wire [127:0] AESDataIn;
    assign AESDataIn = B[16*32+:128];

    wire AES_Status;
    

    // Add user logic here
    AES_Decrypt_10_no_key dec(
        S_AXI_ACLK,
        TriggerAES,
        Key,
        Round1,
        Round2,
        Round3,
        Round4,
        Round5,
        Round6,
        Round7,
        Round8,
        Round9,
        Round10,
        AESDataIn,
        DataOut,
        AES_Status
        );

    
    //---------------------------------- SHA256 --------------------------------------------------
    
    localparam ADDR_NAME0       = 8'h00;
    localparam ADDR_NAME1       = 8'h01;
    localparam ADDR_VERSION     = 8'h02;
  
    localparam ADDR_CTRL        = 8'h08;
    localparam CTRL_INIT_BIT    = 0;
    localparam CTRL_NEXT_BIT    = 1;
    localparam CTRL_MODE_BIT    = 2;
  
    localparam ADDR_STATUS      = 8'h09;
    localparam STATUS_READY_BIT = 0;
    localparam STATUS_VALID_BIT = 1;
  
    localparam ADDR_BLOCK0    = 8'h10;
    localparam ADDR_BLOCK15   = 8'h1f;
  
    localparam ADDR_DIGEST0   = 8'h20;
    localparam ADDR_DIGEST7   = 8'h27;
  
    localparam CORE_NAME0     = 32'h73686132; // "sha2"
    localparam CORE_NAME1     = 32'h2d323536; // "-256"
    localparam CORE_VERSION   = 32'h312e3830; // "1.80"
  
    localparam MODE_SHA_224   = 1'h0;
    localparam MODE_SHA_256   = 1'h1;
      
  
    reg init = 0;
    reg next = 0;

    reg mode_reg = MODE_SHA_256;
  
    wire [255 : 0] core_digest;
    //reg [255:0] digest_holder = 0; //Could use AES data in and data out registers instead to save resources

    wire           core_ready;
    
    wire [511 : 0] core_block;
    assign core_block = A[16*32+:512];
    
    wire           core_digest_valid;

    //----------------------------------------------------------------
    // core instantiation.
    //----------------------------------------------------------------
    sha256_core core(
                     .clk(clk_50Mhz),
                     .reset_n(S_AXI_ARESETN),
  
                     .init(init),  //Toggle high to start new hash
                     .next(next),  //Toggle high to continue hash
                     .mode(mode_reg),
  
                     .block(core_block),
  
                     .ready(core_ready),
  
                     .digest(core_digest),
                     .digest_valid(core_digest_valid)
                    );
                    
                    
                    
   // ------------------------------------ Begin control logic -----------------------------------
        
	
	parameter AES = 1;
	parameter SHA = 2;
	parameter RSA = 3;
	
	
	wire [1:0] Device;
	assign Device = slv_reg1[31:30];
	
    
    // Update status
    always @(posedge S_AXI_ACLK) begin
        slv_reg0[0] <= AES_Status;
        slv_reg0[1] <= core_ready;
        slv_reg0[2] <= core_digest_valid;
        slv_reg0[3] <= finished;
        slv_reg0[4] <= rstn;
        //slv_reg0[7:5] <= debug_state;
        slv_reg0[31:5] <= 0;
    end
    
//    always @(AES_Status or core_digest_valid) begin
    
//    end

    reg init_reg;
    reg last_init_reg;
    reg next_reg;
    reg last_next_reg;
    
    always @ (posedge S_AXI_ACLK ) begin//, negedge S_AXI_ARESETN) begin
       //slv_reg0 = 0;  
       if (  S_AXI_ARESETN == 0 ) begin
        init_reg <= 0;
        next_reg <= 0;
       end 
       else begin
           if( Command_Write ) begin
               case (Device)
                   AES: begin 
                       TriggerAES <= 1;
                   end
                   SHA: begin 
                       if(slv_reg1[0] == 1)begin
                        //if high then init
                        init_reg <= ~init_reg;
                       end
                       else begin 
                        //else next
                        next_reg <= ~next_reg;
                       end
                   end
                   RSA: begin
                       if(slv_reg1[0] == 1)begin
                        rstn <= ~rstn; //Start RSA or stop
                       end
    //                   else begin
    //                    rstn = 0; //Reset RSA
    //                   end
                   end
                   default: begin end
               endcase
           end 
           else begin
              TriggerAES <= 0;
           end
       end
    end
    
    always @(posedge clk_50Mhz) begin
        if (  S_AXI_ARESETN == 0 ) begin
            last_init_reg <= 0;
            last_next_reg <= 0;
        end 
        else begin
            if( init_reg != last_init_reg)begin
                init <= 1;
                last_init_reg <= init_reg;
            end
            else begin
                init <= 0;
            end
            
            if( next_reg != last_next_reg ) begin
                next <= 1;
                last_next_reg <= next_reg;
            end
            else begin
                next <= 0;
            end  
        end
    end
    
    // Handle data
    //always @( negedge S_AXI_ACLK) begin 
    always @( posedge S_AXI_ACLK) begin //TODO check if this fixes timing. Or breaks it more
        if(Data_Read) begin
            Address <= Address + 1;
        end 
        else if( Data_Write) begin
            Address <= Address + 1;
        end 
        else if(Address_Write) begin
            Address <= slv_reg2;
        end
        else if ( (AES_Status == 1) || (core_digest_valid == 1)) begin
        case (Device)
               AES: begin
                   if( AES_Status == 1) begin 
                       DataOutHolder[0+:128] <= DataOut; // Should assign starting at 48
                   end
               end
               SHA: begin
                   if( core_digest_valid == 1) begin 
                         DataOutHolder <= core_digest;
                   end
               end
               RSA: begin
                   
               end
               default: begin end
            endcase
        end
    end
	

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
	// User logic ends

	endmodule
	
	
	
	


