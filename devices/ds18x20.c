#include <stdlib.h>
#include <stdint.h>

#include "common.h"
#include "ds18x20.h"

/********************************************************
 * find DS18X20 Sensors on 1-Wire-Bus
 * input/ouput: diff is the result of the last rom-search
 *              *diff = OW_SEARCH_FIRST for first call
 * output: id is the rom-code of the sensor found
 *******************************************************/
uint8_t DS18X20_find_sensor(uint8_t *diff, uint8_t id[]) {
  uint8_t go;
  uint8_t ret;

  ret = DS18X20_OK;
  go = 1;
  do {
    *diff = ow_rom_search(*diff, &id[0]);
    if(*diff == OW_ERR_PRESENCE || *diff == OW_ERR_DATA || *diff == OW_LAST_DEVICE) { 
      go  = 0;
      ret = DS18X20_ERROR;
    }
    else if(id[0] == DS18B20_FAMILY_CODE || id[0] == DS18S20_FAMILY_CODE || id[0] == DS1822_FAMILY_CODE) go = 0;
  } while(go);

  return ret;
}

/********************************************************
 * get power status of DS18x20
 * input:   id = rom_code
 * returns: DS18X20_POWER_EXTERN or DS18X20_POWER_PARASITE
 *******************************************************/
uint8_t DS18X20_get_power_status(uint8_t id[]) {
  uint8_t pstat;

  ow_reset();
  ow_command(DS18X20_READ_POWER_SUPPLY, id);
  pstat = ow_bit(1);
  ow_reset();
  return (pstat) ? DS18X20_POWER_EXTERN : DS18X20_POWER_PARASITE;
}

/* start measurement (CONVERT_T) for all sensors if input id == NULL or for single sensor where id is the rom-code */
uint8_t DS18X20_start_meas(uint8_t with_power_extern, uint8_t id[]) {
  ow_reset();

  ow_command(DS18X20_CONVERT_T, id);
  return DS18X20_OK;
}

/* returns 1 if conversion is in progress, 0 if finished, not available when parasite powered. */
uint8_t DS18X20_conversion_in_progress(void) {
  return ow_bit(1) ? DS18X20_CONVERSION_DONE : DS18X20_CONVERTING;
}

static uint8_t read_scratchpad(uint8_t id[], uint8_t sp[], uint8_t n) {
  uint8_t i;
  uint8_t ret;

  ow_command( DS18X20_READ, id );
  for(i = 0; i < n; i++) sp[i] = ow_byte_rd();

  if (crc8(&sp[0], DS18X20_SP_SIZE)) ret = DS18X20_ERROR_CRC;
  else ret = DS18X20_OK;

  return ret;
}

/* convert scratchpad data to physical value in unit decicelsius */
static int16_t DS18X20_raw_to_decicelsius(uint8_t familycode, uint8_t sp[]) {
  uint16_t measure;
  uint8_t negative;
  int16_t decicelsius;
  uint16_t fract;

  measure = sp[0] | (sp[1] << 8);
  /* measure = 0xFF5E; test -10.125 */
  /* measure = 0xFE6F; test -25.0625 */

  if(familycode == DS18S20_FAMILY_CODE) { /* 9 -> 12 bit if 18S20 */
    /* Extended measurements for DS18S20 contributed by Carsten Foss */
    measure &= (uint16_t)0xfffe; /* Discard LSB, needed for later extended precicion calc */
    measure <<= 3; /* Convert to 12-bit, now degrees are in 1/16 degrees units */
    measure += (16 - sp[6]) - 4; /* Add the compensation and remember to subtract 0.25 degree (4 / 16) */
  }

  /* check for negative */
  if(measure & 0x8000) {
    negative = 1; /* mark negative */
    measure ^= 0xffff; /* convert to positive => (twos complement)++ */
    measure++;
  }
  else negative = 0;

  /* clear undefined bits for DS18B20 != 12bit resolution */
  if(familycode == DS18B20_FAMILY_CODE || familycode == DS1822_FAMILY_CODE) {
    switch(sp[DS18B20_CONF_REG] & DS18B20_RES_MASK) {
      case DS18B20_9_BIT:
        measure &= ~(DS18B20_9_BIT_UNDF);
        break;
      case DS18B20_10_BIT:
        measure &= ~(DS18B20_10_BIT_UNDF);
        break;
      case DS18B20_11_BIT:
        measure &= ~(DS18B20_11_BIT_UNDF);
        break;
      default: break; // 12 bit - all bits valid
    }
  }

  decicelsius = (measure >> 4);
  decicelsius *= 10;

  /* decicelsius += ((measure & 0x000F) * 640 + 512) / 1024; 625 / 1000 = 640 / 1024 */
  fract = (measure & 0x000F) * 640;
  if(!negative) fract += 512;
  fract /= 1024;
  decicelsius += fract;

  if (negative) decicelsius = -decicelsius;

  if (/* decicelsius == 850 || */ decicelsius < -550 || decicelsius > 1250) return DS18X20_INVALID_DECICELSIUS;
  else return decicelsius;
}

