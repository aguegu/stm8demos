/**
  ******************************************************************************
  * @file    Project/STM8L15x_StdPeriph_Template/stm8l15x_it.h
  * @author  MCD Application Team
  * @version V1.6.1
  * @date    30-September-2014
  * @brief   This file contains the headers of the interrupt handlers.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM8L15x_IT_H
#define __STM8L15x_IT_H

/* Includes ------------------------------------------------------------------*/
#include "stm8l15x.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
#ifdef _COSMIC_
 void _stext(void); /* RESET startup routine */
 INTERRUPT void NonHandledInterrupt(void);
#endif /* _COSMIC_ */

/* SDCC patch: requires separate handling for SDCC (see below) */
#if !defined(_RAISONANCE_) && !defined(_SDCC_)
 INTERRUPT void TRAP_IRQHandler(void); /* TRAP */
 INTERRUPT void FLASH_IRQHandler(void); /* FLASH EOP/PG_DIS */
 INTERRUPT void DMA1_CHANNEL0_1_IRQHandler(void); /* DMA1 Channel0/1*/
 INTERRUPT void DMA1_CHANNEL2_3_IRQHandler(void); /*DMA1 Channel2/3*/
 INTERRUPT void RTC_CSSLSE_IRQHandler(void); /* RTC /CSS_LSE */
 INTERRUPT void EXTIE_F_PVD_IRQHandler(void); /*EXTI PORTE/EXTI PORTF/PVD*/
 INTERRUPT void EXTIB_G_IRQHandler(void); /* EXTI PORTB / EXTI PORTG */
 INTERRUPT void EXTID_H_IRQHandler(void); /* EXTI PORTD / EXTI PORTH*/
 INTERRUPT void EXTI0_IRQHandler(void); /* EXTI PIN0 */
 INTERRUPT void EXTI1_IRQHandler(void); /* EXTI PIN1 */
 INTERRUPT void EXTI2_IRQHandler(void); /* EXTI PIN2 */
 INTERRUPT void EXTI3_IRQHandler(void); /* EXTI PIN3 */
 INTERRUPT void EXTI4_IRQHandler(void); /* EXTI PIN4 */
 INTERRUPT void EXTI5_IRQHandler(void); /* EXTI PIN5 */
 INTERRUPT void EXTI6_IRQHandler(void); /* EXTI PIN6 */
 INTERRUPT void EXTI7_IRQHandler(void); /* EXTI PIN7 */
 INTERRUPT void LCD_AES_IRQHandler(void); /* LCD /AES */
 INTERRUPT void SWITCH_CSS_BREAK_DAC_IRQHandler(void); /* Switch CLK/CSS/TIM1 Break/DAC */
 INTERRUPT void ADC1_COMP_IRQHandler(void); /*ADC1/COMP*/
 INTERRUPT void TIM2_UPD_OVF_TRG_BRK_USART2_TX_IRQHandler(void); /* TIM2 UPD/OVF/TRG/BRK / USART2 TX */
 INTERRUPT void TIM2_CC_USART2_RX_IRQHandler(void); /* TIM2 CAP / USART2 RX */
 INTERRUPT void TIM3_UPD_OVF_TRG_BRK_USART3_TX_IRQHandler(void); /* TIM3 UPD/OVF/TRG/BRK /USART3 TX*/
 INTERRUPT void TIM3_CC_USART3_RX_IRQHandler(void); /* TIM3 CAP/ USART3 RX */
 INTERRUPT void TIM1_UPD_OVF_TRG_COM_IRQHandler(void);/* TIM1 UPD/OVF/TRG/COM */
 INTERRUPT void TIM1_CC_IRQHandler(void);/* TIM1 CAP*/
 INTERRUPT void TIM4_UPD_OVF_TRG_IRQHandler(void); /* TIM4 UPD/OVF/TRI */
 INTERRUPT void SPI1_IRQHandler(void); /* SPI1 */
 INTERRUPT void USART1_TX_TIM5_UPD_OVF_TRG_BRK_IRQHandler(void); /* USART1 TX / TIM5 UPD/OVF/TRG/BRK */
 INTERRUPT void USART1_RX_TIM5_CC_IRQHandler(void); /* USART1 RX / TIM5 CAP */
 INTERRUPT void I2C1_SPI2_IRQHandler(void); /* I2C1 / SPI2 */


