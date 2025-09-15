#include <stdio.h>
#include <stdarg.h>
#include "tremo_uart.h"
#include "tremo_gpio.h"
#include "tremo_rcc.h"

#include "tremo_delay.h"

#include "tremo_timer.h"
//#include <time.h>
#include <string.h>
#include "tremo_spi.h"
#define UART_RX_BUF_SIZE 200



typedef enum {
A_TYPE,
B_TYPE,
C_TYPE,
D_TYPE,
E_TYPE,
}which_type_of_music;


typedef enum {
C1_C5,
C10_C15,
C20_C25,
}which_album_of_C_TYPE;




volatile uint8_t spi1_interrupt_button = 0;

volatile uint8_t g_gpio_interrupt_flag_A_button = 0;
volatile uint8_t g_gpio_interrupt_flag_B_button = 0;
volatile uint8_t g_gpio_interrupt_flag_C_button = 0;

volatile uint8_t g_gpio_interrupt_flag_D_button = 0;
volatile uint8_t g_gpio_interrupt_flag_E_button = 0;



gpio_t *g_test_gpiox = GPIOA;
uint8_t A_BUTTON = GPIO_PIN_9;
uint8_t B_BUTTON = GPIO_PIN_4;
uint8_t C_BUTTON = GPIO_PIN_5;
uint8_t D_BUTTON = GPIO_PIN_6;
uint8_t E_BUTTON = GPIO_PIN_8;


uint8_t SWITCH_SPEAKER = GPIO_PIN_15;




uint8_t BUSY_SPEAKER_1_PIN = GPIO_PIN_14;

//uint8_t BUSY_SPEAKER_2_PIN = GPIO_PIN_15;


uint8_t flag = 0 ; 

#define BUSY GPIO_LEVEL_LOW
#define SIZE 6

int shuffled_numbers[SIZE];
int current_index = 0;



bool timer_flag = 0 ;

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



uint16_t calculate_check_sum(const uint8_t *array, uint8_t size) {
    uint16_t sum = 0;
    for (uint8_t i = 0; i < size; ++i) {
        sum += array[i];
    }
    return (uint16_t)(0xFFFF - sum + 1);
}



void interrupt_init(){


// A BUTTON
    gpio_init(g_test_gpiox, A_BUTTON, GPIO_MODE_INPUT_PULL_UP);
    gpio_config_interrupt(g_test_gpiox, A_BUTTON, GPIO_INTR_RISING_FALLING_EDGE);
    // B BUTTON
    gpio_init(g_test_gpiox, B_BUTTON, GPIO_MODE_INPUT_PULL_UP);
    gpio_config_interrupt(g_test_gpiox, B_BUTTON, GPIO_INTR_RISING_FALLING_EDGE);
    // C BUTTON
    gpio_init(g_test_gpiox, C_BUTTON, GPIO_MODE_INPUT_PULL_UP);
    gpio_config_interrupt(g_test_gpiox, C_BUTTON, GPIO_INTR_RISING_FALLING_EDGE);
    // D BUTTON
    gpio_init(g_test_gpiox, D_BUTTON, GPIO_MODE_INPUT_PULL_UP);
    gpio_config_interrupt(g_test_gpiox, D_BUTTON, GPIO_INTR_FALLING_EDGE);
    // E BUTTON
    gpio_init(g_test_gpiox, E_BUTTON, GPIO_MODE_INPUT_PULL_UP);
    gpio_config_interrupt(g_test_gpiox, E_BUTTON, GPIO_INTR_FALLING_EDGE);
}




void phat_nhac (uint8_t track_nums ){
    uint8_t checksum_calculation [6];
    uint8_t specified[10];

    specified [0] = 0x7E;
    specified [1] = 0xFF;
    specified [2] = 0x06;
    specified [3] = 0x03;
    specified [4] = 0x01; 
    specified [5] = 0;
    specified [6] = track_nums; 
    for ( int i = 0 ; i<6;++i){
        checksum_calculation[i] = specified[i+1];
    }
    uint16_t checksumm = calculate_check_sum (checksum_calculation,6);
    specified[7] = (uint8_t)(checksumm >> 8);   // Checksum High
    specified[8] = (uint8_t)(checksumm & 0xFF); // Checksum Low
    specified[9] = 0xEF;
    for ( int i = 0 ; i < 10 ; ++i ){
        //printf(" %d ",data[i]);
        uart_send_data(UART0,specified[i]);
       while (uart_get_flag_status(UART0, UART_FLAG_TX_FIFO_EMPTY) != SET);
    }

}

