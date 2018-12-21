#include "Action.h"

char shooter;
char g_is_shoot_set = 0;
int g_is_shoot_in_main_loop = 0;
int g_is_chip_in_main_loop = 0;
shooter_t g_shooter[ SHOOTER_NUM ];

extern robot_t g_robot;
extern int wheel_reduction_ratio_x_set;
extern int wheel_reduction_ratio_yz_set;


/*---------function headers----------*/
int shooter_off( int channel );
long V2N( double vel );
int set_shooter( int channel, int value );



int update_shooter()
{
	int i = 0; 
	for(i=0; i<SHOOTER_NUM; i++)
	{
		if(g_shooter[i].count_down > 0)
			g_shooter[i].count_down--;
		else
			g_shooter[i].count_down = 0;
	}
	return 0;
	
}

int shooter_off( int channel )
{
	// shoot & chip disable 
	IOWR(SHOOT_TIMER_SHOOT_BASE,0,0x0);
	IOWR(SHOOT_TIMER_CHIP_BASE,0,0x0);

	g_shooter[ channel ].count_down = 0;

	return 0;
}


int init_shooter()
{
	shooter_off( SHOOT );
	shooter_off( CHIP );

	return 0;
}


/*-----------------------------------------------------------------
 *  �������ƣ� do_dribbler
 *  ���������� �����������Ȳ�����g_robot.dribbler.set����ֵ��
 *			   ����Ϊ�������ȵ�����������Ϊ0-1-2-3��
 *					 
 *  ���ߣ�     ����
 *  �޸����ڣ� 2012��11-29
 -----------------------------------------------------------------*/ 
int do_dribbler( int dribbler )		//1��1/4  	2�� 1/2    	3�� 3/4
{
  	int dribbler_temp;
  	dribbler_temp = fabs(dribbler);

    if(dribbler_temp == 0)
    {
        g_robot.dribbler.set = 0;
    }
    else if(dribbler_temp == 1)
    {
        g_robot.dribbler.set = 8;
    }
    else if(dribbler_temp == 2)
    {
        g_robot.dribbler.set = 16;
    }
    else if(dribbler_temp == 3)
    {
        g_robot.dribbler.set = 24;
    }	
  	
	IOWR_AVALONCONTROL_SET(MOTOR_4_BASE, -g_robot.dribbler.set);

 
	return 0;
}


/*-----------------------------------------------------------------
 *  �������ƣ� V2N
 *  ���������������ӵ��ٶȵ�λ��(m/s)ת�������̵�(��/PID����)
 *
 *
 *  ���ߣ�     �
 *  �޸����ڣ� 2012-11-29
 -----------------------------------------------------------------*/

long V2N( double vel )
{
	return vel * g_robot.kv2n;
}


/*-----------------------------------------------------------------
 *  �������ƣ� do_move
 *  ���������� �趨С����x,y,z���ٶȣ��β�Ϊx,y,z�ٶȣ�
 *			   		 �ٶȵ�λΪ���ʵ�λ
 *					
 *
 *  ��    �ߣ� ����
 *  �޸����ڣ� 2012��11-29
 -----------------------------------------------------------------*/
int do_move( int speed_x, int speed_y, int speed_rot )
{
	int i = 0;


	/* �������ٶ� vx, vy, vz are all measured in m/s */

	double vx = (double)(speed_x) / 100 ;
	double vy = (double)(speed_y)  / 100;
	double vz = (double)(speed_rot+buchang(speed_x, speed_y, speed_rot)) * 0.025 * WHEEL_CENTER_OFFSET;

	/* ���������ٶ�(m/s)�趨ֵ */
	double speed[ CHANNEL_NUM ] = { 0 };


	for( i = 0; i < 4; i++ )
	{
		/* trasnform wheel angle */
		speed[ i ] = ( g_robot.sin_angle[ i ] * vx + g_robot.cos_angle[ i ] * vy + vz );
	}


	/* change wheels' speed set point */
	for( i = 0; i < CHANNEL_NUM; i++ )
	{
		g_robot.wheels[ i ].set = V2N( speed[ i ] );
		switch(i)
		{
			case 0:	IOWR_AVALONCONTROL_SET(MOTOR_0_BASE,g_robot.wheels[ i ].set); break;
	        //IOWR(PIO_0_BASE, 0, g_robot.wheels[ i ].set); break;
			case 1:	IOWR_AVALONCONTROL_SET(MOTOR_1_BASE,g_robot.wheels[ i ].set);break;
	        //IOWR(PIO_1_BASE, 0, g_robot.wheels[ i ].set); break;
			case 2:	IOWR_AVALONCONTROL_SET(MOTOR_2_BASE,g_robot.wheels[ i ].set);break;
	        //IOWR(PIO_2_BASE, 0, g_robot.wheels[ i ].set); break;
			case 3:	IOWR_AVALONCONTROL_SET(MOTOR_3_BASE,g_robot.wheels[ i ].set);break;
	        //IOWR(PIO_3_BASE, 0, g_robot.wheels[ i ].set); break;
			default:break;
		}		
	}

	return 0;
}



