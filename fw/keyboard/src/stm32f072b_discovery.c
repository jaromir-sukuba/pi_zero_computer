#include "stm32f072b_discovery.h"

void mouse_button_Init(void)
{
    RCC->AHBENR = RCC->AHBENR | RCC_AHBENR_GPIOBEN;
}

uint8_t mouse_button_state(void)
{
    uint8_t tmp=0;
    if ((GPIOA->IDR&(1<<0))==0) tmp = tmp | 0x01;
    if ((GPIOA->IDR&(1<<1))==0) tmp = tmp | 0x04;
    if ((GPIOA->IDR&(1<<2))==0) tmp = tmp | 0x02;
    return tmp;
}


void adc_init (void)
{
	RCC->AHBENR = RCC->AHBENR | RCC_AHBENR_GPIOBEN;
	RCC->APB2ENR = RCC->APB2ENR | RCC_APB2ENR_ADCEN;
	GPIOB->MODER = GPIOB->MODER | (0b11<<(2*0)) | (0b11<<(2*1));
	ADC1->CFGR1 = 0x00;
	ADC1->CFGR2 = 0x00;
	ADC1->CR = 0x01;

}


void keyb_init (void)
{
	RCC->AHBENR = RCC->AHBENR | RCC_AHBENR_GPIOBEN;
	RCC->AHBENR = RCC->AHBENR | RCC_AHBENR_GPIOAEN;
	RCC->AHBENR = RCC->AHBENR | RCC_AHBENR_GPIOCEN;
	GPIOA->MODER = GPIOA->MODER | (0x01<<(2*3)) | (0x01<<(2*4));
	GPIOC->MODER = GPIOC->MODER | (0x01<<(2*13)) | (0x01<<(2*14)) | (0x01<<(2*15));
	GPIOB->MODER = GPIOB->MODER | (0x01<<(2*9));
	GPIOC->OTYPER = GPIOC->OTYPER | (0x01<<13) | (0x01<<14) | (0x01<<15);
	GPIOB->OTYPER = GPIOB->OTYPER | (0x01<<9) ;

}

void keyb_set_col (uint8_t col)
{
	GPIOB->BSRR = (1<<9);
	GPIOC->BSRR = (1<<13);
	GPIOC->BSRR = (1<<14);
	GPIOC->BSRR = (1<<15);
	if (col==0)	GPIOB->BRR = (1<<9);
	if (col==1)	GPIOC->BRR = (1<<13);
	if (col==2)	GPIOC->BRR = (1<<14);
	if (col==3)	GPIOC->BRR = (1<<15);

}

uint8_t keyb_get_col (void)
{
uint8_t tmp;
tmp = 0;

if ((GPIOB->IDR)&(1<<4)) tmp = tmp  | 0x01;
if ((GPIOB->IDR)&(1<<5)) tmp = tmp  | 0x02;
if ((GPIOB->IDR)&(1<<6)) tmp = tmp  | 0x04;
if ((GPIOB->IDR)&(1<<8)) tmp = tmp  | 0x08;

return tmp;
}



uint8_t key_last, mod_last, key_tmp, mod_tmp;
uint8_t get_kbd_state (uint8_t col, uint8_t * keys, uint8_t * modifiers)
    {
    uint8_t colr;
    keyb_set_col(col);
    colr = (~keyb_get_col())&0x0F;
    if (col==0)
	{
	if (colr&0x01) key_tmp = 1;
	if (colr&0x02) key_tmp = 2;
	if (colr&0x04) key_tmp = 3;
	if (colr&0x08) key_tmp = 10;
	}
    if (col==1)
	{
	if (colr&0x01) key_tmp = 4;
	if (colr&0x02) key_tmp = 5;
	if (colr&0x04) key_tmp = 6;
	if (colr&0x08) key_tmp = 11;
	}
    if (col==2)
	{
	if (colr&0x01) key_tmp = 7;
	if (colr&0x02) key_tmp = 8;
	if (colr&0x04) key_tmp = 9;
	if (colr&0x08) key_tmp = 12;
	}
    if (col==3)
	{
	mod_tmp = colr;
	key_last = key_tmp;
	mod_last = mod_tmp;
	key_tmp = 0;
	mod_tmp = 0;
	}
    *modifiers = mod_last;
    *keys = key_last;
    if (col==3) return 1;
    return 0;
    }


uint16_t adc_get (uint8_t channel)
{
ADC1->CHSELR = (1<<channel);
ADC1->CR = ADC1->CR | 0x04;
while (ADC1->CR & 0x04);
return ADC1->DR;

}


#define	J_MIDL_X	1650
#define	J_MIDH_X	2100
#define	J_MIDL_Y	1650
#define	J_MIDH_Y	2000

#define	Y_DIV		120
#define	X_DIV		120

uint8_t get_mouse_speed (int16_t * speed_x, int16_t * speed_y)
    {
    uint16_t adcresx,adcresy;
    int16_t speedx,speedy;

    adcresx = adc_get(8);
    adcresy = adc_get(9);



    speedx = 0;
    speedy = 0;
    if (adcresx>J_MIDH_X)
    {
  	  speedx = adcresx - J_MIDH_X;
  	  speedx = speedx / X_DIV;
    }
    if (adcresx<J_MIDL_X)
    {
  	  speedx = J_MIDL_X - adcresx;
  	  speedx = - speedx / X_DIV;
    }
    if (adcresy>J_MIDH_Y)
    {
  	  speedy = adcresy - J_MIDH_Y;
  	  speedy = speedy / Y_DIV;
    }
    if (adcresy<J_MIDL_Y)
    {
  	  speedy = J_MIDL_Y - adcresy;
  	  speedy = - speedy / Y_DIV;
    }
    *speed_x = speedx;
    *speed_y = speedy;
    return 0;
    }


