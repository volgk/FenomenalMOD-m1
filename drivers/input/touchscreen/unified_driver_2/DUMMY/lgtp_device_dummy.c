/***************************************************************************
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *    File  	: lgtp_device_dummy.c
 *    Author(s)   : D3 BSP Touch Team < d3-bsp-touch@lge.com >
 *    Description :
 *
 ***************************************************************************/
#define LGTP_MODULE "[DUMMY]"

/****************************************************************************
* Include Files
****************************************************************************/
#include <linux/input/unified_driver_2/lgtp_common.h>

#include <linux/input/unified_driver_2/lgtp_common_driver.h>
#include <linux/input/unified_driver_2/lgtp_platform_api.h>


/****************************************************************************
* Manifest Constants / Defines
****************************************************************************/


/****************************************************************************
 * Macros
 ****************************************************************************/


/****************************************************************************
* Type Definitions
****************************************************************************/


/****************************************************************************
* Variables
****************************************************************************/
static const char defaultFirmware[] = "leadingUI/dummy_firmware.img";

//====================================================================
// NORMAL : general touch(finger,key) is working
// OFF : touch is not working even knock-on ( lowest power saving )
// KNOCK_ON_ONLY : knock-on is only enabled
// KNOCK_ON_CODE : knock-on and knock-code are enabled
// NORMAL_HOVER : hover detection is enabled and general touch is working
// HOVER : only hover detection is enabled
//====================================================================
static TouchState dummyDeviceState = STATE_UNKNOWN;

/****************************************************************************
* Extern Function Prototypes
****************************************************************************/


/****************************************************************************
* Local Function Prototypes
****************************************************************************/


/****************************************************************************
* Local Functions
****************************************************************************/


/****************************************************************************
* Device Specific Functions
****************************************************************************/
static ssize_t show_device_name(struct i2c_client *client, char *buf)
{
	int ret = 0;

	TOUCH_FUNC();

	ret += sprintf(buf+ret, "%s\n", "Dummy Device");
	
	return ret;
}

static LGE_TOUCH_ATTR(device_name, S_IRUGO | S_IWUSR, show_device_name, NULL);

static struct attribute *Dummy_attribute_list[] = {
	&lge_touch_attr_device_name.attr,
	NULL,
};



static int Dummy_Initialize(struct i2c_client *client)
{
	TOUCH_FUNC();

	return TOUCH_SUCCESS;
}

static void Dummy_Reset(struct i2c_client *client)
{
	TOUCH_FUNC();

	TouchResetCtrl(0);
	msleep(10);
	TouchResetCtrl(1);
	msleep(200);

	dummyDeviceState = STATE_NORMAL;
}


//====================================================================
// Function : Dummy_QueryDeviceConnection
// Description
//   - Check if touch IC was connected
//   - will be called at module init ( to select proper device driver function )
//   - implement using "Maker ID Pin" or "Read special register of IC"
//   - In case of using "Read special register of IC", you should be implement it using "touch_i2c_read_for_query()"
//====================================================================
static int Dummy_Connect(void)
{
	TOUCH_FUNC();

	return TOUCH_SUCCESS;
}

//====================================================================
// Function : Dummy_InitRegister
// Description
//   - Initialize touch IC register
//   - will be called after IC reset ( by reset pin )
//====================================================================
static int Dummy_InitRegister(struct i2c_client *client)
{
	TOUCH_FUNC();

	return TOUCH_SUCCESS;
}

//====================================================================
// Function : Dummy_ClearInterrupt
// Description
//   - Clear interrupt
//   - will be called before interrupt enable to clear interrupt happened during interrupt disabled time
//====================================================================
static void Dummy_ClearInterrupt(struct i2c_client *client)
{
	TOUCH_FUNC();

	return;
}

//====================================================================
// Function : Dummy_InterruptHandler
// Description
//   - process interrupt
//   - will be called if interrupt detected by AP
//====================================================================
static int Dummy_InterruptHandler(struct i2c_client *client,TouchReadData *pData)
{
	return TOUCH_SUCCESS;
}

