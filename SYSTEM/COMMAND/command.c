/*=========================================================================*/
/* Copyright (C), 2017, Yagro Co., Ltd.                                    */
/* File name:   commond.c                                                  */
/* Date:        2017.5.2                                                   */
/* Description:                                                            */
/*=========================================================================*/

/*============================================================================*/
/*                               Header include                               */
/*============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "command.h"
#include "command_bin.h"
#include "send_message.h"

#include "flash.h"
#include "usart.h"
#include "SPI1.h"

/*============================================================================*/
/*                                   Macros                                   */
/*============================================================================*/

#define DATA_VALUE_LOCATION   4

#define MAG_CAL_PARA_NUM			12
#define	MAG_CAL_PARA_INIT			{1.0f,0.0f,0.0f,\
															 0.0f,1.0f,0.0f,\
															 0.0f,0.0f,1.0f,\
															 0.0f,0.0f,0.0f}

#define FREEDBACK_BUFF_SIZE		200
#define SEND_ARM2_BUFF_SIZE		512
															 
/*============================================================================*/
/*                              Global variables                              */
/*============================================================================*/
															 
static Package      package;
static CommondFlag  commond_flag;
															 
static char send_buff[FREEDBACK_BUFF_SIZE];
static TpUint16 out_len = 0;
														 
/*============================================================================*/
/*                            Function definition                             */
/*============================================================================*/ 
															 
void FindAsciiField(TpChar* pBuf,TpChar* Processed[],TpUchar* num)
{
	uint8_t i = 0;
	while((Processed[i] = (TpChar*)strtok((char*)pBuf,COMMA_CHARACTER_ASCII)) != NULL)
	{
		i++;
		pBuf = NULL;
	}
	*num = i-1;
}

