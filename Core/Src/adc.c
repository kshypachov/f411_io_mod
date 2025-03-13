/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    adc.c
  * @brief   This file provides code for the configuration
  *          of the ADC instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "adc.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

ADC_HandleTypeDef hadc1;

/* ADC1 init function */
void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV8;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_VBAT;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{

  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspInit 0 */

  /* USER CODE END ADC1_MspInit 0 */
    /* ADC1 clock enable */
    __HAL_RCC_ADC1_CLK_ENABLE();
  /* USER CODE BEGIN ADC1_MspInit 1 */

  /* USER CODE END ADC1_MspInit 1 */
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{

  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspDeInit 0 */

  /* USER CODE END ADC1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ADC1_CLK_DISABLE();
  /* USER CODE BEGIN ADC1_MspDeInit 1 */

  /* USER CODE END ADC1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
// VREFINT calibration value address from system memory (typical value is 1.21V at 3.3V VDD)

// Function to measure VREFINT and calculate actual VDD
float Read_VDD(void)
{
    uint32_t raw_value = 0;
    float vrefint_cal = (float)(*VREFINT_CAL_ADDR);  // Read calibration value from system memory

    ADC_ChannelConfTypeDef sConfig = {0};

    // Configure ADC channel to VREFINT
    sConfig.Channel = ADC_CHANNEL_VREFINT;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;  // Longer sampling time for better accuracy
    sConfig.Offset = 0;

    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        // Failed to configure VREFINT channel
        return -1.0;
    }

    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
    {
        raw_value = HAL_ADC_GetValue(&hadc1);  // Read raw ADC value for VREFINT
    }
    HAL_ADC_Stop(&hadc1);

    // Calculate actual VDD based on calibration value and raw ADC reading
    float vdd = 3.3f * (vrefint_cal / raw_value);

    return vdd;
}

// Function to measure VBAT voltage with VDD compensation from VREFINT
float Read_VBAT(void)
{
    uint32_t raw_value = 0;
    float vbat_voltage = 0.0;

    ADC_ChannelConfTypeDef sConfig = {0};

    // 1. Get actual VDD using VREFINT measurement
    float vdd = Read_VDD();
    if (vdd < 0)
    {
        // Failed to get valid VDD measurement
        return -1.0;
    }

    // 2. Configure ADC channel to VBAT (Channel 18)
    sConfig.Channel = ADC_CHANNEL_VBAT;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;  // Longer sampling time for better accuracy
    sConfig.Offset = 0;

    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        // Failed to configure VBAT channel
        return -1.0;
    }

    // 3. Start ADC conversion and get raw VBAT value
    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
    {
        raw_value = HAL_ADC_GetValue(&hadc1);  // Read raw ADC value for VBAT
    }
    HAL_ADC_Stop(&hadc1);

    // 4. Calculate VBAT voltage
    // The VBAT channel has an internal divider by 4
    float adc_resolution = 4095.0f;
    vbat_voltage = ((raw_value / adc_resolution) * vdd) * 4.0f;

    return vbat_voltage;
}

/* USER CODE END 1 */
