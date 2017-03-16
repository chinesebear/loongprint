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
    ls1c_spi_configure,
    ls1c_spi_xfer
};
struct rt_spi_configuration spicfg[8];


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
rt_err_t ls1c_spi_configure(struct rt_spi_device *device, struct rt_spi_configuration *configuration)
{
	
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

rt_uint32_t ls1c_spi_xfer(struct rt_spi_device *device, struct rt_spi_message* message)
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
	spicfg[0].max_hz = 48;
	spicfg[0].mode=0;
	spicfg[0].reserved=0;//chip select
	spicfg[1].data_width=8;
	spicfg[1].max_hz = 48;
	spicfg[1].mode=0;
	spicfg[1].reserved=1;//chip select
	spicfg[2].data_width=8;
	spicfg[2].max_hz = 48;
	spicfg[2].mode=0;
	spicfg[2].reserved=2;//chip select
	spicfg[3].data_width=8;
	spicfg[3].max_hz = 48;
	spicfg[3].mode=0;
	spicfg[3].reserved=3;//chip select
	rt_spi_bus_register(&spibus0,"spibus0",&ls1c_spi_ops);	
	rt_spi_bus_attach_device(spidev0  ,"spidev00","spibus0",spicfg);
	rt_spi_bus_attach_device(spidev0+1,"spidev01","spibus0",spicfg+1);
	rt_spi_bus_attach_device(spidev0+2,"spidev02","spibus0",spicfg+2);
	rt_spi_bus_attach_device(spidev0+3,"spidev03","spibus0",spicfg+3);
	#if 0
	rt_spi_bus_register(&spibus1,"spibus1",&ls1c_spi_ops);	
	rt_spi_bus_attach_device(spidev1  ,"spidev10","spibus1",spicfg+4);
	rt_spi_bus_attach_device(spidev1+1,"spidev11","spibus1",spicfg+5);
	rt_spi_bus_attach_device(spidev1+2,"spidev12","spibus1",spicfg+6);
	rt_spi_bus_attach_device(spidev1+3,"spidev13","spibus1",spicfg+7);
	#endif
}

