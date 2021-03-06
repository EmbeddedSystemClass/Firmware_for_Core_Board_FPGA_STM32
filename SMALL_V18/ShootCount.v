module ShootCount(clk,count_shoot,shoot,rst_n); 
input clk,shoot,rst_n; 
output[31:0] count_shoot; 
reg preshoot;
reg[31:0] count_shoot,count;
always@(posedge clk or negedge rst_n)
begin
if(!rst_n)
	begin
	count_shoot <= 0;
	preshoot <= 0;
	count <= 0;
	end
else
	begin
	if(count>0) count_shoot <= count;
	if(shoot)  count <= count+1;
	end
end
endmodule 
