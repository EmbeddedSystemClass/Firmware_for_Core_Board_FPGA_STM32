#include "Flash.h"

/*---------function headers----------*/

//IIC pin configuration
//Change these functions if pin configuration is changed.
void	I2C_SDA_WR(unsigned char v);
int		I2C_SDA_RD(void);
void	I2C_SCK_WR(unsigned char v);
void 	I2C_DIR(unsigned char v);	//0 : input, 1:output
// End of IIC pin configuration

int 	 	I2C_Start(void);
void	 	I2C_Stop(void);
void	 	I2C_Ack(void);
void		I2C_Nack(void);
int   		I2C_Send_Byte( unsigned char);
unsigned char I2C_Receive_Byte(void);



/*----------------------------------------------------------
 *  �������ƣ� I2C_SDA_WR
 *  ���������� ��EEROM SDA�ڽ���д����
 *  
 *
 *
 *  ���ߣ�     ����
 *  �޸����ڣ� 2011-9-15
 ----------------------------------------------------------*/ 
void I2C_SDA_WR(unsigned char v)
{
	  //IOWR(PIO_IO_BASE,1,0x3);
	  IOWR(PIO_SDA_24_BASE, 1, 1);
	  
		if( v )
	  {
		   //iic_reg = iic_reg | 0x1;
	     IOWR(PIO_SDA_24_BASE,0,1);
	  }
		else
	  {
	     //iic_reg = iic_reg & (~0x1);
	     IOWR(PIO_SDA_24_BASE,0,0); 
	  }
}


/*-----------------------------------------------------------
 *  �������ƣ� I2C_SDA_RD
 *  ���������� ��EEROM SDA�ڽ��ж�����
 *  
 *
 *
 *  ���ߣ�     ����
 *  �޸����ڣ� 2011-9-15
 -----------------------------------------------------------*/ 
int I2C_SDA_RD(void)
{
	unsigned int temp;

	IOWR(PIO_SDA_24_BASE,1,0x0);
	temp = IORD(PIO_SDA_24_BASE,0);
	if( temp & 0x1 )
		return 1;
	else
		return 0;
}


/*------------------------------------------------------------
 *  �������ƣ� I2C_SCK_WR
 *  ���������� ��EEROM SCK�ڽ���д����
 *  
 *
 *
 *  ���ߣ�     ����
 *  �޸����ڣ� 2011-9-15
 -------------------------------------------------------------*/ 
void I2C_SCK_WR(unsigned char v)
{
	  if( v )
	  {
	     //iic_reg = iic_reg | 0x2;
	     IOWR(PIO_SCL_24_BASE,0,1);
	  }
	  else
	  {
	     //iic_reg = iic_reg & (~0x2);
	     IOWR(PIO_SCL_24_BASE,0,0); 
	  }
}


/*---------------------------------------------------------------
 *  �������ƣ� I2C_DIR
 *  ���������� ����SDA�ڵķ�������趨��0Ϊ���룬1Ϊ���
 *  
 *
 *
 *  ���ߣ�     ����
 *  �޸����ڣ� 2011-9-15
 --------------------------------------------------------------*/ 
void I2C_DIR(unsigned char v)	//0 : input, 1:output
{
		if( v )
			IOWR(PIO_SDA_24_BASE, 1, 1);

		else
			IOWR(PIO_SDA_24_BASE, 1, 0);

}



/*--------------------------------------------------------------
 *  �������ƣ� I2C_Start
 *  ���������� I2CͨѶ��ʼ��������SCK�ߵ�ƽ��ʱ��SDA�ɸߵ�ƽ�õͣ������Ϊ��EEROM����ʼ�ź�
 *             ��EEROM��ʼ����ͨѶ
 *
 *
 *  ���ߣ�     ����
 *  �޸����ڣ� 2011-9-15
 --------------------------------------------------------------*/ 
int I2C_Start(void)
{
	IOWR(PIO_SCL_24_BASE,0,0);
	IOWR(PIO_SDA_24_BASE,0,0);
	I2C_DIR(1);
	
	delay(90);
	I2C_SDA_WR(1);
	delay(90);
	I2C_SCK_WR(1);
	delay(90);
	
	I2C_SDA_WR(0);
	delay(90);
	I2C_SCK_WR(0);
	delay(90);

	return 1;
}



/*---------------------------------------------------------------------------------
 *  �������ƣ� I2C_Stop
 *  ���������� ��EEROM����ֹͣ�źţ���ʾͨѶֹͣ����SCK�ߵ�ƽʱ����SDA�õ���EEROM��
 *             ֹͣ���ݴ���
 *
 *
 *  ���ߣ�     ����
 *  �޸����ڣ� 2011-9-15
 ---------------------------------------------------------------------------------*/ 
void  I2C_Stop(void)
{
	 delay(90);
	 I2C_SDA_WR(0);
	 delay(90);
	 I2C_SCK_WR(1);
	 delay(90);
	 I2C_SDA_WR(1);
	 delay(90);
}



