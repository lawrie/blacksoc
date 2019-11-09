module icosoc_mod_extirq #(
	parameter integer CLOCK_FREQ_HZ = 0
) (
	input clk,
	input resetn,

	input ctrl_wr,
	input ctrl_rd,
	input [ 7:0] ctrl_addr,
	input [31:0] ctrl_wdat,
	output reg [31:0] ctrl_rdat,
	output reg ctrl_done,
	output reg ctrl_irq,

	input pin
);
	wire pin_in;
	reg pin_q1, pin_q2;

	reg [3:0] config_word;

	wire trigger_hi = config_word[3];
	wire trigger_lo = config_word[2];
	wire trigger_re = config_word[1];
	wire trigger_fe = config_word[0];

	SB_IO #(
		.PIN_TYPE(6'b 0000_01),
		.PULLUP(1'b 1)
	) iobuf (
		.PACKAGE_PIN(pin),
		.D_IN_0(pin_in)
	);

	always @(posedge clk) begin
		pin_q1 <= pin_in;
		pin_q2 <= pin_q1;
	end

	always @(posedge clk) begin
		ctrl_irq <= 0;
		if (resetn) begin
			if (trigger_hi &&  pin_q1 &&  pin_q2) ctrl_irq <= 1;
			if (trigger_lo && !pin_q1 && !pin_q2) ctrl_irq <= 1;
			if (trigger_re &&  pin_q1 && !pin_q2) ctrl_irq <= 1;
			if (trigger_fe && !pin_q1 &&  pin_q2) ctrl_irq <= 1;
		end
	end

	always @(posedge clk) begin
		ctrl_rdat <= 'bx;
		ctrl_done <= 0;

		// Register file:
		//   0x00 config register (MSB reads pin)
		if (!resetn) begin
			config_word <= 0;
		end else
		if (!ctrl_done) begin
			if (ctrl_wr) begin
				ctrl_done <= 1;
				config_word <= ctrl_wdat;
			end
			if (ctrl_rd) begin
				ctrl_done <= 1;
				ctrl_rdat <= config_word | (pin_q2 << 31);
			end
		end
	end
endmodule

