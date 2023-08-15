#include "helper.h"
uint8_t utf8len(char c) {
  if((c & 0xF0) == 0xF0) {
    return 4;
  } else if((c & 0xE0) == 0xE0) {
    return 3;
  } else if((c & 0xC0) == 0xC0) {
    return 2;
  } else if(!(c & 0x80)) {
    return 1;
  } else {
    return 0;
  }
}