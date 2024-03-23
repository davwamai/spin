`timescale 1ns / 1ps

module value_latch(
    input [31:0] gpio_in,
    output reg [27:0] val_o
    );

    reg value_latched; // flag if a non-zero value has been latched
    
    always@(gpio_in) begin
        if(gpio_in[0:0] == 1'b1) begin
            val_o = gpio_in[31:4];
            value_latched <= 1'b1;
        end
        else if (!value_latched) begin
            val_o = 28'b0;
        end        

//    always @(posedge clk) begin
//        if (!value_latched && gpio_in != 32'd0) begin
//            val_o <= gpio_in;
//            value_latched <= 1'b1; // flag once the first non-zero value is latched
//        end
//    end       
end
endmodule