/********************************** (C) COPYRIGHT *******************************
* File Name          : CDC.C
* Author             : WCH
* Version            : V1.0
* Date               : 2017/03/01
* Description        : CH554��CDC�豸ת���ڣ�ѡ�񴮿�1
*******************************************************************************/
#include "CH554.H"
#include "DEBUG.H"
#include <stdio.h>
#include <string.h>
#pragma  NOAREGS
#defien OPEN_CH340  1
#define THIS_ENDP0_SIZE         DEFAULT_ENDP0_SIZE 
UINT8X	Ep0Buffer[THIS_ENDP0_SIZE] _at_ 0x0000;                                //�˵�0 OUT&IN��������������ż��ַ
UINT8X	Ep2Buffer[2*MAX_PACKET_SIZE] _at_ 0x0008;                              //�˵�2 IN&OUT������,������ż��ַ
UINT8X  Ep1Buffer[MAX_PACKET_SIZE] _at_ 0x00a0;

UINT16 SetupLen,ControlData;
UINT8   SetupReq,Count,UsbConfig,down=0;
UINT8   RTS_DTR=0;
UINT8   baudFlag0,baudFlag1,baudFlag2=0;
UINT8   baud0,baud1;
UINT8   num = 0;
PUINT8  pDescr;                                                                //USB���ñ�־
USB_SETUP_REQ   SetupReqBuf;                                                   //�ݴ�Setup��
#define UsbSetupBuf     ((PUSB_SETUP_REQ)Ep0Buffer)

#define  SET_LINE_CODING                0X20            // Configures DTE rate, stop-bits, parity, and number-of-character
#define  GET_LINE_CODING                0X21            // This request allows the host to find out the currently configured line coding.
#define  SET_CONTROL_LINE_STATE         0X22            // This request generates RS-232/V.24 style control signals.

#define   USE_MOS     0
#define   RTS_HIGH   (P1 |= 0x10) //RTS HIGH
#define   DTR_HIGH   (P1 |= 0x20) //DTR HIGH
#define   RTS_LOW    (P1 &= ~0x10) //RTS Low
#define   DTR_LOW    (P1 &= ~0x20) //DTR Low
#define   FLAG_HIGH		(P3 |= 0x04)
#define   FLAG_LOW		(P3 &= ~0x04)


/*�豸������*/
/*
UINT8C DevDesc[] = {0x12,0x01,0x10,0x01,0x02,0x00,0x00,DEFAULT_ENDP0_SIZE,
                    0x86,0x1a,0x22,0x57,0x00,0x01,0x01,0x02,
                    0x03,0x01
                   };
UINT8C CfgDesc[] ={
    0x09,0x02,0x43,0x00,0x02,0x01,0x00,0xa0,0x32,             //�����������������ӿڣ�
	//����Ϊ�ӿ�0��CDC�ӿڣ�������	
    0x09,0x04,0x00,0x00,0x01,0x02,0x02,0x01,0x00,             //CDC�ӿ�������(һ���˵�)
	//����Ϊ����������
    0x05,0x24,0x00,0x10,0x01,                                 //����������(ͷ)
	0x05,0x24,0x01,0x00,0x00,                                 //����������(û��������ӿ�) 03 01
	0x04,0x24,0x02,0x02,                                      //֧��Set_Line_Coding��Set_Control_Line_State��Get_Line_Coding��Serial_State	
	0x05,0x24,0x06,0x00,0x01,                                 //���Ϊ0��CDC�ӿ�;���1��������ӿ�
	0x07,0x05,0x81,0x03,0x08,0x00,0xFF,                       //�ж��ϴ��˵�������
	//����Ϊ�ӿ�1�����ݽӿڣ�������
	0x09,0x04,0x01,0x00,0x02,0x0a,0x00,0x00,0x00,             //���ݽӿ�������
    0x07,0x05,0x02,0x02,0x40,0x00,0x00,                       //�˵�������	
	0x07,0x05,0x82,0x02,0x40,0x00,0x00,                       //�˵�������
};*/


/*
UINT8C DevDesc[18]={0x12,0x01,0x10,0x01,0xff,0x00,0x02,0x08,                   //�豸������
                    0x86,0x1a,0x23,0x55,0x04,0x03,0x00,0x00,
                    0x00,0x01};*/
UINT8C DevDesc[18]={0x12,0x01,0x10,0x01,0xff,0x00,0x00,0x08,                   //�豸������
                    0x86,0x1a,0x23,0x75,0x63,0x02,0x00,0x02,
                    0x00,0x01};
/*
UINT8C CfgDesc[39]={0x09,0x02,0x27,0x00,0x01,0x01,0x00,0x80,0xf0,              //�������������ӿ�������,�˵�������
	                  0x09,0x04,0x00,0x00,0x03,0xff,0x01,0x02,0x00,           
                    0x07,0x05,0x82,0x02,0x20,0x00,0x00,                        //�����ϴ��˵�
		                0x07,0x05,0x02,0x02,0x20,0x00,0x00,                        //�����´��˵�      
			              0x07,0x05,0x81,0x03,0x08,0x00,0x01};                       //�ж��ϴ��˵�*/
										
