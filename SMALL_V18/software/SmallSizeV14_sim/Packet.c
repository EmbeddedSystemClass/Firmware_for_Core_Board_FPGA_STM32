#include "Packet.h"

char packet_flag;
extern robot_t g_robot;
extern unsigned char g_power;
extern int ADvalue;
extern unsigned int g_comm_overflow;			//�յ��Լ������ݣ�ͨѶ������㡣
extern char g_comm_overflow_flag;
extern int max_shot_strength_set;
extern char shooter;
extern int dribbler_speed_max_set;
extern unsigned char last_beep;




/*-----------------------------------------------------------------------------
 *  �������ƣ� stop_mode_packet
 *  ���������� ���յ�stop��־λ��������һ��Э��Ļذ�
 *-------------�μ�Э��0.3������ͣʱ��֡���ذ���ʽ-----------*
 *  ���ߣ�     ����
 *  �޸���:	�����
 *  �޸����ڣ� 2012-12-3
 -------------------------------------------------------------------------------*/ 
int stop_mode_packet(char *q)
{
	q[0]=0xff;
	q[1]=0x01;
	q[2]=g_robot.num&0x0f;
	g_power=(IORD(ADGETNEW2_0_BASE,0)&0x00ff);
	if(g_power==0xFF)
	{
		g_power=0xFE;
	}
	q[3]=g_power;
	ADvalue=(IORD(ADGETNEW2_0_BASE,1)&0x00ff);
	if(ADvalue==0xFF)
	{
		ADvalue=0xFE;
	}
	q[4]=ADvalue;
	q[5]=(is_infra_broken()<<7);//����ֻ���˺����־һλ����Ļ��д����
	packet_flag=1;
	return 0;
}


/*-----------------------------------------------------------------------------
 *  �������ƣ� packet
 *  ���������� ��������������践����λ������Ϣ�����
 		������Ƿ���״̬�仯(ƽ�䡢���䡢������һ�ֱ仯)��
 		������״̬�仯ʱ����λ�ϴ���־ packet_flag
 *-------------�μ�Э��0.2����ʱ��֡���ذ���ʽ-----------*
 *  ���ߣ�     ����
 *  �޸���:	  �
 *  �޸����ڣ� 2014-5-22
 -------------------------------------------------------------------------------*/ 
int packet(char *q)
{
	static int last_infra = 0;
	static int now_infra = 0;

	static int to_shoot = 0;
	static int to_chip = 0;

	static int finish_shoot = 0;
	static int finish_chip = 0;

	static int m = 0;
	static int n = 5;

    now_infra=is_ball_detected();
	
	if(shooter==0x02)
	{
	    to_shoot = 1;
		shooter = 0x00;
	}
        
	if(shooter==0x01)
	{
		to_chip=1;
    	shooter = 0x00;
	}
	
    if(now_infra == 0)
    {
        if(to_shoot == 1)
        {
            finish_shoot = 1;
            to_shoot = 0;
			n=0;
			m++;
			packet_flag = 1;
        }
		
        if(to_chip == 1)
        {
            finish_chip = 1;
            to_chip = 0;
			n=0;
			m++;
			packet_flag = 1;
        }    
    }
    
	if(n >= 5)   	
	{
		if(finish_shoot == 1)
			finish_shoot = 0;
		
		if(finish_chip == 1)
			finish_chip = 0;
		
		if((last_infra != now_infra))
		{
			n=1;
			m++;
			packet_flag = 1;
		}
		else
			packet_flag = 0;
	}			
	else
	{
		n++;
		packet_flag=1;
	}

	if(m==127)
	{	
		m=0;
	}

	q[0]=0xff;
    q[1]=0x02;
    q[2]=g_robot.num&0x0F;
	q[3]=(now_infra<<6)+(finish_shoot<<5)+(finish_chip<<4);
	q[4]=m;
	q[5]=n;

	last_infra = now_infra;

	return 0;

}