/********************************************************
 * reads temperature (scratchpad) of sensor with rom-code id
 * output: decicelsius 
 * returns DS18X20_OK on success
 *******************************************************/
uint8_t DS18X20_read_decicelsius(uint8_t id[], int16_t *decicelsius) {
  uint8_t sp[DS18X20_SP_SIZE];
  uint8_t ret;

  ow_reset();
  ret = read_scratchpad(id, sp, DS18X20_SP_SIZE);
  if(ret == DS18X20_OK) *decicelsius = DS18X20_raw_to_decicelsius(id[0], sp);

  return ret;
}

/********************************************************
 * reads temperature (scratchpad) of sensor without id (single sensor)
 * output: decicelsius 
 * returns DS18X20_OK on success
 *******************************************************/
uint8_t DS18X20_read_decicelsius_single(uint8_t familycode, int16_t *decicelsius) {
  uint8_t sp[DS18X20_SP_SIZE];
  uint8_t ret;

  ret = read_scratchpad(NULL, sp, DS18X20_SP_SIZE);
  if(ret == DS18X20_OK) *decicelsius = DS18X20_raw_to_decicelsius(familycode, sp);
  return ret;
}

static int32_t DS18X20_raw_to_maxres( uint8_t familycode, uint8_t sp[]) {
  uint16_t measure;
  uint8_t negative;
  int32_t temperaturevalue;

  measure = sp[0] | (sp[1] << 8);
  /* measure = 0xFF5E; test -10.125 */
  /* measure = 0xFE6F; test -25.0625 */

  if(familycode == DS18S20_FAMILY_CODE) { /* 9 -> 12 bit if 18S20 */
    /* Extended measurements for DS18S20 contributed by Carsten Foss */
    measure &= (uint16_t)0xfffe; /* Discard LSB, needed for later extended precicion calc */
    measure <<= 3; /* Convert to 12-bit, now degrees are in 1/16 degrees units */
    measure += ( 16 - sp[6] ) - 4; /* Add the compensation and remember to subtract 0.25 degree (4 / 16) */
  }

  /* check for negative */
  if(measure & 0x8000) {
    negative = 1; /* mark negative */
    measure ^= 0xffff; /* convert to positive => (twos complement)++ */
    measure++;
  }
  else negative = 0;

  /* clear undefined bits for DS18B20 != 12bit resolution */
  if(familycode == DS18B20_FAMILY_CODE || familycode == DS1822_FAMILY_CODE) {
    switch(sp[DS18B20_CONF_REG] & DS18B20_RES_MASK) {
      case DS18B20_9_BIT:
        measure &= ~(DS18B20_9_BIT_UNDF);
        break;
      case DS18B20_10_BIT:
        measure &= ~(DS18B20_10_BIT_UNDF);
        break;
      case DS18B20_11_BIT:
        measure &= ~(DS18B20_11_BIT_UNDF);
        break;
      default: break; /* 12 bit - all bits valid */
    }
  }

  temperaturevalue  = (measure >> 4);
  temperaturevalue *= 10000;
  temperaturevalue +=( measure & 0x000F ) * DS18X20_FRACCONV;

  if(negative) temperaturevalue = -temperaturevalue;

  return temperaturevalue;
}

