module icosoc_mod_mixmodssd #(
	parameter integer CLOCK_FREQ_HZ = 20000000
) (
	input clk,
	input resetn,

	input [3:0] ctrl_wr,
	input ctrl_rd,
	input [15:0] ctrl_addr,
	input [31:0] ctrl_wdat,
	output reg [31:0] ctrl_rdat,
	output reg ctrl_done,

	output [10:0] pins
);
	reg [11:0] value;

	function [6:0] segments;
		input [3:0] x;
		case (x)
			// Segments - gfedcba
            		4'h0: segments = 7'b1000000;
            		4'h1: segments = 7'b1111001;
            		4'h2: segments = 7'b0100100;
            		4'h3: segments = 7'b0110000;
            		4'h4: segments = 7'b0011001;
            		4'h5: segments = 7'b0010010;
            		4'h6: segments = 7'b0000010;
            		4'h7: segments = 7'b1111000;
            		4'h8: segments = 7'b0000000;
            		4'h9: segments = 7'b0010000;
            		4'hA: segments = 7'b0001000;
            		4'hB: segments = 7'b0000011;
            		4'hC: segments = 7'b1000110;
            		4'hD: segments = 7'b0100001;
            		4'hE: segments = 7'b0000110;
            		4'hF: segments = 7'b0001110;
		endcase
	endfunction

	wire [6:0] segs = segments(digit_select == 3'b011 ? value[11:8] :
		                   digit_select == 3'b101 ? value[7:4] : 
				                            value[3:0]);

        reg [2:0] digit_select = 3'b110;

	assign pins = {digit_select,
		       segs[5], segs[6], 1'b1, segs[4],
		       segs[1], segs[0], segs[2], segs[3]};

	reg [17:0] count;

	always @(posedge clk) begin
		count <= count + 1;
		if (count == (CLOCK_FREQ_HZ / 3000) - 1) begin // refresh each digit at 1000Hz
			count <= 0;
			digit_select <= {digit_select[1:0], digit_select[2]};
		end
	end

	always @(posedge clk) begin
		ctrl_rdat <= 'bx;
		ctrl_done <= 0;

		if (resetn && !ctrl_done) begin
			if (|ctrl_wr) begin
				ctrl_done <= 1;
				value <= ctrl_wdat;
			end
			if (ctrl_rd) begin
				ctrl_done <= 1;
				ctrl_rdat <= value;
			end
		end
	end

endmodule
