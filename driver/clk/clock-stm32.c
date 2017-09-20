#include "common.h"
#include "driver/base.h"
#include "driver/platform.h"
#include "driver/resource.h"
#include "driver/clock.h"
#include "asm/io.h"
#include "asm/arch/base.h"
#include "asm/arch/clock.h"


struct stm32_u_id_regs {
	uint32_t u_id_low;
	uint32_t u_id_mid;
	uint32_t u_id_high;
};

struct stm32_rcc_regs {
	uint32_t cr;		/* RCC clock control */
	uint32_t pllcfgr;	/* RCC PLL configuration */
	uint32_t cfgr;	/* RCC clock configuration */
	uint32_t cir;	/* RCC clock interrupt */
	uint32_t ahb1rstr;	/* RCC AHB1 peripheral reset */
	uint32_t ahb2rstr;	/* RCC AHB2 peripheral reset */
	uint32_t ahb3rstr;	/* RCC AHB3 peripheral reset */
	uint32_t rsv0;
	uint32_t apb1rstr;	/* RCC APB1 peripheral reset */
	uint32_t apb2rstr;	/* RCC APB2 peripheral reset */
	uint32_t rsv1[2];
	uint32_t ahb1enr;	/* RCC AHB1 peripheral clock enable */
	uint32_t ahb2enr;	/* RCC AHB2 peripheral clock enable */
	uint32_t ahb3enr;	/* RCC AHB3 peripheral clock enable */
	uint32_t rsv2;
	uint32_t apb1enr;	/* RCC APB1 peripheral clock enable */
	uint32_t apb2enr;	/* RCC APB2 peripheral clock enable */
	uint32_t rsv3[2];
	uint32_t ahb1lpenr;	/* RCC AHB1 periph clk enable in low pwr mode */
	uint32_t ahb2lpenr;	/* RCC AHB2 periph clk enable in low pwr mode */
	uint32_t ahb3lpenr;	/* RCC AHB3 periph clk enable in low pwr mode */
	uint32_t rsv4;
	uint32_t apb1lpenr;	/* RCC APB1 periph clk enable in low pwr mode */
	uint32_t apb2lpenr;	/* RCC APB2 periph clk enable in low pwr mode */
	uint32_t rsv5[2];
	uint32_t bdcr;	/* RCC Backup domain control */
	uint32_t csr;	/* RCC clock control & status */
	uint32_t rsv6[2];
	uint32_t sscgr;	/* RCC spread spectrum clock generation */
	uint32_t plli2scfgr;	/* RCC PLLI2S configuration */
	uint32_t pllsaicfgr;
	uint32_t dckcfgr;
};

struct stm32_pwr_regs {
	uint32_t cr;
	uint32_t csr;
};

#define stm32_u_id				((struct stm32_u_id_regs *)U_ID_BASE)
#define stm32_rcc				((struct stm32_rcc_regs *)RCC_BASE)
#define stm32_pwr				((struct stm32_pwr_regs *)PWR_BASE)

#define RCC_CR_HSION			(1 << 0)
#define RCC_CR_HSEON			(1 << 16)
#define RCC_CR_HSERDY			(1 << 17)
#define RCC_CR_HSEBYP			(1 << 18)
#define RCC_CR_CSSON			(1 << 19)
#define RCC_CR_PLLON			(1 << 24)
#define RCC_CR_PLLRDY			(1 << 25)

#define RCC_PLLCFGR_PLLM_MASK	0x3F
#define RCC_PLLCFGR_PLLN_MASK	0x7FC0
#define RCC_PLLCFGR_PLLP_MASK	0x30000
#define RCC_PLLCFGR_PLLQ_MASK	0xF000000
#define RCC_PLLCFGR_PLLSRC		(1 << 22)
#define RCC_PLLCFGR_PLLN_SHIFT	6
#define RCC_PLLCFGR_PLLP_SHIFT	16
#define RCC_PLLCFGR_PLLQ_SHIFT	24

#define RCC_CFGR_AHB_PSC_MASK	0xF0
#define RCC_CFGR_APB1_PSC_MASK	0x1C00
#define RCC_CFGR_APB2_PSC_MASK	0xE000
#define RCC_CFGR_SW0			(1 << 0)
#define RCC_CFGR_SW1			(1 << 1)
#define RCC_CFGR_SW_MASK		0x3
#define RCC_CFGR_SW_HSI			0
#define RCC_CFGR_SW_HSE			RCC_CFGR_SW0
#define RCC_CFGR_SW_PLL			RCC_CFGR_SW1
#define RCC_CFGR_SWS0			(1 << 2)
#define RCC_CFGR_SWS1			(1 << 3)
#define RCC_CFGR_SWS_MASK		0xC
#define RCC_CFGR_SWS_HSI		0
#define RCC_CFGR_SWS_HSE		RCC_CFGR_SWS0
#define RCC_CFGR_SWS_PLL		RCC_CFGR_SWS1
#define RCC_CFGR_HPRE_SHIFT		4
#define RCC_CFGR_PPRE1_SHIFT	10
#define RCC_CFGR_PPRE2_SHIFT	13

