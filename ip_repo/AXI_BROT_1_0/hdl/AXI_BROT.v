
`timescale 1 ns / 1 ps

	module AXI_BROT #
	(
		// Users to add parameters here

		// User parameters ends
		// Do not modify the parameters beyond this line


		// Parameters of Axi Slave Bus Interface S00_AXI
		parameter integer C_S00_AXI_DATA_WIDTH	= 32,
		parameter integer C_S00_AXI_ADDR_WIDTH	= 7
	)
	(
		// Users to add ports here
        input wire FINISHED_1,
        input wire FINISHED_2,
        input wire FINISHED_3,
        input wire FINISHED_4,
        
        input wire [9:0] ITER_1,
        input wire [9:0] ITER_2,
        input wire [9:0] ITER_3,
        input wire [9:0] ITER_4,
        
        output wire RST_1,
        output wire RST_2,
        output wire RST_3,
        output wire RST_4,
        
        output wire [63:0] C_RE_1,
        output wire [63:0] C_RE_2,
        output wire [63:0] C_RE_3,
        output wire [63:0] C_RE_4,
        
        output wire [63:0] C_IM_1,
        output wire [63:0] C_IM_2,
        output wire [63:0] C_IM_3,
        output wire [63:0] C_IM_4,
		// User ports ends
		// Do not modify the ports beyond this line


		// Ports of Axi Slave Bus Interface S00_AXI
		input wire  s00_axi_aclk,
		input wire  s00_axi_aresetn,
		input wire [C_S00_AXI_ADDR_WIDTH-1 : 0] s00_axi_awaddr,
		input wire [2 : 0] s00_axi_awprot,
		input wire  s00_axi_awvalid,
		output wire  s00_axi_awready,
		input wire [C_S00_AXI_DATA_WIDTH-1 : 0] s00_axi_wdata,
		input wire [(C_S00_AXI_DATA_WIDTH/8)-1 : 0] s00_axi_wstrb,
		input wire  s00_axi_wvalid,
		output wire  s00_axi_wready,
		output wire [1 : 0] s00_axi_bresp,
		output wire  s00_axi_bvalid,
		input wire  s00_axi_bready,
		input wire [C_S00_AXI_ADDR_WIDTH-1 : 0] s00_axi_araddr,
		input wire [2 : 0] s00_axi_arprot,
		input wire  s00_axi_arvalid,
		output wire  s00_axi_arready,
		output wire [C_S00_AXI_DATA_WIDTH-1 : 0] s00_axi_rdata,
		output wire [1 : 0] s00_axi_rresp,
		output wire  s00_axi_rvalid,
		input wire  s00_axi_rready
	);
// Instantiation of Axi Bus Interface S00_AXI
	AXI_BROT_slave_lite_v1_0_S00_AXI # ( 
		.C_S_AXI_DATA_WIDTH(C_S00_AXI_DATA_WIDTH),
		.C_S_AXI_ADDR_WIDTH(C_S00_AXI_ADDR_WIDTH)
	) AXI_BROT_slave_lite_v1_0_S00_AXI_inst (
		.S_AXI_ACLK(s00_axi_aclk),
		.S_AXI_ARESETN(s00_axi_aresetn),
		.S_AXI_AWADDR(s00_axi_awaddr),
		.S_AXI_AWPROT(s00_axi_awprot),
		.S_AXI_AWVALID(s00_axi_awvalid),
		.S_AXI_AWREADY(s00_axi_awready),
		.S_AXI_WDATA(s00_axi_wdata),
		.S_AXI_WSTRB(s00_axi_wstrb),
		.S_AXI_WVALID(s00_axi_wvalid),
		.S_AXI_WREADY(s00_axi_wready),
		.S_AXI_BRESP(s00_axi_bresp),
		.S_AXI_BVALID(s00_axi_bvalid),
		.S_AXI_BREADY(s00_axi_bready),
		.S_AXI_ARADDR(s00_axi_araddr),
		.S_AXI_ARPROT(s00_axi_arprot),
		.S_AXI_ARVALID(s00_axi_arvalid),
		.S_AXI_ARREADY(s00_axi_arready),
		.S_AXI_RDATA(s00_axi_rdata),
		.S_AXI_RRESP(s00_axi_rresp),
		.S_AXI_RVALID(s00_axi_rvalid),
		.S_AXI_RREADY(s00_axi_rready),
		
		.FINISHED_1(FINISHED_1),
		.FINISHED_2(FINISHED_2),
		.FINISHED_3(FINISHED_3),
		.FINISHED_4(FINISHED_4),
		
		.ITER_1(ITER_1),
		.ITER_2(ITER_2),
		.ITER_3(ITER_3),
		.ITER_4(ITER_4),
		
		.RST_1(RST_1),
		.RST_2(RST_2),
		.RST_3(RST_3),
		.RST_4(RST_4),
		
		.C_RE_1(C_RE_1),
		.C_RE_2(C_RE_2),
		.C_RE_3(C_RE_3),
		.C_RE_4(C_RE_4),
		
		.C_IM_1(C_IM_1),
		.C_IM_2(C_IM_2),
		.C_IM_3(C_IM_3),
		.C_IM_4(C_IM_4)
	);

	// Add user logic here

	// User logic ends

	endmodule