UINT8C CfgDesc[39]={0x09,0x02,0x27,0x00,0x01,0x01,0x00,0x80,0xf0,              //�������������ӿ�������,�˵�������
	                  0x09,0x04,0x00,0x00,0x03,0xff,0x01,0x02,0x00,           
                    0x07,0x05,0x82,0x02,0x20,0x00,0x00,                        //�����ϴ��˵�
		                0x07,0x05,0x02,0x02,0x20,0x00,0x00,                        //�����´��˵�      
			              0x07,0x05,0x81,0x03,0x08,0x00,0x01};                       //�ж��ϴ���

UINT8C DataBuf[26]={0x30,0x00,0xc3,0x00,0xff,0xec,0x9f,0xec,0xff,0xec,0xdf,0xec,
                    0xdf,0xec,0xdf,0xec,0x9f,0xec,0x9f,0xec,0x9f,0xec,0x9f,0xec,
                    0xff,0xec};

UINT8C StrDesc[28]={
	0x1C, 0x03, 0x55, 0x00, 0x53, 0x00, 0x42, 0x00,
	0x32, 0x00, 0x2E, 0x00, 0x30, 0x00, 0x2D, 0x00,
	0x53, 0x00, 0x65, 0x00, 0x72, 0x00, 0x69, 0x00,
	0x61, 0x00, 0x6C, 0x00
};
										
										
UINT8X UserEp2Buf[64];                                            //�û����ݶ���

/*�ַ���������*/
										/*
 unsigned char  code LangDes[]={0x04,0x03,0x09,0x04};           //����������
 unsigned char  code SerDes[]={                                 //���к��ַ���������
                0x14,0x03,
				0x32,0x00,0x30,0x00,0x31,0x00,0x37,0x00,0x2D,0x00,
				0x32,0x00,0x2D,0x00,
				0x32,0x00,0x35,0x00
                };     
 unsigned char  code Prod_Des[]={                                //��Ʒ�ַ���������
				0x14,0x03,
				0x43,0x00,0x48,0x00,0x35,0x00,0x35,0x00,0x34,0x00,0x5F,0x00,
				0x43,0x00,0x44,0x00,0x43,0x00,
 };
 unsigned char  code Manuf_Des[]={  
				0x0A,0x03,
				0x5F,0x6c,0xCF,0x82,0x81,0x6c,0x52,0x60,
 };*/

//cdc����
UINT8X LineCoding[7]={0x00,0xe1,0x00,0x00,0x00,0x00,0x08};   //��ʼ��������Ϊ57600��1ֹͣλ����У�飬8����λ��

#define UART_REV_LEN  128                 //���ڽ��ջ�������С
UINT8I Receive_Uart_Buf[UART_REV_LEN];   //���ڽ��ջ�����
volatile UINT8I Uart_Input_Point = 0;   //ѭ��������д��ָ�룬���߸�λ��Ҫ��ʼ��Ϊ0
volatile UINT8I Uart_Output_Point = 0;  //ѭ��������ȡ��ָ�룬���߸�λ��Ҫ��ʼ��Ϊ0
volatile UINT8I UartByteCount = 0;      //��ǰ������ʣ���ȡ�ֽ���


volatile UINT8I USBByteCount = 0;      //����USB�˵���յ�������
volatile UINT8I USBBufOutPoint = 0;    //ȡ����ָ��

volatile UINT8I UpPoint2_Busy  = 0;   //�ϴ��˵��Ƿ�æ��־