//====================================================================
// Function : Dummy_ReadIcFirmwareInfo
// Description
//   - Read firmware information from touch IC
//   - will be called at boot time or after writing new firmware
//====================================================================
static int Dummy_ReadIcFirmwareInfo(struct i2c_client *client, TouchFirmwareInfo *pFwInfo)
{
	TOUCH_FUNC();

	/* below value is same as default firmware image ( dummy_firmware.img ) */
	pFwInfo->moduleMakerID = 2;
	pFwInfo->moduleVersion = 0;
	pFwInfo->modelID = 3;
	pFwInfo->isOfficial = 1;
	pFwInfo->version = 4;

	return TOUCH_SUCCESS;	
}

//====================================================================
// Function : Dummy_GetBinFirmwareInfo
// Description
//   - parse and return firmware information from firmware image
//   - if filename is NULL, return information of default firmware image
//   - will be called at boot time or needed
//====================================================================
static int Dummy_GetBinFirmwareInfo(struct i2c_client *client, char *pFilename, TouchFirmwareInfo *pFwInfo)
{
	int ret = 0;
	const struct firmware *fw = NULL;
	u8 *pBin = NULL;
	char *pFwFilename = NULL;

	TOUCH_FUNC();
	
	if( pFilename == NULL ) {
		pFwFilename = (char *)defaultFirmware;
	} else {
		pFwFilename = pFilename;
	}

	TOUCH_LOG("Firmware filename = %s\n", pFwFilename);
	
	/* Get firmware image buffer pointer from file */
	ret = request_firmware(&fw, pFwFilename, &client->dev);
	if( ret ) {
		TOUCH_ERR("failed at request_firmware() ( error = %d )\n", ret);
		return TOUCH_FAIL;
	}

	pBin = (u8 *)(fw->data);

	/* parse and get firmware information function */
	pFwInfo->moduleMakerID = ( *(pBin) >> 4 ) & 0xF;
	pFwInfo->moduleVersion = *(pBin) & 0xF;
	pFwInfo->modelID = *(pBin+1);
	pFwInfo->isOfficial = ( *(pBin+3) >> 7 ) & 0x1;
	pFwInfo->version = *(pBin+3) & 0x7F;

	/* Free firmware image buffer */
	release_firmware(fw);

	return TOUCH_SUCCESS;
		
}

//====================================================================
// Function : Dummy_UpdateFirmware
// Description
//   - Write firmware to touch IC
//   - if filename is NULL, use default firmware image
//   - common driver will call Reset(), InitRegister() and ReadIcFirmwareInfo() one by one after writing
//====================================================================
static int Dummy_UpdateFirmware(struct i2c_client *client, char *pFilename)
{
	int ret = 0;
	const struct firmware *fw = NULL;
	u8 *pBin = NULL;
	char *pFwFilename = NULL;

	TOUCH_FUNC();
	
	if( pFilename == NULL ) {
		pFwFilename = (char *)defaultFirmware;
	} else {
		pFwFilename = pFilename;
	}

	TOUCH_LOG("Firmware filename = %s\n", pFwFilename);
	
	/* Get firmware image buffer pointer from file */
	ret = request_firmware(&fw, pFwFilename, &client->dev);
	if( ret ) {
		TOUCH_ERR("failed at request_firmware() ( error = %d )\n", ret);
		return TOUCH_FAIL;
	}

	pBin = (u8 *)(fw->data);

	/* IMPLEMENT : firmware update function */

	/* Free firmware image buffer */
	release_firmware(fw);

	return TOUCH_SUCCESS;
	
}

