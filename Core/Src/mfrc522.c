#include "mfrc522.h"
extern SPI_HandleTypeDef hspi1;

/* RC522 register addresses */
#define CommandReg       0x01
#define ComIEnReg        0x02
#define DivIEnReg        0x03
#define ComIrqReg        0x04
#define DivIrqReg        0x05
#define ErrorReg         0x06
#define Status2Reg       0x08
#define FIFODataReg      0x09
#define FIFOLevelReg     0x0A
#define ControlReg       0x0C
#define BitFramingReg    0x0D
#define ModeReg          0x11
#define TxControlReg     0x14
#define TxASKReg         0x15
#define TModeReg         0x2A
#define TPrescalerReg    0x2B
#define TReloadRegH      0x2C
#define TReloadRegL      0x2D
#define CRCResultRegH    0x21
#define CRCResultRegL    0x22

/* RC522 commands */
#define PCD_Idle         0x00
#define PCD_Mem          0x01
#define PCD_CalcCRC      0x03
#define PCD_Transceive   0x0C
#define PCD_SoftReset    0x0F

/* PICC commands */
#define PICC_REQA        0x26
#define PICC_ANTICOLL    0x93

#define MAX_LEN          16

static uint8_t MFRC522_ReadRegister(uint8_t reg);
static void MFRC522_WriteRegister(uint8_t reg, uint8_t value);
static void MFRC522_SetBitMask(uint8_t reg, uint8_t mask);
static void MFRC522_ClearBitMask(uint8_t reg, uint8_t mask);
static uint8_t MFRC522_Transceive(
    uint8_t *sendData,
    uint8_t sendLen,
    uint8_t *backData,
    uint8_t *backLen
);

/* Read one RC522 register */
static uint8_t MFRC522_ReadRegister(uint8_t reg)
{
    uint8_t address = (uint8_t)((reg << 1) | 0x80);
    uint8_t tx[2] = {address, 0x00};
    uint8_t rx[2] = {0, 0};

    HAL_GPIO_WritePin(MFRC522_CS_PORT, MFRC522_CS_PIN, GPIO_PIN_RESET);

    HAL_SPI_TransmitReceive(&hspi1, tx, rx, 2, HAL_MAX_DELAY);

    HAL_GPIO_WritePin(MFRC522_CS_PORT, MFRC522_CS_PIN, GPIO_PIN_SET);

    return rx[1];
}
uint8_t MFRC522_GetVersion(void)
{
    return MFRC522_ReadRegister(0x37);
}
/* Write one RC522 register */
static void MFRC522_WriteRegister(uint8_t reg, uint8_t value)
{
    uint8_t tx[2];

    tx[0] = (uint8_t)((reg << 1) & 0x7E);
    tx[1] = value;

    HAL_GPIO_WritePin(MFRC522_CS_PORT, MFRC522_CS_PIN, GPIO_PIN_RESET);

    HAL_SPI_Transmit(&hspi1, tx, 2, HAL_MAX_DELAY);

    HAL_GPIO_WritePin(MFRC522_CS_PORT, MFRC522_CS_PIN, GPIO_PIN_SET);
}

/* Set selected register bits */
static void MFRC522_SetBitMask(uint8_t reg, uint8_t mask)
{
    MFRC522_WriteRegister(reg, MFRC522_ReadRegister(reg) | mask);
}

/* Clear selected register bits */
static void MFRC522_ClearBitMask(uint8_t reg, uint8_t mask)
{
    MFRC522_WriteRegister(reg, MFRC522_ReadRegister(reg) & (uint8_t)(~mask));
}

