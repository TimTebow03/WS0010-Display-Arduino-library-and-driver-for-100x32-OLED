#include "WS0010_WEG010032.h"

WS0010_Display::WS0010_Display(const uint8_t db_pins[], uint8_t rs_pin, uint8_t rw_pin, uint8_t e_pin, uint8_t cs1_pin, uint8_t cs2_pin, uint8_t busy_pin)
    : _rs_pin(rs_pin), _rw_pin(rw_pin), _e_pin(e_pin), _cs1_pin(cs1_pin), _cs2_pin(cs2_pin), _busy_pin(busy_pin) {
    // Copy DB_PINS array
    for (int i = 0; i < 8; i++) {
        _db_pins[i] = db_pins[i];
    }
}

void WS0010_Display::showPic(const unsigned char pic_data[][100], unsigned char size)
{
    unsigned char i;
    byte data_byte;

    // --- Chip 1 ---
    // Disable chip 2 and enable chip 1
    digitalWrite(_cs2_pin, HIGH);
    digitalWrite(_cs1_pin, LOW);

    // First line
    // Set DDRAM address to 0 (base) amd Y address to 0
    _writeCommand(SET_DDRAM_ADDR);
    _writeCommand(SET_Y_ADDR_0);
    for (i = 0; i < size; i++) {
        data_byte = pgm_read_byte(&pic_data[0][i]);
        _writeData(data_byte);
    }

    // Second line
    // Set DDRAM address to 0 (base) amd Y address to 1
    _writeCommand(SET_DDRAM_ADDR);
    _writeCommand(SET_Y_ADDR_1);
    for (i = 0; i < size; i++) {
        data_byte = pgm_read_byte(&pic_data[1][i]);
        _writeData(data_byte);
    }

    // --- Chip 2 ---
    // Disable chip 1 and enable chip 2
    digitalWrite(_cs1_pin, HIGH);
    digitalWrite(_cs2_pin, LOW);

    // Third line
    // Set DDRAM address to 0 (base) amd Y address to 0
    _writeCommand(SET_DDRAM_ADDR);
    _writeCommand(SET_Y_ADDR_0);
    for (i = 0; i < size; i++) {
        data_byte = pgm_read_byte(&pic_data[2][i]);
        _writeData(data_byte);
    }

    // Fourth line
    // Set DDRAM address to 0 (base) amd Y address to 1
    _writeCommand(SET_DDRAM_ADDR);
    _writeCommand(SET_Y_ADDR_1);
    for (i = 0; i < size; i++) {
        data_byte = pgm_read_byte(&pic_data[3][i]);
        _writeData(data_byte);
    }

    // Disable chip 2
    digitalWrite(_cs2_pin, HIGH); 
}


void WS0010_Display::init()
{
    // Set control pins to output
    pinMode(_rs_pin, OUTPUT);
    pinMode(_rw_pin, OUTPUT);
    pinMode(_e_pin, OUTPUT);
    pinMode(_cs1_pin, OUTPUT);
    pinMode(_cs2_pin, OUTPUT);

    // Initial pin states
    digitalWrite(_e_pin, LOW);
    digitalWrite(_cs1_pin, HIGH); // Deselect initially
    digitalWrite(_cs2_pin, HIGH); // Deselect initially
    digitalWrite(_rw_pin, LOW);

    delay(50); // Wait for OLED power-up

    // Select both chips for initial commands
    digitalWrite(_cs1_pin, LOW);
    digitalWrite(_cs2_pin, LOW);
}

