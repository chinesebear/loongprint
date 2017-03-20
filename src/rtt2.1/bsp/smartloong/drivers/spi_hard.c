/*
********************************************************************************
*	Copyright (c) 2017 loongprint.com
*   File Name:
*       spi_hard.c
*   Author:
*       chinesebear
*   Version:
*       V1.0
*   Description:
*       spi driver for smartloong
*
********************************************************************************
*/

#define _SPI_HARD_C

/*-----------------------------------------------------------------------------
|   Includes
+----------------------------------------------------------------------------*/
/** Local includes */
#include <rthw.h>
#include <rtthread.h>

#include <drivers/spi.h>
#include "spi_hard.h"
#include <ls1c.h>

/*-----------------------------------------------------------------------------
|   Macros
+----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
|   Enumerations
+----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
|   Typedefs
+----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
|   Variables
+----------------------------------------------------------------------------*/

struct rt_spi_bus spibus0,spibus1;
struct rt_spi_device spidev0[4],spidev1[4];
static struct rt_spi_ops ls1c_spi_ops =
{
    rt_ls1x_spi_configure,
    rt_ls1x_spi_xfer
};
struct rt_spi_configuration spicfg[8];

rt_uint32_t freq_div[][3]={
						{0,0,2},{0,1,4},{1,0,8},
						{0,2,16},{0,3,32},{1,1,64},
						{1,2,128},{1,3,256},{2,0,512},
						{2,1,1024},{2,2,2048},{2,3,4096}};//spre+spr+divval
/*-----------------------------------------------------------------------------
|   Constants
+----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
|   Functions
+----------------------------------------------------------------------------*/

/*--------------------------------------
|   Function Name:
|		ls1x_spi_wait_txe
|   Description:
|		
|   Parameters:
|		void 
|   Returns:
|		void
+-------------------------------------*/
static void ls1x_spi_wait_txe(rt_uint32_t addr)
{
		int timeout = 20000;

	while (timeout) {
		if (SPSR(addr) & 0x80) {
			break;
		}
		timeout--;
//		cpu_relax();
	}
}

/*--------------------------------------
|   Function Name:
|		ls1x_spi_wait_rxe
|   Description:
|		
|   Parameters:
|		addr 
|   Returns:
|		void
+-------------------------------------*/

static void ls1x_spi_wait_rxe(rt_uint32_t addr)
{
	rt_uint8_t ret;

	ret = SPSR(addr);
	ret = ret | 0x80;
	SPSR(addr) = ret;	/* Int Clear */

	ret = SPSR(addr);
	if (ret & 0x40) {
		SPSR(addr) = ret & 0xbf;	/* Write-Collision Clear */
	}
}

/*--------------------------------------
|   Function Name:
|		ls1x_spi_wait_rxe
|   Description:
|		
|   Parameters:
|		*spi 
|		is_active
|   Returns:
|		void
+-------------------------------------*/

static void ls1x_spi_chipselect(struct rt_spi_device *spi, int is_active)
{
	rt_uint8_t ret;
	rt_uint32_t addr = SPI_BASE0;
	ret = SFC_SOFTCS(addr);
	ret = (ret & 0xf0) | (0x01 << spi->config.reserved);
	
	if (is_active) {
		ret = ret & (~(0x10 << spi->config.reserved));
		SFC_SOFTCS(addr)=ret;
	} else {
		ret = ret | (0x10 << spi->config.reserved);
		SFC_SOFTCS(addr)=ret;
	}

}
static void ls1x_spi_freq_setting(rt_uint32_t addr, rt_uint32_t spibaud)
{
	rt_uint32_t div_val =  SYS_DEV_CLK / 32 / spibaud;
	rt_uint32_t value1,value2,midval;
	rt_uint8_t regval;
	int i;
	for(i = 0;i <12 ;i++)
	{
		if(i <11 )
		{
			value1 = freq_div[i][2];
			value2 = freq_div[i+1][2];
			if(value1 < div_val && div_val <= value2)
			{
				midval = (value1+ value2)/2;
				if(div_val > midval )
				{
					div_val = value2;
				}
				break;
			}
		}
		else
		{
			div_val = freq_div[i][2];
		}
		
	}
	
	regval = SPER(addr) ;
	regval &= 0xFC;
	regval |= freq_div[i][0];
	SPER(addr)= regval;
	
	regval = SPCR(addr) ;
	regval &= 0xFC;
	regval |= freq_div[i][1];
	SPCR(addr)= regval;
	rt_kprintf("spi baud,spre=%d,spr=%d,divval=%d,baud=%d\n",
		freq_div[i][0],freq_div[i][1],div_val,SYS_DEV_CLK / 32 /div_val);
}