// SDCC patch: __interrupt keyword required after function name --> requires new block
#elif defined (_SDCC_)

 void TRAP_IRQHandler(void);                                          /* TRAP */
 void FLASH_IRQHandler(void) INTERRUPT(1);                            /* FLASH EOP/PG_DIS */
 void DMA1_CHANNEL0_1_IRQHandler(void) INTERRUPT(2);                  /* DMA1 Channel0/1*/
 void DMA1_CHANNEL2_3_IRQHandler(void) INTERRUPT(3);                  /* DMA1 Channel2/3*/
 void RTC_CSSLSE_IRQHandler(void) INTERRUPT(4);                       /* RTC /CSS_LSE */
 void EXTIE_F_PVD_IRQHandler(void) INTERRUPT(5);                      /* EXTI PORTE/EXTI PORTF/PVD*/
 void EXTIB_G_IRQHandler(void) INTERRUPT(6);                          /* EXTI PORTB / EXTI PORTG */
 void EXTID_H_IRQHandler(void) INTERRUPT(7);                          /* EXTI PORTD / EXTI PORTH*/
 void EXTI0_IRQHandler(void) INTERRUPT(8);                            /* EXTI PIN0 */
 void EXTI1_IRQHandler(void) INTERRUPT(9);                            /* EXTI PIN1 */
 void EXTI2_IRQHandler(void) INTERRUPT(10);                           /* EXTI PIN2 */
 void EXTI3_IRQHandler(void) INTERRUPT(11);                           /* EXTI PIN3 */
 void EXTI4_IRQHandler(void) INTERRUPT(12);                           /* EXTI PIN4 */
 void EXTI5_IRQHandler(void) INTERRUPT(13);                           /* EXTI PIN5 */
 void EXTI6_IRQHandler(void) INTERRUPT(14);                           /* EXTI PIN6 */
 void EXTI7_IRQHandler(void) INTERRUPT(15);                           /* EXTI PIN7 */
 void LCD_AES_IRQHandler(void) INTERRUPT(16);                         /* LCD /AES */
 void SWITCH_CSS_BREAK_DAC_IRQHandler(void) INTERRUPT(17);            /* Switch CLK/CSS/TIM1 Break/DAC */
 void ADC1_COMP_IRQHandler(void) INTERRUPT(18);                       /* ADC1/COMP*/
 void TIM2_UPD_OVF_TRG_BRK_USART2_TX_IRQHandler(void) INTERRUPT(19);  /* TIM2 UPD/OVF/TRG/BRK / USART2 TX */
 void TIM2_CC_USART2_RX_IRQHandler(void) INTERRUPT(20);               /* TIM2 CAP / USART2 RX */
 void TIM3_UPD_OVF_TRG_BRK_USART3_TX_IRQHandler(void) INTERRUPT(21);  /* TIM3 UPD/OVF/TRG/BRK /USART3 TX*/
 void TIM3_CC_USART3_RX_IRQHandler(void) INTERRUPT(22);               /* TIM3 CAP/ USART3 RX */
 void TIM1_UPD_OVF_TRG_COM_IRQHandler(void) INTERRUPT(23);            /* TIM1 UPD/OVF/TRG/COM */
 void TIM1_CC_IRQHandler(void) INTERRUPT(24);                         /* TIM1 CAP*/
 void TIM4_UPD_OVF_TRG_IRQHandler(void) INTERRUPT(25);                /* TIM4 UPD/OVF/TRI */
 void SPI1_IRQHandler(void) INTERRUPT(26);                            /* SPI1 */
 void USART1_TX_TIM5_UPD_OVF_TRG_BRK_IRQHandler(void) INTERRUPT(27);  /* USART1 TX / TIM5 UPD/OVF/TRG/BRK */
 void USART1_RX_TIM5_CC_IRQHandler(void) INTERRUPT(28);               /* USART1 RX / TIM5 CAP */
 void I2C1_SPI2_IRQHandler(void) INTERRUPT(29);                       /* I2C1 / SPI2 */

#endif /* !(_RAISONANCE_) && !(_SDCC_) */

#endif /* __STM8L15x_IT_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
