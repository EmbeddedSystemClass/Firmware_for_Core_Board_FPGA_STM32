#include "Comm.h"

packet_type_t type;                         //���ݰ�����
unsigned char packet_cnt=0;                 //�հ���������  
unsigned char check_frq_start=0;
unsigned int check_frq_counter1=0;
unsigned int check_frq_counter2=0;

extern robot_t g_robot;
extern char packet_flag;
extern nRF2401 nRF2401_dev;
extern char receive_flag;
extern char g_set_receive_mode_flag;
extern char g_comm_up_finish_flag;
extern char g_comm_overflow_flag;

RFConfig g_default_nRF2401_conf = {
	15,
	ADDR_COUNT,
	DATA_LEN2*8,            			//Length of data payload section TX channel 2
	DATA_LEN1*8,                     	//Length of data payload section TX channel 1
	ADDR_CH2 ,                    		//Up to 5 byte address for TX channel 2
	ADDR_CH1 ,                       	//Up to 5 byte address for TX channel 1
	(((ADDR_LEN) << 2) + ((CRC_LEN) << 1) + (CRC_ENABLE)),   
	//Number of address bits (both TX channels),8 or 16 bit CRC,Enable on-chip CRC generation/checking
	//0x4f,                               //Enable two channel receive mode,Communication mode (Direct or ShockBurst?),RF data rate (1Mbps requires 16MHz crystal),Crystal frequency,RF output power
	(((RX2_EN) << 7) + ((CM) << 6) + ((RF_DATA_RATE) << 5) + ((XO_F) << 2) + (RF_PWR)) ,
	(((RF_CH) << 1) + 1)      			//Frequency channel,RX or TX operation

};


/*---------function headers----------*/
int do_comm_up();
int do_packet_process( unsigned char *data, int len );


/*----------------------------------------------------------------------------------------
 *  �������ƣ� init_comm
 *  ���������� ͨ��ģ���ʼ����������ʼ�������Ƚ�ͨ��ģ���ʼ��Ϊ����ģʽ��������
 *             ʼ��2401_dev�ĸ�����Ա
 *
 *
 *  ���ߣ�     ����
 *  �޸����ڣ� 2011-9-15
 *  ��ע:      ����2401��ʼ�������ָ���λ��ζ��壬����<<PTR4000>>������������ط���Ľ���
 -----------------------------------------------------------------------------------------*/ 

int init_comm()
{
	//uchar *transfer_buf;

	//transfer_buf = (uchar *)malloc( sizeof(uchar) * nRF2401_BUFFER_LEN);

	//nRF2401_dev.buf.buf = transfer_buf;
	nRF2401_dev.buf.len = nRF2401_BUFFER_LEN;
	nRF2401_dev.buf.pos = 0;
	nRF2401_dev.get_packet = get_nRF2401_packet;
	nRF2401_dev.send_packet = send_nRF2401_packet;
	nRF2401_dev.init_dev = init_nRF2401_dev_rx;
	nRF2401_dev.packet_error = 0;
	nRF2401_dev.RFconf = g_default_nRF2401_conf;//������init_nRF2401_dev_rx��������д��һ��RFconf��ֵ�����ظ�
	SetPowerUp(1);
	SetCE( 0 );
	SetCS( 1 );
	delay(1000);
	init_nRF2401_dev_rx();  

	set_receive_flag();

	return 0;
}


/*--------------------------------------------------------------------------------------------
 *  �������ƣ� do_comm
 *  ���������� ͨѶ���ܺ����������ж��Ƿ��յ�ͨѶ��������յ�����ͨѶ��������g_comm.buffer�У�
 *             ���н�������ϴ�һ������
 *
 *
 *  ���ߣ�     ����
 *  �޸����ڣ� 2011-9-15
 ---------------------------------------------------------------------------------------------*/ 

