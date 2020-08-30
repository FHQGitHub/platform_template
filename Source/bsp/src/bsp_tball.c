#include "bsp_tball.h"

void tball_gpio_init()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(TBALL_GPIO_RCC, ENABLE); 
	
	GPIO_InitStructure.GPIO_Pin = TBALL_RHT | TBALL_LFT | TBALL_UP | TBALL_DWN | TBALL_BTN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;	
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 
	GPIO_Init(GPIOE, &GPIO_InitStructure); 
}

ev_sta_t tball_input_event()
{
	static ev_sta_t tball_sta[2][5] = {0};
	ev_sta_t event = 0;
	int i = 0;
	
	tball_sta[0][0] = tball_gpio_read(TBALL_RHT);
	tball_sta[0][1] = tball_gpio_read(TBALL_LFT);
	tball_sta[0][2] = tball_gpio_read(TBALL_UP);
	tball_sta[0][3] = tball_gpio_read(TBALL_DWN);
	tball_sta[0][4] = tball_gpio_read(TBALL_BTN);
	
	if(tball_sta[0][0] != tball_sta[1][0])	event |= EV_RHT;
	if(tball_sta[0][1] != tball_sta[1][1])	event |= EV_LFT;
	if(tball_sta[0][2] != tball_sta[1][2])	event |= EV_UP;
	if(tball_sta[0][3] != tball_sta[1][3])	event |= EV_DWN;
	if(tball_sta[0][4] != tball_sta[1][4])	event |= EV_BTN;
	
	for(i = 0; i < 5; i++)
		tball_sta[1][i] = tball_sta[0][i];
	
	return event;
}

