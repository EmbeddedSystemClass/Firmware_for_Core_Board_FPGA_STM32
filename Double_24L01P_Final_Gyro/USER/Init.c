#include "Init.h"

void Init_all(void)
{
//	 g_robot.current_state.robot_speed_x = 0;
//	 g_robot.current_state.robot_speed_y = 0;
//	 g_robot.current_state.robot_speed_r = 0;
	SPI_Master_Init();

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	delay_init(168);

	LED_Init();
	TIM3_Int_Init(100-1,840-1);	//��ʱ��ʱ��84M����Ƶϵ��8400��84M/8400=10Khz�ļ���Ƶ�ʣ�����5000��Ϊ500ms
	EXTIX_Init();       //��ʼ���ⲿ�ж�����
	
/*����ģʽ��24L01��ʼ��*/	
  NRF24L01_1_GPIO_Init();
	NRF24L01_check( );			
	RF24L01_Init( );
	RF24L01_Set_Mode( MODE_RX );

/*����ģʽ��24L01��ʼ��*/		
//  NRF24L01_2_GPIO_Init();
//	NRF24L01_check_2( );			
//	RF24L01_Init_2( );
//	RF24L01_Set_Mode_2( MODE_TX );
	robot_init();
}