void phat_nhac_theo_folder ( uint8_t track_nums,  uint8_t folder_nums){

    uint8_t checksum_calculation [6];
    uint8_t specified[10];

    specified [0] = 0x7E;
    specified [1] = 0xFF;
    specified [2] = 0x06;
    specified [3] = 0x0F;
    specified [4] = 0x01; 
    specified [5] = folder_nums;
    specified [6] = track_nums; 
    for ( int i = 0 ; i<6;++i){
        checksum_calculation[i] = specified[i+1];
    }
    uint16_t checksumm = calculate_check_sum (checksum_calculation,6);
    specified[7] = (uint8_t)(checksumm >> 8);   // Checksum High
    specified[8] = (uint8_t)(checksumm & 0xFF); // Checksum Low
    specified[9] = 0xEF;
    for ( int i = 0 ; i < 10 ; ++i ){
        //printf(" %d ",data[i]);
        uart_send_data(UART0,specified[i]);
       while (uart_get_flag_status(UART0, UART_FLAG_TX_FIFO_EMPTY) != SET);
    }



}

void lam_gi_thi_lam (uint8_t what_u_do , uint8_t byte_six, uint8_t byte_seven){

    uint8_t checksum_calculation [6];
    uint8_t specified[10];

    specified [0] = 0x7E;
    specified [1] = 0xFF;
    specified [2] = 0x06;
    specified [3] = what_u_do;
    specified [4] = 0x01; 
    specified [5] = byte_six;
    specified [6] = byte_seven; 
    for ( int i = 0 ; i<6;++i){
        checksum_calculation[i] = specified[i+1];
    }
    uint16_t checksumm = calculate_check_sum (checksum_calculation,6);
    specified[7] = (uint8_t)(checksumm >> 8);   // Checksum High
    specified[8] = (uint8_t)(checksumm & 0xFF); // Checksum Low
    specified[9] = 0xEF;
    for ( int i = 0 ; i < 10 ; ++i ){
        //printf(" %d ",data[i]);
        uart_send_data(UART0,specified[i]);
       while (uart_get_flag_status(UART0, UART_FLAG_TX_FIFO_EMPTY) != SET);
    }


}






void gptimer_simple_timer(timer_gp_t* TIMERx)
{
    timer_init_t timerx_init;

    timer_config_interrupt(TIMERx, TIMER_DIER_UIE, ENABLE);

    timerx_init.prescaler          = 23999;  //sysclock defaults to 24M, is divided by (prescaler + 1) to 1k
    timerx_init.counter_mode       = TIMER_COUNTERMODE_UP;
    timerx_init.period             = 5000;   //time period is ((1 / 1k) * 000) 
    timerx_init.clock_division     = TIMER_CKD_FPCLK_DIV1;
    timerx_init.autoreload_preload = false;
    timer_init(TIMERx, &timerx_init);
    timer_generate_event(TIMERx, TIMER_EGR_UG, ENABLE);
    timer_clear_status(TIMER0, TIMER_SR_UIF);
    timer_cmd(TIMERx, true);

}







void gptim0_IRQHandler(void)
{
    bool state;

    timer_get_status(TIMER0, TIMER_SR_UIF, &state);

    if (state) {

        timer_flag = 1 ; 
        timer_clear_status(TIMER0, TIMER_SR_UIF);
       
        timer_cmd(TIMER0, false);  
    }
}









