#include "main.h"
#include "bsp_dwt.h"

#define  DWT_CYCCNT  *(volatile unsigned int *)0xE0001004
#define  DWT_CR      *(volatile unsigned int *)0xE0001000
#define  DEM_CR      *(volatile unsigned int *)0xE000EDFC
#define  DBGMCU_CR   *(volatile unsigned int *)0xE0042004

#define  DEM_CR_TRCENA               (1 << 24)
#define  DWT_CR_CYCCNTENA            (1 <<  0)



void bsp_InitDWT(void)
{
	DEM_CR         |= (unsigned int)DEM_CR_TRCENA;   /* Enable Cortex-M4's DWT CYCCNT reg.  */
	DWT_CYCCNT      = (unsigned int)0u;
	DWT_CR         |= (unsigned int)DWT_CR_CYCCNTENA;
}

void bsp_Delayms(uint32_t _ulDelayTime)
{
	bsp_Delayus(1000*_ulDelayTime);
}

void bsp_Delayus(uint32_t _ulDelayTime)
{
    uint32_t tCnt, tDelayCnt;
	uint32_t tStart;
		
	tStart = DWT_CYCCNT;
	tCnt = 0;
	tDelayCnt = _ulDelayTime * (SystemCoreClock / 1000000);		      

	while(tCnt < tDelayCnt)
	{
		tCnt = DWT_CYCCNT - tStart; 
	}
}
