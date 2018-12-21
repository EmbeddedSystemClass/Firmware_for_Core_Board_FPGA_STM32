#include "exti.h"
#include "delay.h" 
#include "led.h" 
#include "key.h"
#include "NRF24L01.h"
#include "Comm.h"

extern uint8_t tmp_buf[32];

//�ⲿ�ж�15�������
//��24L01��IRQ��������ʱ�����յ����ݰ�
void EXTI15_10_IRQHandler(void)
{
	int i;
	i = NRF24L01_RxPacket( tmp_buf );
	if( 27 == i )
	{
		LED1 = ~LED1;
		do_comm();//24L01ͨ�Ŵ����ܺ���
	}
	EXTI_ClearITPendingBit(EXTI_Line15); //���LINE0�ϵ��жϱ�־λ 
}	

//�ⲿ�жϳ�ʼ������
void EXTIX_Init(void)
{
	NVIC_InitTypeDef   NVIC_InitStructure;
	EXTI_InitTypeDef   EXTI_InitStructure;
 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);//ʹ��SYSCFGʱ��
 
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource15);
	
  /* ����EXTI_Line15 */
  EXTI_InitStructure.EXTI_Line = EXTI_Line15;//LINE15
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//�ж��¼�
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; //�½��ش���
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;//ʹ��LINE0
  EXTI_Init(&EXTI_InitStructure);//����
 
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;//�ⲿ�ж�0
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;//��ռ���ȼ�0
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;//�����ȼ�2
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//ʹ���ⲿ�ж�ͨ��
  NVIC_Init(&NVIC_InitStructure);//����
}
