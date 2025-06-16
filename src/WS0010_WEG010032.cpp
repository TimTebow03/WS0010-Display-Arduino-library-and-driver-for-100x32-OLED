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
    digitalWrite(_cs2_pin, HIGH); // Disable chip 2
    digitalWrite(_cs1_pin, LOW);  // Enable chip 1

    // First line (Page 0?)
    _writeCommand(SET_DDRAM_ADDR); // Set DDRAM address base (as per original)
    _writeCommand(SET_Y_ADDR_0);   // Set Y address 0
    for (i = 0; i < size; i++) {
        data_byte = pgm_read_byte(&pic_data[0][i]);
        _writeData(data_byte);
    }

    // Second line (Page 1?)
    _writeCommand(SET_DDRAM_ADDR); // Set DDRAM address base
    _writeCommand(SET_Y_ADDR_1);   // Set Y address 1
    for (i = 0; i < size; i++) {
        data_byte = pgm_read_byte(&pic_data[1][i]);
        _writeData(data_byte);
    }

    // --- Chip 2 ---
    digitalWrite(_cs1_pin, HIGH); // Disable chip 1
    digitalWrite(_cs2_pin, LOW);  // Enable chip 2

    // Third line (Page 0?)
    _writeCommand(SET_DDRAM_ADDR); // Set DDRAM address base
    _writeCommand(SET_Y_ADDR_0);   // Set Y address 0
    for (i = 0; i < size; i++) {
        data_byte = pgm_read_byte(&pic_data[2][i]);
        _writeData(data_byte);
    }

    // Fourth line (Page 1?)
    _writeCommand(SET_DDRAM_ADDR); // Set DDRAM address base
    _writeCommand(SET_Y_ADDR_1);   // Set Y address 1
    for (i = 0; i < size; i++) {
        data_byte = pgm_read_byte(&pic_data[3][i]);
        _writeData(data_byte);
    }

    digitalWrite(_cs2_pin, HIGH); // Disable chip 2
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
    //writeInstruction(0x38); // Function set (8-bit interface, basic instruction set)
    _writeCommand(0x38);
    delay(1); // Small delay after instructions
    //writeInstruction(0x38);
    _writeCommand(0x38);
    delay(1);
    // --- Now use writeCommand (with busy check) ---
    _writeCommand(0x38); // Function set (Graphics On selection needs this repeated sometimes)
    _writeCommand(0x08); // Display OFF
    _writeCommand(0x06); // Entry Mode Set (Increment cursor, no display shift)
    // Graphic Mode Enable (Extended Instruction Set must be selected first usually)
    // Let's assume 0x38 was enough based on original code.
    _writeCommand(0x1F); // Set Graphic Mode (Value specific to S0010? Check datasheet if issues)
                        // Often needs Function Set with RE=1 first, but original didn't show that.
    _writeCommand(0x01); // Clear Display
    delay(10);          // Clear display takes longer
    _writeCommand(0x02); // Return Home
    _writeCommand(0x0C); // Display ON

    digitalWrite(_cs1_pin, HIGH); // Deselect chips
    digitalWrite(_cs2_pin, HIGH);
    delay(20); // Delay after initialization
}

void WS0010_Display::_writeCommand(byte cmd)
{
    // Busy check should happen BEFORE sending the next command/data
    // However, the original code checked *after* pulsing E. Let's stick to that for now.
    // checkBusy(); // Check busy *before* sending? Typically yes. Let's test original way first.
    digitalWrite(_rs_pin, LOW); // Instruction mode
    digitalWrite(_rw_pin, LOW); // Write mode
    _setDataBusMode(OUTPUT);    // Ensure data pins are output
    _writeDataBus(cmd);
    _pulseEnable();
    _checkBusy(); // Check busy *after* command sent (as per original)
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
    delayMicroseconds(1); // Min E cycle time is ~1us, pulse width ~450ns
    digitalWrite(_e_pin, HIGH);
    delayMicroseconds(1); // E high needs to be > 450ns
    digitalWrite(_e_pin, LOW);
    delayMicroseconds(1); // E low duration
}

void WS0010_Display::_checkBusy()
{
    pinMode(_busy_pin, INPUT); // Set D7/Busy pin to input
    digitalWrite(_rs_pin, LOW); // Instruction mode
    digitalWrite(_rw_pin, HIGH); // Read mode

    bool busy = true;
    do {
        // Pulse Enable to read the busy flag
        digitalWrite(_e_pin, HIGH);
        delayMicroseconds(1); // Wait for data to be available
        busy = digitalRead(_busy_pin); // Read the busy flag (D7)
        digitalWrite(_e_pin, LOW);
        delayMicroseconds(1); // Add short delay between checks if needed

        // The original code pulsed E repeatedly within the loop.
        // Some controllers might require this to update the busy status output.
        // If the above doesn't work, try pulsing within the loop again.
        // Example alternative pulse inside loop:
        // digitalWrite(E_PIN, HIGH); delayMicroseconds(1);
        // busy = digitalRead(BUSY_PIN);
        // digitalWrite(E_PIN, LOW); delayMicroseconds(1);

    } while (busy);

    digitalWrite(_rw_pin, LOW); // Return to write mode
    pinMode(_busy_pin, OUTPUT); // Set D7/Busy pin back to output
    // Note: setDataBusMode(OUTPUT) is called implicitly by the next write operation
}

void WS0010_Display::_writeData(byte data)
{
    // checkBusy(); // Check busy *before* sending? Typically yes. Let's test original way first.
    digitalWrite(_rs_pin, HIGH); // Data mode
    digitalWrite(_rw_pin, LOW);  // Write mode
    _setDataBusMode(OUTPUT);     // Ensure data pins are output
    _writeDataBus(data);
    _pulseEnable();
    _checkBusy(); // Check busy *after* data sent (as per original)
}