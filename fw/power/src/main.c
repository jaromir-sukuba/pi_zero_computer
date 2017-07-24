#include "stm32l0xx.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

uint8_t tmp,btn_cnt,state,state_change;

void dly_ms (void);
void dly_nms (uint16_t num);

#define	LED_G_MASK	0x08
#define	LED_O_MASK	0x10

#define	OUT_5VM_MASK	0x01
#define	OUT_5VU_MASK	0x02
#define	OUT_3V_MASK	0x04

#define	OUT_5VM_ON	GPIOA->BSRR = GPIOA->BSRR | (OUT_5VM_MASK<<0)
#define	OUT_5VM_OFF	GPIOA->BSRR = GPIOA->BSRR | (OUT_5VM_MASK<<16)
#define	OUT_5VU_ON	GPIOA->BSRR = GPIOA->BSRR | (OUT_5VU_MASK<<0)
#define	OUT_5VU_OFF	GPIOA->BSRR = GPIOA->BSRR | (OUT_5VU_MASK<<16)
#define	OUT_3V_ON	GPIOA->BSRR = GPIOA->BSRR | (OUT_3V_MASK<<0)
#define	OUT_3V_OFF	GPIOA->BSRR = GPIOA->BSRR | (OUT_3V_MASK<<16)

#define	LED_G_ON	GPIOA->BSRR = GPIOA->BSRR | (LED_G_MASK<<0)
#define	LED_G_OFF	GPIOA->BSRR = GPIOA->BSRR | (LED_G_MASK<<16)
#define	LED_O_ON	GPIOA->BSRR = GPIOA->BSRR | (LED_O_MASK<<0)
#define	LED_O_OFF	GPIOA->BSRR = GPIOA->BSRR | (LED_O_MASK<<16)

#define	BTN_STATE	(!(GPIOC->IDR&0x8000))

#define	CHG_STATE	(!(GPIOC->IDR&0x4000))


void power_on (void);
void power_off (void);
void adc_init (void);
void adc_uninit (void);
uint16_t adc_get (uint8_t channel);
uint16_t get_volt_in (void);
uint16_t get_volt_batt (void);
void usart_init (void);
void usart_uninit (void);
void usart_send_byte(uint8_t data);
void usart_send_str (uint8_t * data);

void EXTI15_4_IRQHandler(void)
    {
//	LED_O_ON;
	EXTI->PR = EXTI->PR | (1<<15);
    }


uint8_t main_state,btn_cnt,volt_bat_cnt;
uint16_t volt_bat,volt_in;

volatile uint8_t rx_msg[50],rx_msg_ptr,rx_flag;
uint8_t tx_msg[50];


#define	MAIN_STATE_SHUTDOWN	1
#define	MAIN_STATE_RUN		2
#define	MAIN_STATE_RUN_ENTRY	3
#define	MAIN_STATE_RUN_ENTRY2	4
#define MAIN_STATE_SHDN_ENTRY	5

#define	ADC_BATT		5
#define	ADC_IN			7
#define	ADC_VREF		3300
#define	ADC_BATT_R1		100
#define	ADC_BATT_R2		27
#define	ADC_IN_R1		560
#define	ADC_IN_R2		100

uint16_t ledg_cnt, ledg_state;


