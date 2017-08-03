/*
	Copyright 2012-2015 Benjamin Vedder	benjamin@vedder.se

	This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    */

/*
 * servo_simple.c
 *
 *  Created on: 31 jul 2015
 *      Author: benjamin
 */

#include "servo_simple.h"
#include "ch.h"
#include "hal.h"
#include "hw.h"
#include "conf_general.h"
#include "stm32f4xx_conf.h"
#include "utils.h"

// Settings
#define TIM_CLOCK			1000000 // Hz

#if SERVO_OUT_ENABLE && SERVO_OUT_SIMPLE

void servo_simple_init(void) {
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;


	/**
	 * Use the same pin for servo out as the servo in pin, aka the 3 pin connector
	 * on the VESC. Set it to its alternate function which is connected to TIM3
	 */

	// TODO: change this to regular pin on a different pin too
	palSetPadMode(HW_ICU_GPIO, HW_ICU_PIN, PAL_MODE_ALTERNATE(HW_ICU_GPIO_AF) |
			PAL_STM32_OSPEED_HIGHEST | PAL_STM32_PUDR_FLOATING);

	// TODO: figure out if pin has to be on APB1 periph
	// TIM7 clock enable
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);

	/**
	 * Set up the timer for the pwm generation.
	 * Prescaler is set such that the clock operates at 1MHz with a period of
	 * set to match the servo out hz (like 400hz, 1000hz etc)
	 */
	TIM_TimeBaseStructure.TIM_Period = (uint16_t)((uint32_t)TIM_CLOCK / (uint32_t)SERVO_OUT_RATE_HZ);
	TIM_TimeBaseStructure.TIM_Prescaler = (uint16_t)((168000000 / 2) / TIM_CLOCK) - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);

	// Prescaler configuration
	TIM_PrescalerConfig(TIM7, PrescalerValue, TIM_PSCReloadMode_Immediate);

	// Disable ARR buffering
	TIM_ARRPreloadConfig(TIM7, DISABLE);

	// Interrupt generation
	TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);

	// TIM6 enable counter
	TIM_Cmd(TIM7, ENABLE);

	// NVIC
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	servo_simple_set_output(0.5);


}

void servo_simple_set_output(float out) {
	utils_truncate_number(&out, 0.0, 1.0);

	float us = (float)SERVO_OUT_PULSE_MIN_US + out * (float)(SERVO_OUT_PULSE_MAX_US - SERVO_OUT_PULSE_MIN_US);
	us *= (float)TIM_CLOCK / 1000000.0;
	TIM3->CCR2 = (uint32_t)us;
}

#endif
