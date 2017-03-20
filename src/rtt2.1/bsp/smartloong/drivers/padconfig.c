/*
 * File      : padconfig.c
 * http://www.openloongson.org/forum.php
 * description:
 * pad multiplex for smart loong(ls1c)
 * Change Logs:
 * Date                Author       Notes
 * 2015-07-16     chinesebear  first version
 * 2017-03-20     chinesebear  simplify multi reg setting
 */
#include <rthw.h>
#include <rtthread.h>
#include "padconfig.h"


/*--------------------------------------
|	Function Name:
|		ls1x_pad_multi_setting
|	Description:
|		pad multiplex fucntion setting
|	Parameters:
|		padNo - GPIO number
|		type - multiplex function type
|	Returns:
|		void
+-------------------------------------*/

static void ls1x_pad_multi_setting(rt_uint32_t padNo,rt_uint32_t type)
{
	rt_uint32_t base0,base1;
	rt_uint32_t padNumber;
	if(PAD_MUTI_MAX < type || 127 < padNo)return ;
	/*
		prioperty of multiplex function:
		CBUS_FIRST > CBUS_SECOND > CBUS_THIRD >
		CBUS_FOURTH > CBUS_FIFTH > GPIO_CFG > default
	*/
	if(0< padNo && padNo <= 31)
	{
		base0 = GPIO_BASE0;
		base1 = PAD_MUTI_BASE0;
		padNumber = padNo - 0;
	}
	else if(32< padNo && padNo <= 63)
	{
		base0 = GPIO_BASE1;
		base1 = PAD_MUTI_BASE1;
		padNumber = padNo - 32;
	}
	else if(64< padNo && padNo <= 95)
	{
		base0 = GPIO_BASE2;
		base1 = PAD_MUTI_BASE2;
		padNumber = padNo - 64;
	}
	else if(96< padNo && padNo <= 127)
	{
		base0 = GPIO_BASE3;
		base1 = PAD_MUTI_BASE3;
		padNumber = padNo - 96;
	}
	else
	{
		rt_kprintf("padNo error\n");
	}
	//clear all muti function 
	CBUS_FIRST(base1) &= (1<<padNumber);
	CBUS_SECOND(base1) &= (1<<padNumber);
	CBUS_THIRD(base1) &= (1<<padNumber);
	CBUS_FOURTH(base1) &= (1<<padNumber);
	CBUS_FIFTH(base1) &= (1<<padNumber);
	GPIO_CFG(base0) &= (1<<padNumber);
	//multi function type setting
	switch(type)
	{
		case PAD_MUTI_FIRST:
			CBUS_FIRST(base1) |= (1<<padNumber);
			break;
		case PAD_MUTI_SECOND:
			CBUS_SECOND(base1) |= (1<<padNumber);
			break;
		case PAD_MUTI_THIRD:
			CBUS_THIRD(base1) |= (1<<padNumber);
			break;
		case PAD_MUTI_FOURTH:
			CBUS_FOURTH(base1) |= (1<<padNumber);
			break;
		case PAD_MUTI_FIFTH:
			CBUS_FIFTH(base1) |= (1<<padNumber);
			break;
		case PAD_MUTI_GPIO:
			GPIO_CFG(base1) |= (1<<padNumber);
			break;
		case PAD_MUTI_DEFAULT:
			//do nothing
			break;
		default:
			break;
	}
}

