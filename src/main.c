#include <stdio.h>
#include <stdarg.h>
#include "tremo_uart.h"
#include "tremo_gpio.h"
#include "tremo_rcc.h"

#include "tremo_delay.h"


#include <time.h>
#define UART_RX_BUF_SIZE 200


volatile uint8_t g_gpio_interrupt_flag_A_button = 0;
volatile uint8_t g_gpio_interrupt_flag_B_button = 0;
volatile uint8_t g_gpio_interrupt_flag_C_button = 0;

gpio_t *g_test_gpiox = GPIOA;
uint8_t A_BUTTON = GPIO_PIN_9;
uint8_t B_BUTTON = GPIO_PIN_4;
uint8_t C_BUTTON = GPIO_PIN_5;

uint8_t flag = 0 ; 
uint8_t highbyte ( uint16_t input  );

uint8_t lowbyte ( uint16_t input );


#define SIZE 6

int shuffled_numbers[SIZE];
int current_index = 0;


uint8_t pool_original[SIZE] = {1,2,3,4,5,6};

uint8_t pool[SIZE] = {1, 2, 3, 4, 5, 6};
uint8_t pool_size = SIZE;


uint32_t seed = 12345; 

uint32_t simple_rand()
{
    seed = seed * 1103515245 + 12345;
    return (seed >> 16) & 0x7FFF;
}



int get_random_unique_embedded()
{
    if (pool_size == 0)
        return -1; // Không còn số nào

    uint32_t index = simple_rand() % pool_size;
    int val = pool[index];

    // Loại bỏ phần tử đã dùng bằng cách swap với cuối mảng
    pool[index] = pool[pool_size - 1];
    pool_size--;

    return val;
}





bool handle_uart_rx(uint16_t *lora_buf_size,uint8_t *rx_buf,uint16_t *rx_index)
{
   // *rx_index = 1;
    bool new_data_available=false;
   //  printf(" receive happened \n");
    while (uart_get_flag_status(UART0, UART_FLAG_RX_FIFO_EMPTY) != SET

) {
        char ch = uart_receive_data(UART0);
          //  printf(" %c",ch);


        //  printf(" data received \n");
       printf("%02X ",ch);
      if (*rx_index < UART_RX_BUF_SIZE - 1) {

            rx_buf[(*rx_index)++] = ch;
            if (ch == '\n' || ch=='\r') {
                //printf(" end of line" );
                rx_buf[*rx_index] = '\0'; // null-terminate
                *lora_buf_size = *rx_index;
                *rx_index = 1;
                //printf(" rx buf tai vi tri 1 la %c",rx_buf[(*(lora_buf_size))-2]  );
                new_data_available = true;
                break; //?
                
            }
        } else {
            // buffer overflow protection
            rx_index = 1;
            new_data_available = false;
        }

       
    }
    return new_data_available;
}









void uart_log_init(void)
{
    // uart0
    gpio_set_iomux(GPIOB, GPIO_PIN_0, 1);
    gpio_set_iomux(GPIOB, GPIO_PIN_1, 1);

    /* uart config struct init */
    uart_config_t uart_config;
    uart_config_init(&uart_config);

    uart_config.baudrate = UART_BAUDRATE_9600;//



    uart_config.data_width = UART_DATA_WIDTH_8;
    uart_config.parity = UART_PARITY_NO;
    uart_config.stop_bits = UART_STOP_BITS_1;
    uart_config.flow_control = UART_FLOW_CONTROL_DISABLED;
    uart_config.mode = UART_MODE_TXRX;
    uart_config.fifo_mode = 1;

    uart_init(UART0, &uart_config);
    uart_cmd(UART0, ENABLE);

    uart_config_interrupt(UART0, UART_INTERRUPT_RX_DONE, true);
    uart_config_interrupt(UART0, UART_INTERRUPT_RX_TIMEOUT , true);
}



uint16_t calculate_check_sum(uint8_t *array, uint8_t size) {
    uint16_t sum = 0;
    for (int i = 0; i < size; ++i) {
        sum += array[i];
    }
    return 0xFFFF - sum + 1;
}


uint8_t highbyte ( uint16_t input  ){


    return (uint8_t)(input>>8);
}


uint8_t lowbyte ( uint16_t input){
    
    
    return (uint8_t)(input & 0xFF);
}

void interrupt_init(){


// A BUTTON
    gpio_init(g_test_gpiox, A_BUTTON, GPIO_MODE_INPUT_PULL_UP);
    gpio_config_interrupt(g_test_gpiox, A_BUTTON, GPIO_INTR_RISING_EDGE);
    // B BUTTON
    gpio_init(g_test_gpiox, B_BUTTON, GPIO_MODE_INPUT_PULL_UP);
    gpio_config_interrupt(g_test_gpiox, B_BUTTON, GPIO_INTR_RISING_EDGE);
    // C BUTTON
    gpio_init(g_test_gpiox, C_BUTTON, GPIO_MODE_INPUT_PULL_UP);
    gpio_config_interrupt(g_test_gpiox, C_BUTTON, GPIO_INTR_RISING_EDGE);
}