#define RCC_APB1ENR_PWREN		(1 << 28)

#define PWR_CR_VOS0				(1 << 14)
#define PWR_CR_VOS1				(1 << 15)
#define PWR_CR_VOS_MASK			0xC000
#define PWR_CR_VOS_SCALE_MODE_1	(PWR_CR_VOS0 | PWR_CR_VOS1)
#define PWR_CR_VOS_SCALE_MODE_2	(PWR_CR_VOS1)
#define PWR_CR_VOS_SCALE_MODE_3	(PWR_CR_VOS0)


struct pll_psc {
	uint8_t		pll_m;
	uint16_t	pll_n;
	uint8_t		pll_p;
	uint8_t		pll_q;
	uint8_t		ahb_psc;
	uint8_t		apb1_psc;
	uint8_t		apb2_psc;
};

#define AHB_PSC_1				0
#define AHB_PSC_2				0x8
#define AHB_PSC_4				0x9
#define AHB_PSC_8				0xA
#define AHB_PSC_16				0xB
#define AHB_PSC_64				0xC
#define AHB_PSC_128				0xD
#define AHB_PSC_256				0xE
#define AHB_PSC_512				0xF

#define APB_PSC_1				0
#define APB_PSC_2				0x4
#define APB_PSC_4				0x5
#define APB_PSC_8				0x6
#define APB_PSC_16				0x7

#if (CONFIG_SYS_CLK_FREQ == 84000000)
/* 84 MHz */
static const struct pll_psc sys_pll_psc = {
	.pll_m = (CONFIG_STM32_HSE_HZ)/1000000,
	.pll_n = 336,
	.pll_p = 4,
	.pll_q = 7,
	.ahb_psc = AHB_PSC_1,
	.apb1_psc = APB_PSC_2,
	.apb2_psc = APB_PSC_1
};
#elif (CONFIG_SYS_CLK_FREQ == 180000000)
/* 180 MHz */
static const struct pll_psc sys_pll_psc = {
	.pll_m = (CONFIG_STM32_HSE_HZ)/1000000,
	.pll_n = 360,
	.pll_p = 2,
	.pll_q = 7,
	.ahb_psc = AHB_PSC_1,
	.apb1_psc = APB_PSC_4,
	.apb2_psc = APB_PSC_2
};
#else
/* default 168 MHz */
static const struct pll_psc sys_pll_psc = {
	.pll_m = (CONFIG_STM32_HSE_HZ)/1000000,
	.pll_n = 336,
	.pll_p = 2,
	.pll_q = 7,
	.ahb_psc = AHB_PSC_1,
	.apb1_psc = APB_PSC_4,
	.apb2_psc = APB_PSC_2
};
#endif

/* Prescaler table lookups for clock computation */
static const uint8_t ahb_psc_table[16] = {
	0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9
};

static const uint8_t apb_psc_table[8] = {
	0, 0, 0, 0, 1, 2, 3, 4
};


unsigned long clk_get(unsigned int clck)
{
	uint32_t sysclk = 0;
	uint32_t shift = 0;

	if ((readl(&stm32_rcc->cfgr) & RCC_CFGR_SWS_MASK) ==
			RCC_CFGR_SWS_PLL) {
		uint16_t pllm, plln, pllp;
		pllm = (readl(&stm32_rcc->pllcfgr) & RCC_PLLCFGR_PLLM_MASK);
		plln = ((readl(&stm32_rcc->pllcfgr) & RCC_PLLCFGR_PLLN_MASK)
			>> RCC_PLLCFGR_PLLN_SHIFT);
		pllp = ((((readl(&stm32_rcc->pllcfgr) & RCC_PLLCFGR_PLLP_MASK)
			>> RCC_PLLCFGR_PLLP_SHIFT) + 1) << 1);
		sysclk = ((CONFIG_STM32_HSE_HZ / pllm) * plln) / pllp;
	}

	switch (clck) {
	case CLOCK_CORE:
		return sysclk;
		break;
	case CLOCK_AHB:
		shift = ahb_psc_table[(
			(readl(&stm32_rcc->cfgr) & RCC_CFGR_AHB_PSC_MASK)
			>> RCC_CFGR_HPRE_SHIFT)];
		return sysclk >>= shift;
		break;
	case CLOCK_APB1:
		shift = apb_psc_table[(
			(readl(&stm32_rcc->cfgr) & RCC_CFGR_APB1_PSC_MASK)
			>> RCC_CFGR_PPRE1_SHIFT)];
		return sysclk >>= shift;
		break;
	case CLOCK_APB2:
		shift = apb_psc_table[(
			(readl(&stm32_rcc->cfgr) & RCC_CFGR_APB2_PSC_MASK)
			>> RCC_CFGR_PPRE2_SHIFT)];
		return sysclk >>= shift;
		break;
	default:
		return 0;
		break;
	}
}