int main (void)
    {
    RCC->IOPENR = RCC_IOPENR_GPIOAEN | RCC_IOPENR_GPIOBEN | RCC_IOPENR_GPIOCEN;
    RCC->APB2ENR = RCC->APB2ENR | RCC_APB2ENR_SYSCFGEN;
    RCC->APB1ENR = RCC->APB1ENR | RCC_APB1ENR_PWREN;
//    RCC->CFGR = RCC->CFGR | (0x09<<4);
    RCC->CR = RCC->CR & ~(RCC_CR_HSEON);
    GPIOA->MODER = GPIOA->MODER & 0xFFFF0000;
    GPIOA->MODER = GPIOA->MODER | (0b01<<(2*0)) | (0b01<<(2*1)) | (0b01<<(2*2)) | (0b01<<(2*3)) | (0b01<<(2*4));
    GPIOA->MODER = GPIOA->MODER | (0b11<<(2*5));
    GPIOA->MODER = GPIOA->MODER | (0b11<<(2*7));
    GPIOC->PUPDR = GPIOC->PUPDR | (0b01<<(2*15)) | (0b01<<(2*14));
    GPIOC->MODER = GPIOC->MODER & (0x3FFFFFFF);
    GPIOC->MODER = GPIOC->MODER & (0xCFFFFFFF);

    SYSCFG->EXTICR[3] = (0b0010<<12);
    EXTI->FTSR = (1<<15);
    EXTI->IMR = (1<<15);
    NVIC_SetPriority(EXTI4_15_IRQn,1);
    NVIC_EnableIRQ(EXTI4_15_IRQn);
    main_state=MAIN_STATE_RUN_ENTRY2;
    while (1)
	{
	dly_nms(10);
	if (rx_flag)
	    {
	    rx_flag = 0;
	    if (strncmp(rx_msg,"gtv",3)==0)
		{
		usart_send_str("gtv: ");
		usart_send_str("pz_c ver 0.9");
		}
	    if (strncmp(rx_msg,"gvb",3)==0)
		{
		usart_send_str("gvb: ");
		itoa(volt_bat,tx_msg,10);
		usart_send_str(tx_msg);
		}
	    if (strncmp(rx_msg,"gve",3)==0)
		{
		usart_send_str("gve: ");
		itoa(volt_in,tx_msg,10);
		usart_send_str(tx_msg);
		}
	    if (strncmp(rx_msg,"gcs",3)==0)
		{
		usart_send_str("gcs: ");
		if (CHG_STATE)
		    usart_send_str("1");
		else
		    usart_send_str("0");
		}
	    usart_send_str("\r\n");
	    }
	if (main_state==MAIN_STATE_SHUTDOWN)
	    {
	    power_off();
	    main_state=MAIN_STATE_RUN_ENTRY;
	    btn_cnt = 0;
	    }
	if (main_state==MAIN_STATE_RUN_ENTRY)
	    {
		LED_G_ON;
	    if (BTN_STATE)
			{
			btn_cnt++;
			if (btn_cnt>50)
				{
				main_state=MAIN_STATE_RUN_ENTRY2;
				}
			}
	    else
			{
	    	btn_cnt=0;
	    	main_state = MAIN_STATE_SHUTDOWN;
			}
	    }
	 if (main_state==MAIN_STATE_RUN_ENTRY2)
	    {
		LED_O_ON;
	    if (!(BTN_STATE))
		{
	    LED_O_OFF;
		power_on();
		main_state=MAIN_STATE_RUN;
		}
	    }
	if (main_state==MAIN_STATE_SHDN_ENTRY)
	    {
	    LED_G_OFF;
	    if (!(BTN_STATE))
		{
		dly_nms(250);
		main_state = MAIN_STATE_SHUTDOWN;
		}
	    }
	if (main_state==MAIN_STATE_RUN)
	    {
	    volt_bat = get_volt_batt();
	    dly_nms(1);
	    volt_in = get_volt_in();
	    dly_nms(1);
	    if (CHG_STATE)
		LED_O_ON;
	    else
		LED_O_OFF;
	    if (BTN_STATE)
			{
			btn_cnt++;
			if (btn_cnt>100)
				{
				main_state=MAIN_STATE_SHDN_ENTRY;
				}
			}
	    else
			{
	    	btn_cnt=0;
			}
	    if (volt_bat<6500)
	    	{
	    	volt_bat_cnt++;
	    	if (volt_bat_cnt>40)
	    		main_state=MAIN_STATE_SHDN_ENTRY;
	    	}
	    else
	    	volt_bat_cnt = 0;
	    if (volt_bat < 7000)
	    	{
	    	if (ledg_state==0) ledg_state = 1;
	    	}
	    else
	    	{
	    	if (ledg_state>0) ledg_state = 0;
	    	}
		ledg_cnt++;
		if (ledg_state==1)
			{
			if (ledg_cnt>10)
				{
				LED_G_ON;
				ledg_cnt=0;
				ledg_state = 2;
				}
			}
		if (ledg_state==2)
			{
			if (ledg_cnt>190)
				{
				LED_G_OFF;
				ledg_cnt=0;
				ledg_state = 1;
				}
			}
		if (ledg_state==0) LED_G_ON;


	    }
	}
    }