/***************** mag cali online use ***************************/
#include "MagCaliOnline.h"
extern unsigned char flag_cali;
/*****************************************************************/
TpBool SetClass(Package* pa)
{
	TpBool result = RESULT_ERROR;
	TpChar* buff_processed[35]; 
	TpUchar  num = 0;
	
/****************** set baudrate *****************************/
  if(!memcmp(SET_BAUDRATE,pa->buf,sizeof(SET_BAUDRATE)-1))
	{
		TpInt32 baud_rate = 0;
		FindAsciiField(pa->buf,buff_processed,&num);
		result = My_atoi((const char*)buff_processed[DATA_VALUE_LOCATION],&baud_rate);
		
		if(result != RESULT_OK)/*Invalid data*/
		{
			return result;
		}
		else
		{
			 if(baud_rate == GetUserUsartBaudrate())
			 {
					return result;
			 }
			 else
			 {
				 result = SetUserUsartBaudrate(baud_rate);
				 return result;
			 }
		}	
	}
/****************** set product id*****************************/
  if(!memcmp(SET_PRODUCT_ID,pa->buf,sizeof(SET_PRODUCT_ID)-1))
	{
		TpInt32 product_id = 0;
		FindAsciiField(pa->buf,buff_processed,&num);
		result = My_atoi((const char*)buff_processed[3],&product_id);
		
		if(result != RESULT_OK)
		{
			return result;
		}	
		else if(product_id < 0)
		{
		  result = RESULT_PARA_NOT_VALID;
			return result;
		}
		else
		{
			SetProductID(product_id);
			result = RESULT_OK;	
			return  result;
		}
	}
	
/****************** set hardware version *****************************/
  if(!memcmp(SET_HARD_VERSION,pa->buf,sizeof(SET_HARD_VERSION)-1))
	{
		TpInt32 hardware_version = 0;
		FindAsciiField(pa->buf,buff_processed,&num);
		result = My_atoi((const char*)buff_processed[DATA_VALUE_LOCATION],&hardware_version);
		
		if(result != RESULT_OK)
		{
			return result;
		}	
		else if(hardware_version < 0)
		{
		  result = RESULT_PARA_NOT_VALID;
			return result;
		}
		else
		{
			SetHardwareVersion(hardware_version);
			result = RESULT_OK;
			return result;
		}
	}
/****************** set software version *****************************/
  if(!memcmp(SET_SOFT_VERSION,pa->buf,sizeof(SET_SOFT_VERSION)-1))
	{
		TpInt32 software_version = 0;
		FindAsciiField(pa->buf,buff_processed,&num);
		result = My_atoi((const char*)buff_processed[DATA_VALUE_LOCATION],&software_version);
		
		if(result != RESULT_OK)
		{
			return result;
		}	
		else if(software_version < 0)
		{
		  result = RESULT_PARA_NOT_VALID;
			return result;
		}
		else
		{
			SetSoftwareVersion(software_version);
			result = RESULT_OK;
			return result;
		}	
	}	
/*************************  set mag sample rate  ************************/
	if(!memcmp(SET_MAG_RATE,pa->buf,sizeof(SET_MAG_RATE)-1))
	{	
		TpInt32 mag_rate = 0;
		FindAsciiField(pa->buf,buff_processed,&num);
		result = My_atoi((const char*)buff_processed[DATA_VALUE_LOCATION],&mag_rate);
		
		if(result != RESULT_OK)
		{
			return result;
		}	
		else if(mag_rate < 0)
		{
		  result = RESULT_PARA_NOT_VALID;
			return result;
		}
		else 
		{
		  result = SetMagSampleRate(mag_rate);
			return result;
		}		
	}
/****************** set mag cali para *****************************/
  if(!memcmp(SET_MAG_CAL_PARA,pa->buf,sizeof(SET_MAG_CAL_PARA)-1))
	{
		float mag_para[MAG_CAL_PARA_NUM];
   	FindAsciiField(pa->buf,buff_processed,&num);

    /*Calibration_parameter for buff_processed[3] to buff_processed[15] */	
    for(uint8_t i=DATA_VALUE_LOCATION;i<(DATA_VALUE_LOCATION + MAG_CAL_PARA_NUM);i++)
		{
			mag_para[i-DATA_VALUE_LOCATION] = atof((const char*)buff_processed[i]);
		}
		SetMagCaliPara(mag_para);
		result = RESULT_OK;
		return result;
	}	
/****************** set mag cali para erase *****************************/
  if(!memcmp(SET_MAG_CAL_PARA_ERASE,pa->buf,sizeof(SET_MAG_CAL_PARA_ERASE)-1))
	{
     float mag_para[MAG_CAL_PARA_NUM] = MAG_CAL_PARA_INIT;
     SetMagCaliParaErase(mag_para);
		 result = RESULT_OK;
		 return result;
	}	
/****************** set mag cali online *****************************/
  if(!memcmp(SET_MAG_CAL_ONLINE_NO_OUT,pa->buf,sizeof(SET_MAG_CAL_ONLINE_NO_OUT)-1))
	{
		commond_flag.magcali_online = 1;
		commond_flag.mag_output_progress = 0;
		result = RESULT_OK; 	
		return result;    
	}	
	
	 if(!memcmp(SET_MAG_CAL_ONLINE_OUTPUT,pa->buf,sizeof(SET_MAG_CAL_ONLINE_OUTPUT)-1))
	{
		commond_flag.magcali_online = 1;
		commond_flag.mag_output_progress = 1;
		result = RESULT_OK; 	
		return result;    
	}	

	
/*----------------------------------- WORK MODE -----------------------------------*/
	if(!memcmp(SET_NAV_WORK_MODE,pa->buf,sizeof(SET_NAV_WORK_MODE)-1))
	{
		FindAsciiField(pa->buf,buff_processed,&num);
		
		if(!memcmp(NAV_LAND_MODE,buff_processed[3],sizeof(NAV_LAND_MODE)-1))
		{  
			SetAppMode(LAND);
			return RESULT_OK;
		}
		if(!memcmp(NAV_AIR_MODE,buff_processed[3],sizeof(NAV_AIR_MODE)-1))
		{		
			SetAppMode(AIR);
			return RESULT_OK;
		}
		if(!memcmp(NAV_OCEAN_MODE,buff_processed[3],sizeof(NAV_OCEAN_MODE)-1))
		{
			SetAppMode(OCEAN);
			return RESULT_OK;
		}
		
		return RESULT_ERROR;		
	}
	
	if(!memcmp(SET_LEVER_ARM,pa->buf,sizeof(SET_LEVER_ARM)-1))
	{
	
		FindAsciiField(pa->buf,buff_processed,&num);
		
		float gnss_arm[3];
		
		gnss_arm[0] = atof(buff_processed[4]);
		gnss_arm[1] = atof(buff_processed[5]);		
		gnss_arm[2] = atof(buff_processed[6]);
		
		
		SetGnssArm(gnss_arm);
	
		return RESULT_OK;
	}
	
		if(!memcmp(SET_BASE_LINE,pa->buf,sizeof(SET_BASE_LINE)-1))
	{
		FindAsciiField(pa->buf,buff_processed,&num);
		
		float baseline;
		
		baseline = atof(buff_processed[4]);
		
		SetGnssBaseline(baseline);
		
		return RESULT_OK;
	}

/*--------------------------------- send to arm1 -----------------------------------*/	
//	if(strstr((const char*)pa->buf,SET_ACC_PARA_COMMAND) !=NULL)
//	{ 
//		FindAsciiField(pa->buf,buff_processed,&num);
//		UsartPushMainBuf(GetUsartAddress(COM_ARM3_TO_ARM2),(TpUchar*)PackAccPara(buff_processed,COMMAND_ID_ACC_PARA),Commond_ACC_GYRO_PARA_SIZE);	
//		return NO_ANSWER;
//	}
//	
//	if(strstr((const char*)pa->buf,SET_GYRO_PARA_COMMAND) !=NULL)
//	{	
//		FindAsciiField(pa->buf,buff_processed,&num);
//		UsartPushMainBuf(GetUsartAddress(COM_ARM3_TO_ARM2),(TpUchar*)PackAccPara(buff_processed,COMMAND_ID_GYRO_PARA),Commond_ACC_GYRO_PARA_SIZE);	
//		return NO_ANSWER;
//	}
//	
//	if(strstr((const char*)pa->buf,ERASE_ACC_GYRO_PARA) !=NULL)
//	{
//		UsartPushMainBuf(GetUsartAddress(COM_ARM3_TO_ARM2),(TpUchar*)CommondBin(FRAME_ID_SET,COMMAND_ID_ERASE_ACC_GYRO_PARA,NULL),Commond_BIN_SIZE);
//		return NO_ANSWER;
//	}
	
	return result;
}