/*******************************************************************************
* Function Name  : USBDeviceCfg()
* Description    : USB�豸ģʽ����
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USBDeviceCfg()
{
    USB_CTRL = 0x00;                                                           //���USB���ƼĴ���
    USB_CTRL &= ~bUC_HOST_MODE;                                                //��λΪѡ���豸ģʽ
    USB_CTRL |=  bUC_DEV_PU_EN | bUC_INT_BUSY | bUC_DMA_EN;                    //USB�豸���ڲ�����ʹ��,���ж��ڼ��жϱ�־δ���ǰ�Զ�����NAK
    USB_DEV_AD = 0x00;                                                         //�豸��ַ��ʼ��
//     USB_CTRL |= bUC_LOW_SPEED;
//     UDEV_CTRL |= bUD_LOW_SPEED;                                                //ѡ�����1.5Mģʽ
    USB_CTRL &= ~bUC_LOW_SPEED;
    UDEV_CTRL &= ~bUD_LOW_SPEED;                                             //ѡ��ȫ��12Mģʽ��Ĭ�Ϸ�ʽ
	  UDEV_CTRL = bUD_PD_DIS;  // ��ֹDP/DM��������
    UDEV_CTRL |= bUD_PORT_EN;                                                  //ʹ�������˿�
	
}
/*******************************************************************************
* Function Name  : USBDeviceIntCfg()
* Description    : USB�豸ģʽ�жϳ�ʼ��
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USBDeviceIntCfg()
{
    USB_INT_EN |= bUIE_SUSPEND;                                               //ʹ���豸�����ж�
    USB_INT_EN |= bUIE_TRANSFER;                                              //ʹ��USB��������ж�
    USB_INT_EN |= bUIE_BUS_RST;                                               //ʹ���豸ģʽUSB���߸�λ�ж�
    USB_INT_FG |= 0x1F;                                                       //���жϱ�־
    IE_USB = 1;                                                               //ʹ��USB�ж�
    EA = 1;                                                                   //������Ƭ���ж�
}
/*******************************************************************************
* Function Name  : USBDeviceEndPointCfg()
* Description    : USB�豸ģʽ�˵����ã�ģ�����HID�豸�����˶˵�0�Ŀ��ƴ��䣬�������˵�2�������´�
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USBDeviceEndPointCfg()
{
    UEP1_DMA = Ep1Buffer;                                                      //�˵�1 �������ݴ����ַ
    UEP2_DMA = Ep2Buffer;                                                      //�˵�2 IN���ݴ����ַ	
    UEP2_3_MOD = 0xCC;                                                         //�˵�2/3 �������շ�ʹ��
    UEP2_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK | UEP_R_RES_ACK;                 //�˵�2�Զ���תͬ����־λ��IN���񷵻�NAK��OUT����ACK
    UEP1_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK | UEP_R_RES_ACK;
    //UEP1_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK;                                 //�˵�1�Զ���תͬ����־λ��IN���񷵻�NAK	
    UEP0_DMA = Ep0Buffer;                                                      //�˵�0���ݴ����ַ
    UEP4_1_MOD = 0X40;                                                         //�˵�1�ϴ����������˵�0��64�ֽ��շ�������
    UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;                                 //�ֶ���ת��OUT���񷵻�ACK��IN���񷵻�NAK
}
/*******************************************************************************
* Function Name  : Config_Uart1(UINT8 *cfg_uart)
* Description    : ���ô���1����
* Input          : �������ò��� ��λ�����ʡ�ֹͣλ��У�顢����λ
* Output         : None
* Return         : None
*******************************************************************************/
void Config_Uart1(UINT8 *cfg_uart)
{
	UINT32 x;
	UINT8 x2;
	UINT32 uart1_buad = 0;
	*((UINT8 *)&uart1_buad) = cfg_uart[3];
	*((UINT8 *)&uart1_buad+1) = cfg_uart[2];
	*((UINT8 *)&uart1_buad+2) = cfg_uart[1];
	*((UINT8 *)&uart1_buad+3) = cfg_uart[0];
	IE_UART1 = 0;
	x = 10 * FREQ_SYS / uart1_buad / 16;                                       //���������Ƶ��ע��x��ֵ��Ҫ���                            
	x2 = x % 10;
	x /= 10;
	if ( x2 >= 5 ) x ++;                                                       //��������
	SBAUD1 =  0 - x;
	IE_UART1 = 1;
}


void SetDTR_RTS(UINT8 temp){
#if USE_UART_UDISK
    if(!UdiskOCCUart){
#endif
        switch(temp){
            case 0x9f:
#if  USE_MOS
								RTS_LOW;
								DTR_LOW;
#else
								P3 |=  0x08; //P3.3 HIGH
								RTS_HIGH;
								DTR_HIGH;
#endif
                break;
            case 0xdf:
#if 	USE_MOS
								RTS_HIGH;
								DTR_LOW;
#else
								P3 &=  ~0x08; //P3.3 LOW
								RTS_LOW;
								DTR_HIGH;
#endif
                break;
            case 0xff:
#if  USE_MOS
								RTS_LOW;
								DTR_LOW;
#else
								P3 |=  0x08; //RTS HIGH
								RTS_HIGH;
								DTR_HIGH;
#endif
                break;
            case 0xbf:
#if  USE_MOS
								RTS_LOW;
								DTR_HIGH;
#else
								P3 |=  0x08; //RTS HIGH
								RTS_HIGH;
								DTR_LOW;
#endif
                break;
            default:
                break;
        }
#if USE_UART_UDISK
    }
#endif
}

void clearUart1(){
    //if(UartByteCount > 0){
        UartByteCount = 0;
        Uart_Output_Point = 0;
        Uart_Input_Point = 0;
				U1RI = 0;
		//}
}