/*--------------------------------------
|   Function Name:
|		ls1c_spi_configure
|   Description:
|		
|   Parameters:
|		param1 - 
|   Returns:
|		value = 0 - OK
|		value = (-1) - normal error
+-------------------------------------*/
rt_err_t rt_ls1x_spi_configure(struct rt_spi_device * device,struct rt_spi_configuration * configuration)
{
	/*
		1. 模块初始化
		 停止 SPI 控制器工作，对控制寄存器 spcr 的 spe 位写 0
		 重置状态寄存器 spsr，对寄存器写入 8'b1100_0000
		 设置外部寄存器 sper，包括中断申请条件 sper[7:6]和分频系数
		sper[1:0]，具体参考寄存器说明
		 配置 SPI 时序，包括 spcr 的 cpol、 cpha 和 sper 的 mode 位。 mode 为
		1 时是标准 SPI 实现，为 0 时为兼容模式。
		 配置中断使能， spcr 的 spie 位
		 启动 SPI 控制器，对控制寄存器 spcr 的 spe 位写 1

		2. 模块的发送/传输操作
		 往数据传输寄存器写入数据
		 传输完成后从数据传输寄存器读出数据。由于发送和接收同时进行，
		即使 SPI 从设备没有发送有效数据也必须进行读出操作。

		3. 中断处理
		 接收到中断申请
		 读状态寄存器 spsr的值，若 spsr[2]为 1则表示数据发送完成，若 spsr[0]
		为 1 则表示已经接收数据
		 读或写数据传输寄存器
		 往状态寄存器 spsr 的 spif 位写 1，清除控制器的中断申请
	*/
	rt_uint32_t addr = SPI_BASE0;
	SPCR(addr) &= ~(1<<6);//stop spi device
	SPSR(addr) = 0xC0;// clear status
	SPER(addr) = 0x00;
	ls1x_spi_freq_setting(addr,configuration->max_hz);//freq setting
	SPCR(addr) &= ~(1<<3);//cpol
	SPCR(addr) &= ~(1<<2);//cpha
	SPER(addr) |= (1<<2);//mode
	SPCR(addr) |= (1<<7);//spie
	SPCR(addr) |= (1<<6);//start spi device
}
/*--------------------------------------
|   Function Name:
|		ls1c_spi_xfer
|   Description:
|		
|   Parameters:
|		param1 - 
|   Returns:
|		value = 
+-------------------------------------*/

rt_uint32_t rt_ls1x_spi_xfer(struct rt_spi_device *device, struct rt_spi_message* message)
{
	const rt_uint8_t* txp = message->send_buf;
	rt_uint8_t* rxp = message->recv_buf;
	unsigned int i;
	rt_uint32_t addr;
	rt_uint8_t dummy;
	ls1x_spi_chipselect(device,SPI_CS_LOW);
	addr = SPI_BASE0;
	if (txp && rxp) {
		for (i = 0; i < message->length; i += 1) {
			FIFO(addr)= *txp++;
			ls1x_spi_wait_txe(addr);
			*rxp++ = FIFO(addr);
			ls1x_spi_wait_rxe(addr);
		}
	} else if (rxp) {
		for (i = 0; i < message->length; i += 1) {
			FIFO(addr)=0;
			ls1x_spi_wait_txe(addr);
			*rxp++ = FIFO(addr);
			ls1x_spi_wait_rxe(addr);
		}
	} else if (txp) {
		for (i = 0; i < message->length; i += 1) {
			FIFO(addr)=*txp++;
			ls1x_spi_wait_txe(addr);
			dummy = FIFO(addr);
			ls1x_spi_wait_rxe(addr);
		}
	} else {
		for (i = 0; i < message->length; i += 1) {
			FIFO(addr) = 0;
			ls1x_spi_wait_txe(addr);
			dummy = FIFO(addr);
			ls1x_spi_wait_rxe(addr);
		}
	}
	ls1x_spi_chipselect(device,SPI_CS_HIGH);
	return i;
}

/*--------------------------------------
|   Function Name:
|		ls1c_spi_xfer
|   Description:
|		
|   Parameters:
|		void 
|   Returns:
|		void
+-------------------------------------*/
void rt_hw_spi_init(void)
{
	struct rt_spi_ops* pOps;
	struct rt_spi_bus* pBus;
	pBus= &spibus0;
	pOps = &ls1c_spi_ops;
	spicfg[0].data_width=8;
	spicfg[0].max_hz = 1000000;
	spicfg[0].mode=0;
	spicfg[0].reserved=0;//chip select
	spicfg[1].data_width=8;
	spicfg[1].max_hz = 1000000;
	spicfg[1].mode=0;
	spicfg[1].reserved=1;//chip select
	spicfg[2].data_width=8;
	spicfg[2].max_hz = 48;
	spicfg[2].mode=0;
	spicfg[2].reserved=2;//chip select
	spicfg[3].data_width=8;
	spicfg[3].max_hz = 1000000;
	spicfg[3].mode=0;
	spicfg[3].reserved=3;//chip select
	rt_spi_bus_register(&spibus0,"spibus0",&ls1c_spi_ops);	
	rt_spi_bus_attach_device(spidev0  ,"spidev00","spibus0",spicfg);
	rt_spi_bus_attach_device(spidev0+1,"spidev01","spibus0",spicfg+1);
	rt_spi_bus_attach_device(spidev0+2,"spidev02","spibus0",spicfg+2);
	rt_spi_bus_attach_device(spidev0+3,"spidev03","spibus0",spicfg+3);
}