/*�޶����ӵļ��ٶ�*/
/*-----------------------------------------------------------------
 *  �������ƣ� do_acc_handle_move
 *  �����������޶�С��xy�����Ϻϼ��ٶ�ֵС��MAX_ACC,
 *			  �����趨С��x,y,z�������ƶ��ٶȷֱ�Ϊ
 *			  speed_x, speed_y, speed_rot����λΪ���ʵ�λ
 *
 *  ���ߣ�     ����
 *  �޸����ڣ� 2012��11-29
 -----------------------------------------------------------------*/

int do_acc_handle_move(int speed_x,int speed_y,int speed_rot)
{
	static double last_speed_x = 0;
	static double last_speed_y = 0;


	double acc_x = 0;
	double acc_y = 0;
	double acc_whole = 0;
	double sin_x = 0;
	double sin_y = 0;
	acc_x = speed_x - last_speed_x;
	acc_y = speed_y - last_speed_y;
	acc_whole = acc_x * acc_x + acc_y * acc_y ;
	acc_whole = sqrt(acc_whole) + 0.001;
	sin_x = acc_x / acc_whole;
	sin_y = acc_y / acc_whole;


	if (acc_whole > MAX_ACC)
	{
		acc_whole = MAX_ACC;
		acc_x = acc_whole * sin_x;
		acc_y = acc_whole * sin_y;
		speed_x = ceil(last_speed_x + acc_x);
		speed_y = ceil(last_speed_y + acc_y); 
	}
	else
	{
		;
	}

	do_move(speed_x,speed_y,speed_rot);
	last_speed_x = speed_x;
	last_speed_y = speed_y;
    g_robot.tempspeed_x=speed_x;
    g_robot.tempspeed_y=speed_y;
	return 0;
}


/*-----------------------------------------------------------------
 *  �������ƣ� set_shooter
 *  ���������� ִ������ָ��β�channelΪ0ʱƽ��,
 * 			   Ϊ1ʱ���䣻�β�valueΪ�������ȷ�Χ
 *			   һ��Ϊ......,ִ������ǰ�����ݵ�ѹ��
 *			   ʲô���ȴ�С��ֻ�е��ݵ�ѹ�㹻��
 *             �������Ȳ�Ϊ0ʱ�������š�
 *					 
 *  ���ߣ�     ����
 *  �޸����ڣ� 2012��11-29
 -----------------------------------------------------------------*/ 

int set_shooter( int channel, int value )
{
    if(g_shooter[ channel ].count_down && (g_robot.mode!=OFFLINE_TEST_MODE) && (g_robot.mode!=SHOOT_CHIP_MODE)) 
	{
		return -1;
	}

	if(IORD(PIO_SHOOT_OFF_BASE, 0))
	{
		g_robot.is_cap_low = 0;
	}
	else
	{
		g_robot.is_cap_low = 1;
	}

	if((!g_robot.is_cap_low) && value )		//value=0��ʱ�� ���� ������Ҫ����count_down��ֵ������Ӱ������������
	{
		g_shooter[ channel ].strength = value;
		g_shooter[ channel ].count_down = SHOOTER_INTERVAL;

		switch ( channel )
		{
		    // channel 0 is shoot , high is active , 
			case SHOOT : 
				{
		    	IOWR(SHOOT_TIMER_SHOOT_BASE,0,value * 2500);	
			 	shooter=0x02;
		    	}break;
		    // channel 1 is chip ,  high is active , 
		    case CHIP :
				{
		      	IOWR(SHOOT_TIMER_CHIP_BASE,0,value * 2500);	//0~127*2500  ->   60*2500 ~ n*2500 
                                                            //value * (100*2500 - 2500*55)/127 + 2500*55
                                                            //(value*1.0 / 127.0 * 45.0) + 55
                //IOWR(SHOOT_TIMER_CHIP_BASE,0, (value*1.0 / 127.0 * 45.0 + 55) * 2500);
			 	shooter=0x01;
	      	}break;
		    // default
		    default:
		    	IOWR(SHOOT_TIMER_SHOOT_BASE,0,0x0);
		    	IOWR(SHOOT_TIMER_CHIP_BASE,0,0x0);
		    	return -1;
		    	break;
		  }
	  }

	return 0;
	
}



/*-----------------------------------------------------------------
 *  �������ƣ� do_shoot
 *  �����������β�shootΪ1ʱ���������������ƽ�䣬
 *			  ������û�����������¿��������жϡ�
 *			  ���β�shoot��chip��Ϊ�㣬��رպ����ж�
 *
 *  ���ߣ�    ����
 *  �޸����ڣ�2012��11-29
 -----------------------------------------------------------------*/ 
