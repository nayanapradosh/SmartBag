
#ifndef MFRC522_H
#define MFRC522_H

#include "main.h"

#define MFRC522_CS_PORT GPIOA
#define MFRC522_CS_PIN  GPIO_PIN_4

#define MFRC522_RST_PORT GPIOB
#define MFRC522_RST_PIN  GPIO_PIN_0

#define MFRC522_OK       0
#define MFRC522_ERROR    1
#define MFRC522_NOTAG    2

void MFRC522_Init(void);
uint8_t MFRC522_CheckCard(uint8_t *uid);
uint8_t MFRC522_GetVersion(void);

#endif
