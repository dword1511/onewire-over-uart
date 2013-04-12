#ifndef __ONEWIRE_H__
#define __ONEWIRE_H__

#include <stdint.h>

#define OW_SEARCH_FIRST 0xFF /* start new search */
#define OW_LAST_DEVICE  0x00 /* last device found */
#define OW_ROMCODE_SIZE 8    /* rom-code size including CRC */

#define OW_ERR_SUCCESS  0x00
#define OW_ERR_SHORT    0x01
#define OW_ERR_UART     0x02
#define OW_ERR_PRESENCE 0xFF
#define OW_ERR_DATA     0xFE

uint8_t ow_init(char *dev_path);
void    ow_finit(void);
uint8_t ow_reset(void);
uint8_t ow_bit(uint8_t b);
uint8_t ow_byte_wr(uint8_t b);
uint8_t ow_byte_rd(void);

uint8_t ow_rom_search(uint8_t diff, uint8_t *id);
void    ow_command(uint8_t command, uint8_t *id);

#endif /* __ONEWIRE_H__ */