TpBool GetClass(Package* pa)
{
	TpBool result = RESULT_ERROR;

	/************************ get user usart baudrate*******************************/
  if(!memcmp(GET_BAUDRATE,pa->buf,sizeof(GET_BAUDRATE)-1))
	{
	  out_len = snprintf(send_buff,FREEDBACK_BUFF_SIZE,GET_BAUDRATE_OUT,GetUserUsartBaudrate());
		return RESULT_OK;
	}
	
	/************************ get product id*******************************/
  if(!memcmp(GET_PRODUCT_ID,pa->buf,sizeof(GET_PRODUCT_ID)-1))
	{
	  out_len = snprintf(send_buff,FREEDBACK_BUFF_SIZE,GET_PRODUCT_ID_OUT,GetProductID());
		return RESULT_OK;
	}
	
	/************************ get hardware version *******************************/
	if(!memcmp(GET_HARD_VERSION,pa->buf,sizeof(GET_HARD_VERSION)-1))
	{
	  out_len = snprintf(send_buff,FREEDBACK_BUFF_SIZE,GET_HARD_VERSION_OUT,GetHardwareVersion());
		return RESULT_OK;
	}
	
	/************************ get software version *******************************/
	if(!memcmp(GET_SOFT_VERSION,pa->buf,sizeof(GET_SOFT_VERSION)-1))
	{
	  out_len = snprintf(send_buff,FREEDBACK_BUFF_SIZE,GET_SOFT_VERSION_OUT,GetSoftwareVersion());
		return RESULT_OK;
	}
	
	/************************ get mag sample rate*******************************/
  if(!memcmp(GET_MAG_RATE,pa->buf,sizeof(GET_MAG_RATE)-1))
	{
	  out_len = snprintf(send_buff,FREEDBACK_BUFF_SIZE,GET_MAG_RATE_OUT,GetMagSampleRate());
	  return RESULT_OK;
	}
	
	/************************ get magcali parameters *******************************/
	if(!memcmp(GET_MAG_CAL_PARA,pa->buf,sizeof(GET_MAG_CAL_PARA)-1))
	{	
		float k_mag[9],dift_mag[3];
		
		GetMatrixMag(&k_mag);
		GetBiasMag(&dift_mag);
		
	  out_len = snprintf(send_buff,FREEDBACK_BUFF_SIZE,GET_MAG_CAL_PARA_OUT,	
		                                 k_mag[0],k_mag[1],k_mag[2],
							                       k_mag[3],k_mag[4],k_mag[5],
						                         k_mag[6],k_mag[7],k_mag[8],
						                         dift_mag[0],dift_mag[1],dift_mag[2]);
		
		return RESULT_OK;
	}
	
/*----------------------------------- send to arm2,1  -----------------------------------*/	
//	if(strstr((const char*)pa->buf,GET_ACC_PARA_COMMAND) !=NULL)
//	{ 
//		UsartPushMainBuf(GetUsartAddress(COM_ARM3_TO_ARM2),(TpUchar*)CommondBin(FRAME_ID_GET,COMMAND_ID_ACC_PARA,NULL),Commond_BIN_SIZE);	
//		return NO_ANSWER;		
//	}
//	
//	if(strstr((const char*)pa->buf,GET_GYRO_PARA_COMMAND) !=NULL)
//	{
//		UsartPushMainBuf(GetUsartAddress(COM_ARM3_TO_ARM2),(TpUchar*)CommondBin(FRAME_ID_GET,COMMAND_ID_GYRO_PARA,NULL),Commond_BIN_SIZE);
//		return NO_ANSWER;		
//	}
	
	if(!memcmp(GET_NAV_WORK_MODE,pa->buf,sizeof(GET_NAV_WORK_MODE)-1))
	{
		uint32_t mode;
		mode = GetAppMode();
		
		switch(mode)
		{
			case LAND:
									  out_len = snprintf(send_buff,FREEDBACK_BUFF_SIZE,NAV_LAND_MODE_OUT);
									  return RESULT_OK;
					
			case AIR:
										out_len = snprintf(send_buff,FREEDBACK_BUFF_SIZE,NAV_AIR_MODE_OUT);
										return RESULT_OK;
			case OCEAN:
										out_len = snprintf(send_buff,FREEDBACK_BUFF_SIZE,NAV_OCEAN_MODE_OUT);
										return RESULT_OK;
			
			default :   	break;
		}
		
	}
	
	if(!memcmp(GET_LEVER_ARM,pa->buf,sizeof(GET_LEVER_ARM)-1))
	{
		float gnss_arm[3];
		
		GetGnssArm(&gnss_arm);
		
		out_len = snprintf(send_buff,FREEDBACK_BUFF_SIZE,GET_LEVER_ARM_OUT,gnss_arm[0],gnss_arm[1],gnss_arm[2]);
		return RESULT_OK;
	}
	
	if(!memcmp(GET_BASE_LINE,pa->buf,sizeof(GET_BASE_LINE)-1))
	{
		float base_line;	
		base_line = GetGnssBaseLine();
		
		out_len = snprintf(send_buff,FREEDBACK_BUFF_SIZE,GET_BASE_LINE_OUT,base_line);
		return RESULT_OK;
	}
	
	return result;	
}


