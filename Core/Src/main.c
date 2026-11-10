/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "mfrc522.h"
#include "ssd1306.h"
#include <stdio.h>
#include <string.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* Smart Bag subject definitions */

#define SUBJECT_COUNT 5

typedef struct
{
    char name[20];
    uint8_t uid[4];
    uint8_t present;
} Subject;

/*
 * TEMPORARY RFID UIDs
 *
 * These are PLACEHOLDERS.
 *
 * After we read the real UID from each RFID tag,
 * replace these four numbers for each subject.
 */
Subject subjects[SUBJECT_COUNT] =
{
    {
        "Mathematics",
        {0xB3, 0x38, 0x49, 0x56},
        0
    },

    {
        "Physics",
        {0x11, 0x12, 0x13, 0x14},
        0
    },

    {
        "Chemistry",
        {0x21, 0x22, 0x23, 0x24},
        0
    },

    {
        "Computer Science",
        {0x31, 0x32, 0x33, 0x34},
        0
    },

    {
        "English",
        {0x41, 0x42, 0x43, 0x44},
        0
    }
};
/* Compare two 4-byte RFID UIDs */
static uint8_t UID_Matches(uint8_t *uid1, uint8_t *uid2)
{
    for (uint8_t i = 0; i < 4; i++)
    {
        if (uid1[i] != uid2[i])
        {
            return 0;
        }
    }

    return 1;
}
/* Find which subject belongs to an RFID UID */
static int8_t FindSubject(uint8_t *uid)
{
    for (uint8_t i = 0; i < SUBJECT_COUNT; i++)
    {
        if (UID_Matches(uid, subjects[i].uid))
        {
            return i;
        }
    }

    return -1;
}

/* Display all currently detected subjects */
static void DisplaySubjectStatus(void)
{
    SSD1306_Clear();

    SSD1306_SetCursor(0, 0);
    SSD1306_WriteString("SMART BAG");

    for (uint8_t i = 0; i < SUBJECT_COUNT; i++)
    {
        SSD1306_SetCursor(0, i + 1);

        if (subjects[i].present)
        {
            SSD1306_WriteString("OK ");
        }
        else
        {
            SSD1306_WriteString("-- ");
        }

        SSD1306_WriteString(subjects[i].name);
    }

    SSD1306_UpdateScreen();
}

/* Check whether all subjects have been scanned */
static uint8_t AllSubjectsPresent(void)
{
    for (uint8_t i = 0; i < SUBJECT_COUNT; i++)
    {
        if (!subjects[i].present)
        {
            return 0;
        }
    }

    return 1;
}

/* Short buzzer beep */
static void Buzzer_Beep(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);

    HAL_Delay(200);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
}
/* Display that all subjects are present */
static void DisplayBagComplete(void)
{
    SSD1306_Clear();

    SSD1306_SetCursor(0, 1);
    SSD1306_WriteString("SMART BAG");

    SSD1306_SetCursor(0, 3);
    SSD1306_WriteString("BAG COMPLETE");

    SSD1306_SetCursor(0, 5);
    SSD1306_WriteString("All subjects OK");

    SSD1306_UpdateScreen();
}
/* Display an unknown RFID warning */
static void DisplayUnknownTag(void)
{
    SSD1306_Clear();

    SSD1306_SetCursor(0, 1);
    SSD1306_WriteString("SMART BAG");

    SSD1306_SetCursor(0, 3);
    SSD1306_WriteString("UNKNOWN TAG");

    SSD1306_SetCursor(0, 5);
    SSD1306_WriteString("Not registered");

    SSD1306_UpdateScreen();

    Buzzer_Beep();
}
/* Display the RFID UID on the OLED */
static void DisplayUID(uint8_t *uid)
{
    char uidText[20];

    SSD1306_Clear();

    SSD1306_SetCursor(0, 0);
    SSD1306_WriteString("RFID TAG");

    SSD1306_SetCursor(0, 2);
    SSD1306_WriteString("UID:");

    sprintf(uidText, "%02X %02X %02X %02X",
            uid[0], uid[1], uid[2], uid[3]);

    SSD1306_SetCursor(0, 4);
    SSD1306_WriteString(uidText);

    SSD1306_UpdateScreen();
}
    /* Check for missing books */
    static void CheckMissingBooks(void)
    {
        uint8_t missingFound = 0;

        for (uint8_t i = 0; i < SUBJECT_COUNT; i++)
        {
            if (subjects[i].present == 0)
            {
                missingFound = 1;

                SSD1306_Clear();

                SSD1306_SetCursor(0, 0);
                SSD1306_WriteString("MISSING BOOK");

                SSD1306_SetCursor(0, 2);
                SSD1306_WriteString(subjects[i].name);

                SSD1306_UpdateScreen();

                /* Buzzer alert */
                Buzzer_Beep();

                HAL_Delay(500);
            }
        }

        if (!missingFound)
        {
            DisplayBagComplete();
        }
    }
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  MFRC522_Init();

    uint8_t version = MFRC522_GetVersion();

    SSD1306_Init();

    SSD1306_Clear();
    SSD1306_SetCursor(0, 0);
    SSD1306_WriteString("RFID TEST");

    SSD1306_SetCursor(0, 2);

    if (version == 0x91 || version == 0x92)
    {
        SSD1306_WriteString("RFID OK");
    }
    else
    {
        SSD1306_WriteString("RFID ERROR");
    }

    SSD1306_UpdateScreen();
    HAL_Delay(10000);

  SSD1306_SetCursor(0, 0);
  SSD1306_WriteString("SMART BAG");

  SSD1306_SetCursor(0, 2);
  SSD1306_WriteString("Initializing...");

  SSD1306_UpdateScreen();

  HAL_Delay(2000);

  DisplaySubjectStatus();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      uint8_t uid[4];

      /* Time of the most recent RFID card scan */
      static uint32_t lastCardTime = 0;

      /* Becomes 1 after at least one card has been scanned */
      static uint8_t scanningBag = 0;

      /* RFID card scanning */
      if (MFRC522_CheckCard(uid) == MFRC522_OK)
      {
          int8_t subjectIndex = FindSubject(uid);

          if (subjectIndex >= 0)
          {
              SSD1306_Clear();

              SSD1306_SetCursor(0, 0);
              SSD1306_WriteString("REGISTERED");

              SSD1306_SetCursor(0, 2);
              SSD1306_WriteString(subjects[subjectIndex].name);

              subjects[subjectIndex].present = 1;

              SSD1306_UpdateScreen();

              HAL_Delay(1000);

              DisplaySubjectStatus();

              /* Start/restart the bag checking timer */
              lastCardTime = HAL_GetTick();
              scanningBag = 1;
          }
          else
          {
              DisplayUnknownTag();

              HAL_Delay(1000);

              DisplaySubjectStatus();
          }
      }

      /*
       * After 5 seconds without scanning another card,
       * automatically check for missing books.
       */
      if (scanningBag)
      {
          if ((HAL_GetTick() - lastCardTime) >= 5000)
          {
              scanningBag = 0;

              CheckMissingBooks();

              HAL_Delay(2000);

              DisplaySubjectStatus();
          }
      }

      HAL_Delay(100);
  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(RFID_CS_GPIO_Port, RFID_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(RFID_RST_GPIO_Port, RFID_RST_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : RFID_CS_Pin */
  GPIO_InitStruct.Pin = RFID_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(RFID_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : RFID_RST_Pin BUZZER_Pin */
  GPIO_InitStruct.Pin = RFID_RST_Pin|BUZZER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