int main(void)
{    
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOA, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_UART0, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOB, true);  
    uart_log_init();
    interrupt_init();
    //printf("hello world\r\n");
    NVIC_EnableIRQ(GPIO_IRQn);
    NVIC_SetPriority(GPIO_IRQn, 2);
    uint16_t lora_buf_size ; 
    uint8_t rx_buf[200];
    uint16_t rx_index = 1 ; 
    bool new_data_available = false ; 
    uint8_t folder =0;
    uint8_t file =6 ; 
    uint8_t checksum_calculation [6];
    uint8_t specified[10];
    

/*
 ////////////////////////////////////////////////
 for ( int i = 0 ; i < 10 ; ++i ){
    //printf(" %d ",data[i]);
    uart_send_data(UART0,specified[i]);
   while (uart_get_flag_status(UART0, UART_FLAG_TX_FIFO_EMPTY) != SET);
}

/////////////////////////////////////////////////

    for ( int i = 0 ; i < 10000;++i);

*/

uint32_t prevent_bouncing = 0 ; 
uint32_t prevent_bouncing_edge = 120000;
uint8_t random_num= 0 ; 
    /* Infinite loop */
    while (1) { 
        if (uart_get_interrupt_status(UART0, UART_INTERRUPT_RX_DONE) == SET||
        uart_get_interrupt_status(UART0, UART_INTERRUPT_RX_TIMEOUT) == SET) 
        {
            uart_clear_interrupt(UART0, UART_INTERRUPT_RX_DONE);
            uart_clear_interrupt(UART0, UART_INTERRUPT_RX_TIMEOUT);
              //  printf ( " handle uart normal \n");
            new_data_available=handle_uart_rx(&lora_buf_size,rx_buf,&rx_index);   
    }     
                if ( new_data_available == true ){
                    new_data_available = false ; 
                  //  printf(" receive happened \n");
                   // printf(" debug ");
                    for ( int i = 1 ; i <= lora_buf_size-2 ; ++i ){
                        printf("%d",rx_buf[i]);
                    }
                }
            if ( g_gpio_interrupt_flag_A_button ==1 )
            {   
                g_gpio_interrupt_flag_A_button = 0;
                if ( prevent_bouncing == prevent_bouncing_edge)
                {

                    random_num++;
                prevent_bouncing = 0 ;
               // printf(" button a pressed \n");
                int result = get_random_unique_embedded();
                if (result!=-1)
                {
                 //   printf("Random: %d\n", result);
                }

                file = result ;


                specified [0]= 0x7E;
                specified [1]= 0xFF;
                specified  [2] = 0x06;
                specified [3] = 0x03;
                specified [4] = 0x01; 
                specified [5] = folder;
                specified [6] = file; 
                for ( int i = 0 ; i<6;++i){
                    checksum_calculation[i] = specified[i+1];
                }
                uint16_t checksumm = calculate_check_sum (checksum_calculation,6);
                specified[7] = highbyte(checksumm);
                specified[8] = lowbyte(checksumm)+file;
                specified[9] = 0xEF;



                for ( int i = 0 ; i < 10 ; ++i ){
                    //printf(" %d ",data[i]);
                    uart_send_data(UART0,specified[i]);
                   while (uart_get_flag_status(UART0, UART_FLAG_TX_FIFO_EMPTY) != SET);
                }
                }   
            }
            if ( g_gpio_interrupt_flag_B_button ==1 )
            {   
                g_gpio_interrupt_flag_B_button = 0;
                if ( prevent_bouncing == prevent_bouncing_edge)
                {
                prevent_bouncing = 0 ;
                printf(" button b pressed \n");
                }  
            }
        
            if ( g_gpio_interrupt_flag_C_button ==1 )
            {          
                g_gpio_interrupt_flag_C_button = 0;
                if ( prevent_bouncing == prevent_bouncing_edge)
                {
                prevent_bouncing = 0 ;
                printf(" button c pressed \n");
                }
            }
                        
            if(prevent_bouncing < prevent_bouncing_edge)
prevent_bouncing ++;

            

            if ( random_num == 6){
                random_num = 0 ; 
                for ( int i = 0 ; i < SIZE ; ++i){
                    pool[i] = pool_original[i];
                }
                pool_size = SIZE;
            }

                
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(void* file, uint32_t line)
{
    (void)file;
    (void)line;

    while (1) { }
}
#endif