TpBool OutputClass(Package* pa)
{
	TpBool result = RESULT_ERROR;
	TpChar* buff_processed[20]; 
	TpUchar  num = 0;
	TpInt32 rate = 0;
	OutputFormat  output_format;
	TpChar  rate_location = 0;
	
/*-----------------------------------------------------------------------------------------------------
																					ASCII OURS
-----------------------------------------------------------------------------------------------------*/
		
	if(!memcmp(OUTPUT_IMU_ASCII_RATE,pa->buf,sizeof(OUTPUT_IMU_ASCII_RATE)-1))/* output imu ascii */
	{
		rate_location = DATA_VALUE_LOCATION;
		output_format = IMU_ASCII;
	}	
	else if(!memcmp(OUTPUT_AHRS_ASCII_RATE,pa->buf,sizeof(OUTPUT_AHRS_ASCII_RATE)-1))/* output ahrs ascii */
	{
		rate_location = DATA_VALUE_LOCATION;
		output_format = AHRS_ASCII;
	}			
	else if(!memcmp(OUTPUT_NAV_ASCII_RATE,pa->buf,sizeof(OUTPUT_NAV_ASCII_RATE)-1))/* output nav ascii */
	{
		rate_location = DATA_VALUE_LOCATION;
		output_format = NAV_ASCII;
	}		
	else if(!memcmp(OUTPUT_AVNAV_ASCII_RATE,pa->buf,sizeof(OUTPUT_AVNAV_BIN_RATE)-1))/* output avnav ascii */
	{
		rate_location = DATA_VALUE_LOCATION;
		output_format = AVNAV_ASCII;
	}
	else if(!memcmp(OUTPUT_LVNAV_ASCII_RATE,pa->buf,sizeof(OUTPUT_LVNAV_BIN_RATE)-1))/* output lvnav ascii */
	{
		rate_location = DATA_VALUE_LOCATION;
		output_format = LVNAV_BIN;
	}
	else if(!memcmp(OUTPUT_MVNAV_ASCII_RATE,pa->buf,sizeof(OUTPUT_MVNAV_BIN_RATE)-1))/* output mvnav ascii */
	{
		rate_location = DATA_VALUE_LOCATION;
		output_format = MVNAV_BIN;
	}
	

/*-----------------------------------------------------------------------------------------------------
																					ASCII NEMA
-----------------------------------------------------------------------------------------------------*/
	
	else if(!memcmp(OUTPUT_GPGGA_RATE,pa->buf,sizeof(OUTPUT_GPGGA_RATE)-1))	/* output imu ascii */
	{
		rate_location = 3;
		output_format = GPGGA;
	}
	else if(!memcmp(OUTPUT_GPRMC_RATE,pa->buf,sizeof(OUTPUT_GPRMC_RATE)-1))/* output imu ascii */
	{
		rate_location = 3;
		output_format = GPRMC;
	}
	
/*-----------------------------------------------------------------------------------------------------
																					BIN
-----------------------------------------------------------------------------------------------------*/	
		
	else if(!memcmp(OUTPUT_IMU_BIN_RATE,pa->buf,sizeof(OUTPUT_IMU_BIN_RATE)-1))	/* output imu bin */
	{
		rate_location = DATA_VALUE_LOCATION;
		output_format = IMU_BIN;
	}
	else if(!memcmp(OUTPUT_AHRS_BIN_RATE,pa->buf,sizeof(OUTPUT_AHRS_BIN_RATE)-1))/* output ahrs bin */
	{
		rate_location = DATA_VALUE_LOCATION;
		output_format = AHRS_BIN;
	}
	else if(!memcmp(OUTPUT_NAV_BIN_RATE,pa->buf,sizeof(OUTPUT_NAV_BIN_RATE)-1))	/* output nav bin */
	{
		rate_location = DATA_VALUE_LOCATION;
		output_format = NAV_BIN;
	}		
	else if(!memcmp(OUTPUT_AVNAV_BIN_RATE,pa->buf,sizeof(OUTPUT_AVNAV_BIN_RATE)-1))/* output avnav bin */
	{
		rate_location = DATA_VALUE_LOCATION;
		output_format = AVNAV_BIN;
	}
	else if(!memcmp(OUTPUT_LVNAV_BIN_RATE,pa->buf,sizeof(OUTPUT_LVNAV_BIN_RATE)-1))/* output lvnav bin */
	{
		rate_location = DATA_VALUE_LOCATION;
		output_format = LVNAV_BIN;
	}
	else if(!memcmp(OUTPUT_MVNAV_BIN_RATE,pa->buf,sizeof(OUTPUT_MVNAV_BIN_RATE)-1))/* output mvnav bin */
	{
		rate_location = DATA_VALUE_LOCATION;
		output_format = MVNAV_BIN;
	}
	
/*-----------------------------------------------------------------------------------------------------
																					developer
-----------------------------------------------------------------------------------------------------*/	
	
	else if(!memcmp(OUTPUT_DEV_AHRS_BIN_RATE,pa->buf,sizeof(OUTPUT_DEV_AHRS_BIN_RATE)-1))
	{
		rate_location = DATA_VALUE_LOCATION;
		output_format = DEV_AHRS_BIN;
	}
		else if(!memcmp(OUTPUT_DEV_IGM_BIN_RATE,pa->buf,sizeof(OUTPUT_DEV_IGM_BIN_RATE)-1))
	{
		rate_location = DATA_VALUE_LOCATION;
		output_format = DEV_NAV_BIN;
	}
	
	FindAsciiField(pa->buf,buff_processed,&num);
	
	result = My_atoi((const char*)buff_processed[rate_location],&rate);
	
	if(result != RESULT_OK)
	{
		return result;
	}
	else if((rate < 0)||(rate > OUTPUT_RATE_MAX))
	{
		result = RESULT_PARA_NOT_VALID;
		return result;
	}
	else
	{
		SetOutputRate(output_format,rate);
		return result;			
	}		
	
//	return result;/*The program can't get here*/
}

