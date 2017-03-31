/**
  ******************************************************************************
  * @file    USB_Device/HID_Standalone/Src/stm32f0xx_it.c
  * @author  MCD Application Team
  * @version V1.7.0
  * @date    04-November-2016
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics International N.V. 
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f0xx_it.h"

#define CURSOR_STEP     5
extern PCD_HandleTypeDef hpcd;
extern USBD_HandleTypeDef USBD_Device;

void NMI_Handler(void)
{
}

void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

void SVC_Handler(void)
{
}

void PendSV_Handler(void)
{
}


uint8_t key_m_scancodes_def [10*5] =
	{
	0xFF,0xFF,0xFF,0xFF,0xFF,
	KEY_SPACE,0xFF,0xFF,0xFF,0xFF,
	KEY_A,KEY_B,KEY_C,0xFF,0xFF,
	KEY_D,KEY_E,KEY_F,0xFF,0xFF,
	KEY_G,KEY_H,KEY_I,0xFF,0xFF,
	KEY_J,KEY_K,KEY_L,0xFF,0xFF,
	KEY_M,KEY_N,KEY_O,0xFF,0xFF,
	KEY_P,KEY_Q,KEY_R,KEY_S,0xFF,
	KEY_T,KEY_U,KEY_V,0xFF,0xFF,
	KEY_W,KEY_X,KEY_Y,KEY_Z,0xFF,
	};

uint8_t key_m_scancodes_ex [11*5] =
	{
	0xFF,0xFF,0xFF,0xFF,0xFF,
	KEY_DOT,KEY_COMMA,KEY_SLASH,0xFF,0xFF,
	KEY_SLASH,KEY_BACKSLASH,KEY_BACKSLASH,0xFF,0xFF,
	KEY_LEFTBRACE,KEY_RIGHTBRACE,KEY_6,0xFF,0xFF,
	KEY_LEFTBRACE,KEY_RIGHTBRACE,KEY_5,0xFF,0xFF,
	KEY_0,KEY_9,KEY_6,0xFF,0xFF,
	KEY_DOT,KEY_COMMA,KEY_MINUS,0xFF,0xFF,
	KEY_EQUAL,KEY_MINUS,KEY_8,0xFF,0xFF,
	KEY_APOSTROPHE,KEY_APOSTROPHE,KEY_GRAVE,0xFF,0xFF,
	KEY_1,KEY_2,KEY_3,0xFF,0xFF,
	KEY_EQUAL,KEY_7,0xFF,0xFF,0xFF,
	};

uint8_t key_m_scancodes_ex_mod [11*5] =
	{
	KEY_MOD_NONE,KEY_MOD_NONE,KEY_MOD_NONE,KEY_MOD_NONE,KEY_MOD_NONE,
	KEY_MOD_NONE,KEY_MOD_NONE,KEY_MOD_LSHIFT,KEY_MOD_NONE,KEY_MOD_NONE,
	KEY_MOD_NONE,KEY_MOD_NONE,KEY_MOD_LSHIFT,KEY_MOD_NONE,KEY_MOD_NONE,
	KEY_MOD_NONE,KEY_MOD_NONE,KEY_MOD_LSHIFT,KEY_MOD_NONE,KEY_MOD_NONE,
	KEY_MOD_LSHIFT,KEY_MOD_LSHIFT,KEY_MOD_LSHIFT,KEY_MOD_NONE,KEY_MOD_NONE,
	KEY_MOD_LSHIFT,KEY_MOD_LSHIFT,KEY_MOD_LSHIFT,KEY_MOD_NONE,KEY_MOD_NONE,
	KEY_MOD_LSHIFT,KEY_MOD_LSHIFT,KEY_MOD_LSHIFT,KEY_MOD_NONE,KEY_MOD_NONE,
	KEY_MOD_LSHIFT,KEY_MOD_NONE,KEY_MOD_LSHIFT,KEY_MOD_NONE,KEY_MOD_NONE,
	KEY_MOD_NONE,KEY_MOD_LSHIFT,KEY_MOD_LSHIFT,KEY_MOD_NONE,KEY_MOD_NONE,
	KEY_MOD_LSHIFT,KEY_MOD_LSHIFT,KEY_MOD_LSHIFT,KEY_MOD_NONE,KEY_MOD_NONE,
	KEY_MOD_NONE,KEY_MOD_LSHIFT,KEY_MOD_NONE,KEY_MOD_NONE,KEY_MOD_NONE,
	};

int16_t speedx,speedy;
uint8_t mouse_k,kbd_cols=0,keys, keys_last, mods,keys_out,mod_out,tmp,m_tmr;
uint8_t key_buffer_char[20],key_buffer_mod[20],key_buffer_p=0,key_m_p=0,key_m_last,caps_lock;
uint16_t key_m_cnt=0,counter=0;


uint8_t key_buffer_get (uint8_t * mod)
    {
    if (key_buffer_p>0)
	{
	*mod = key_buffer_mod[0];
	return key_buffer_char[0];
	}
    else
	return 0;
    }

void key_buffer_put (uint8_t key, uint8_t mod)
    {
    key_buffer_char[key_buffer_p] = key;
    key_buffer_mod[key_buffer_p++] = mod;
    }

uint8_t key_buffer_len (void)
    {
    return key_buffer_p;
    }

uint8_t key_buffer_dec (void)
    {
    uint8_t i;
    for (i=1;i<key_buffer_p;i++) key_buffer_char[i-1] = key_buffer_char[i];
    for (i=1;i<key_buffer_p;i++) key_buffer_mod[i-1] = key_buffer_mod[i];
    key_buffer_p--;
    return key_buffer_p;
    }


//#ifdef qqqqq
void SysTick_Handler(void)
{
  uint8_t HID_Buffer[10];
    HAL_IncTick();
  
  get_kbd_state(kbd_cols,&keys,&mods);
  kbd_cols++;
  if (kbd_cols==4) kbd_cols = 0;
  LED_R_OFF;
  if (key_m_cnt>0) key_m_cnt--;
  else	key_m_p = 0;
  if ((keys_last==0)&(keys!=0))
      {
      LED_R_ON;
      if ((mods==0)|(mods==0x08))
	  {
	  if (((keys>9)&(mods==0))|((keys>10)&(mods==0x08)))
	      {
	      if ((keys==10)&(mods==0x00)) key_buffer_put(KEY_DELETE,0);
	      if ((keys==10)&(mods==0x80)) {}					//never to be assigned
	      if ((keys==11)&(mods==0x00)) key_buffer_put(KEY_BACKSPACE,0);
	      if ((keys==11)&(mods==0x08)) key_buffer_put(KEY_ESC,0);
	      if ((keys==12)&(mods==0x00)) key_buffer_put(KEY_ENTER,0);
	      if ((keys==12)&(mods==0x08)) caps_lock++;
	      }
	  else
	      {
	      if (mods==0x00)				//assign scancodes to M9 mode, no modifier keys
		  {
		  if ((key_m_scancodes_def[(keys*5)+key_m_p]) == 0xFF) key_m_p = 0;
		  keys_out = key_m_scancodes_def[(keys*5)+key_m_p];
		  if (key_m_cnt>0)
		      {
		      if (key_m_last!=keys)
			  {
			  key_m_p = 0;
			  keys_out = key_m_scancodes_def[(keys*5)];
			  }
		      else
			  key_buffer_put(KEY_BACKSPACE,0);
		      }
		  if (caps_lock&0x01)
		      key_buffer_put(keys_out,KEY_MOD_LSHIFT);
		  else
		      key_buffer_put(keys_out,0);
		  }
	      else					//assign scancodes to M9 mode, symbols
		  {
		  if ((key_m_scancodes_ex[(keys*5)+key_m_p]) == 0xFF) key_m_p = 0;
		  keys_out = key_m_scancodes_ex[(keys*5)+key_m_p];
		  mod_out = key_m_scancodes_ex_mod[(keys*5)+key_m_p];
		  if (key_m_cnt>0)
		      {
		      if (key_m_last!=keys)
			  {
			  key_m_p = 0;
			  keys_out = key_m_scancodes_ex[(keys*5)];
			  mod_out = key_m_scancodes_ex_mod[(keys*5)];
			  }
		      else
			  key_buffer_put(KEY_BACKSPACE,0);
		      }
		  key_buffer_put(keys_out,mod_out);
		  }
	      key_m_cnt = 800;
	      key_m_p++;
	      key_m_last = keys;
	      }
	  }
      else
	  {
	  if (mods==0x01)
	      {
	      if (keys==1) key_buffer_put(KEY_TAB,0);
	      if (keys==2) key_buffer_put(KEY_UP,0);
	      if (keys==3) key_buffer_put(KEY_PAGEUP,0);
	      if (keys==4) key_buffer_put(KEY_LEFT,0);
	      if (keys==5) key_buffer_put(KEY_DOWN,0);
	      if (keys==6) key_buffer_put(KEY_RIGHT,0);
	      if (keys==7) key_buffer_put(KEY_HOME,0);
	      if (keys==8) key_buffer_put(KEY_END,0);
	      if (keys==9) key_buffer_put(KEY_PAGEDOWN,0);
	      if (keys==10) key_buffer_put(KEY_F11,0);
	      }
	  if (mods==0x02)
	      {
	      if (keys==1) key_buffer_put(KEY_1,0);
	      if (keys==2) key_buffer_put(KEY_2,0);
	      if (keys==3) key_buffer_put(KEY_3,0);
	      if (keys==4) key_buffer_put(KEY_4,0);
	      if (keys==5) key_buffer_put(KEY_5,0);
	      if (keys==6) key_buffer_put(KEY_6,0);
	      if (keys==7) key_buffer_put(KEY_7,0);
	      if (keys==8) key_buffer_put(KEY_8,0);
	      if (keys==9) key_buffer_put(KEY_9,0);
	      if (keys==10) key_buffer_put(KEY_0,0);
	      }
	  if (mods==0x04)
	      {

	      }
	  if (mods==0x09)
	      {
	      if (keys==2) key_buffer_put(KEY_Y,0);
	      if (keys==3) key_buffer_put(KEY_N,0);
	      if (keys==5) key_buffer_put(KEY_X,KEY_MOD_LCTRL);
	      if (keys==6) key_buffer_put(KEY_Z,KEY_MOD_LCTRL);
	      if (caps_lock&0x01)
		  {
		  if (keys==8) key_buffer_put(KEY_C,KEY_MOD_LCTRL|KEY_MOD_LSHIFT);
		  if (keys==9) key_buffer_put(KEY_V,KEY_MOD_LCTRL|KEY_MOD_LSHIFT);
		  }
	      else
		  {
		  if (keys==8) key_buffer_put(KEY_C,KEY_MOD_LCTRL);
		  if (keys==9) key_buffer_put(KEY_V,KEY_MOD_LCTRL);
		  }

	      }
	  }
      }

  keys_last = keys;

  get_mouse_speed(&speedx,&speedy);
  mouse_k = mouse_button_state();
/*
  if (get_hhid_state(&USBD_Device)==1) LED_Y_ON;
  else LED_Y_OFF;
*/
  if (caps_lock&0x01)
      {
      if (key_m_cnt>0) LED_Y_OFF;
      else LED_Y_ON;
      }
  else
      {
      if (key_m_cnt>0) LED_Y_ON;
      else LED_Y_OFF;
      }
  counter++;
  if (counter == 1)
  {
      m_tmr++;
      if (m_tmr>2)
	  {
	  HID_Buffer[0] = 1;
	  HID_Buffer[1] = mouse_k;
	  HID_Buffer[2] = speedx;
	  HID_Buffer[3] = speedy;
	  m_tmr = 0;
	  }
      else
	  {
	  HID_Buffer[0] = 1;
	  HID_Buffer[1] = mouse_k;
	  HID_Buffer[2] = 0;
	  HID_Buffer[3] = 0;
	  }
      /*
	if ((speedx==0)&(speedy==0)&(mouse_k==0))
	    {}
	else*/
      USBD_HID_SendReport(&USBD_Device, HID_Buffer, 4);
  }
  if (counter == 5)
  {  
	HID_Buffer[0] = 2;
	HID_Buffer[1] = 0;
	HID_Buffer[2] = 0;
	HID_Buffer[3] = 0;
	HID_Buffer[4] = 0;
	HID_Buffer[5] = 0;
	HID_Buffer[6] = 0;
	HID_Buffer[7] = 0;
	HID_Buffer[8] = 0;
	if (key_buffer_len()>0)
	    {
	    HID_Buffer[3] = key_buffer_get(&mod_out);
	    HID_Buffer[1] = mod_out;
	    }
	if (get_hhid_state(&USBD_Device) == 0)
	    {
	    if (key_buffer_len()>0) key_buffer_dec();
	    USBD_HID_SendReport(&USBD_Device, HID_Buffer, 9);
	    }
  }
  if (counter == 20) counter = 0;

}

