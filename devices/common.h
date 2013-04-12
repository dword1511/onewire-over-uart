#ifndef __COMMON_H__
#define __COMMON_H__

#include "../onewire.h"

uint8_t crc8(uint8_t *data_in, uint16_t number_of_bytes_to_read);
void    delay_ms(uint32_t ms);

#endif /* __COMMON_H__ */