void rt_pad_multiplex_init(void)
{

	/***************0~31*******************/
	/*GPIO 0~1*/
	ls1x_pad_multi_setting(0,PAD_MUTI_GPIO);
	ls1x_pad_multi_setting(1,PAD_MUTI_GPIO);
	//GPIO_CFG(GPIO_BASE0) |= (1<<0);
	//GPIO_CFG(GPIO_BASE0) |= (1<<1);
	/*GPIO 2~3 uart1*/
	ls1x_pad_multi_setting(2,PAD_MUTI_FOURTH);
	ls1x_pad_multi_setting(3,PAD_MUTI_FOURTH);
	//CBUS_FOURTH(GPIO_BASE0) |=(1<<2);
	//CBUS_FOURTH(GPIO_BASE0) |=(1<<3);
	/*GPIO 4~5*/
	ls1x_pad_multi_setting(4,PAD_MUTI_GPIO);
	ls1x_pad_multi_setting(5,PAD_MUTI_GPIO);
	//GPIO_CFG(GPIO_BASE0) |= (1<<4);
	//GPIO_CFG(GPIO_BASE0) |= (1<<5);

	/**************32~63******************/
	/*GPIO 36~37 uart2*/
	//CBUS_FIFTH(PAD_MUTI_BASE1) |= (1<<4);//36-32=4
	//CBUS_FIFTH(PAD_MUTI_BASE1) |= (1<<5);//37-32=5
	/*GPIO 46~47 pwm2& pwm3*/
	ls1x_pad_multi_setting(46,PAD_MUTI_FOURTH);
	ls1x_pad_multi_setting(47,PAD_MUTI_FOURTH);
	//CBUS_FOURTH(PAD_MUTI_BASE1) |= (1<<14);
	//CBUS_FOURTH(PAD_MUTI_BASE1) |= (1<<15);
	/*GPIO 48~53*/
	ls1x_pad_multi_setting(48,PAD_MUTI_GPIO);
	ls1x_pad_multi_setting(49,PAD_MUTI_GPIO);
	ls1x_pad_multi_setting(50,PAD_MUTI_GPIO);
	ls1x_pad_multi_setting(51,PAD_MUTI_GPIO);
	ls1x_pad_multi_setting(52,PAD_MUTI_GPIO);
	ls1x_pad_multi_setting(53,PAD_MUTI_GPIO);
	//GPIO_CFG(GPIO_BASE1) |= (1<<16);//48-32=15
	//GPIO_CFG(GPIO_BASE1) |= (1<<17);
	//GPIO_CFG(GPIO_BASE1) |= (1<<18);
	//GPIO_CFG(GPIO_BASE1) |= (1<<19);
	//GPIO_CFG(GPIO_BASE1) |= (1<<20);
	//GPIO_CFG(GPIO_BASE1) |= (1<<21);
	/*GPIO 54~55 I2C1*/
	//CBUS_FOURTH(PAD_MUTI_BASE1) |= (1<<22);//54-32=22
	//CBUS_FOURTH(PAD_MUTI_BASE1) |= (1<<23);
	/*GPIO 56~57 UART7*/
	//CBUS_FIFTH(PAD_MUTI_BASE1) |= (1<<24);//56-32=24
	//CBUS_FIFTH(PAD_MUTI_BASE1) |= (1<<25);


	/**************64~95*****************/
	/*GPIO79 cs2*/
	/*GPIO89 cs3*/
	/*GPIO90 cs0*/
	/*GPIO91 cs1*/
	ls1x_pad_multi_setting(79,PAD_MUTI_DEFAULT);
	ls1x_pad_multi_setting(89,PAD_MUTI_DEFAULT);
	ls1x_pad_multi_setting(90,PAD_MUTI_DEFAULT);
	ls1x_pad_multi_setting(91,PAD_MUTI_DEFAULT);
	/*GPIO 87~90*/
	ls1x_pad_multi_setting(87,PAD_MUTI_GPIO);
	ls1x_pad_multi_setting(88,PAD_MUTI_GPIO);
	ls1x_pad_multi_setting(89,PAD_MUTI_GPIO);
	ls1x_pad_multi_setting(90,PAD_MUTI_GPIO);
	//GPIO_CFG(GPIO_BASE2) |= (1<<23);//87-64=23
	//GPIO_CFG(GPIO_BASE2) |= (1<<24);
	//GPIO_CFG(GPIO_BASE2) |= (1<<25);
	//GPIO_CFG(GPIO_BASE2) |= (1<<26);
	/*GPIO 91*/
	ls1x_pad_multi_setting(91,PAD_MUTI_GPIO);
	//GPIO_CFG(GPIO_BASE2) |= (1<<27);
	
	
}

void rt_pad_show(void)
{
	rt_kprintf("Pad Multplex Show:\n");
	rt_kprintf("**Pad00~31**\n");
	rt_kprintf("1: 0x%08X\n",CBUS_FIRST(PAD_MUTI_BASE0));
	rt_kprintf("2: 0x%08X\n",CBUS_SECOND(PAD_MUTI_BASE0));
	rt_kprintf("3: 0x%08X\n",CBUS_THIRD(PAD_MUTI_BASE0));
	rt_kprintf("4: 0x%08X\n",CBUS_FOURTH(PAD_MUTI_BASE0));
	rt_kprintf("5: 0x%08X\n",CBUS_FIFTH(PAD_MUTI_BASE0));
	rt_kprintf("G: 0x%08X\n",GPIO_CFG(GPIO_BASE0));
	rt_kprintf("**Pad32~63**\n");
	rt_kprintf("1: 0x%08X\n",CBUS_FIRST(PAD_MUTI_BASE1));
	rt_kprintf("2: 0x%08X\n",CBUS_SECOND(PAD_MUTI_BASE1));
	rt_kprintf("3: 0x%08X\n",CBUS_THIRD(PAD_MUTI_BASE1));
	rt_kprintf("4: 0x%08X\n",CBUS_FOURTH(PAD_MUTI_BASE1));
	rt_kprintf("5: 0x%08X\n",CBUS_FIFTH(PAD_MUTI_BASE1));
	rt_kprintf("G: 0x%08X\n",GPIO_CFG(GPIO_BASE1));
	rt_kprintf("**Pad64~95**\n");
	rt_kprintf("1: 0x%08X\n",CBUS_FIRST(PAD_MUTI_BASE2));
	rt_kprintf("2: 0x%08X\n",CBUS_SECOND(PAD_MUTI_BASE2));
	rt_kprintf("3: 0x%08X\n",CBUS_THIRD(PAD_MUTI_BASE2));
	rt_kprintf("4: 0x%08X\n",CBUS_FOURTH(PAD_MUTI_BASE2));
	rt_kprintf("5: 0x%08X\n",CBUS_FIFTH(PAD_MUTI_BASE2));
	rt_kprintf("G: 0x%08X\n",GPIO_CFG(GPIO_BASE2));
	rt_kprintf("**Pad96~127**\n");
	rt_kprintf("1: 0x%08X\n",CBUS_FIRST(PAD_MUTI_BASE3));
	rt_kprintf("2: 0x%08X\n",CBUS_SECOND(PAD_MUTI_BASE3));
	rt_kprintf("3: 0x%08X\n",CBUS_THIRD(PAD_MUTI_BASE3));
	rt_kprintf("4: 0x%08X\n",CBUS_FOURTH(PAD_MUTI_BASE3));
	rt_kprintf("5: 0x%08X\n",CBUS_FIFTH(PAD_MUTI_BASE3));
	rt_kprintf("G: 0x%08X\n",GPIO_CFG(GPIO_BASE3));

		
}