void WS0010_Display::begin()
{
    _writeCommand(FUNCTION_SET); // Function set (8-bit interface, basic instruction set)
    _writeCommand(FUNCTION_SET); // Function set (8-bit interface, basic instruction set)
    _writeCommand(FUNCTION_SET); // Function set (8-bit interface, basic instruction set)
    _writeCommand(SWITCH_DISPLAY_OFF); // Display OFF
    _writeCommand(ENTRY_MODE_SET); // Entry Mode Set (Increment cursor, no display shift)
    _writeCommand(SET_GRAPHICS_MODE); // Set Graphic Mode
    _writeCommand(CLEAR_DISPLAY); // Clear Display *THIS SEEMS TO DO NOTHING*
    delay(10); // this makes sure clear display finishes to my understanding
    _writeCommand(RETURN_HOME); // Return Home *THIS SEEMS TO DO NOTHING*
    _writeCommand(SWITCH_DISPLAY_ON); // Display ON

    digitalWrite(_cs1_pin, HIGH); // Deselect chips
    digitalWrite(_cs2_pin, HIGH);
    delay(20); // Delay after initialisation
}

void WS0010_Display::clearDisplay()
{
    digitalWrite(_cs1_pin, LOW);
    digitalWrite(_cs2_pin, LOW);
    _writeCommand(CLEAR_DISPLAY);
    digitalWrite(_cs1_pin, HIGH);
    digitalWrite(_cs2_pin, HIGH);
}

void WS0010_Display::switchDisplayOn()
{
    digitalWrite(_cs1_pin, LOW);
    digitalWrite(_cs2_pin, LOW);
    _writeCommand(SWITCH_DISPLAY_ON);
    digitalWrite(_cs1_pin, HIGH);
    digitalWrite(_cs2_pin, HIGH);
}

void WS0010_Display::switchDisplayOff()
{
    digitalWrite(_cs1_pin, LOW);
    digitalWrite(_cs2_pin, LOW);
    _writeCommand(SWITCH_DISPLAY_OFF);
    digitalWrite(_cs1_pin, HIGH);
    digitalWrite(_cs2_pin, HIGH);
}

void WS0010_Display::_writeCommand(byte cmd)
{
    /* write command is interesting but no issues yet,
       the busy flag check happens at the end of the function
       but it would make more sense for it to happen at the beginning?
    */

    digitalWrite(_rs_pin, LOW); // Instruction mode
    digitalWrite(_rw_pin, LOW); // Write mode
    _setDataBusMode(OUTPUT); // Ensure data pins are output
    _writeDataBus(cmd);
    _pulseEnable();
    _checkBusy();
}

void WS0010_Display::_setDataBusMode(uint8_t mode)
{
    for (int i = 0; i < 8; i++) {
        pinMode(_db_pins[i], mode);
    }
}

void WS0010_Display::_writeDataBus(byte data)
{
    for (int i = 0; i < 8; i++) {
        digitalWrite(_db_pins[i], (data >> i) & 0x01 ? HIGH : LOW);
    }
}

void WS0010_Display::_pulseEnable()
{
    digitalWrite(_e_pin, LOW);
    delayMicroseconds(1); // Wait (similar to _nop_)
    digitalWrite(_e_pin, HIGH);
    delayMicroseconds(1); // Wait
    digitalWrite(_e_pin, LOW);
    delayMicroseconds(1); // Wait
}

void WS0010_Display::_checkBusy()
{
    pinMode(_busy_pin, INPUT); // Set Busy pin to input
    digitalWrite(_rs_pin, LOW); // Instruction mode
    digitalWrite(_rw_pin, HIGH); // Read mode

    bool busy = true;
    do {
        // Pulse Enable to read the busy flag
        digitalWrite(_e_pin, HIGH);
        delayMicroseconds(1); // Wait (similar to _nop_)
        busy = digitalRead(_busy_pin); // Read the busy flag (D7)
        digitalWrite(_e_pin, LOW);
        delayMicroseconds(1); // Wait (similar to _nop_)
    } while (busy);

    digitalWrite(_rw_pin, LOW); // Return to write mode
    pinMode(_busy_pin, OUTPUT); // Set Busy pin back to output
}

void WS0010_Display::_writeData(byte data)
{
    digitalWrite(_rs_pin, HIGH); // Data mode
    digitalWrite(_rw_pin, LOW);  // Write mode
    _setDataBusMode(OUTPUT);
    _writeDataBus(data);
    _pulseEnable();
    _checkBusy();
}