TpBool DecodeCommond(uint8_t * pBuf,uint16_t len)
{
	TpBool result = RESULT_ERROR;
	
	if((BIN_HEAD_0==pBuf[0])&&(BIN_HEAD_1==pBuf[1])) /*check head*/
	{
		/*bin*/
	}
  else if(!memcmp(ASCII_CLASS_HEAD,pBuf,sizeof(ASCII_CLASS_HEAD)-1))/*ASCII*/
	{
			if(strstr((const char*)pBuf,",,") != NULL)
			{
			  UsartPushMainBuf(GetUsartAddress(USART_2),(TpUchar*)NOT_A_VALID_COMMAND,sizeof(NOT_A_VALID_COMMAND)-1);
				return result;
			}
			if(strstr((const char*)pBuf,",*ff\r\n") == NULL)
			{
			  UsartPushMainBuf(GetUsartAddress(USART_2),(TpUchar*)NOT_A_VALID_COMMAND,sizeof(NOT_A_VALID_COMMAND)-1);
				return result;
			}	
			result = GetAsciiCommond(pBuf,len,&package);				
	}
	else 
	{
		UsartPushMainBuf(GetUsartAddress(USART_2),(TpUchar*)NOT_A_VALID_COMMAND,sizeof(NOT_A_VALID_COMMAND)-1);
	}

	return result;
}

