
#include "tremo_it.h"


#include "tremo_gpio.h"




extern volatile uint8_t g_gpio_interrupt_flag_A_button;
extern volatile uint8_t g_gpio_interrupt_flag_B_button;
extern volatile uint8_t g_gpio_interrupt_flag_C_button;

extern uint8_t A_BUTTON ;
extern uint8_t B_BUTTON ;
extern uint8_t C_BUTTON ;


/**
 * @brief  This function handles NMI exception.
 * @param  None
 * @retval None
 */
void NMI_Handler(void)
{
}

/**
 * @brief  This function handles Hard Fault exception.
 * @param  None
 * @retval None
 */
void HardFault_Handler(void)
{

    /* Go to infinite loop when Hard Fault exception occurs */
    while (1) { }
}

/**
 * @brief  This function handles Memory Manage exception.
 * @param  None
 * @retval None
 */
void MemManage_Handler(void)
{
    /* Go to infinite loop when Memory Manage exception occurs */
    while (1) { }
}

/**
 * @brief  This function handles Bus Fault exception.
 * @param  None
 * @retval None
 */
void BusFault_Handler(void)
{
    /* Go to infinite loop when Bus Fault exception occurs */
    while (1) { }
}

/**
 * @brief  This function handles Usage Fault exception.
 * @param  None
 * @retval None
 */
void UsageFault_Handler(void)
{
    /* Go to infinite loop when Usage Fault exception occurs */
    while (1) { }
}

/**
 * @brief  This function handles SVCall exception.
 * @param  None
 * @retval None
 */
void SVC_Handler(void)
{
}

/**
 * @brief  This function handles PendSVC exception.
 * @param  None
 * @retval None
 */
void PendSV_Handler(void)
{
}

/**
 * @brief  This function handles SysTick Handler.
 * @param  None
 * @retval None
 */
void SysTick_Handler(void)
{
}

/**
 * @brief  This function handles PWR Handler.
 * @param  None
 * @retval None
 */
void PWR_IRQHandler()
{
}

/******************************************************************************/
/*                 Tremo Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_cm4.S).                                               */
/******************************************************************************/

/**
 * @brief  This function handles PPP interrupt request.
 * @param  None
 * @retval None
 */
/*void PPP_IRQHandler(void)
{
}*/



void GPIO_IRQHandler(void)
{
    if (gpio_get_interrupt_status(GPIOA, A_BUTTON) == SET) {
        gpio_clear_interrupt(GPIOA, A_BUTTON);
        g_gpio_interrupt_flag_A_button = 1;
    }
    
   else if (gpio_get_interrupt_status(GPIOA, B_BUTTON) == SET) {
        gpio_clear_interrupt(GPIOA, B_BUTTON);
        g_gpio_interrupt_flag_B_button = 1;
    }



   else  if (gpio_get_interrupt_status(GPIOA, C_BUTTON) == SET) {
        gpio_clear_interrupt(GPIOA, C_BUTTON);
        g_gpio_interrupt_flag_C_button = 1;
    }

}
