#include "sys.h"
#include "ff.h"

extern FATFS *fs[2]; 

void FatFs_Open(char *filename);
void FilePath_process(char * source,char *addstring);
void ClearPath(char *path);
void FileOpen_Txt(char *buf);
uint8_t exfuns_init(void);
uint8_t exf_getfree(uint8_t *drv,uint32_t *total,uint32_t *free);

uint8_t f_typetell(uint8_t *fname);