void UsbSetBaud(UINT8 Baud0,UINT8 Baud1){
    switch(Baud0){
        case 0x80:
            switch(Baud1){
                case 0x96:                    //110 = 0x6E
                    LineCoding[3] = 0x00;
                    LineCoding[2] = 0x00;
                    LineCoding[1] = 0x00;
                    LineCoding[0] = 0x6E;
                    break;
                case 0xd9:                    //300 = 0x12C
                    LineCoding[3] = 0x00;
                    LineCoding[2] = 0x00;
                    LineCoding[1] = 0x01;
                    LineCoding[0] = 0x2C;
                    break;
                default:
                    break;
            }
            break;
        case 0x81:
            switch(Baud1){
                case 0x64:                    //600 = 0x258
                    LineCoding[3] = 0x00;
                    LineCoding[2] = 0x00;
                    LineCoding[1] = 0x02;
                    LineCoding[0] = 0x58;
                    break;
                case 0xb2:                    //1200 = 0x4B0
                    LineCoding[3] = 0x00;
                    LineCoding[2] = 0x00;
                    LineCoding[1] = 0x04;
                    LineCoding[0] = 0xB0;
                    break;
                case 0xd9:                    //2400 = 0x960
                    LineCoding[3] = 0x00;
                    LineCoding[2] = 0x00;
                    LineCoding[1] = 0x09;
                    LineCoding[0] = 0x60;
                    break;
                default:
                    break;
            }
            break;
        case 0x82:
            switch(Baud1){
                case 0x64:                    //4800 = 0x12C0
                    LineCoding[3] = 0x00;
                    LineCoding[2] = 0x00;
                    LineCoding[1] = 0x12;
                    LineCoding[0] = 0xC0;
                    break;
                case 0xb2:                    //9600 = 0x2580
                    LineCoding[3] = 0x00;
                    LineCoding[2] = 0x00;
                    LineCoding[1] = 0x25;
                    LineCoding[0] = 0x80;
                    break;
                case 0xcc:                    //14400 = 0x3840
                    LineCoding[3] = 0x00;
                    LineCoding[2] = 0x00;
                    LineCoding[1] = 0x38;
                    LineCoding[0] = 0x40;
                    break;
                case 0xd9:                    //19200 = 0x4B00
                    LineCoding[3] = 0x00;
                    LineCoding[2] = 0x00;
                    LineCoding[1] = 0x4B;
                    LineCoding[0] = 0x00;
                    break;
                default:
                    break;
            }
            break;
        case 0x83:
            switch(Baud1){
                case 0x64:                    //38400 = 0x9600
                    LineCoding[3] = 0x00;
                    LineCoding[2] = 0x00;
                    LineCoding[1] = 0x96;
                    LineCoding[0] = 0x00;
                    break;
                case 0x95:                    //5600 = 0xDAC0
                    LineCoding[3] = 0x00;
                    LineCoding[2] = 0x00;
                    LineCoding[1] = 0xDA;
                    LineCoding[0] = 0xC0;
                    break;
                case 0x98:                    //57600 = 0xE100
                    LineCoding[3] = 0x00;
                    LineCoding[2] = 0x00;
                    LineCoding[1] = 0xE1;
                    LineCoding[0] = 0x00;
                    break;
                case 0xcc:                    //115200 = 0x1c200
                    LineCoding[3] = 0x00;
                    LineCoding[2] = 0x01;
                    LineCoding[1] = 0xC2;
                    LineCoding[0] = 0x00;
                    break;
                case 0xd1:                    //128000 = 0x1f400
                    LineCoding[3] = 0x00;
                    LineCoding[2] = 0x01;
                    LineCoding[1] = 0xF4;
                    LineCoding[0] = 0x00;
                    break;
                case 0xe6:                    //230400 = 0x38400
                    LineCoding[3] = 0x00;
                    LineCoding[2] = 0x03;
                    LineCoding[1] = 0x84;
                    LineCoding[0] = 0x00;
                    break;
                case 0xe9:                    //256000 = 0x3e800
                    LineCoding[3] = 0x00;
                    LineCoding[2] = 0x03;
                    LineCoding[1] = 0xE8;
                    LineCoding[0] = 0x00;
                    break;
                case 0xf3:                    //460800 = 0x70800
                    LineCoding[3] = 0x00;
                    LineCoding[2] = 0x07;
                    LineCoding[1] = 0x08;
                    LineCoding[0] = 0x00;
                    break;
                case 0xf4:                    //512000 = 0x7D000
                    LineCoding[3] = 0x00;
                    LineCoding[2] = 0x07;
                    LineCoding[1] = 0xD0;
                    LineCoding[0] = 0x00;
                    break;
                case 0xf6:                    //600000 = 0x927C0
                    LineCoding[3] = 0x00;
                    LineCoding[2] = 0x09;
                    LineCoding[1] = 0x27;
                    LineCoding[0] = 0xC0;
                    break;
                case 0xf8:                    //750000 = 0xB71B0
                    LineCoding[3] = 0x00;
                    LineCoding[2] = 0x0B;
                    LineCoding[1] = 0x71;
                    LineCoding[0] = 0xB0;
                    break;
                default:
                    break;
            }
            break;
        case 0x87:
            switch(Baud1){
                case 0xf3:                    //921600 = 0xE1000
                    LineCoding[3] = 0x00;
                    LineCoding[2] = 0x0E;
                    LineCoding[1] = 0x10;
                    LineCoding[0] = 0x00;
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
		Config_Uart1(LineCoding);
}


/*******************************************************************************
* Function Name  : DeviceInterrupt()
* Description    : CH559USB�жϴ�������
*******************************************************************************/
void    DeviceInterrupt( void ) interrupt INT_NO_USB using 1                    //USB�жϷ������,ʹ�üĴ�����1
{
		
    UINT8 len,i;
    if(UIF_TRANSFER)                                                            //USB������ɱ�־
    {
        switch (USB_INT_ST & (MASK_UIS_TOKEN | MASK_UIS_ENDP))
        {
        case UIS_TOKEN_IN | 2:                                                  //endpoint 2# �˵������ϴ�
						//printf("UIS_TOKEN_IN | 2\r\n");
             UEP2_T_LEN = 0;                                                    //Ԥʹ�÷��ͳ���һ��Ҫ���
            //UEP1_CTRL ^= bUEP_T_TOG;                                          //����������Զ���ת����Ҫ�ֶ���ת
            UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_NAK;           //Ĭ��Ӧ��NAK
						UpPoint2_Busy = 0;// not  busy
						break;
        case UIS_TOKEN_OUT | 2:                                                 //endpoint 2# �˵������´�
					//printf("UIS_TOKEN_OUT | 2\r\n");
            if ( U_TOG_OK )                                                     // ��ͬ�������ݰ�������
            {
                len = USB_RX_LEN;
							  USBByteCount = USB_RX_LEN;
							  USBBufOutPoint = 0;
							  UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_R_RES | UEP_R_RES_NAK;
							  
            }
            break;
        case UIS_TOKEN_SETUP | 0:                                               //SETUP����
            len = USB_RX_LEN;
            if(len == (sizeof(USB_SETUP_REQ)))
            {
                SetupLen = UsbSetupBuf->wLengthL;
								RTS_DTR = UsbSetupBuf->wValueL;
                if(UsbSetupBuf->wLengthH || SetupLen > 0x7F )
                {
                    SetupLen = 0x7F;                                             // �����ܳ���
                }
                len = 0;                                                         // Ĭ��Ϊ�ɹ������ϴ�0����
                SetupReq = UsbSetupBuf->bRequest;							
                if ( ( UsbSetupBuf->bRequestType & USB_REQ_TYP_MASK ) != USB_REQ_TYP_STANDARD ){/*HID������*/
										switch( SetupReq )                                             
										{
												case 0xC0:                                                  
														pDescr = &DataBuf[num];
														len = 2;
														if(num<24){	
																num += 2;
														}else{
																num = 24;
														}											
														break;
												case 0x40:
														len = 9;   //��֤״̬�׶Σ�����ֻҪ��8���Ҳ�����0xff����
														break;
												
												case 0xa4:
														FLAG_HIGH;
														clearUart1();
														SetDTR_RTS(RTS_DTR);
												    break;
												case 0x9a://set baud
														baudFlag0 = UsbSetupBuf->wValueL;
														baudFlag1 = UsbSetupBuf->wValueH;
														if((baudFlag0 == 0x12) && (baudFlag1 == 0x13)){
																baud0 = UsbSetupBuf->wIndexL;
																baud1 = UsbSetupBuf->wIndexH;
															  UsbSetBaud(baud0,baud1);
																//Config_Uart1(LineCoding);
														}
														//UART1Setup();
													break;
											   case 0xA1://set baud   ����ƻ��
														baudFlag0 = UsbSetupBuf->wValueL;
														baudFlag1 = UsbSetupBuf->wValueH;
														if((baudFlag0 == 0x9C) && (baudFlag1 == 0xC3)){
																baud0 = UsbSetupBuf->wIndexL;
																baud1 = UsbSetupBuf->wIndexH;
															  UsbSetBaud(baud0,baud1);
																//Config_Uart1(LineCoding);
														}
														//UART1Setup();
													break;
												default:
														len = 0xFF;  				                                   /*���֧��*/					
														break;
										}
										if ( SetupLen > len )
										{
												SetupLen = len;    //�����ܳ���
										}
										len = SetupLen >= THIS_ENDP0_SIZE ? THIS_ENDP0_SIZE : SetupLen;//���δ��䳤��
										memcpy(Ep0Buffer,pDescr,len);                            //�����ϴ�����
										SetupLen -= len;
										pDescr += len;
						
                }else                                                             //��׼����
                {
                    switch(SetupReq)                                             //������
                    {
                    case USB_GET_DESCRIPTOR:
                        switch(UsbSetupBuf->wValueH)
                        {
                        case 1:                                                  //�豸������
                            pDescr = DevDesc;                                    //���豸�������͵�Ҫ���͵Ļ�����
                            len = sizeof(DevDesc);
                            break;
                        case 2:                                                  //����������
                            pDescr = CfgDesc;                                    //���豸�������͵�Ҫ���͵Ļ�����
                            len = sizeof(CfgDesc);
                            break;
												case 3:
														pDescr = StrDesc;
														len = (UsbSetupBuf->wLengthH<<8) | (UsbSetupBuf->wLengthL);
														break;
                        default:
                            len = 0xff;                                          //��֧�ֵ�������߳���
                            break;
                        }
                        if ( SetupLen > len )
                        {
                            SetupLen = len;    //�����ܳ���
                        }
                        len = SetupLen >= THIS_ENDP0_SIZE ? THIS_ENDP0_SIZE : SetupLen;//���δ��䳤��
                        memcpy(Ep0Buffer,pDescr,len);                            //�����ϴ�����
                        SetupLen -= len;
                        pDescr += len;
                        break;
                    case USB_SET_ADDRESS:
                        SetupLen = UsbSetupBuf->wValueL;                         //�ݴ�USB�豸��ַ
                        break;
                    case USB_GET_CONFIGURATION:
                        Ep0Buffer[0] = UsbConfig;
                        if ( SetupLen >= 1 )
                        {
                            len = 1;
                        }
                        break;
                    case USB_SET_CONFIGURATION:
                        UsbConfig = UsbSetupBuf->wValueL;
                        break;
                    case 0x0A:
                        break;
                    case USB_CLEAR_FEATURE:                                      //Clear Feature
                        if ( ( UsbSetupBuf->bRequestType & USB_REQ_RECIP_MASK ) == USB_REQ_RECIP_ENDP )// �˵�
                        {
                            switch( UsbSetupBuf->wIndexL )
                            {
                            case 0x82:
                                UEP2_CTRL = UEP2_CTRL & ~ ( bUEP_T_TOG | MASK_UEP_T_RES ) | UEP_T_RES_NAK;
                                break;
                            case 0x02:
                                UEP2_CTRL = UEP2_CTRL & ~ ( bUEP_R_TOG | MASK_UEP_R_RES ) | UEP_R_RES_ACK;
                                break;
                            case 0x81:
                                UEP1_CTRL = UEP1_CTRL & ~ ( bUEP_T_TOG | MASK_UEP_T_RES ) | UEP_T_RES_NAK;
                                break;
						
                            default:
                                len = 0xFF;                                       // ��֧�ֵĶ˵�
                                break;
                            }
                        }
                        else
                        {
                            len = 0xFF;                                           // ���Ƕ˵㲻֧��
                        }
                        break;
                    case USB_SET_FEATURE:                                         /* Set Feature */
                        if( ( UsbSetupBuf->bRequestType & 0x1F ) == 0x00 )        /* �����豸 */
                        {
                            if( ( ( ( UINT16 )UsbSetupBuf->wValueH << 8 ) | UsbSetupBuf->wValueL ) == 0x01 )
                            {
                                if( CfgDesc[ 7 ] & 0x20 )
                                {
                                    /* ���û���ʹ�ܱ�־ */
                                }
                                else
                                {
                                    len = 0xFF;                                    /* ����ʧ�� */
                                }
                            }
                            else
                            {
                                len = 0xFF;                                        /* ����ʧ�� */
                            }
                        }
                        else if( ( UsbSetupBuf->bRequestType & 0x1F ) == 0x02 )    /* ���ö˵� */
                        {
                            if( ( ( ( UINT16 )UsbSetupBuf->wValueH << 8 ) | UsbSetupBuf->wValueL ) == 0x00 )
                            {
                                switch( ( ( UINT16 )UsbSetupBuf->wIndexH << 8 ) | UsbSetupBuf->wIndexL )
                                {
                                case 0x82:
                                    UEP2_CTRL = UEP2_CTRL & (~bUEP_T_TOG) | UEP_T_RES_STALL;/* ���ö˵�2 IN STALL */
                                    break;
                                case 0x02:
                                    UEP2_CTRL = UEP2_CTRL & (~bUEP_R_TOG) | UEP_R_RES_STALL;/* ���ö˵�2 OUT Stall */
                                    break;
                                case 0x81:
                                    UEP1_CTRL = UEP1_CTRL & (~bUEP_T_TOG) | UEP_T_RES_STALL;/* ���ö˵�1 IN STALL */
                                    break;
                                default:
                                    len = 0xFF;                                     /* ����ʧ�� */
                                    break;
                                }
                            }
                            else
                            {
                                len = 0xFF;                                         /* ����ʧ�� */
                            }
                        }
                        else
                        {
                            len = 0xFF;                                             /* ����ʧ�� */
                        } 
                        break;
                    case USB_GET_STATUS:
                        Ep0Buffer[0] = 0x00;
                        Ep0Buffer[1] = 0x00;
                        if ( SetupLen >= 2 )
                        {
                            len = 2;
                        }
                        else
                        {
                            len = SetupLen;
                        }
                        break;
                    default:
                        len = 0xff;                                                  //����ʧ��
                        break;
                    }
                }
            }
            else
            {
                len = 0xff;                                                          //�����ȴ���
            }
            if(len == 0xff)
            {
                SetupReq = 0xFF;
                UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_STALL | UEP_T_RES_STALL;//STALL
            }
            else if(len <= THIS_ENDP0_SIZE)                                         //�ϴ����ݻ���״̬�׶η���0���Ȱ�
            {
                UEP0_T_LEN = len;
                UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;//Ĭ�����ݰ���DATA1������Ӧ��ACK
            }
            else
            {
                UEP0_T_LEN = 0;  //��Ȼ��δ��״̬�׶Σ�������ǰԤ���ϴ�0�������ݰ��Է�������ǰ����״̬�׶�
                UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;//Ĭ�����ݰ���DATA1,����Ӧ��ACK
            }
            break;
        case UIS_TOKEN_IN | 0:                                                      //endpoint0 IN
            switch(SetupReq)
            {
            case USB_GET_DESCRIPTOR:
                len = SetupLen >= THIS_ENDP0_SIZE ? THIS_ENDP0_SIZE : SetupLen;     //���δ��䳤��
                memcpy( Ep0Buffer, pDescr, len );                                   //�����ϴ�����
                SetupLen -= len;
                pDescr += len;
                UEP0_T_LEN = len;
                UEP0_CTRL ^= bUEP_T_TOG;                                            //ͬ����־λ��ת
                break;
            case USB_SET_ADDRESS:
                USB_DEV_AD = USB_DEV_AD & bUDA_GP_BIT | SetupLen;
                UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                break;
            default:
                UEP0_T_LEN = 0;                                                      //״̬�׶�����жϻ�����ǿ���ϴ�0�������ݰ��������ƴ���
                UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                break;
            }
            break;
        case UIS_TOKEN_OUT | 0:  // endpoint0 OUT
            len = USB_RX_LEN;
            if(SetupReq == 0x09)
            {
                if(Ep0Buffer[0])
                {
                    printf("Light on Num Lock LED!\n");
                }
                else if(Ep0Buffer[0] == 0)
                {
                    printf("Light off Num Lock LED!\n");
                }
            }
            UEP0_T_LEN = 0;  //��Ȼ��δ��״̬�׶Σ�������ǰԤ���ϴ�0�������ݰ��Է�������ǰ����״̬�׶�
            UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_ACK;//Ĭ�����ݰ���DATA0,����Ӧ��ACK
            break;
        default:
					printf("error\r\n");
            break;
        }
        UIF_TRANSFER = 0;                                                           //д0����ж�
    }
    if(UIF_BUS_RST)                                                                 //�豸ģʽUSB���߸�λ�ж�
    {
        UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        UEP1_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK | UEP_R_RES_ACK;
        UEP2_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK | UEP_R_RES_ACK;
        USB_DEV_AD = 0x00;
        UIF_SUSPEND = 0;
        UIF_TRANSFER = 0;
        UIF_BUS_RST = 0;                                                             //���жϱ�־
		    Uart_Input_Point = 0;   //ѭ������������ָ��
				Uart_Output_Point = 0;  //ѭ������������ָ��
				UartByteCount = 0;      //��ǰ������ʣ���ȡ�ֽ���
				USBByteCount = 0;       //USB�˵��յ��ĳ���
				UsbConfig = 0;          //�������ֵ
				UpPoint2_Busy = 0;
    }
    if (UIF_SUSPEND)                                                                 //USB���߹���/�������
    {
        UIF_SUSPEND = 0;
        if ( USB_MIS_ST & bUMS_SUSPEND )                                             //����
        {
#ifdef DE_PRINTF
            printf( "zz" );                                                          //˯��״̬
#endif
            while ( XBUS_AUX & bUART0_TX )
            {
                ;    //�ȴ��������
            }
            SAFE_MOD = 0x55;
            SAFE_MOD = 0xAA;
            WAKE_CTRL = bWAK_BY_USB | bWAK_RXD0_LO | bWAK_RXD1_LO;                      //USB����RXD0/1���ź�ʱ�ɱ�����
            PCON |= PD;                                                                 //˯��
            SAFE_MOD = 0x55;
            SAFE_MOD = 0xAA;
            WAKE_CTRL = 0x00;
        }
    }
    else {                                                                             //������ж�,�����ܷ��������
        USB_INT_FG = 0xFF;                                                             //���жϱ�־
      //printf("UnknownInt  N");
    }
}
/*******************************************************************************
* Function Name  : Uart1_ISR()
* Description    : ���ڽ����жϺ�����ʵ��ѭ���������
*******************************************************************************/
void Uart1_ISR(void) interrupt INT_NO_UART1
{
	if(U1RI)   //�յ�����
	{
		Receive_Uart_Buf[Uart_Input_Point++] = SBUF1;
		UartByteCount++;                    //��ǰ������ʣ���ȡ�ֽ���
		if(Uart_Input_Point>=UART_REV_LEN)
			Uart_Input_Point = 0;           //д��ָ��
		U1RI =0;		
	}
	
}

UINT8 testBuf[4] = {0x00, 0x00, 0xFF, 0xFE};
void InitUSB_Device(){
    USB_CTRL = 0x00;                                                //���USB���ƼĴ���
    USB_CTRL &= ~bUC_HOST_MODE;                                     //��λΪѡ���豸ģʽ
    USB_CTRL |=  bUC_DEV_PU_EN | bUC_INT_BUSY | bUC_DMA_EN;         //USB�豸���ڲ�����ʹ��,���ж��ڼ��жϱ�־δ���ǰ�Զ�����NAK
    USB_DEV_AD = 0x00;                                              //�豸��ַ��ʼ��
    USB_CTRL &= ~bUC_LOW_SPEED;
    UDEV_CTRL &= ~bUD_LOW_SPEED; 	                                  //ѡ��ȫ��12Mģʽ��Ĭ�Ϸ�ʽ
    UDEV_CTRL = bUD_PD_DIS;                                         // ��ֹDP/DM��������
    UDEV_CTRL |= bUD_PORT_EN;                                       //ʹ�������˿�

    UEP0_DMA = Ep0Buffer;
    UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
    UEP0_T_LEN = 0;

    UEP2_3_MOD = 0xCC;
    UEP4_1_MOD = 0X40;

    UEP1_DMA = Ep1Buffer;
    UEP1_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK;
		UEP1_T_LEN = 0;

    UEP2_DMA = Ep2Buffer;
    UEP2_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK | UEP_R_RES_ACK;
    UEP2_T_LEN = 0;

    USB_INT_EN |= bUIE_SUSPEND;                                     //ʹ���豸�����ж�
    USB_INT_EN |= bUIE_TRANSFER;                                    //ʹ��USB��������ж�
    USB_INT_EN |= bUIE_BUS_RST;                                     //ʹ���豸ģʽUSB���߸�λ�ж�
    USB_INT_FG |= 0x1F;                                             //���жϱ�־
    IE_USB = 1;                                                     //������Ƭ���ж�
    EA = 1;
}

//������
main()
{
	UINT8 lenth;
	UINT8 j = 0;
	UINT8 i = 0;
	UINT8 Uart_Timeout = 0;
	UINT8 nowStatus = 0;
    CfgFsys( );                                                           //CH559ʱ��ѡ������
    mDelaymS(100);                                                          //�޸���Ƶ�ȴ��ڲ������ȶ�,�ؼ�	
		P1_MOD_OC &= ~0x40;//rx �������
		P1_DIR_PU &= ~0x40; //rx ��������ģʽ
	  P1_MOD_OC |= 0x80; //tx ��©���
		P1_DIR_PU &= ~0x80; //tx ��ֹ����
#if  USE_MOS
		P1_MOD_OC &= ~0x10; //RTS �������
		P1_DIR_PU |= 0x10; //RTS ���
		P1_MOD_OC &= ~0x20; //DTR �������
		P1_DIR_PU |= 0x20; //DTR ���
		RTS_LOW;
		DTR_LOW;
#else
		P3_MOD_OC |= 0x08;  //P3.3 ��©���
		P3_DIR_PU &= ~0x08; //P3.3 ��ֹ����
		P3_MOD_OC |= 0x04;  //P3.2 ��©��� FLAG
		P3_DIR_PU &= ~0x04; //P3.2 ��ֹ���� FLAG
		P1_MOD_OC |= 0x10;  //RTS  ��©���
		P1_DIR_PU &= ~0x10; //RTS  ��ֹ����
		P1_MOD_OC |= 0x20;  //DTR  ��©���
		P1_DIR_PU &= ~0x20; //DTR  ��ֹ����
		RTS_HIGH;
		DTR_HIGH;
#endif
		mDelaymS(200);  
    mInitSTDIO( );                                                        //����0,�������ڵ���
    UART1Setup( );                                                        //����CDC
#ifdef DE_PRINTF
    printf("start ...\n");
#endif	

    /*USBDeviceCfg(); 
    USBDeviceEndPointCfg();                                               //�˵�����
    USBDeviceIntCfg();                                                    //�жϳ�ʼ��*/
		InitUSB_Device();
		clearUart1();
		FLAG_HIGH;
	  //UEP0_T_LEN = 0;
    //UEP1_T_LEN = 0;                                                       //Ԥʹ�÷��ͳ���һ��Ҫ���
    //UEP2_T_LEN = 0;                                                       //Ԥʹ�÷��ͳ���һ��Ҫ���
    while(1)
    {
		if(UsbConfig)
		{
			if(USBByteCount)   //USB���ն˵�������
			{
				CH554UART1SendByte(Ep2Buffer[USBBufOutPoint++]);
				USBByteCount--;
				if(USBByteCount==0) 
					UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_R_RES | UEP_R_RES_ACK;
			}
			if(UartByteCount){
				Uart_Timeout++;
			}
				
			if(!UpPoint2_Busy)   //�˵㲻��æ�����к�ĵ�һ�����ݣ�ֻ���������ϴ���
			{
				lenth = UartByteCount;
				if(lenth>0)
				{
					if(lenth>31 || Uart_Timeout>100)
					{		
						//printf("send ok\r\n");
						Uart_Timeout = 0;
						if(Uart_Output_Point+lenth>UART_REV_LEN)
							lenth = UART_REV_LEN-Uart_Output_Point;
						UartByteCount -= lenth;			
						//д�ϴ��˵�
						memcpy(Ep2Buffer+MAX_PACKET_SIZE,&Receive_Uart_Buf[Uart_Output_Point],lenth);
						Uart_Output_Point += lenth;
						if(Uart_Output_Point >= UART_REV_LEN)
							Uart_Output_Point = 0;						
						UEP2_T_LEN = lenth;                                                    //Ԥʹ�÷��ͳ���һ��Ҫ���
						UpPoint2_Busy = 1;
						UEP2_CTRL = UEP2_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_ACK;            //Ӧ��ACK
						//UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_ACK;            //Ӧ��ACK
					}
				}
			}else{
				//printf("else\r\n");
				lenth = UartByteCount;
				if(lenth>0){
					if(lenth>31 || Uart_Timeout>100){
						Uart_Timeout = 0;
							//printf("MAC MAC\r\n");
							UartByteCount-=lenth;
							//memcpy(Ep1Buffer+MAX_PACKET_SIZE,testBuf,4);
							//UEP1_T_LEN = 4; 
							//UEP1_CTRL = UEP1_CTRL & ~ MASK_UEP_T_RES | UEP_T_RES_ACK; 
					}
				}
				
			}
		}		
    }
}