TpBool GetAsciiCommond(uint8_t * pBuf,uint16_t len,Package* pa)
{
	TpBool result = INLIB_ERROR; /*123 Meaningless,just for judge the end [*ff\r\n]*/
	
	TpUchar  ch = 0;
	TpUint16 i = 0;
	
 
	for(i=0;i<len;i++)
	{
    ch = pBuf[i];
    switch(ch)
    {
			case COMMOND_START:
      {
        pa->start = 1;
        pa->buf[0] = ch;
        pa->pt = 1;
        pa->num=0;
      }
      break;
			
      case COMMOND_END_6:
      {
				  if(pBuf[i -1] == COMMOND_END_5)
					{
							if(!memcmp(COMMOND_END,&pBuf[i - sizeof(COMMOND_END) + 2],sizeof(COMMOND_END)-1))/*end:  *ff\r\n  */
							{				
								pa->buf[pa->pt] = ch;
								pa->pt++;
								pa->num =pa->pt;
								pa->start = 0;
								result = DecodeAsciiCommond(pa);
							}											
					}
					else
					{
						if(pa->start)
						{
							pa->buf[pa->pt] = ch;
							pa->pt++;
						}
					}
      }
      break;
      default:
      {
        if(pa->start)
        {
          pa->buf[pa->pt] = ch;
          pa->pt++;
        }
      }
      break;
		}
	}
	return result;	
}