void clk_setup_periph(unsigned int periph_addr)
{
	unsigned int offset;

	if (periph_addr < APB2PERIPH_BASE) {
		offset = (periph_addr - APB1PERIPH_BASE) >> 10;
		setbits_le32(&stm32_rcc->apb1enr, 1 << offset);
	} else if (periph_addr < AHB1PERIPH_BASE) {
		offset = (periph_addr - APB2PERIPH_BASE) >> 10;
		setbits_le32(&stm32_rcc->apb2enr, 1 << offset);
	} else if (periph_addr < AHB2PERIPH_BASE) {
		offset = (periph_addr - AHB1PERIPH_BASE) >> 10;
		setbits_le32(&stm32_rcc->ahb1enr, 1 << offset);
	} else if (periph_addr < AHB3PERIPH_BASE) {
		offset = (periph_addr - AHB2PERIPH_BASE) >> 10;
		setbits_le32(&stm32_rcc->ahb2enr, 1 << offset);
	} else {
		setbits_le32(&stm32_rcc->ahb3enr, 1);
	}
}

void clk_setup_dev(struct device *dev)
{
	struct resource *res;
	unsigned char i = 0;
	struct platform_device *pdev = to_platform_device(dev);

	if (!pdev || !pdev->num_resources)
		return;

	do {
		res = &pdev->resource[i];
		if (res->flags & RESOURCE_IO)
			clk_setup_periph(res->start);
	} while (i++ < pdev->num_resources);
}

int clk_update(unsigned int clk_freq)
{
	/* Reset RCC configuration */
	setbits_le32(&stm32_rcc->cr, RCC_CR_HSION);
	writel(0, &stm32_rcc->cfgr);				/* Reset CFGR */
	clrbits_le32(&stm32_rcc->cr, (RCC_CR_HSEON | RCC_CR_CSSON
		| RCC_CR_PLLON));
	writel(0x24003010, &stm32_rcc->pllcfgr);	/* Reset value from RM */
	clrbits_le32(&stm32_rcc->cr, RCC_CR_HSEBYP);
	writel(0, &stm32_rcc->cir);					/* Disable all interrupts */

	/* Configure for HSE+PLL operation */
	setbits_le32(&stm32_rcc->cr, RCC_CR_HSEON);
	while (!(readl(&stm32_rcc->cr) & RCC_CR_HSERDY))
		;

	/* Enable high performance mode, System frequency up to 180 MHz */
	setbits_le32(&stm32_rcc->apb1enr, RCC_APB1ENR_PWREN);
	writel(PWR_CR_VOS_SCALE_MODE_1, &stm32_pwr->cr);

	setbits_le32(&stm32_rcc->cfgr, ((
		sys_pll_psc.ahb_psc << RCC_CFGR_HPRE_SHIFT)
		| (sys_pll_psc.apb1_psc << RCC_CFGR_PPRE1_SHIFT)
		| (sys_pll_psc.apb2_psc << RCC_CFGR_PPRE2_SHIFT)));

	writel(sys_pll_psc.pll_m
		| (sys_pll_psc.pll_n << RCC_PLLCFGR_PLLN_SHIFT)
		| (((sys_pll_psc.pll_p >> 1) - 1) << RCC_PLLCFGR_PLLP_SHIFT)
		| (sys_pll_psc.pll_q << RCC_PLLCFGR_PLLQ_SHIFT),
		&stm32_rcc->pllcfgr);
	setbits_le32(&stm32_rcc->pllcfgr, RCC_PLLCFGR_PLLSRC);

	setbits_le32(&stm32_rcc->cr, RCC_CR_PLLON);

	while (!(readl(&stm32_rcc->cr) & RCC_CR_PLLRDY))
		;

	stm32_flash_latency_cfg(5);
	clrbits_le32(&stm32_rcc->cfgr, (RCC_CFGR_SW0 | RCC_CFGR_SW1));
	setbits_le32(&stm32_rcc->cfgr, RCC_CFGR_SW_PLL);

	while ((readl(&stm32_rcc->cfgr) & RCC_CFGR_SWS_MASK) !=
			RCC_CFGR_SWS_PLL)
		;

	return 0;
}