void USART2_IRQHandler(void)
{
uint8_t b;
b = USART2->RDR;
if (b>=' ')
    {
    rx_msg[rx_msg_ptr++] = b;
    if (rx_msg_ptr>50) rx_msg_ptr = 0;
    }
else
    {
    if (rx_msg_ptr>2)
	{
	rx_msg[rx_msg_ptr++] = 0;
	rx_msg_ptr = 0;
	rx_flag = 1;
	}
    else
	rx_msg_ptr = 0;
    }
}


void usart_init (void)
    {
    RCC->APB1ENR = RCC->APB1ENR | RCC_APB1ENR_USART2EN;
    GPIOA->MODER = GPIOA->MODER & ~((0b11<<(2*9)) | (0b11<<(2*10)));
    GPIOA->MODER = GPIOA->MODER | (0b10<<(2*9)) | (0b10<<(2*10));
    USART2->BRR = 2100000 / 9600;
    USART2->CR1 = USART_CR1_UE | USART_CR1_RE | USART_CR1_TE;
    GPIOA->AFR[1] = GPIOA->AFR[1] | (0b0100<<(4*1)) | (0b0100<<(4*2));
    USART2->CR1 = USART2->CR1 | USART_CR1_RXNEIE ;
    NVIC_EnableIRQ(USART2_IRQn);
    }

void usart_uninit (void)
    {

    USART2->CR1 = 0;

    }

void usart_send_byte(uint8_t data)
    {

    while (((USART2->ISR)&USART_ISR_TXE)==0);
    USART2->TDR = data;

    }


void usart_send_str (uint8_t * data)
    {
    while (*data) usart_send_byte(*data++);
    }


uint16_t get_volt_in (void)
    {
    uint32_t tmp;
    tmp = adc_get(ADC_IN);

    tmp = tmp * ADC_VREF;
    tmp = tmp * (ADC_IN_R1+ADC_IN_R2);
    tmp = tmp / ADC_IN_R2;
    tmp = tmp / 4096;
    return tmp;
    }

uint16_t get_volt_batt (void)
    {
    uint32_t tmp;
    tmp = adc_get(ADC_BATT);
    tmp = tmp * ADC_VREF;
    tmp = tmp * (ADC_BATT_R1+ADC_BATT_R2);
    tmp = tmp / ADC_BATT_R2;
    tmp = tmp / 4096;
    return tmp;
    }


void adc_init (void)
{
RCC->APB2ENR = RCC->APB2ENR | RCC_APB2ENR_ADC1EN;
ADC1->CFGR1 = 0x00;
ADC1->CFGR2 = 0x40000000;
ADC1->SMPR = 0x07;
ADC1->CR = ADC_CR_ADCAL;
while (ADC1->CR & ADC_CR_ADCAL);
ADC1->CR = 0x01;
}

void adc_uninit (void)
    {
    ADC1->CR = ADC_CR_ADDIS;
    while (ADC1->CR&ADC_CR_ADDIS);
    RCC->APB2ENR = RCC->APB2ENR & (~RCC_APB2ENR_ADC1EN);
    }

uint16_t adc_get (uint8_t channel)
{
ADC1->CHSELR = (1<<channel);
ADC1->CR = ADC1->CR | 0x04;
while (ADC1->CR & 0x04);
return ADC1->DR;
}

void power_on (void)
    {
	LED_G_ON;
	OUT_5VM_ON;
	OUT_5VU_ON;
	OUT_3V_ON;
	adc_init();
	usart_init();
    }



void power_off (void)
    {
    adc_uninit();
    usart_uninit();
    LED_G_OFF;
    LED_O_OFF;
    OUT_5VM_OFF;
    OUT_5VU_OFF;
    OUT_3V_OFF;
//    PWR->CR = PWR->CR | PWR_CR_LPSDSR | PWR_CR_PDDS;

   PWR->CR = PWR->CR | PWR_CR_LPSDSR;
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    __WFI();
    }


void dly_ms (void)
    {
    volatile uint32_t cnt;
    for (cnt=0;cnt<190;cnt++);
    }


void dly_nms (uint16_t num)
    {
    uint16_t i;
    for (i=0;i<num;i++) dly_ms();

    }
