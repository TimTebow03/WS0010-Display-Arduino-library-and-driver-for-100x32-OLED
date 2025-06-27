#ifndef WS0010_WEG010032_H
#define WS0010_WEG010032_H

#include <Arduino.h>
#include <avr/pgmspace.h>

#define WS0010_WIDTH 100
#define WS0010_HIGHT 32
#define SET_DDRAM_ADDR 0x80
#define SET_Y_ADDR_0   0x40
#define SET_Y_ADDR_1   0x41

// Function Set
// 0 0 1 1(8 bit select) 1(2 Line Display) 0(fonts *n/a*) 0(fonts *n/a*) 0(fonts *n/a*)
#define FUNCTION_SET 0x38

// Entry Mode Set
// 0 0 0 0 0 1 1(Increment DDRAM by 1) 0(Display Shift)
#define ENTRY_MODE_SET 0x06

// Cursor/Display Shift Instruction
// 0 0 0 1 1(Graphics mode) 1(Internal power on) 1 1
#define SET_GRAPHICS_MODE 0x1F

#define CLEAR_DISPLAY 0x01
#define SWITCH_DISPLAY_ON 0x0C
#define SWITCH_DISPLAY_OFF 0x08
#define RETURN_HOME 0x02

class WS0010_Display {
public:
    WS0010_Display(const uint8_t db_pins[], uint8_t rs_pin,
                    uint8_t rw_pin, uint8_t e_pin, uint8_t cs1_pin, uint8_t cs2_pin, uint8_t busy_pin);
    
    void begin();
    void showPic(const unsigned char pic_data[][100], unsigned char size);
    void clearDisplay();
    void switchDisplayOn();
    void switchDisplayOff();


private:
    uint8_t _db_pins[8];
    uint8_t _rs_pin;
    uint8_t _rw_pin;
    uint8_t _e_pin;
    uint8_t _cs1_pin;
    uint8_t _cs2_pin;
    uint8_t _busy_pin;

    void _init();
    void _pulseEnable();
    void _setDataBusMode(uint8_t mode);
    void _writeDataBus(byte data);
    void _checkBusy();
    void _writeCommand(byte cmd);
    void _writeData(byte data);
    void _selectController(int x_coord);
};

#endif //WS0010_WEG010032_H