module cpmg (
    input clk,             //125MHz
    input rst,               
    input wire [15:0] delay_reg,
    output reg [15:0] data   
);

parameter TAU = 875;                 // first pulse width in clock cycles (7us)
parameter TAU_LOW = 78125;           // first low period in clock cycles (625us)
parameter TWO_TAU = 1750;            // subsequent high pulse width in clock cycles (14us)
parameter TWO_TAU_LOW = 156250;      // subsequent low period in clock cycles (1250us)
parameter HIGH_VALUE = 16'h7FF8;
parameter LOW_VALUE = 16'h0000;
parameter DELAY_CYCLES = 2;          // DDS compiler resetn must be asserted for a minimum of 2 cycles

// internal registers
reg [17:0] pulse_counter = 0;
reg [17:0] period_counter = 0;
reg pulse_state = 1;
reg tau_done = 0;
reg [17:0] delay_counter = 0;        
always @(posedge clk) begin
    if (!rst) begin
        // reset
        pulse_counter <= 0;
        period_counter <= 0;
        pulse_state <= 1;
        tau_done <= 0;
        data <= LOW_VALUE;
        delay_counter <= delay_reg; // init delay counter
    end else if (delay_counter > 0) begin
        // decrement delay counter until 0
        delay_counter <= delay_counter - 1;
    end else begin
        // delay complete, proceed 
        if (pulse_state) begin
            // pulse is high
            if (!tau_done && pulse_counter < TAU || tau_done && pulse_counter < TWO_TAU) begin
                pulse_counter <= pulse_counter + 1;
                data <= HIGH_VALUE;
            end else begin
                // pulse width reached, switch to low
                pulse_state <= 0;
                pulse_counter <= 0;
                period_counter <= 1;
                data <= LOW_VALUE;
            end
        end else begin
            // pulse is low
            if (!tau_done && period_counter < TAU_LOW || tau_done && period_counter < TWO_TAU_LOW) begin
                period_counter <= period_counter + 1;
                data <= LOW_VALUE;
            end else begin
                // period complete, switch to high
                pulse_state <= 1;
                period_counter <= 0;
                pulse_counter <= 1;
                data <= HIGH_VALUE;
                if (!tau_done) tau_done <= 1; // mark first pulse as done
            end
        end
    end
end

endmodule