int do_comm()
{
	if(receive_flag==1){   
		if( ( nRF2401_dev.get_packet( &nRF2401_dev ) ) > 0 )//���� nRF2401_dev.get_packet,����յ����ú���������һ������0�����ݳ���
		{

			receive_flag=0;  
			memcpy( g_comm.buffer, nRF2401_dev.buf.buf, nRF2401_dev.buf.pos );

			g_comm.buffer_pos = (unsigned char)nRF2401_dev.buf.pos;
                          
			do_comm_up();

			do_packet_process( g_comm.buffer, g_comm.buffer_pos ); 

			if(packet_cnt>254)
				packet_cnt = 0;
			else
				packet_cnt++;

		}
	}           //abao
	return 0;

}


/*-----------------------------------------------------------------------------
 *  �������ƣ� do_comm_up
 *  ���������� �ϴ�ִ�к���������ȫ�ֱ���type����ֵ���������ú���ģʽ�����ϴ�
 *  
 *
 *
 *  ���ߣ�     ����
 *  �޸����ڣ� 2011-9-15
 -------------------------------------------------------------------------------*/ 

int do_comm_up()
{
	short temp;
	int i;

	nRF2401_dev.buf.pos = PACKET_LEN_UP;

    static char data[7]={0,0,0,0,0,0,0};           //2011��������ǰ������ÿ�η���ռ�������data���䣬����ֱ�Ӹ�һ������ռ䣬ÿ�θ���

	/*----------------------------ͨ��ģʽ�µ������ϴ�--------------------------------*/

	if(type==PACKET_Normal)                      //����ʱ�����ϴ�
	{
		  
			  /* ---------�μ�Э��3.0������ͣʱ��֡���ذ���ʽ-----------*/
		if((((g_comm.buffer[3]&0x8)>>3)==1)||(((g_comm.buffer[7]&0x8)>>3)==1)||(((g_comm.buffer[11]&0x8)>>3)==1))//�ж�stop��־//test-lnc
		{
			temp=0;
            packet_flag=0;
            
			if(g_robot.num > 8)
			{
				if(((g_comm.buffer[1]&0x0f)&(0x01<<(g_robot.num-9)))==0)
				{   
				  temp=-1;
				}
			}
			else
			{
				if(((g_comm.buffer[2]&0xff)&(0x01<<(g_robot.num-1)))==0)	//�жϰ��������������Ƿ����Լ���
				{ 
				  temp=-1;
				}
			}
		
			if(temp!=-1)
			{
                temp=0;
		      	if(g_robot.num < 9)
			    {
				    for(i=0;i<(g_robot.num-1);i++)
					{
						if(g_comm.buffer[2]&(0x01<<i))
							temp++;
					}
			    }
                else
			    {
			        for(i=0;i<8;i++)
				    {
				        if(g_comm.buffer[2]&(0x01<<i))
						    temp++; 
				    }
				    for(i=0;i<(g_robot.num-9);i++)
				    {
				        if((g_comm.buffer[1]&0x0f)&(0x01<<i))
					       temp++;
			        }
                }//���Լ��ĳ���������������
			}
		
			if((temp!=-1)&&(((g_comm.buffer[3+4*temp]&0x8)>>3)==1))
			{
				stop_mode_packet(data);

				if(packet_flag==1)
				{   
					nRF2401_dev.buf.buf[0]=data[0];
					nRF2401_dev.buf.buf[1]=data[1];
					nRF2401_dev.buf.buf[2]=data[2]; 
					nRF2401_dev.buf.buf[3]=data[3];
					nRF2401_dev.buf.buf[4]=data[4];
					nRF2401_dev.buf.buf[5]=data[5]; 
					memset(&nRF2401_dev.buf.buf[6], 0, sizeof(unsigned char) * (PACKET_LEN_UP - 6));//����Ķ���ֵ0
										  
					nRF2401_dev.send_packet( &nRF2401_dev );
				}
			}
			else 
			{
				//delay(1500);
				//set_a_bit_receive();
				//g_comm_up_finish_flag = 1;
			}
		}


			/*--------------�μ�Э��0.2����ʱ��֡���ذ���ʽ----------------------*/
		else
		{
			packet(data);
		    if(packet_flag == 1)
			{
				nRF2401_dev.buf.buf[0]=0xFF;	            
				nRF2401_dev.buf.buf[1]=0x02;	                
				nRF2401_dev.buf.buf[2]=data[2]; 
				nRF2401_dev.buf.buf[3]=data[3];
				nRF2401_dev.buf.buf[4]=data[4];
				nRF2401_dev.buf.buf[5]=data[5];
				memset(&nRF2401_dev.buf.buf[6], 0, sizeof(unsigned char) * (PACKET_LEN_UP - 6));			
				nRF2401_dev.send_packet( &nRF2401_dev );
			}
		}
	}
	
	/*----------------------------------Ƶ�����ģʽ�ϴ�-----------------------------------*/
	/*--------------------���ǵ�������ʱ�����⣬��������ϴ��������ϴ���-------------------*/
	else if(type==PACKET_DEBUG_Check_Frq)
	{
	    if(g_comm.buffer[0]==0xFF && g_comm.buffer[1]==0xCF)
		{
			if(g_comm.buffer[2]==0x0A && g_comm.buffer[3]==0x0B && g_comm.buffer[4]==0x0C)
			{
				check_frq_start=1;
				check_frq_counter1=0;
				check_frq_counter2=0;
				{
					int temp;
					
					nRF2401_dev.buf.buf[0]=0xFF; 
					nRF2401_dev.buf.buf[1]=0xCF;
					nRF2401_dev.buf.buf[2]=0xAA;
					nRF2401_dev.buf.buf[3]=0xAA;
					nRF2401_dev.buf.buf[4]=0xAA;
					memset(&nRF2401_dev.buf.buf[5], 0, sizeof(unsigned char) * (PACKET_LEN_UP - 5));			
					nRF2401_dev.send_packet( &nRF2401_dev );

					temp = g_comm.buffer[5];

					if(temp<=7)
						temp= (temp<<2);
					else
						temp=(temp<<2)+58;
					if(g_robot.frq != temp)
					{
						delay(1000);
                        g_robot.frq = temp;
						change_nRF2401_frq(g_robot.frq);
					}
				}
			}
			else if(check_frq_start == 1 && g_comm.buffer[2]==0xBB && g_comm.buffer[3]==0xBB && g_comm.buffer[4]==0xBB)
			{
				static int comm_count;

				check_frq_counter1++;

				/*count the right packet */
				comm_count++;
				if(comm_count<15)
				{
					led_on(1);
				}
				else 
				{
					led_off(1);
				}
				if(comm_count>30)
					comm_count=0;
				
			}
			else if(check_frq_start == 1 && g_comm.buffer[2]==0xAA && g_comm.buffer[3]==0xAA && g_comm.buffer[4]==0xAA)
			{
				check_frq_counter2++;
				nRF2401_dev.buf.buf[0] = 0xFF;
				nRF2401_dev.buf.buf[1] = 0xA7;
				nRF2401_dev.buf.buf[2] = 0x99;
				nRF2401_dev.buf.buf[3] = 0x88;
				nRF2401_dev.buf.buf[4] = 0x77;
				memset(&nRF2401_dev.buf.buf[5],0,sizeof(unsigned char) * 20);
				nRF2401_dev.send_packet( &nRF2401_dev );
			}
			else if(check_frq_start == 1 && g_comm.buffer[2]==0x0D && g_comm.buffer[3]==0x0E && g_comm.buffer[4]==0x0F)
			{
				unsigned int temp1;
				unsigned int temp2;
				unsigned int i1;
				unsigned int i2;
				unsigned char num;

				temp1=check_frq_counter1;
				i1=0;
				do{
					num=temp1%10;
					temp1=temp1/10;
					nRF2401_dev.buf.buf[2+i1]=num;
					i1++;
				}while(temp1 != 0);
				
				temp2=check_frq_counter2;
				i2=0;
				do{
					num=temp2%10;
					temp2=temp2/10;
					nRF2401_dev.buf.buf[3+i1+i2]=num;
					i2++;
				}while(temp2 != 0);
				
				nRF2401_dev.buf.buf[0]=0xFF; 
				nRF2401_dev.buf.buf[1]=0xCF;
				nRF2401_dev.buf.buf[2+i1]=0xDD;
				nRF2401_dev.buf.buf[3+i1+i2]=0xEE;
				memset(&nRF2401_dev.buf.buf[4+i1+i2], 0xa, sizeof(unsigned char) * (PACKET_LEN_UP - 4 - i1-i2));			
				nRF2401_dev.send_packet( &nRF2401_dev );


			}
			else if(check_frq_start == 1 && g_comm.buffer[2]==0xFF && g_comm.buffer[3]==0xFF && g_comm.buffer[4]==0xFF)
			{
				check_frq_start=0;
			}
		}
		g_comm_overflow_flag = 0;
	}
	
	g_comm_up_finish_flag = 1;//
	return 0;
}


