#ifndef __TREMO_IT_H
#define __TREMO_IT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "std_lib.h"


#include "petitfatfs/diskio.h"



 //DSTATUS  (*disk_initialize) (void* disk_obj);
   // DRESULT  (*disk_writep)     (void* disk_obj, const BYTE* buff, DWORD sc);
    //DRESULT  (*disk_readp)      (void* disk_obj, BYTE* buff,
                              //   DWORD sector, UINT offser, UINT count);







typedef struct {
    uint8_t **CMD_init;
    uint8_t **CMD_do;


} MySDCard;




DSTATUS disk_init_impl_func ( MySDCard *disk_object);
DRESULT disk_write_impl_func ( MySDCard *disk_object, const BYTE* buff , DWORD sc );
DRESULT disk_read_impl_func (MySDCard *disk_object,BYTE* buff, DWORD sector, UINT offser, UINT count  );
#ifdef __cplusplus
}
#endif

#endif /* __TREMO_IT_H */
