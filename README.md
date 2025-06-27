To use this Library in the Arduino IDE:
1. Download repo as a ZIP
2. In a new Arduino sketch, navigate to 'Sketch' -> 'Include Library' -> 'Add .ZIP Library...'
3. make sure to then include the header file with `#include <WS0010_WEG010032.h>` at the beginning of your code

To generate the images used in the example.ino file. I used a small program I created to convert a png to the hex array, it can be found here: https://github.com/TimTebow03/WinstarImageConverter

To use this tool:
1. put your 100x32 png into the `InputFolder`
2. run `python main.py <your_image_name>
3. the output folder should contain a .txt file with formatted hex values, this can be directly copied and pasted into the code with the proper declaration:
const unsigned char your_picture_name[4][100] PROGMEM = { *your hex vals* }