/*----------------------------------------------------------------------------
 *  �������ƣ� I2C_Ack
 *  ���������� ��EEROM�����źţ���ʾ���ڽ���EEROMӦ���״̬�У��ȴ�EEROM��Ӧ��
 *  
 *
 *
 *  ���ߣ�     ����
 *  �޸����ڣ� 2011-9-15
 ----------------------------------------------------------------------------*/ 
void I2C_Ack(void)
{
	 delay(90);
	 I2C_SDA_WR(0);
	 delay(90);
	 I2C_SCK_WR(1);
	 delay(90);
	 I2C_SCK_WR(0);
	 delay(90);
}


/*-------------------------------------------------------------------
 *  �������ƣ� I2C_Nack
 *  ���������� ��EEROM�����źţ���ʾ���ٽ���Ӧ���źţ�����ȡ�������
 *  
 *
 *
 *  ���ߣ�     ����
 *  �޸����ڣ� 2011-9-15
 -------------------------------------------------------------------*/ 
void I2C_Nack(void)
{
	 delay(90);
	 I2C_SDA_WR(1);
	 delay(90);
	 I2C_SCK_WR(1);
	 delay(90);
	 I2C_SCK_WR(0);
	 delay(90);
}



/*-------------------------------------------------------------------
 *  �������ƣ� I2C_Send_Byte
 *  ���������� ��EEROM����һ���ֽ�
 *  
 *
 *
 *  ���ߣ�     ����
 *  �޸����ڣ� 2011-9-15
 -------------------------------------------------------------------*/ 
int I2C_Send_Byte( unsigned char d)
{
	 unsigned char i = 8;
	 int bit_ack;
	 while( i-- )
	 {
		 delay(90);
	 	 if ( d &0x80 )   
	  		I2C_SDA_WR(1);
		  else
		  	I2C_SDA_WR(0);
		 delay(90);
		 I2C_SCK_WR(1);
		 delay(90);
		 I2C_SCK_WR(0);
		 d = d << 1;
	 }
	 delay(90);
	 I2C_SDA_WR(1);
	 I2C_DIR(0);
	 delay(90);

	 I2C_SCK_WR(1);
	 delay(90);
	 bit_ack = I2C_SDA_RD();
	 I2C_SCK_WR(0);

	 delay(90);
	 I2C_DIR(1);

	 return bit_ack;
}




/*--------------------------------------------------------
 *  �������ƣ� I2C_Receive_Byte
 *  ���������� ��EEROM�ж�ȡһ���ֽ�
 *  
 *
 *
 *  ���ߣ�     ����
 *  �޸����ڣ� 2011-9-15
 -------------------------------------------------------*/ 
unsigned char I2C_Receive_Byte(void)
{
	 unsigned char i = 8, d=0;
	 delay(90);
	 I2C_SDA_WR(1);
	 I2C_DIR(0);
	 I2C_SCK_WR(0);

	 while ( i--)
	 {
		  d = d << 1;
		 delay(90);
		  I2C_SCK_WR(1);
		  if ( I2C_SDA_RD() ) d++;
		 delay(90);
		 I2C_SCK_WR(0);
	 }
	 return d;
}



/*------------------------------------------------------------------
 *  �������ƣ� flash_set
 *  ���������� ����������ĳ��ַ�ϵ�һ������д�뵽EEROM��һ�ε�ַ����
 *  
 *
 *
 *  ���ߣ�     ����
 *  �޸����ڣ� 2011-9-15
 ------------------------------------------------------------------*/ 
int flash_set(unsigned short addr,unsigned char *data,unsigned int len)
{
	if( data == NULL )
		return -1;
	
	while(len--) {
		IOWR( TIMER_WATCHDOG_BASE, 2, 0x1234 );

		I2C_Start();
		I2C_Send_Byte( 0xa0 + addr / 256 *2);  /* 24C08  USE */
		
		I2C_Send_Byte(addr %256 );
		I2C_Send_Byte( *(unsigned char*)data );
		I2C_Stop();  
		delay(36000);				/* waiting for write cycle to be completed */
		data++;
		addr++;
	 }

	 return 0;
}



/*----------------------------------------------------------------------
 *  �������ƣ� flash_get
 *  ���������� ��EEROM��ĳһ��ַ��ʼ��һ�����ݣ����浽��������һ�ε�ַ��
 *  
 *
 *
 *  ���ߣ�     ����
 *  �޸����ڣ� 2011-9-15
 ----------------------------------------------------------------------*/ 
int flash_get(unsigned short addr,unsigned char *data,unsigned int len)
{
	if( data == NULL )
		return -1;

	 while(len--) {
	 	IOWR( TIMER_WATCHDOG_BASE, 2, 0x1234 );

		I2C_Start();
		I2C_Send_Byte( 0xa0 + addr / 256 *2 );  /* 24C08 USE */

		I2C_Send_Byte( addr % 256 );
		I2C_Start();
		I2C_Send_Byte( 0xa1 + addr /256 *2 );

		*data = I2C_Receive_Byte();
		I2C_Nack();
		I2C_Stop();
		delay(36000);
		data++;
		addr++;
	 }

	 return 0;
}