//====================================================================
// Function : Dummy_SetLpwgMode
// Description
//   - Set device to requested state
//====================================================================
static int Dummy_SetLpwgMode(struct i2c_client *client, TouchState newState, LpwgSetting  *pLpwgSetting)
{
	int ret = TOUCH_SUCCESS;
	
	TOUCH_FUNC();

	if( dummyDeviceState == newState ) {
		TOUCH_LOG("device state is same as driver requested\n");
		return TOUCH_SUCCESS;
	}

	switch( newState )
	{
		case STATE_NORMAL:
			TOUCH_LOG("device was set to NORMAL\n");
			break;
		case STATE_OFF:
			TOUCH_LOG("device was set to OFF\n");
			break;
		case STATE_KNOCK_ON_ONLY:
			TOUCH_LOG("device was set to KNOCK_ON_ONLY\n");
			break;
		case STATE_KNOCK_ON_CODE:
			TOUCH_LOG("device was set to KNOCK_ON_CODE\n");
			break;
		case STATE_NORMAL_HOVER:
			TOUCH_LOG("device was set to NORMAL_HOVER\n");
			break;
		case STATE_HOVER:
			TOUCH_LOG("device was set to HOVER\n");
			break;
		default:
			TOUCH_LOG("invalid state ( state = %d )\n", newState);
			ret = TOUCH_FAIL;
			break;
			
	}

	if( ret == TOUCH_SUCCESS ) {
		dummyDeviceState = newState;
	}

	return TOUCH_SUCCESS;
	
}

//====================================================================
// Function : Dummy_DoSelfDiagnosis
// Description
//   - diagnose touch pannel and return result
//   - can use pBuf to give more information ( be careful not to exceed buffer size )
//   - should create a file of result ( TBD : consider file saving on common driver side )
//====================================================================
static int Dummy_DoSelfDiagnosis(struct i2c_client *client, int* pRawStatus, int* pChannelStatus, char* pBuf, int bufSize, int* pDataLen)
{
	int dataLen = 0;

	TOUCH_FUNC();

	*pRawStatus = TOUCH_SUCCESS;
	*pChannelStatus = TOUCH_SUCCESS;

	dataLen += sprintf(pBuf, "%s", "========= Additional Information =========\n");
	dataLen += sprintf(pBuf+dataLen, "%s", "Device Name = Dummy\n");

	*pDataLen = dataLen;

	return TOUCH_SUCCESS;
}

//====================================================================
// Function : Dummy_AccessRegister
// Description
//   - read from or write to touch IC
//====================================================================
static int Dummy_AccessRegister(struct i2c_client *client, int cmd, int reg, int *pValue)
{
	int ret = 0;
	
	TOUCH_FUNC();

	switch( cmd )
	{
		case READ_IC_REG:
			ret = Touch_I2C_Read_Byte(client, (u8)reg, (u8 *)pValue);
			if( ret == TOUCH_FAIL ) {
				return TOUCH_FAIL;
			}
			break;

		case WRITE_IC_REG:
			ret = Touch_I2C_Write_Byte(client, (u8)reg, (u8)*pValue);
			if( ret == TOUCH_FAIL ) {
				return TOUCH_FAIL;
			}
			break;

		default:
			TOUCH_ERR("Invalid access command ( cmd = %d )\n", cmd);
			return TOUCH_FAIL;
			break;
	}

	return TOUCH_SUCCESS;

}


TouchDeviceSpecificFunction Dummy_Func = {

	.Initialize = Dummy_Initialize,
	.Reset = Dummy_Reset,
	.Connect = Dummy_Connect,
	.InitRegister = Dummy_InitRegister,
	.ClearInterrupt = Dummy_ClearInterrupt,
	.InterruptHandler = Dummy_InterruptHandler,
	.ReadIcFirmwareInfo = Dummy_ReadIcFirmwareInfo,
	.GetBinFirmwareInfo = Dummy_GetBinFirmwareInfo,
	.UpdateFirmware = Dummy_UpdateFirmware,
	.SetLpwgMode = Dummy_SetLpwgMode,
	.DoSelfDiagnosis = Dummy_DoSelfDiagnosis,
	.AccessRegister = Dummy_AccessRegister,
	.device_attribute_list = Dummy_attribute_list,
	
};


/* End Of File */


