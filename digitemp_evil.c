#include <stdio.h>
#include <stdlib.h>

#include "onewire.h"
#include "devices/ds18x20.h"
#include "devices/common.h"

uint8_t id[OW_ROMCODE_SIZE];

char *get_type_by_id(uint8_t id) {
  switch (id) {
    case DS18S20_FAMILY_CODE: return "DS18S20";
    case DS18B20_FAMILY_CODE: return "DS18B20";
    case DS1822_FAMILY_CODE : return "DS1822";
    default: return "WTF?!";
  }
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    puts("Path to COM port required.\n");
    return 1;
  }
  if (ow_init(argv[1])) {
    puts("Bus INIT failed. Check COM port.\n");
    return 1;
  }

  uint8_t c = 0, diff = OW_SEARCH_FIRST;
  int16_t temp_dc;

  while (diff != OW_LAST_DEVICE) {
    DS18X20_find_sensor(&diff, id);
    if (diff == OW_ERR_PRESENCE) {
      puts("All sensors are offline now.\n");
      ow_finit();
      return 1;
    }
    if (diff == OW_ERR_DATA) {
      puts("Bus error.\n");
      ow_finit();
      return 1;
    }
    fprintf(stdout, "Bus %s Device %03u Type 0x%02hx (%s) ID %02hx%02hx%02hx%02hx%02hx%02hx CRC 0x%02hx ", \
           argv[1], c, id[0], get_type_by_id(id[0]), id[6], id[5], id[4], id[3], id[2], id[1], id[7]);
    fflush(stdout);
    c ++;

    if (DS18X20_start_meas(DS18X20_POWER_EXTERN, NULL) == DS18X20_OK) {
      while (DS18X20_conversion_in_progress() == DS18X20_CONVERTING) {
        delay_ms(100); /* It will take a while */
      }
      if (DS18X20_read_decicelsius(id, &temp_dc) == DS18X20_OK) {
        /* Copied from my MCU code, so no float point */
        fprintf(stdout, "TEMP %3d.%01d C\n", temp_dc / 10, temp_dc > 0 ? temp_dc % 10 : -temp_dc % 10);
        continue;
      }
    }

    puts("MEASURE FAILED!\n");

  }
  puts("Sensors listed.\n");

  ow_finit();
  return 0;
}