//#endif

/*
void SysTick_Handler(void)
{
  uint8_t HID_Buffer[10];
    HAL_IncTick();
  counter++;

  if (counter == 1400)
  {
	HID_Buffer[0] = 2;
	if (mod_out&0x01)
	    HID_Buffer[1] = 0;
	else
	    HID_Buffer[1] = 2;
	mod_out++;
	HID_Buffer[2] = 0;
	HID_Buffer[3] = 0;
	HID_Buffer[4] = 0x04;
	HID_Buffer[5] = 0;
	HID_Buffer[6] = 0;
	HID_Buffer[7] = 0;
	HID_Buffer[8] = 0;
	USBD_HID_SendReport(&USBD_Device, HID_Buffer, 9);

  }
  if (counter == 1500)
  {
	HID_Buffer[0] = 2;
	HID_Buffer[1] = 2;
	HID_Buffer[2] = 0;
	HID_Buffer[3] = 0;
	HID_Buffer[4] = 0x00;
	HID_Buffer[5] = 0;
	HID_Buffer[6] = 0;
	HID_Buffer[7] = 0;
	HID_Buffer[8] = 0;
	USBD_HID_SendReport(&USBD_Device, HID_Buffer, 9);
	counter = 0;
  }

}
*/
void USB_IRQHandler(void)
{
  HAL_PCD_IRQHandler(&hpcd);
}



/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
