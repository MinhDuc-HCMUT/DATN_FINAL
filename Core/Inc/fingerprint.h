#ifndef INC_FINGER_PRINT_H_
#define INC_FINGER_PRINT_H_

#include "stm32f1xx_hal.h"
uint8_t receive_finger(uint8_t len);
uint8_t receive_finger_match(uint8_t len);
uint8_t receive_finger_search(uint8_t len);
int collect_finger(void);
int img2tz(uint8_t local);
int match(void);
int regmodel(void);
int store(uint8_t ID);
int search(void);
int empty(void);
int search1(void);
int del(uint8_t id);
uint8_t delete_finger(uint8_t id);
uint8_t delete_all_fingers();

#endif