int main(void)
{    
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOA, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_UART0, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOB, true); 
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_TIMER0, true);



    // timer setup
    gptimer_simple_timer(TIMER0);
     NVIC_SetPriority(TIMER0_IRQn, 1);
    NVIC_EnableIRQ(TIMER0_IRQn);
   
        //timer setup 


    uart_log_init();
    interrupt_init();

    NVIC_EnableIRQ(GPIO_IRQn);
    NVIC_SetPriority(GPIO_IRQn, 2);
    uint16_t lora_buf_size ; 
    uint8_t rx_buf[200];
    uint16_t rx_index = 1 ; 
    bool new_data_available = false ; 


    uint32_t prevent_bouncing = 0 ; 
    uint32_t prevent_bouncing_edge = 50000;








    bool allow_B_BUTTON_Works= false;
        



    which_album_of_C_TYPE current_album_name = C1_C5;

    uint8_t album[10] = {1,2,3,4,5,0 }; 
    uint8_t current_song =0 ; 



    uint8_t current_D_song = 0 ; 
    uint8_t current_E_song = 0 ; 




        gpio_init(GPIOA,BUSY_SPEAKER_1_PIN,GPIO_MODE_INPUT_FLOATING);
        gpio_init(GPIOA,SWITCH_SPEAKER,GPIO_MODE_OUTPUT_PP_LOW);
        //gpio_init(GPIOA,BUSY_SPEAKER_2_PIN,GPIO_MODE_INPUT_FLOATING);



      // CONFIG BUSY_SPEAKER_2
      //gpio_init(GPIOA,BUSY_SPEAKER_2_PIN,GPIO_MODE_INPUT_FLOATING);
      //gpio_config_interrupt(GPIOA,BUSY_SPEAKER_2_PIN, GPIO_INTR_FALLING_EDGE);


      which_type_of_music what_kind_of_music_is_playing = A_TYPE; 


    /* Infinite loop */
    while (1) { 
        if ( timer_flag == 1 && allow_B_BUTTON_Works == false){

            allow_B_BUTTON_Works = true ; 
            timer_flag = 0 ; 
        }

        if (uart_get_interrupt_status(UART0, UART_INTERRUPT_RX_DONE) == SET||
        uart_get_interrupt_status(UART0, UART_INTERRUPT_RX_TIMEOUT) == SET) 
        {
            uart_clear_interrupt(UART0, UART_INTERRUPT_RX_DONE);
            uart_clear_interrupt(UART0, UART_INTERRUPT_RX_TIMEOUT);
             
            new_data_available=handle_uart_rx(&lora_buf_size,rx_buf,&rx_index);   
    }     
                if ( new_data_available == true ){
                    new_data_available = false ; 
                
                    for ( int i = 1 ; i <= lora_buf_size-2 ; ++i ){
                        printf("%d",rx_buf[i]);
                    }
                }
            if ( g_gpio_interrupt_flag_A_button ==1 )
            {   

                printf(" a button pressed \n");
                g_gpio_interrupt_flag_A_button = 0;
                if ( prevent_bouncing == prevent_bouncing_edge)
                {

        

            prevent_bouncing = 0 ;
            gpio_init(GPIOA,SWITCH_SPEAKER,GPIO_MODE_OUTPUT_PP_LOW);

                    switch ( gpio_read (GPIOA,A_BUTTON)){

                        case GPIO_LEVEL_HIGH :
                        phat_nhac(3);
                        break;

                        case GPIO_LEVEL_LOW:
                       
                        phat_nhac(2);
                        
                        break;
                    }
                
                    what_kind_of_music_is_playing = A_TYPE;
            
                }   
            }
            if ( g_gpio_interrupt_flag_B_button ==1 )
            {   
                g_gpio_interrupt_flag_B_button = 0;
               // printf(" does interrupt b button \n");
                if ( prevent_bouncing == prevent_bouncing_edge  && allow_B_BUTTON_Works == true ) //&&gpio_read(GPIOA, BUSY_SPEAKER_1_PIN)!=BUSY )
                {

                    if ( (what_kind_of_music_is_playing != A_TYPE) || 
                    (gpio_read(GPIOA, BUSY_SPEAKER_1_PIN) != BUSY) ) 
               {
                   prevent_bouncing = 0;
                   // timer_cmd(TIMER0, false);


                   gpio_init(GPIOA,SWITCH_SPEAKER,GPIO_MODE_OUTPUT_PP_LOW);

               
                   switch (gpio_read(GPIOA, B_BUTTON)) {
                       case GPIO_LEVEL_HIGH:
                           phat_nhac(5);
                           break;
               
                       case GPIO_LEVEL_LOW:
                           phat_nhac(4);
                           break;
                   }
                   what_kind_of_music_is_playing = B_TYPE;
                   timer_cmd(TIMER0, false);  

                   //timer_flag= 0 ; 
                   //NVIC_DisableIRQ(TIMER0_IRQn);  
                  // TIMER0->CNT = 0 ; 
               }
               }



                   
            }
            if ( g_gpio_interrupt_flag_C_button ==1 )
            {         

                g_gpio_interrupt_flag_C_button = 0;
                if ( prevent_bouncing == prevent_bouncing_edge)
                {
                    if  (what_kind_of_music_is_playing!= A_TYPE && what_kind_of_music_is_playing !=B_TYPE|| (gpio_read(GPIOA, BUSY_SPEAKER_1_PIN) != BUSY)   )
                    {
                       

                        switch (gpio_read(GPIOA, C_BUTTON)) {
                            case GPIO_LEVEL_HIGH:
                           printf("CNT La %d \n",TIMER0->CNT);
                            timer_cmd(TIMER0,false);
                            if ( TIMER0->CNT >2000){
                               
                                phat_nhac(2);
                               // printf(" album changed \nw");


                                switch ( current_album_name ){
                                    case C1_C5:
                                    {
                                    uint8_t data[] = { 10,11,12,13,14,15};
                                    memcpy(album,data,sizeof(data));
                                    current_album_name = C10_C15;
                                   // printf(" goes to c10 15\n");
                                    break;
                                    }
                                    case C10_C15:
                                    {
                                    uint8_t data[] = { 20,21,22,23,24,25};
                                    memcpy(album,data,sizeof(data));
                                    current_album_name = C20_C25;
                                   // printf(" goes to c20 25\n");
                                    break;
                                    }

                                    case C20_C25:
                                    {
                                    uint8_t data[] = { 1,2,3,4,5,0};
                                    memcpy(album,data,sizeof(data));
                                    current_album_name = C1_C5;
                                   // printf(" goes to c1 c5\n");
                                    break;
                                    }
                                }
                            }
                         
                             TIMER0->CNT = 0 ; 
                                break;
                    
                            case GPIO_LEVEL_LOW:
                            
                            TIMER0->CNT = 0 ; 
                           timer_cmd(TIMER0,true);
                           // printf(" count la %d \n", TIMER0->CNT);
                            phat_nhac_theo_folder(album[current_song],1);
                           //phat_nhac(4);
                            //printf(" in thu index %d\n",current_song);


                            
                            gpio_init(GPIOA,SWITCH_SPEAKER,GPIO_MODE_OUTPUT_PP_HIGH);

                       
                            what_kind_of_music_is_playing= C_TYPE;
                            current_song ++;
                            if ( current_song == 5)
                                current_song = 0 ;         
                                break;
                        }
                prevent_bouncing = 0 ;
                    }


                }




            }  

            // if ( g_gpio_interrupt_flag_D_button ==1 ){
            //     g_gpio_interrupt_flag_D_button = 0 ; 

            //     if ( prevent_bouncing == prevent_bouncing_edge && what_kind_of_music_is_playing != E_TYPE)
            //     {


            //         gpio_init(GPIOA,SWITCH_SPEAKER,GPIO_MODE_OUTPUT_PP_HIGH);
            //         delay_us(10);

            //         uint8_t data[] = {1,2,3,4,5,6,7,8,9,10};

            //         memcpy(album,data,sizeof(data));

            //         phat_nhac_theo_folder(album[current_song],2);

            //         current_D_song ++;

            //         if (current_D_song == 10 ){

            //             current_D_song = 0 ; 
            //         }
            //         what_kind_of_music_is_playing = D_TYPE;
            //         prevent_bouncing = 0 ; 
            //     }

            // }


            // if (g_gpio_interrupt_flag_E_button ==1 ){
            //     g_gpio_interrupt_flag_E_button = 0 ; 
            //     if ( prevent_bouncing == prevent_bouncing_edge)
            //     {
            //         gpio_init(GPIOA,SWITCH_SPEAKER,GPIO_MODE_OUTPUT_PP_HIGH);
            //         delay_us(10);
            //         if ( (gpio_read(GPIOA, BUSY_SPEAKER_1_PIN) == BUSY))
            //         {
                   

            //             lam_gi_thi_lam(0x0E,0,0); //pause
                        

                    
            //         }

            //         uint8_t data[] = {1,2,3,4,5,6,0,0,0,0};

            //         memcpy(album,data,sizeof(data));
            //         phat_nhac_theo_folder(current_E_song,3);
            //         current_E_song++;
            //         if ( current_E_song ==6){
            //             current_E_song = 0 ; 
            //         }
                    
            //         what_kind_of_music_is_playing = E_TYPE;
            //         prevent_bouncing = 0 ; 
            //     }

            // }


            if(prevent_bouncing < prevent_bouncing_edge)
                prevent_bouncing ++;
                



                
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