uint8_t DS18X20_read_maxres(uint8_t id[], int32_t *temperaturevalue) {
  uint8_t sp[DS18X20_SP_SIZE];
  uint8_t ret;

  ow_reset();
  ret = read_scratchpad(id, sp, DS18X20_SP_SIZE);
  if(ret == DS18X20_OK) *temperaturevalue = DS18X20_raw_to_maxres(id[0], sp);
  return ret;
}

uint8_t DS18X20_read_maxres_single(uint8_t familycode, int32_t *temperaturevalue) {
  uint8_t sp[DS18X20_SP_SIZE];
  uint8_t ret;

  ret = read_scratchpad( NULL, sp, DS18X20_SP_SIZE );
  if(ret == DS18X20_OK) *temperaturevalue = DS18X20_raw_to_maxres(familycode, sp);
  return ret;
}

uint8_t DS18X20_format_from_maxres(int32_t temperaturevalue, char str[], uint8_t n) {
  uint8_t sign = 0;
  char temp[10];
  int8_t temp_loc = 0;
  uint8_t str_loc = 0;
  ldiv_t ldt;
  uint8_t ret;

  /* range from -550000: -55.0000°C to 1250000: +125.0000°C -> min. 9 + 1 chars */
  if(n >= (9 + 1) && temperaturevalue > -1000000L && temperaturevalue < 10000000L) {
    if(temperaturevalue < 0) {
      sign = 1;
      temperaturevalue = -temperaturevalue;
    }

    do {
      ldt = ldiv(temperaturevalue, 10);
      temp[temp_loc++] = ldt.rem + '0';
      temperaturevalue = ldt.quot;
    } while(temperaturevalue > 0);

    if(sign) temp[temp_loc] = '-';
    else temp[temp_loc] = '+';

    while(temp_loc >= 0) {
      str[str_loc++] = temp[(uint8_t)temp_loc--];
      if(temp_loc == 3) str[str_loc++] = DS18X20_DECIMAL_CHAR;
    }
    str[str_loc] = '\0';

    ret = DS18X20_OK;
  }
  else ret = DS18X20_ERROR;

  return ret;
}

uint8_t DS18X20_write_scratchpad(uint8_t id[], uint8_t th, uint8_t tl, uint8_t conf) {
  ow_reset();

  ow_command(DS18X20_WRITE_SCRATCHPAD, id);
  ow_byte_wr(th);
  ow_byte_wr(tl);
  if(id[0] == DS18B20_FAMILY_CODE || id[0] == DS1822_FAMILY_CODE) ow_byte_wr(conf); /* config only available on DS18B20 and DS1822 */
  return DS18X20_OK;
}

uint8_t DS18X20_read_scratchpad(uint8_t id[], uint8_t sp[], uint8_t n) {
  ow_reset();

  return read_scratchpad(id, sp, n);
}

uint8_t DS18X20_scratchpad_to_eeprom(uint8_t with_power_extern, uint8_t id[]) {
  ow_reset();

  ow_command(DS18X20_COPY_SCRATCHPAD, id);
  delay_ms(DS18X20_COPYSP_DELAY); /* wait for 10 ms */

  return DS18X20_OK;
}

uint8_t DS18X20_eeprom_to_scratchpad(uint8_t id[]) {
  uint8_t ret;
  uint8_t retry_count = 255;

  ow_reset();

  ow_command(DS18X20_RECALL_E2, id);
  while(retry_count-- && !(ow_bit(1)));
  if(retry_count) ret = DS18X20_OK;
  else ret = DS18X20_ERROR;

  return ret;
}
