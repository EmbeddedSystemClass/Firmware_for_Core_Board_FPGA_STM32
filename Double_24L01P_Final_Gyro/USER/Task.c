#include "Task.h"
#include "robot.h"
#include "string.h"
#include "spi.h"
#include "math.h"

#define SHOOT_INTERVAL 500
extern robot_t g_robot;

/*task_create������
1.����λ��������ָ�������λ��ִ��ʲô������timeΪ��ָ��ִ�е�ʱ��
2.modeΪ0ʱ˵��ָ��δ�����mode = 1����ƶ�ָ� mode = 2�������ָ� mode = 3 �������ָ��
3.ָ���Խ��˵�����ȼ�Խ�ߣ�ͬmode��ָ��ֻ�ᱻִ�����ȼ���ߵ��Ǹ�
4.�����������ִ��ʱ�䳬����λ�������������ᱻ��һ�����ݰ��������˳���ִ�е�ָ��time����Ϊ100���㹻�󣩼���
5.����ָ����δ�漰�Ĳ���������ı䵱ǰ״ֱ̬����ʱ�����ڳ��ƶ�ʱδ������ָ�������ָ��ᱻ���ģ�ֱ����ʱ��
*/
void task_create(unsigned char mode, 
                double speed_x, double speed_y, double speed_r, 
                unsigned int dribbler, unsigned int strength, unsigned int chip_shoot_choose, 
                int start_ms_counter, int end_ms_counter, int task_priority)
{
   switch(mode)
   {
        case 1:
            g_robot.its_action[task_priority].mode = mode;
            g_robot.its_action[task_priority].robot_speed_x = speed_x / 100.0;
            g_robot.its_action[task_priority].robot_speed_y = speed_y / 100.0;//��λ��cm/sת����m/s
            g_robot.its_action[task_priority].robot_speed_r = speed_r * WHEEL_CENTER_OFFSET;//��λ��rad/s
            g_robot.its_action[task_priority].start_ms_counter = start_ms_counter;
            g_robot.its_action[task_priority].end_ms_counter = end_ms_counter;
            return;
        case 2:
            g_robot.its_action[task_priority].mode = mode;
            g_robot.its_action[task_priority].its_ball_action.dribbler = dribbler; 
            g_robot.its_action[task_priority].start_ms_counter = start_ms_counter;
            g_robot.its_action[task_priority].end_ms_counter = end_ms_counter;
            return;
        case 3:
            g_robot.its_action[task_priority].mode = mode;
            g_robot.its_action[task_priority].its_ball_action.strength = strength;
            g_robot.its_action[task_priority].its_ball_action.shoot_chip_choose = chip_shoot_choose;
            g_robot.its_action[task_priority].start_ms_counter = start_ms_counter;
            g_robot.its_action[task_priority].end_ms_counter = end_ms_counter;
            return;
        default:
            return;
   }
}

