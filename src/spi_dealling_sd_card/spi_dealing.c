#include <stdio.h>
#include <stdarg.h>
#include "tremo_uart.h"
#include "tremo_gpio.h"
#include "tremo_rcc.h"

#include "tremo_delay.h"


//#include <time.h>

#include "tremo_spi.h"


#include "spi_dealing_sd_card/spi_dealing.h"

#include "petitfatfs/pff.h"


#include "spi_dealing_sd_card/implement_functions/FATFS_object/disk_object_f.h"


#define CMD_INDEX_NUMS 5
typedef enum {
CMD0,
CMD8,
CMD55,
ACMD41,
CMD58
}cmd_index; 






void spi1_get_ready(void);

MySDCard init_disk_object(uint8_t how_many_cmd_you_want_to_have_for_init_cmd, uint8_t how_many_cmd_you_want_to_have_for_do_cmd) ;

void calculate_the_cmd_binaries ( cmd_index index , uint8_t *array_input );


uint8_t crc7(const uint8_t *data, size_t len) ;

void spi_init (){
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_SSP1, true);
    gpio_set_iomux(GPIOA, GPIO_PIN_4, 4); // CLK:GP4
    gpio_set_iomux(GPIOA, GPIO_PIN_5, 4); // NSS:GP5
    gpio_set_iomux(GPIOA, GPIO_PIN_6, 4); // TX:GP6
    gpio_set_iomux(GPIOA, GPIO_PIN_7, 4); // RX:GP7
    spi1_get_ready();
}


void spi_send_data(uint8_t data_send){


}





// function noi bo 
void spi1_get_ready(void)
{
    int i;
    ssp_init_t init_struct;
    ssp_init_struct(&init_struct);
    ssp_init(SSP1, &init_struct);
    ssp_config_interrupt(SSP1, SSP_INTERRUPT_RX_FIFO_TRIGGER | SSP_INTERRUPT_RX_TIMEOUT | SSP_INTERRUPT_RX_FIFO_OVERRUN, ENABLE);
    /* NVIC config */
    NVIC_EnableIRQ(SSP1_IRQn);
    NVIC_SetPriority(SSP1_IRQn, 2);
    ssp_cmd(SSP1, ENABLE);

   // for (i = 0; i < 10; i++) {
     //   ssp_send_data(SSP0, data_to_send + i, 1);
    //}
}





//function noi bo 

MySDCard init_disk_object(uint8_t how_many_cmd_you_want_to_have_for_init_cmd, uint8_t how_many_cmd_you_want_to_have_for_do_cmd){
    MySDCard  object_return;
    object_return.CMD_init = (uint8_t**) calloc(how_many_cmd_you_want_to_have_for_init_cmd, sizeof(uint8_t*));   
    for (int i = 0; i < how_many_cmd_you_want_to_have_for_init_cmd; i++) {
        object_return.CMD_init[i] = (uint8_t*) calloc(6, sizeof(uint8_t));
    } 
   object_return.CMD_do = (uint8_t**) calloc(how_many_cmd_you_want_to_have_for_do_cmd, sizeof(uint8_t*));
   for (int i = 0; i < how_many_cmd_you_want_to_have_for_do_cmd; i++) {
    object_return.CMD_do[i] = (uint8_t*) calloc(6, sizeof(uint8_t));
}
return object_return;
}



uint8_t crc7(const uint8_t *data, size_t len) {
    uint8_t crc = 0;
    for (size_t i = 0; i < len; i++) {
        uint8_t d = data[i];
        for (int j = 0; j < 8; j++) {
            crc <<= 1;
            if ((d ^ crc) & 0x80)
                crc ^= 0x09;  // 0x89 >> 1, vì CRC7 chỉ 7 bit
            d <<= 1;
        }
    }
    return crc & 0x7F;
}








void calculate_the_cmd_binaries ( cmd_index index , uint8_t *array_input ){

//MySDCard result = init_disk_object(5,5);

//uint8_t* return_object ;

uint8_t bytes[6] ;
    switch ( index)
    {
    case CMD0:
        bytes[0]= 0x40;
        bytes[1]= 0x00;
        bytes[2]= 0x00;
        bytes[3]= 0x00;
        bytes[4]= 0x00;
        break;
    case CMD8:
        bytes[0]=0x48;
        bytes[1]= 0x00;
        bytes[2]= 0x00;
        bytes[3]= 0x01;
        bytes[4]= 0xAA;
     //   bytes[5]= 0x95;
    case CMD55:
        bytes[0]= 0x77;
        bytes[1]= 0x00;
        bytes[2]= 0x00;
        bytes[3]= 0x00;
        bytes[4]= 0x00;
       // bytes[5]= 0x95; 
        
    case ACMD41:
        bytes[0]=0x69;
        bytes[1]=0x40;
        bytes[2]=0x00;
        bytes[3]=0x00;
        bytes[4]=0x00;
         
    case CMD58:
        bytes[0]=0x7A;
        bytes[1]=0x00;
        bytes[2]=0x00;
        bytes[3]=0x00;
        bytes[4]=0x00;

    default:
        break;
    }
    bytes[5]= (cr7(bytes,5)<<1) | 0x01;


for ( int i = 0 ; i < 6 ; ++i )
{
    *(array_input+i) = bytes[i];

}

    //for ( int i = 0 ; i < 6 ; ++i )
  //  *((result.CMD_init[index])+i)= bytes[i];

        //(uint8_t*) calloc(6, sizeof(uint8_t))
  
}





void Work_with_sd_spi_module () {
    FATFS fatfs_object ; 
    MySDCard disk_object = init_disk_object(5,5);
    //MySDCard disk_object

    
    for ( int i = 0 ; i < CMD_INDEX_NUMS;++i)
    {
    calculate_the_cmd_binaries(i,disk_object.CMD_init[i]); 
    }
    fatfs_object.disk.disk_obj = &disk_object;
    fatfs_object.disk.disk_initialize = disk_init_impl_func; // implement later
    fatfs_object.disk.disk_readp =  disk_read_impl_func; // implement later 
    fatfs_object.disk.disk_writep = disk_write_impl_func; // implement later 
    FRESULT result = pf_mount(&fatfs_object) ; 
    switch (result){
        case FR_OK : 
            printf(" FR_OKE \n");
            break;
        default :
            break;
    }  






}



//DSTATUS disk_init_impl_func ( MySDCard *disk_object);
//DRESULT disk_write_impl_func ( MySDCard *disk_object, const BYTE* buff , DWORD sc );
//DRESULT disk_read_impl_func (







