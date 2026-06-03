#include "checo/avr/spi.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#define SHIFT_REGISTER_PORT PORTB
#define SHIFT_REGISTER_DATA_PIN PB2
#define SHIFT_REGISTER_CLOCK_PIN PB1

#define DIGIT_GROUND_PORT PORTB
#define DIGIT1_GROUND_PIN PB3
#define DIGIT2_GROUND_PIN PB4
#define DIGIT3_GROUND_PIN PB0

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

using ShiftRegisterSpi = checo::avr::SoftSpi<checo::avr::PortB, PB2, checo::avr::PortB, PB1>;
ShiftRegisterSpi shiftRegister;

/*checo::avr::SoftSpi shiftRegister{SHIFT_REGISTER_PORT, SHIFT_REGISTER_DATA_PIN, SHIFT_REGISTER_PORT,
    SHIFT_REGISTER_CLOCK_PIN};*/

static inline void displayDigit(const uint8_t digit)
{
    shiftRegister.shiftOut(DIGIT_SEGMENTS[digit]);
}

static void displayNumber(const uint16_t number)
{
    // Display the first digit (rightmost)
    DIGIT_GROUND_PORT |= (1 << DIGIT1_GROUND_PIN);
    displayDigit(number % 10);
    _delay_ms(3);
    DIGIT_GROUND_PORT &= ~(1 << DIGIT1_GROUND_PIN);

    // Display the second digit
    DIGIT_GROUND_PORT |= (1 << DIGIT2_GROUND_PIN);
    displayDigit((number / 10) % 10);
    _delay_ms(3);
    DIGIT_GROUND_PORT &= ~(1 << DIGIT2_GROUND_PIN);

    // Display the third digit
    DIGIT_GROUND_PORT |= (1 << DIGIT3_GROUND_PIN);
    displayDigit((number / 100) % 10);
    _delay_ms(3);
    DIGIT_GROUND_PORT &= ~(1 << DIGIT3_GROUND_PIN);
}

int main()
{
    // Configure controller pins
    DDRB |= (1 << SHIFT_REGISTER_DATA_PIN);
    DDRB |= (1 << SHIFT_REGISTER_CLOCK_PIN);
    DDRB |= (1 << DIGIT1_GROUND_PIN);
    DDRB |= (1 << DIGIT2_GROUND_PIN);
    DDRB |= (1 << DIGIT3_GROUND_PIN);

    // Turn off all digits
    DIGIT_GROUND_PORT &= ~(1 << DIGIT1_GROUND_PIN);
    DIGIT_GROUND_PORT &= ~(1 << DIGIT2_GROUND_PIN);
    DIGIT_GROUND_PORT &= ~(1 << DIGIT3_GROUND_PIN);

    // Loop through digits 0-9 and display them on the 7-segment display
    int num = 0;
    int cycle = 0;
    while (true) {
        displayNumber(num);

        ++cycle;
        if (cycle >= 50) {
            ++num;
            cycle = 0;
        }
    }

    // This will never be reached, but it's good practice to return something from main
    return 0;
}
