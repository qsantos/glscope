#include <avr/sleep.h>
#include <core/USB.h>

void setup() {
}

void loop() {
    unsigned char v = (unsigned char) (analogRead(0) >> 2);
    USB_Send(CDC_TX, &v, 1);
    delay(10);
}
