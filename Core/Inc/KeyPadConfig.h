#ifndef	_KEYPADCONFIG_H
#define	_KEYPADCONFIG_H

#define           _KEYPAD_DEBOUNCE_TIME_MS        20
#define           _KEYPAD_USE_FREERTOS            0

const GPIO_TypeDef* _KEYPAD_COLUMN_GPIO_PORT[] =
{
  C1_GPIO_Port,
  C2_GPIO_Port,
  C3_GPIO_Port,
  C4_GPIO_Port
};

const uint16_t _KEYPAD_COLUMN_GPIO_PIN[] =
{
  C1_Pin,
  C2_Pin,
  C3_Pin,
  C4_Pin
};

const GPIO_TypeDef* _KEYPAD_ROW_GPIO_PORT[] =
{
  R1_GPIO_Port,
  R2_GPIO_Port,
  R3_GPIO_Port,
  R4_GPIO_Port
};

const uint16_t _KEYPAD_ROW_GPIO_PIN[] =
{
  R1_Pin,
  R2_Pin,
  R3_Pin,
  R4_Pin,
};

#endif