/*-----------------------------------------------------------------------------
 *  �������ƣ� get_packet_type
 *  ������������ȡ����ģʽ������֮����
 *  ���ߣ�     ����
 *  �޸���:	�����
 *  �޸����ڣ� 2012-12-3
 -------------------------------------------------------------------------------*/ 
packet_type_t get_packet_type( unsigned char* data, int len )
{
	/* check run-time mode */
	volatile unsigned char temp;
	temp = data[1];
	temp = temp & 0xF0;
	temp = temp >> 4;
	
	switch(temp){
		case DEBUG_FUNCTION_Normal:                  	//����ģʽ
			return PACKET_Normal;

		case DEBUG_FUNCTION_Set_9557_error:
			return PACKET_DEBUG_Set_9557_error; 

		default:
			return PACKET_NONE;                    	//�������ݰ�
	}
}

/*-----------------------------------------------------------------------------
 *  �������ƣ� decode_packet_robot
 *  ���������� ����Ӧ���ŵ�С�������˶�����
 *  ���ߣ�     ����
 *  �޸���:	�����
 *  �޸����ڣ� 2012-11-24
 -------------------------------------------------------------------------------*/ 
int decode_packet_robot( packet_robot_t *packet, unsigned char *data, int len )
{
	unsigned short temp = 0;
	unsigned char  i=0;  
  
	if( packet == NULL || data == NULL)
		return -1; 

    if(g_robot.num > 8)
    {
	    if(((data[1]&0x0f)&(0x01<<(g_robot.num-9)))==0)
	    {
		    return  -1;  	
	    }
    }
    else
    {
 	    if(((data[2]&0xff)&(0x01<<(g_robot.num-1)))==0)  //�ж��������Ƿ����Լ�������
	    { 
	      	return  -1;  	
	    }
    }

    g_comm_overflow = 0;			//�յ��Լ������ݣ�ͨѶ������㡣
    g_comm_overflow_flag = 0;
    temp=0;
  
    if(g_robot.num < 9)
    {
	    for(i=0;i<(g_robot.num-1);i++)
	  	{
	  	    if(data[2]&(0x01<<i))
		    temp++;
		 }
    }
    else
    {
 	    for(i=0;i<8;i++)
	  	{
	  	 	if(data[2]&(0x01<<i))
		 	temp++;	
		}
        for(i=0;i<(g_robot.num-9);i++)
    	{
        	if((data[1]&0x0f)&(0x01<<i))
        	temp++;
     	}
  	}
  	i=temp*4+3;                           //������ʼ��
	
 
  	packet->robot_num = g_robot.num;


	switch (temp)
	{
	   	case 0:
		{	
			temp=data[ i ];
			packet->dribbler = ((( temp >> 4 ) & 0x03));	
			packet->dribbler = ( ( temp & 0x80) ? (-packet->dribbler) : packet->dribbler);
			temp= data[ i+15 ];
			if( ( data[ i ] >> 6 ) & 0x1 )//����
			{
	        	if(temp == 127 )
					packet->chip = max_shot_strength_set;
			    else
					packet->chip= temp;
	
			}

			else//ƽ��
			{   	
				if(temp == 127 )
					packet->shoot = max_shot_strength_set;
				else
					packet->shoot= temp;
			}					
			
			temp = data[ i+1 ];
			packet->speed_x = temp & 0x7F;
			unsigned short high_value_x = (unsigned short)data[15];
			high_value_x = ((unsigned short)(high_value_x & 0xc0)) << 1;
			packet->speed_x = packet->speed_x+high_value_x;
			temp = data[ i+1 ];
			packet->speed_x = ( ( temp & 0x80 ) ? ( -packet->speed_x ) : packet->speed_x );
				 
			temp = data[ i+2 ];
			packet->speed_y = temp & 0x7F;
			unsigned short high_value_y = (unsigned short)data[15];
			high_value_y = ((unsigned short)(high_value_y & 0x30)) << 3;
			packet->speed_y = packet->speed_y+high_value_y;
			temp = data[ i+2 ];
			packet->speed_y = ( ( temp & 0x80 ) ? ( -packet->speed_y ) : packet->speed_y );
				 
			temp = data[i+3];
			packet->speed_rot = temp & 0x7F;
			unsigned short high_value_r = (unsigned short)data[15];
		 	high_value_r = ((unsigned short)(high_value_r & 0x0c)) << 5;
			packet->speed_rot = packet->speed_rot+high_value_r;

			temp = data[ i+3 ];
			packet->speed_rot = ( ( temp & 0x80 ) ? ( -packet->speed_rot ) : packet->speed_rot );
			   
		}break;



	   	case 1:
		{
			temp=data[ i ];
			packet->dribbler = ((( temp >> 4 ) & 0x03));
			packet->dribbler = ( ( temp & 0x80) ? (-packet->dribbler) : packet->dribbler);
			temp= data[ i+12 ];
			if( ( data[ i ] >> 6 ) & 0x1 )//tiao she
			{
	    		if(temp == 127 )
					packet->chip = max_shot_strength_set;
				else
					packet->chip= temp;
	
			}

			else//ping she
			{
				if(temp == 127 )
					packet->shoot = max_shot_strength_set;
				else
					packet->shoot= temp;

			}

			temp = data[ i+1 ];
			packet->speed_x = temp & 0x7F;
			temp = (data[16]&0xc0)<<1;
			packet->speed_x = packet->speed_x+temp;
			temp = data[ i+1 ];
			packet->speed_x = ( ( temp & 0x80 ) ? ( -packet->speed_x ) : packet->speed_x );
				 
			temp = data[ i+2 ];
			packet->speed_y = temp & 0x7F;
			temp = (data[16]&0x30)<<3;
			packet->speed_y = packet->speed_y+temp;
			temp = data[ i+2 ];
			packet->speed_y = ( ( temp & 0x80 ) ? ( -packet->speed_y ) : packet->speed_y );
				 
			temp = data[i+3];
			packet->speed_rot = temp & 0x7F;
			temp = (data[16]&0x0c)<<5;
			packet->speed_rot = packet->speed_rot+temp;
			temp = data[ i+3 ];
			packet->speed_rot = ( ( temp & 0x80 ) ? ( -packet->speed_rot ) : packet->speed_rot );			 
		}break;


		case 2:
		{
			temp=data[ i ];
			packet->dribbler = ((( temp >> 4 ) & 0x03));
			packet->dribbler = ( ( temp & 0x80) ? (-packet->dribbler) : packet->dribbler);
			temp= data[ i+9 ];
			if( ( data[ i ] >> 6 ) & 0x1 )//tiao she
			{
	          	if(temp == 127 )
					packet->chip = max_shot_strength_set;
				else
					packet->chip= temp;
	
			}
			else//ping she
			{   
				if(temp == 127 )
					packet->shoot = max_shot_strength_set;
				else
					packet->shoot= temp;

			}
			
			temp = data[ i+1 ];
			packet->speed_x = temp & 0x7F;
			temp = (data[17]&0xc0)<<1;
			packet->speed_x = packet->speed_x+temp;
			temp = data[ i+1 ];
			packet->speed_x = ( ( temp & 0x80 ) ? ( -packet->speed_x ) : packet->speed_x );
			   
			temp = data[ i+2 ];
			packet->speed_y = temp & 0x7F;
			temp = (data[17]&0x30)<<3;
			packet->speed_y = packet->speed_y+temp;
			temp = data[ i+2 ];
			packet->speed_y = ( ( temp & 0x80 ) ? ( -packet->speed_y ) : packet->speed_y );
			   
			temp = data[i+3];
			packet->speed_rot = temp & 0x7F;
			temp = (data[17]&0x0c)<<5;
			packet->speed_rot = packet->speed_rot+temp;
			temp = data[ i+3 ];
			packet->speed_rot = ( ( temp & 0x80 ) ? ( -packet->speed_rot ) : packet->speed_rot );
	
	
		}break;


		default: 

		break;

	}

/*	if(packet->shoot == max_shot_strength_set || packet->shoot >= 120) last_beep = 1; //����	*/
	
	return 0;
}