/*
˵����
	1.���ж���ÿ��һ��ʱ��ִ��һ��task_update��������������滮(��ʱ���ж�)
	2.���ü���Ƿ��п���ָ���Ϊ���������ȼ���͵�Ĭ��ָ����û������ָ��ͻ�ִ��Ĭ��ָ�ʹ������ֹͣ
	3.�����û��ָ�������ʱ�䣬�ͺ��Դ�ָ�Ѱ����һ�����ȼ����͵�ָ��
	4.������Ӧ�����ж���ִ��
	5.Ŀǰ�������Ƿ�����������ָ������и���
	6.Ŀǰ������ת�����֧��4Wת
*/
void task_update()
{
   
     int temp, mode_temp = 0;
     //��3ģʽ��ʼɸ��������ȼ���ָ�ִ��
     for(mode_temp = 3; mode_temp >0; mode_temp--)
     {
        for(temp = 15;temp >=0; temp--) 
        {
            if(g_robot.its_action[temp].start_ms_counter > 0)
            {
                g_robot.its_action[temp].start_ms_counter--;
                continue;
            }
            if(g_robot.its_action[temp].mode == mode_temp)
            {
                if(mode_temp == 3)
                {
                    
//                    if((g_robot.current_state.shoot_off_timer == 0)&&
//                       (IORD(PIO_SHOOT_OFF_BASE, 0))&&
//                       (is_ball_detected()))
//                    {
//                        switch ( g_robot.its_action[temp].its_ball_action.shoot_chip_choose )
//                        {
//                            // channel 0 is shoot , high is active , 
//                            case SHOOT : 
//                                IOWR(SHOOT_TIMER_SHOOT_BASE, 0, g_robot.its_action[temp].its_ball_action.strength);    //change the shoot time function
//                                g_robot.its_action[temp].end_ms_counter = 0;
//                                g_robot.current_state.shoot_off_timer = SHOOT_INTERVAL;
//                            break;
//                            // channel 1 is chip ,  high is active , 
//                            case CHIP :
//                                IOWR(SHOOT_TIMER_CHIP_BASE, 0, g_robot.its_action[temp].its_ball_action.strength);
//                                g_robot.its_action[temp].end_ms_counter = 0; 
//                                g_robot.current_state.shoot_off_timer = SHOOT_INTERVAL;
//                            break;
//                            // default
//                            default :
//                                IOWR(SHOOT_TIMER_SHOOT_BASE,0,0x0);
//                                IOWR(SHOOT_TIMER_CHIP_BASE,0,0x0);
//                            break;
//                         }
//                    }
//                    g_robot.current_state.task[2] = temp;
//                    break;
                }
                else if(mode_temp == 2)
                {
                   
//                    IOWR_AVALONCONTROL_SET(MOTOR_4_BASE, -g_robot.its_action[temp].its_ball_action.dribbler);
										SPI_Master_ReadWrite_Byte(g_robot.its_action[temp].its_ball_action.dribbler, 0x04);
                    g_robot.current_state.task[1] = temp;

                    break;
                }
                else if(mode_temp == 1)
                {
                   
                    {
                        int i; 
                       
                             if (sqrt((g_robot.its_action[temp].robot_speed_x - g_robot.current_state.robot_speed_x) * (g_robot.its_action[temp].robot_speed_x - g_robot.current_state.robot_speed_x) 
                                    + (g_robot.its_action[temp].robot_speed_y - g_robot.current_state.robot_speed_y) * (g_robot.its_action[temp].robot_speed_y - g_robot.current_state.robot_speed_y)) > 0.01)
                            {
                                g_robot.current_state.robot_speed_x += 0.01 * (g_robot.its_action[temp].robot_speed_x - g_robot.current_state.robot_speed_x)
                                                                         / sqrt((g_robot.its_action[temp].robot_speed_x - g_robot.current_state.robot_speed_x) 
                                                                                * (g_robot.its_action[temp].robot_speed_x - g_robot.current_state.robot_speed_x) 
                                                                                + (g_robot.its_action[temp].robot_speed_y - g_robot.current_state.robot_speed_y) 
                                                                                * (g_robot.its_action[temp].robot_speed_y - g_robot.current_state.robot_speed_y));
                                g_robot.current_state.robot_speed_y += 0.01 * (g_robot.its_action[temp].robot_speed_y - g_robot.current_state.robot_speed_y) 
                                                                         / sqrt((g_robot.its_action[temp].robot_speed_x - g_robot.current_state.robot_speed_x) 
                                                                                * (g_robot.its_action[temp].robot_speed_x - g_robot.current_state.robot_speed_x) 
                                                                                + (g_robot.its_action[temp].robot_speed_y - g_robot.current_state.robot_speed_y) 
                                                                                * (g_robot.its_action[temp].robot_speed_y - g_robot.current_state.robot_speed_y));
                                g_robot.current_state.robot_speed_r += 0.01 * g_robot.its_action[temp].robot_speed_r
                                                                         / sqrt((g_robot.its_action[temp].robot_speed_x - g_robot.current_state.robot_speed_x) 
                                                                                * (g_robot.its_action[temp].robot_speed_x - g_robot.current_state.robot_speed_x) 
                                                                                + (g_robot.its_action[temp].robot_speed_y - g_robot.current_state.robot_speed_y) 
                                                                                * (g_robot.its_action[temp].robot_speed_y - g_robot.current_state.robot_speed_y));
                            }
                            else
                                {
                                    g_robot.current_state.robot_speed_x = g_robot.its_action[temp].robot_speed_x;
                                    g_robot.current_state.robot_speed_y = g_robot.its_action[temp].robot_speed_y;
                                    g_robot.current_state.robot_speed_r = g_robot.its_action[temp].robot_speed_r;
                                }

                        /* change wheels' speed set point */
                        for( i = 0; i < CHANNEL_NUM; i++ )
                        {
                            g_robot.wheels[ i ].set =V2N(g_robot.sin_angle[ i ] * g_robot.current_state.robot_speed_x
                                                        + g_robot.cos_angle[ i ] * g_robot.current_state.robot_speed_y
                                                        + g_robot.current_state.robot_speed_r);
                        }
                        for( i = 0; i < CHANNEL_NUM; i++ )
                        {
                            switch(i)
                            {
                                case 0: SPI_Master_ReadWrite_Byte(g_robot.wheels[ i ].set, 0x00); break;                      
                                case 1: SPI_Master_ReadWrite_Byte(g_robot.wheels[ i ].set, 0x01); break;                  
                                case 2: SPI_Master_ReadWrite_Byte(g_robot.wheels[ i ].set, 0x02); break;                  
                                case 3: SPI_Master_ReadWrite_Byte(g_robot.wheels[ i ].set, 0x03); break;                        
                                default:break;
                            }       
                        }   
                        
                     }
                     g_robot.current_state.task[0] = temp;
                     break;
                }
								else if(mode_temp == 0)
								{
									int i;
									for( i = 0; i < CHANNEL_NUM+1; i++ )
									{
											switch(i)
											{
													case 0: SPI_Master_ReadWrite_Byte(0, 0x00); break;                      
													case 1: SPI_Master_ReadWrite_Byte(0, 0x01); break;                  
													case 2: SPI_Master_ReadWrite_Byte(0, 0x02); break;                  
													case 3: SPI_Master_ReadWrite_Byte(0, 0x03); break; 
													case 4: SPI_Master_ReadWrite_Byte(0, 0x04); break;
													default:break;
											}       
									}   
									
								}
                
            }//g_robot.its_action[temp].mode == mode_temp
            
        }//for(temp = 15;temp >=0;temp--)
        
     }//for(mode_temp = 3;mode_temp >0;mode_temp--)
     for(temp = 15;temp >2; temp--)
     {
        if(g_robot.its_action[temp].end_ms_counter <= 0)
        {
            task_delete(temp);
        }
        else
            g_robot.its_action[temp].end_ms_counter--;
     }
 

}//task_update

void task_delete(int priority)
{
    //memset(&g_robot.its_action[priority], 0, sizeof(g_robot.its_action[priority]));//priority????????
    
    g_robot.its_action[priority].mode = 0;
}

void task_delete_all()
{
    memset(&g_robot.its_action[3], 0, 13*sizeof(g_robot.its_action[0]));
}
