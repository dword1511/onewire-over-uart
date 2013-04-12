#include "onewire.h"
#include <stdio.h>

uint8_t id[OW_ROMCODE_SIZE];

int main(int argc, char *argv[]) {
  if(argc != 2) return 1;
  ow_init(argv[1]);

  uint8_t c = 0, diff = OW_SEARCH_FIRST;

  while(diff != OW_LAST_DEVICE) {
    diff = ow_rom_search(diff, id);
    if(diff == OW_ERR_PRESENCE) {
      puts("All devices are offline now.\n");
      ow_finit();
      return 1;
    }
    if(diff == OW_ERR_DATA) {
      puts("Bus error.\n");
      ow_finit();
      return 1;
    }
    printf("Bus %s Device %03u Type 0x%02hx ID %02hx%02hx%02hx%02hx%02hx%02hx CRC 0x%02hx\n", \
           argv[1], c, id[0], id[6], id[5], id[4], id[3], id[2], id[1], id[7]);
    c ++;
  }
  puts("Bus listed.\n");

  ow_finit();
  return 0;
}