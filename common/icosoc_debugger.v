
module icosoc_debugger #(
	parameter WIDTH = 32,
	parameter DEPTH = 256,
	parameter TRIGAT = 64,
	parameter MODE = "NORMAL"
) (
	input clk,
	input resetn,

	input             enable,
	input             trigger,
	output            triggered,
	input [WIDTH-1:0] data,

	input            dump_en,
	output reg       dump_valid,
	input            dump_ready,
	output reg [7:0] dump_data
);
	localparam DEPTH_BITS = $clog2(DEPTH);

	localparam BYTES = (WIDTH + 7) / 8;
	localparam BYTES_BITS = $clog2(BYTES);

	reg [WIDTH-1:0] memory [0:DEPTH-1];
	reg [DEPTH_BITS-1:0] mem_pointer, stop_counter;
	reg [BYTES_BITS-1:0] bytes_counter;
	reg dump_en_r;

	reg [1:0] state;
	localparam state_running   = 0;
	localparam state_triggered = 1;
	localparam state_waitdump  = 2;
	localparam state_dump      = 3;

	localparam mode_normal        = MODE == "NORMAL";         // first trigger after dump_en
	localparam mode_free_running  = MODE == "FREE_RUNNING";   // trigger on dump_en
	localparam mode_first_trigger = MODE == "FIRST_TRIGGER";  // block on first trigger
	localparam mode_last_trigger  = MODE == "LAST_TRIGGER";   // wait for last trigger

	initial begin
		if (!{mode_normal, mode_free_running, mode_first_trigger, mode_last_trigger}) begin
			$display("Invalid debugger MODE: %s", MODE);
			$finish;
		end
	end

	always @(posedge clk)
		dump_data <= memory[mem_pointer] >> (8*bytes_counter);
	
	assign triggered = resetn && state != state_running;

	always @(posedge clk) begin
		dump_valid <= 0;
		if (dump_en)
			dump_en_r <= 1;
		if (!resetn) begin
			mem_pointer <= 0;
			stop_counter <= DEPTH-1;
			state <= state_running;
			dump_en_r <= 0;
		end else
		case (state)
			state_running: begin
				if (enable) begin
					memory[mem_pointer] <= data;
					mem_pointer <= mem_pointer == DEPTH-1 ? 0 : mem_pointer+1;
					if (!stop_counter) begin
						if (mode_free_running) begin
							if (dump_en_r) begin
								state <= state_dump;
								stop_counter <= DEPTH-1;
								bytes_counter <= 0;
							end
						end else begin
							if (trigger && (dump_en_r || !mode_normal)) begin
								stop_counter <= DEPTH - TRIGAT - 2;
								state <= state_triggered;
							end
						end
					end else
						stop_counter <= stop_counter - 1;
				end
			end
			state_triggered: begin
				if (enable) begin
					memory[mem_pointer] <= data;
					mem_pointer <= mem_pointer == DEPTH-1 ? 0 : mem_pointer+1;
					stop_counter <= stop_counter - 1;
					if (mode_last_trigger && trigger) begin
						stop_counter <= DEPTH - TRIGAT - 2;
					end
					if (!stop_counter) begin
						state <= state_waitdump;
					end
				end
			end
			state_waitdump: begin
				if (dump_en_r)
					state <= state_dump;
				stop_counter <= DEPTH-1;
				bytes_counter <= 0;
			end
			state_dump: begin
				if (dump_valid && dump_ready) begin
					if (bytes_counter == BYTES-1) begin
						bytes_counter <= 0;
						stop_counter <= stop_counter - 1;
						mem_pointer <= mem_pointer == DEPTH-1 ? 0 : mem_pointer+1;
						if (!stop_counter) begin
							stop_counter <= DEPTH-1;
							state <= state_running;
							dump_en_r <= 0;
						end
					end else begin
						bytes_counter <= bytes_counter + 1;
					end
				end else begin
					dump_valid <= 1;
				end
			end
		endcase
	end
endmodule

