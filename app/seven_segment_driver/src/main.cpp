/*
 * MIT License
 *
 * Copyright (c) 2026 Sergei Cherevichko
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "checo/avr/spi.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <util/atomic.h>

#include <assert.h>

// Pins used to input number to be displayed
#define INPUT_DATA_PIN PB3
#define INPUT_CLOCK_PIN PB4
#define INPUT_CLOCK_PIN_INTERRUPT PCINT4

// Shift register controls all 7 segments and also grounds of 2nd and 3rd digit (this requires shift register with
// forwarding pin, we will use it as 9th output)
#define SHIFT_REGISTER_DATA_PIN PB2
#define SHIFT_REGISTER_CLOCK_PIN PB1

// Directly from the MCU we control only the 1st digit (the least significant one)
#define DIGIT_GROUND_PORT PORTB
#define DIGIT1_GROUND_PIN PB0

// Interval for switching between digits
#define DIGIT_REFRESH_INTERVAL_MS 3

using ShiftRegisterSpi =
    checo::avr::SoftSpi<checo::avr::PortB, SHIFT_REGISTER_DATA_PIN, checo::avr::PortB, SHIFT_REGISTER_CLOCK_PIN>;

constexpr uint8_t DIGIT_SEGMENTS[10] = {
    0b11111100, // 0
    0b01100000, // 1
    0b11011010, // 2
    0b11110010, // 3
    0b01100110, // 4
    0b10110110, // 5
    0b10111110, // 6
    0b11100000, // 7
    0b11111110, // 8
    0b11110110  // 9
};

constexpr uint16_t NUMBER_MASK = 0b0000'0011'1111'1111;
volatile uint16_t number{0};
volatile uint8_t displayBuffer[3] = {DIGIT_SEGMENTS[0], DIGIT_SEGMENTS[0], DIGIT_SEGMENTS[0]};

static void configureMcu()
{
    // Configure pins
    DDRB &= ~(1 << INPUT_DATA_PIN);
    DDRB |= (1 << SHIFT_REGISTER_DATA_PIN);
    DDRB |= (1 << SHIFT_REGISTER_CLOCK_PIN);
    DDRB |= (1 << DIGIT1_GROUND_PIN);

    // Configure interrupts to input number to be displayed
    PORTB |= (1 << INPUT_CLOCK_PIN);
    DDRB &= ~(1 << INPUT_CLOCK_PIN);
    GIMSK |= (1 << PCIE);
    PCMSK |= (1 << INPUT_CLOCK_PIN_INTERRUPT);

    // Configure timer to refresh each digit (interval 3ms)
    TCCR0A = (1 << WGM01);
    TCCR0B = (1 << CS02) | (1 << CS00); // Prescaler 1024
    constexpr uint32_t rawTimerValue = (((F_CPU / 1024UL) * DIGIT_REFRESH_INTERVAL_MS) / 1000UL - 1UL);
    static_assert(rawTimerValue <= 255UL, "Timer overflow! OCR0A value is larger than 255. Increase prescaler.");
    static_assert(rawTimerValue > 0UL, "Timer underflow! OCR0A value is 0 or negative. Decrease prescaler.");
    OCR0A = static_cast<uint8_t>(rawTimerValue);
    TIMSK0 |= (1 << OCIE0A);

    // Enable interrupts globally
    sei();
}

// Update the number each time we get high level on INPUT_CLOCK_PIN. SPI protocol is used
ISR(PCINT0_vect)
{
    if (PINB & (1 << INPUT_CLOCK_PIN)) {
        uint16_t tempNumber = number;
        tempNumber <<= 1;
        if (PINB & (1 << INPUT_DATA_PIN)) {
            tempNumber |= 1;
        }
        number = tempNumber & NUMBER_MASK;
    }
}

// Update digits visibility within specified intervals of time
ISR(TIM0_COMPA_vect)
{
    static ShiftRegisterSpi shiftRegister;
    static uint8_t digitNum{0};

    if (digitNum == 0) {
        DIGIT_GROUND_PORT |= (1 << DIGIT1_GROUND_PIN);

        shiftRegister.shiftOut(0, 1);
        shiftRegister.shiftOut(DIGIT_SEGMENTS[number % 10]);

        digitNum = 1;
    } else if (digitNum == 1) {
        DIGIT_GROUND_PORT &= ~(1 << DIGIT1_GROUND_PIN);

        const uint8_t digit2 = (number / 10) % 10;
        shiftRegister.shiftOut(1, 1);
        shiftRegister.shiftOut(DIGIT_SEGMENTS[digit2]);

        digitNum = 2;
    } else {
        DIGIT_GROUND_PORT &= ~(1 << DIGIT1_GROUND_PIN);

        const uint8_t digit3 = (number / 100) % 10;
        shiftRegister.shiftOut(0, 1);
        shiftRegister.shiftOut(DIGIT_SEGMENTS[digit3] | 1);

        digitNum = 0;
    }
}

int main()
{
    // Configure controller
    configureMcu();

    // Display selected number
    uint16_t lastProcessedNumber{0xFFFF};
    while (true) {
        // Copy number to some local variable to prevent accidental modification
        uint16_t localNumber{0};
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            localNumber = number;
        }

        // Prepare data to be displayed when the number was really changed
        if (localNumber != lastProcessedNumber) {
            lastProcessedNumber = localNumber;

            uint8_t n = localNumber % 10;
            displayBuffer[0] = DIGIT_SEGMENTS[n];

            localNumber /= 10;
            n = localNumber % 10;
            displayBuffer[1] = DIGIT_SEGMENTS[n];

            localNumber /= 10;
            n = localNumber % 10;
            displayBuffer[2] = DIGIT_SEGMENTS[n] | 1; // 1 in the end to enable the 3rd digit
        }

        // Sleep until next interrupt
        set_sleep_mode(SLEEP_MODE_IDLE);
        sleep_enable();
        sleep_cpu();
        sleep_disable();
    }

    // This will never be reached, but it's good practice to return something from main
    return 0;
}