int do_shoot( int shoot, int chip )
{
	/* �ȱ���ƽ��ָ�� */
	g_robot.shoot = shoot;

	if( shoot )
	{
		if ( is_ball_detected()==1 )
		{
		   set_shooter( SHOOT, shoot );
		}
		else
		{
			IOWR(PIO_INFRA_BASE, 3, 0x00);        //���жϱ�־
			//IOWR(PIO_INFRA_BASE, 2, 0x1);			//���������жϡ�
		}
	}
	else if((chip == 0) && (shoot == 0))      //û������ֱҲû������ֵ��ʱ��ر��ж� 
	{   
		IOWR(PIO_INFRA_BASE, 2, 0x0);			//�رպ����жϡ�
	}

	return 0;
}


/*-----------------------------------------------------------------
 *  �������ƣ� do_chip
 *  �����������β�chipΪ1ʱ������������������䣬
 *			  ������û�����������¿��������жϡ�
 *			  ���β�shoot��chip��Ϊ�㣬��رպ����ж�
 *
 *  ���ߣ�     ����
 *  �޸����ڣ� 2012��11-29
 -----------------------------------------------------------------*/

int do_chip(int shoot, int chip )
{
  /* �ȱ�������ָ�� */
	g_robot.chip = chip;
    
  	if( chip )
	{	
		if ( is_ball_detected()==1 )
		{
			set_shooter( CHIP, chip );
		}
		else
		{
			IOWR(PIO_INFRA_BASE, 3, 0x00);        //���жϱ�־
			//IOWR(PIO_INFRA_BASE, 2, 0x1);			//���������жϡ�
		}
	}
  	else if((chip == 0) && (shoot == 0))
	{
		IOWR(PIO_INFRA_BASE, 2, 0x0);			//�رպ����жϡ�
	}

	return 0;
}



/* robot command handler */
/*-----------------------------------------------------------------
 *  �������ƣ� on_robot_command
 *  ���������� �β�ָ��ָ��洢�˶�������packet_robot_t
 *			   �ṹ��packet packet��ִ�и����˶�ָ��
 *					
 *
 *  ���ߣ�     ����
 *  �޸����ڣ� 2012-11-29
 -----------------------------------------------------------------*/

int on_robot_command( packet_robot_t *packet )
{   
	static int comm_count;

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

	if( g_robot.mode == NORMAL_MODE )
	{
	 		
		do_dribbler( packet->dribbler );
		 
		#ifdef ENABLE_SHOOTER
            // hwj hot fix in 2016.4.8 Iran Open
            g_is_shoot_in_main_loop = packet->shoot;
            g_is_chip_in_main_loop = packet->chip;
            
            //do_shoot(packet->shoot, packet->chip);
			//do_chip(packet->shoot, packet->chip);
		#endif

		//do_move( packet->speed_x, packet->speed_y, packet->speed_rot);
		do_acc_handle_move(packet->speed_x, packet->speed_y, packet->speed_rot);
		
	}
		
	return 0;
}

double buchang(double speed_x, double speed_y, double speed_rot)
{
    //speed_y             -180  -150  -120 -90    -60   -30  0  30     60     90     120     150   180
/*0*/double table[4][13]={-16,  -16,  -14, -10.2, -7,   -4,  0, 1.569, 4.614, 8.811, 13.836, 17,   25,
/*60*/                    -18,  -13,  -5,  -2,    0.2,  -1,  1, 2,     1,     1,     7,      10.8, 17.5, 
/*120*/                   -10,  -3,   6,   2.5,   0.5,  2,   1, 1,     2,     3,     4,      6,    18,
/*180*/                   -18,  0,    0,   1,     0,    0,   5, 3,     3,     5,     6,      12,   15   };
int index_x=0, index_y=0;
double weight[4];
double square[4];
double result;
if(1)
{
    if(speed_y>180)speed_y=180;
    if(speed_y<-180)speed_y=-180;
    if(speed_x>180)speed_x=180;
    if(speed_x<-180)speed_x=-180;
    if((speed_y>=-180)&&(speed_y<=180)&&(speed_x>=0)&&(speed_x<=180))
    {
        while(speed_x>60)
        {
            speed_x-=60;
            index_x++;
        }
        while(speed_y>-150)
        {
            speed_y-=30;
            index_y++;
        }//find the direct index in the table
        //calculate the value in the square
        square[0]=table[index_x][index_y];
        square[1]=table[index_x+1][index_y];
        square[2]=table[index_x][index_y+1];
        square[3]=table[index_x+1][index_y+1];
        weight[0]=sqrt((speed_x-60)*(speed_x-60)+(speed_y+150)*(speed_y+150));
        weight[1]=sqrt((speed_x)*(speed_x)+(speed_y+150)*(speed_y+150));
        weight[2]=sqrt((speed_x-60)*(speed_x-60)+(speed_y+180)*(speed_y+180));
        weight[3]=sqrt((speed_x)*(speed_x)+(speed_y+180)*(speed_y+180));
        result=(square[0]*weight[0]+square[1]*weight[1]+square[2]*weight[2]+square[3]*weight[3])/(weight[0]+weight[1]+weight[2]+weight[3]);
        return result;
    }
    else 
    return 0;
}
return 0;

}
double max_acc()
{
    if(g_robot.tempspeed_x<-0.5)return 35;
    else if(g_robot.tempspeed_x<-2)return 45;
    else return 26;
}
