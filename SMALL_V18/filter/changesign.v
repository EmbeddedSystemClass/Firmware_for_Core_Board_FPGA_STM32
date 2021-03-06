module changesign(datain,dataout,signout,clk,rst_n);
input clk, rst_n;
input [31:0]datain;
output [31:0]dataout;
output signout;
reg [31:0]dataout;
reg signout;

always @(posedge clk or negedge rst_n)
begin
if(!rst_n)
	begin
		dataout <= 32'd0;
		signout <= 0;
	end
else
    if(datain[31] == 0)
    begin
    dataout <= datain;
    signout <= 0;
    end
    else
    begin
    dataout <= 0 - datain;
    signout <= 1;
    end
end
endmodule
    