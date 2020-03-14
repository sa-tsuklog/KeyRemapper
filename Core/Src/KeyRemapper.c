/*
 * KeyRemapper.cpp
 *
 *  Created on: Jan 26, 2020
 *      Author: sa
 */

#include "usbh_hid.h"
#include "usbh_hid_keybd.h"
#include "usb_device.h"
#include "usbd_custom_hid_if.h"

#include "usbd_customhid.h"
#include "usbd_conf.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// When you regenerate code from CubeIDE, fix following parts.
//
// in  usbd_customhid.h
// #define CUSTOM_HID_EPIN_SIZE                 0x02U -> 0x08U
//
// in usbd_conf.h
// #define USBD_CUSTOM_HID_REPORT_DESC_SIZE     2U -> Match to report descriptor size(default keyboard = 62U)
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct keyboardHID_t{
	uint8_t modifiers;
	uint8_t reserved;
	uint8_t key[6];
};

static const uint8_t KEYCODE_J = 0x0D;
static const uint8_t KEYCODE_K = 0x0E;
static const uint8_t KEYCODE_L = 0x0F;
static const uint8_t KEYCODE_U = 0x18;
static const uint8_t KEYCODE_I = 0x0C;
static const uint8_t KEYCODE_O = 0x12;
static const uint8_t KEYCODE_LEFT = 0x50;
static const uint8_t KEYCODE_RIGHT = 0x4F;
static const uint8_t KEYCODE_UP = 0x52;
static const uint8_t KEYCODE_DOWN = 0x51;
static const uint8_t KEYCODE_HOME = 0x4A;
static const uint8_t KEYCODE_END = 0x4D;

extern USBD_HandleTypeDef hUsbDeviceHS;
extern USBH_HandleTypeDef hUsbHostFS;

uint8_t getModifier(HID_KEYBD_Info_TypeDef* info){
	return info->rgui<<7 | info->ralt<<6 | info->rshift<<5 | info->rctrl<<4 | info->lgui<<3 | info->lalt<<2 | info->lshift<<1 | info->lctrl;
}

uint8_t isShiftPressed(HID_KEYBD_Info_TypeDef* info){
	return info->rshift || info->lshift;
}

uint8_t isCtrlPressed(HID_KEYBD_Info_TypeDef* info){
	return info->rctrl || info->lctrl;
}

uint8_t isAltPressed(HID_KEYBD_Info_TypeDef* info){
	return info->ralt || info->rctrl;
}

uint8_t isVirtualArrowKeyPressed(HID_KEYBD_Info_TypeDef* info){
	if(isCtrlPressed(info)){
		for(int i=0;i<6;i++){
			if(info->keys[i]== KEYCODE_J){
				return KEYCODE_LEFT;
			}
			if(info->keys[i]== KEYCODE_K){
				return KEYCODE_DOWN;
			}
			if(info->keys[i]== KEYCODE_L){
				return KEYCODE_RIGHT;
			}
			if(info->keys[i]== KEYCODE_U){
				return KEYCODE_HOME;
			}
			if(info->keys[i]== KEYCODE_I){
				return KEYCODE_UP;
			}
			if(info->keys[i]== KEYCODE_O){
				return KEYCODE_END;
			}
		}
	}
	return 0;
}

void USBH_HID_EventCallback(USBH_HandleTypeDef *phost){
	HID_KEYBD_Info_TypeDef* info = USBH_HID_GetKeybdInfo(phost);

	struct keyboardHID_t keyboardHID;

	keyboardHID.modifiers = getModifier(info);
	keyboardHID.reserved = 0;

	uint8_t virtualArrow;
	if((virtualArrow = isVirtualArrowKeyPressed(info)) != 0){
		keyboardHID.key[0] = virtualArrow;
		for(int i=1;i<6;i++){
			keyboardHID.key[i] = 0;
		}
		keyboardHID.modifiers &= 0xEE;
	}else{
		for(int i=0;i<6;i++){
			keyboardHID.key[i] = info->keys[i];
		}
	}

	USBD_CUSTOM_HID_SendReport(&hUsbDeviceHS, (uint8_t*)&keyboardHID, sizeof(struct keyboardHID_t));
}

int8_t ledState = 0;
uint8_t prevLedState = 0;
USBH_StatusTypeDef usbhState = USBH_OK;
int32_t count = 0;

void setLed(uint8_t led){
	ledState = led;
}

void updateLedState(){
	HID_HandleTypeDef *HID_Handle = (HID_HandleTypeDef *) hUsbHostFS.pActiveClass->pData;

	if(HID_Handle->state == HID_POLL){
		if((ledState != prevLedState) || (usbhState == USBH_BUSY)){
			prevLedState = ledState;
			usbhState = USBH_HID_SetReport(&hUsbHostFS,0x02,0,&ledState,1);
		}
	}



//	if(((ledState != prevLedState) && (hUsbHostFS.gState == HOST_CLASS)) || (usbhState == USBH_BUSY)){
//		prevLedState = ledState;
//		usbhState = USBH_HID_SetReport(&hUsbHostFS,0x02,0,&ledState,1);
//	}
}

