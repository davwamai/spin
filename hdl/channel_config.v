module channel_config (
    input wire clk,      
    input wire [15:0] data_src_a,  
    input wire [15:0] data_src_b,   
    output wire [31:0] combined_data,
    output wire t_valid
);

assign t_valid = 1'b1;
assign combined_data = {2'b00, data_src_b[15:2], 2'b00, data_src_a[15:2]};

endmodule