/*-----------------------------------------------------------------------------------------
 *  �������ƣ� do_packet_process
 *  ���������� �Խ��յ�������data���н�������ݲ�ͬ��ģʽ�в�ͬ�Ľ����ʽ����Ӧ�в�ͬ�Ľ������
 *             ����Ľ��������λ��Packet.c��
 *
 *
 *  ���ߣ�     ����
 *  �޸����ڣ� 2011-9-15
 -------------------------------------------------------------------------------------------*/ 
int do_packet_process( unsigned char *data, int len )
{   


	type = get_packet_type( data, len );
	
	
    /*----------------------------ͨ��ģʽ�µ����ݽ��--------------------------------*/
	/*-----------------------�μ�Э��0.1����ʱͨѶ����ʽ-----------------------------*/
	if(type == PACKET_Normal&& (g_robot.mode == NORMAL_MODE))
	{
		/* parse robot command */
		packet_robot_t packet;
		
		memset( &packet, 0, sizeof( packet ) );

		if( decode_packet_robot( &packet, data, len ) < 0 )
		{
			/* parse error */
			g_comm.packet_error++;

			return -1;
		}
		
		/* execute the command */
		on_robot_command( &packet );
	}
	
	return 0;

}


/*-----------------------------------------------------------------------------------------
 *  �������ƣ� is_comm_overflow
 *  ���������� ͨѶ��ʱ��ʲô������
 *
 *
 *  ���ߣ�     lyc
 *  �޸����ڣ� 2016-3-19
 -------------------------------------------------------------------------------------------*/ 

int is_comm_overflow()
{
	if(g_comm_overflow_flag)					//ͨѶ��ʱ��ʲô��������
	{
		g_comm_up_finish_flag = 1; 		//��ֹͨѶ�жϣ���λ����ͨѶΪ����ģ���־λ
		do_dribbler(0);
		do_move(0,0,0);
		do_shoot(0,0);
		do_chip(0,0);
		
		#ifdef ENABLE_2401_RESTART
			rst_nRF2401();
		#else
			set_a_bit_receive(); 
		#endif
		
		g_comm_overflow_flag = 0;
	}

	return 0;
}


/*-----------------------------------------------------------------------------------------
 *  �������ƣ� comm_overflow
 *  ���������� ��ͨѶ����Ϊ����ģʽ������λ�ɽ��ܱ�־λ
 *
 *
 *  ���ߣ�     lyc
 *  �޸����ڣ� 2016-3-19
 -------------------------------------------------------------------------------------------*/ 
int set_receive_mode()
{
	if(g_set_receive_mode_flag == 1)
	{				
		set_a_bit_receive();
		g_set_receive_mode_flag = 0;
	}
	
	return 0;
}