/* Initialize the RC522 */
void MFRC522_Init(void)
{
    HAL_GPIO_WritePin(MFRC522_CS_PORT, MFRC522_CS_PIN, GPIO_PIN_SET);

    HAL_GPIO_WritePin(MFRC522_RST_PORT, MFRC522_RST_PIN, GPIO_PIN_RESET);
    HAL_Delay(5);
    HAL_GPIO_WritePin(MFRC522_RST_PORT, MFRC522_RST_PIN, GPIO_PIN_SET);
    HAL_Delay(5);

    MFRC522_WriteRegister(CommandReg, PCD_SoftReset);
    HAL_Delay(50);

    MFRC522_WriteRegister(TModeReg, 0x8D);
    MFRC522_WriteRegister(TPrescalerReg, 0x3E);
    MFRC522_WriteRegister(TReloadRegL, 30);
    MFRC522_WriteRegister(TReloadRegH, 0);

    MFRC522_WriteRegister(TxASKReg, 0x40);
    MFRC522_WriteRegister(ModeReg, 0x3D);

    MFRC522_SetBitMask(TxControlReg, 0x03);
}

/* Send data and receive response */
static uint8_t MFRC522_Transceive(
    uint8_t *sendData,
    uint8_t sendLen,
    uint8_t *backData,
    uint8_t *backLen)
{
    uint8_t irq;
    uint8_t error;
    uint8_t fifoLevel;
    uint8_t waitCount = 0;

    MFRC522_WriteRegister(CommandReg, PCD_Idle);
    MFRC522_WriteRegister(ComIrqReg, 0x7F);
    MFRC522_WriteRegister(FIFOLevelReg, 0x80);

    for (uint8_t i = 0; i < sendLen; i++)
    {
        MFRC522_WriteRegister(FIFODataReg, sendData[i]);
    }

    MFRC522_WriteRegister(CommandReg, PCD_Transceive);
    MFRC522_SetBitMask(BitFramingReg, 0x80);

    do
    {
        irq = MFRC522_ReadRegister(ComIrqReg);
        waitCount++;
    }
    while (!(irq & 0x30) && waitCount < 200);

    MFRC522_ClearBitMask(BitFramingReg, 0x80);

    if (waitCount >= 200)
    {
        return MFRC522_ERROR;
    }

    error = MFRC522_ReadRegister(ErrorReg);

    if (error & 0x1B)
    {
        return MFRC522_ERROR;
    }

    fifoLevel = MFRC522_ReadRegister(FIFOLevelReg);

    if (fifoLevel > MAX_LEN)
    {
        return MFRC522_ERROR;
    }

    *backLen = fifoLevel;

    for (uint8_t i = 0; i < fifoLevel; i++)
    {
        backData[i] = MFRC522_ReadRegister(FIFODataReg);
    }

    return MFRC522_OK;
}

/* Detect a card and read its 4-byte UID */
uint8_t MFRC522_CheckCard(uint8_t *uid)
{
    uint8_t request[1] = {PICC_REQA};
    uint8_t response[MAX_LEN];
    uint8_t responseLen = 0;

    uint8_t anticoll[2] = {PICC_ANTICOLL, 0x20};
    uint8_t uidResponse[MAX_LEN];
    uint8_t uidLen = 0;

    uint8_t bcc = 0;

    /* Request card: REQA uses 7 bits */
    MFRC522_WriteRegister(BitFramingReg, 0x07);

    if (MFRC522_Transceive(request, 1, response, &responseLen) != MFRC522_OK)
    {
        MFRC522_WriteRegister(BitFramingReg, 0x00);
        return MFRC522_NOTAG;
    }

    MFRC522_WriteRegister(BitFramingReg, 0x00);

    /* Read UID using anti-collision */
    if (MFRC522_Transceive(anticoll, 2, uidResponse, &uidLen) != MFRC522_OK)
    {
        return MFRC522_ERROR;
    }

    if (uidLen < 5)
    {
        return MFRC522_ERROR;
    }

    for (uint8_t i = 0; i < 4; i++)
    {
        uid[i] = uidResponse[i];
        bcc ^= uidResponse[i];
    }

    if (bcc != uidResponse[4])
    {
        return MFRC522_ERROR;
    }

    return MFRC522_OK;
}


