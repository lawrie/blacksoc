// Interface to ZipSTORM-MX wishbone SDRAM controller
module sdram #(parameter CLOCK_FREQ_HZ = 0) (
	input clk,
	input resetn,
	input mem_valid,
	input [3:0] mem_wstrb,
	input [31:0] mem_addr,
	input [31:0] mem_wdata,
	output mem_ready,
	output [31:0] mem_rdata,

	output o_ram_clk, o_ram_cke, o_ram_cs_n, o_ram_ras_n, o_ram_cas_n,
	output o_ram_we_n, o_ram_udqm, o_ram_ldqm,
	output [11:0] o_ram_addr,
	inout  [15:0] io_ram_data
);
	// Registered output pins
	wire w_ram_cs_n, w_ram_ras_n, w_ram_cas_n, w_ram_we_n;
	wire [11:0] w_ram_addr;
	wire [1:0] w_ram_dqm;
	SB_IO #(.PIN_TYPE(6'b0101_01)) ramctrl [4+12+2-1:0] (
		.OUTPUT_CLK(clk),
		.D_OUT_0({w_ram_cs_n, w_ram_ras_n, w_ram_cas_n, w_ram_we_n,
			w_ram_addr, w_ram_dqm}),
		.PACKAGE_PIN({o_ram_cs_n, o_ram_ras_n, o_ram_cas_n, o_ram_we_n,
			o_ram_addr, o_ram_udqm, o_ram_ldqm}));

	// DDR output pin
	SB_IO #(.PIN_TYPE(6'b0100_01)) ramck (
		.OUTPUT_CLK(clk),
		.D_OUT_0(1'b0),
		.D_OUT_1(1'b1),
		.PACKAGE_PIN(o_ram_clk));

	// DDR tri-state inout pins
	wire w_ram_drive_data;
	wire [15:0] w_ram_data, w_ram_data_pedge, w_ram_data_nedge;
	SB_IO #(.PIN_TYPE(6'b1100_00)) ramio [15:0] (
		.OUTPUT_CLK(clk),
		.INPUT_CLK(clk),
		.OUTPUT_ENABLE(w_ram_drive_data),
		.D_OUT_0(w_ram_data),
		.D_OUT_1(w_ram_data),
		.D_IN_0(w_ram_data_pedge),
		.D_IN_1(w_ram_data_nedge),
		.PACKAGE_PIN(io_ram_data));

    // SDRAM controller
	reg strobed;
	wire wbm_stall_i;
	wire [15:0] i_ram_data = w_ram_data_nedge;
	wire wbm_stb_o = mem_valid & ~strobed;
	always @(posedge clk)
		if (~resetn)
			strobed <= 0;
		else if (mem_valid)
			strobed <= ~mem_ready & ~wbm_stall_i;
	wbsdram #(.CLOCK_FREQUENCY_HZ(CLOCK_FREQ_HZ)) wbsdram (
		.i_clk(clk),
		.i_wb_cyc(mem_valid),
		.i_wb_stb(wbm_stb_o),
		.i_wb_we(|mem_wstrb),
		.i_wb_addr(mem_addr[20:2]),
		.i_wb_data(mem_wdata),
		.i_wb_sel(mem_wstrb),
		.o_wb_ack(mem_ready),
		.o_wb_stall(wbm_stall_i),
		.o_wb_data(mem_rdata),
		.o_ram_cs_n(w_ram_cs_n),
		.o_ram_cke(o_ram_cke),
		.o_ram_ras_n(w_ram_ras_n),
		.o_ram_cas_n(w_ram_cas_n),
		.o_ram_we_n(w_ram_we_n),
		.o_ram_addr(w_ram_addr),
		.o_ram_dmod(w_ram_drive_data),
		.i_ram_data(i_ram_data),
		.o_ram_data(w_ram_data),
		.o_ram_dqm(w_ram_dqm),
	);
endmodule