TpBool DecodeAsciiCommond(Package* pa)
{
	TpBool  result = RESULT_ERROR;
	
	TpUint16 j = 0;
  
  for(j=0;j<pa->num;j++)
  {
    if( (pa->buf[j]>=LETTER_CAPITAL_A)&&(pa->buf[j]<=LETTER_CAPITAL_Z) )
    {
      pa->buf[j] += CAPITAL_LOWERCASE_LEN;
    }
  }
	
/*----------------------------------- arm3 command -----------------------------------*/			
  if(!memcmp(ASCII_CLASS_SET,pa->buf,sizeof(ASCII_CLASS_SET)-1))
	{
	    result = SetClass(pa);
			if(result == RESULT_OK)
			{
						UsartPushMainBuf(GetUsartAddress(USART_2),(TpUchar*)FREEDBACK_OK,sizeof(FREEDBACK_OK)-1);
				    return RESULT_OK;
			}	
			else if(result == RESULT_ERROR)
			{
						UsartPushMainBuf(GetUsartAddress(USART_2),(TpUchar*)FREEDBACK_ERROR,sizeof(FREEDBACK_ERROR)-1);
				    return RESULT_ERROR;
			}
			else if(result == RESULT_PARA_NOT_VALID)
			{
						UsartPushMainBuf(GetUsartAddress(USART_2),(TpUchar*)NOT_A_VALID_CONFIG,sizeof(NOT_A_VALID_CONFIG)-1);
				    return RESULT_PARA_NOT_VALID;
			}
			else if(result == RESULT_COMMAND_INVALID)
			{
						UsartPushMainBuf(GetUsartAddress(USART_2),(TpUchar*)NOT_A_VALID_COMMAND,sizeof(NOT_A_VALID_COMMAND)-1);
				    return RESULT_COMMAND_INVALID;
			}					
			else if(result == NO_ANSWER)
			{
				    return NO_ANSWER;
			}
			else
			{
				UsartPushMainBuf(GetUsartAddress(USART_2),(TpUchar*)NOT_A_VALID_COMMAND,sizeof(NOT_A_VALID_COMMAND)-1);
			}						
	}
	else if(!memcmp(ASCII_CLASS_GET,pa->buf,sizeof(ASCII_CLASS_GET)-1))
	{
	    result = GetClass(pa);
		
			if(result == RESULT_OK)
			{
				UsartPushMainBuf(GetUsartAddress(USART_2),(TpUchar*)send_buff,out_len);
				return RESULT_OK;
			}
			if(result ==  NO_ANSWER)
			{
				return NO_ANSWER;
			}
			else if(result == RESULT_COMMAND_INVALID)
			{
						UsartPushMainBuf(GetUsartAddress(USART_2),(TpUchar*)NOT_A_VALID_COMMAND,sizeof(NOT_A_VALID_COMMAND)-1);
				    return RESULT_COMMAND_INVALID;
			}	
			else if(result == RESULT_ERROR)
			{
				UsartPushMainBuf(GetUsartAddress(USART_2),(TpUchar*)FREEDBACK_ERROR,sizeof(FREEDBACK_ERROR)-1);
				return RESULT_ERROR;
			}
	}
	else if(!memcmp(ASCII_CLASS_OUTPUT,pa->buf,sizeof(ASCII_CLASS_OUTPUT)-1))
	{
	    result = OutputClass(pa);
		
			switch(result)
			{
				case RESULT_OK:
						UsartPushMainBuf(GetUsartAddress(USART_2),(TpUchar*)FREEDBACK_OK,sizeof(FREEDBACK_OK)-1);
						break;		
				
				case RESULT_ERROR:
						UsartPushMainBuf(GetUsartAddress(USART_2),(TpUchar*)FREEDBACK_ERROR,sizeof(FREEDBACK_ERROR)-1);
						break;	
				
				case RESULT_PARA_NOT_VALID:
						UsartPushMainBuf(GetUsartAddress(USART_2),(TpUchar*)NOT_A_VALID_CONFIG,sizeof(NOT_A_VALID_CONFIG)-1);
						break;	
				case NO_ANSWER:		
				    return NO_ANSWER;	
				
				default :
						UsartPushMainBuf(GetUsartAddress(USART_2),(TpUchar*)FREEDBACK_ERROR,sizeof(FREEDBACK_ERROR)-1);
						break;
			}
	}
		/****************** save configs*****************************/
	else if(!memcmp(ALL_SETTING_SAVE,pa->buf,sizeof(ALL_SETTING_SAVE)-1))
	{
		__disable_irq() ;
		FlashWrite();	
    __enable_irq() ;
		
		UsartPushMainBuf(GetUsartAddress(COM_ARM3_TO_USER),(TpUchar*)FREEDBACK_OK,sizeof(FREEDBACK_OK)-1);
		result = RESULT_OK;
		return result;		
	}
	
	/****************** all settings restore *****************************/
	else if(!memcmp(ALL_SETTING_RESTORE,pa->buf,sizeof(ALL_SETTING_RESTORE)-1))
	{
	  FlashInit();
		
		UsartPushMainBuf(GetUsartAddress(COM_ARM3_TO_USER),(TpUchar*)FREEDBACK_OK,sizeof(FREEDBACK_OK)-1);
		result = RESULT_OK;
		return result;
	}
	else
	{
		  UsartPushMainBuf(GetUsartAddress(USART_2),(TpUchar*)NOT_A_VALID_CONFIG,sizeof(NOT_A_VALID_CONFIG)-1);
	}
	
	return result;
}



TpBool DecodeARM2Bin(TpUchar* pUsart_buff,TpUint16 len,UmiIgmBin* pUmiOutput)
{
	TpBool  result = RESULT_OK;
	TpUint16 check_sum = 0;
	TpUint16 pUmiOutput_length = sizeof(UmiIgmBin);
	
	
	if(len == pUmiOutput_length)
	{
			check_sum =  CheckSum((TpUchar*)pUsart_buff,pUmiOutput_length);
			memcpy(pUmiOutput,pUsart_buff,pUmiOutput_length); 
		
			if((pUmiOutput->head[0] == DATA_FROM_ARM2_HEAD_1)&&(pUmiOutput->head[1] == DATA_FROM_ARM2_HEAD_2))
			{
				if(pUmiOutput->check == check_sum)
				{
					 LogOutput(pUmiOutput);		
				}
				else
				{
					result = 	RESULT_ERROR;	
				}
			}
			else
			{
				result = 	RESULT_ERROR;	
			}
	}
	else 
	{
		result = 	RESULT_ERROR;	
	}
	return result;	
}


TpVoid OutMagCaliOnlinePara(TpFloat bias[3],TpFloat matrix[9])
{
	TpUchar len = 0;
	TpUchar buf_magpara[50];
//	len  = snprintf((char*)buf_magpara,200,"$cmd,bias,%.4f,%.4f,%.4f\r\n",bias[0],bias[1],bias[2]);
//	len += snprintf((char*)buf_magpara+len,200,"$cmd,matirx,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\r\n",
//		                                           matrix[0],matrix[1],matrix[2],
//	                                             matrix[3],matrix[4],matrix[5],
//	                                             matrix[6],matrix[7],matrix[8]);
	
	len  = snprintf((char*)buf_magpara,50,"$cmd,mag,calibration,OK!\r\n");
	UsartPushMainBuf(GetUsartAddress(USART_2),buf_magpara,len);
}



CommondFlag* GetCommondFlag(TpVoid)
{
	 return &commond_flag;

}
