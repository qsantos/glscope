#include <core/c_USB.h>

#define a_buffer 32
byte buffer0[a_buffer];
byte buffer1[a_buffer];
byte* input_buffer = buffer0;
byte* output_buffer = buffer1;
int n_buffer;

void setup() {
    /* The following sets up ADC (Analog to Digital Conversion)
     *
     * ADC is the module from the MCU (MicroController Unit, i.e. the ATmega
     * within the Arduino board) that lets you run analogRead(). It can be
     * configured extensively (inputs, speed, precision, power consumption).
     *
     * The documentation comes from Atmel's ATmega16U4/ATmega32U4 complete datasheet
     * available at http://www.atmel.com/devices/ATMEGA32U4.aspx
     *
     * For reference Arduino's analogRead(0) sets:
     * ADMUX = (0<<REFS1) | (1<<REFS0) | (0<<ADLAR) | B111;
     * ADCSRA = (1<<ADEN) | (0<<ADSC) | (0<<ADATE) | (1<<ADIF) | (0<<ADIE) | B011;
     * ADCSRC = (1<<ADHSM) | (0<<MUX5) | B000;
     */

    /* 24.9.1 ADC Multiplexer Selection Register – ADMUX
     * (page 313)
     *
     * Flag REFS1 REFS0 ADLAR MUX4 MUX3 MUX2 MUX1 MUX0
     * Bit      7     6     5    4    3    2    1    0
     *
     * • Bit 7:6 – REFS1:0: Reference Selection Bits
     * These bits select the voltage reference for the ADC, as shown in the
     * table. If these bits are changed during a conversion, the change will
     * not go in effect until this conversion is complete (ADIF in ADCSRA is
     * set). The internal voltage reference options may not be used if an
     * external reference voltage is being applied to the AREF pin.
     *
     * Table 24-3. Voltage Reference Selections for ADC
     * REFS1 REFS0 Voltage Reference Selection
     *     0     0 AREF, Internal V REF turned off
     *     0     1 AV CC with external capacitor on AREF pin
     *     1     0 Reserved
     *     1     1 Internal 2.56V Voltage Reference with external capacitor on
     *             AREF pin
     *
     * • Bit 5 – ADLAR: ADC Left Adjust Result
     *
     * The ADLAR bit affects the presentation of the ADC conversion result in
     * the ADC Data Register. Write one to ADLAR to left adjust the result.
     * Otherwise, the result is right adjusted. Changing the ADLAR bit will
     * affect the ADC Data Register immediately, regardless of any ongoing
     * conversions. For a complete description of this bit, see “The ADC Data
     * Register – ADCL and ADCH” on page 316.
     *
     * • Bits 4:0 – MUX4:0: Analog Channel Selection Bits
     *
     * The value of these bits selects which combination of analog inputs are
     * connected to the ADC. These bits also select the gain for the
     * differential channels as shown in the table. If these bits are changed
     * during a conversion, the change will not go in effect until this
     * conversion is complete (ADIF in ADCSRA is set).
     *
     * • Bits 4:0 – MUX4:0: Analog Channel Selection Bits
     *
     * The value of these bits selects which combination of analog inputs are
     * connected to the ADC. These bits also select the gain for the
     * differential channels as shown in the table. If these bits are changed
     * during a conversion, the change will not go in effect until this
     * conversion is complete (ADIF in ADCSRA is set).
     *
     * MUX5..0  Input
     * 000000   ADC0
     * 000001   ADC1
     * 000010   N/A
     * 000011   N/A
     * 000100   ADC4
     * 000101   ADC5
     * 000110   ADC6
     * 000111   ADC7
     * [rest of table skipped]
     * Note: MUX5 bit makes part of ADCSRB register.
     */
    ADMUX = (0<<REFS1) | (1<<REFS0) | (1<<ADLAR) | B00111;

    /* 24.9.2 ADC Control and Status Register A – ADCSRA
     * (page 315)
     *
     * Flag ADEN ADSC ADATE ADIF ADIE ADPS2 ADPS1 ADPS0
     * Bit     7    6     5    4    3     2     1     0
     *
     * • Bit 7 – ADEN: ADC Enable
     *
     * Writing this bit to one enables the ADC. By writing it to zero, the ADC
     * is turned off. Turning the ADC off while a conversion is in progress,
     * will terminate this conversion.
     *
     * • Bit 6 – ADSC: ADC Start Conversion
     *
     * In Single Conversion mode, write this bit to one to start each
     * conversion. In Free Running mode, write this bit to one to start the
     * first conversion. The first conversion after ADSC has been written after
     * the ADC has been enabled, or if ADSC is written at the same time as the
     * ADC is enabled, will take 25 ADC clock cycles instead of the normal 13.
     * This first conversion performs initialization of the ADC. ADSC will read
     * as one as long as a conversion is in progress. When the conversion is
     * complete, it returns to zero. Writing zero to this bit has no effect.
     *
     * • Bit 5 – ADATE: ADC Auto Trigger Enable
     *
     * When this bit is written to one, Auto Triggering of the ADC is enabled.
     * The ADC will start a conversion on a positive edge of the selected
     * trigger signal. The trigger source is selected by setting the ADC
     * Trigger Select bits, ADTS in ADCSRB.
     *
     * • Bit 4 – ADIF: ADC Interrupt Flag
     *
     * This bit is set when an ADC conversion completes and the Data Registers
     * are updated. The ADC Conversion Complete Interrupt is executed if the
     * ADIE bit and the I-bit in SREG are set. ADIF is cleared by hardware when
     * executing the corresponding interrupt handling vector. Alternatively,
     * ADIF is cleared by writing a logical one to the flag. Beware that if
     * doing a Read-Modify-Write on ADCSRA, a pending interrupt can be
     * disabled. This also applies if the SBI and CBI instructions are used.
     *
     * • Bit 3 – ADIE: ADC Interrupt Enable
     *
     * When this bit is written to one and the I-bit in SREG is set, the ADC
     * Conversion Complete Interrupt is activated.
     *
     * • Bits 2:0 – ADPS2:0: ADC Prescaler Select Bits
     *
     * These bits determine the division factor between the XTAL frequency and
     * the input clock to the ADC.
     *
     * ADPS2 ADPS1 ADPS0 Division Factor
     *     0     0     0               2
     *     0     0     1               2
     *     0     1     0               4
     *     0     1     1               8
     *     1     0     0              16
     *     1     0     1              32
     *     1     1     0              64
     *     1     1     1             128
     */
    ADCSRA = (1<<ADEN) | (0<<ADSC) | (1<<ADATE) | (0<<ADIF) | (1<<ADIE) | B011;

    /* 24.9.4 ADC Control and Status Register B – ADCSRB
     * (page 317)
     *
     * Flag ADHSM ACME MUX5   – ADTS3 ADTS2 ADTS1 ADTS0
     * Bit      7    6    5   4     3     2     1     0
     *
     * • Bit 7 – ADHSM: ADC High Speed Mode
     *
     * Writing this bit to one enables the ADC High Speed mode. This mode
     * enables higher conversion rate at the expense of higher power
     * consumption.
     *
     * • Bit 5 – MUX5: Analog Channel Additional Selection Bits
     *
     * This bit make part of MUX5:0 bits of ADRCSRB and ADMUX register, that
     * select the combination of analog inputs connected to the ADC (including
     * differential amplifier configuration).
     *
     * • Bit 3:0 – ADTS3:0: ADC Auto Trigger Source
     *
     * If ADATE in ADCSRA is written to one, the value of these bits selects
     * which source will trigger an ADC conversion. If ADATE is cleared, the
     * ADTS3:0 settings will have no effect. A conversion will be triggered by
     * the rising edge of the selected interrupt flag. Note that switching from
     * a trigger source that is cleared to a trigger source that is set, will
     * generate a positive edge on the trigger signal. If ADEN in ADCSRA is
     * set, this will start a conversion. Switching to Free Running mode
     * (ADTS[3:0]=0) will not cause a trigger event, even if the ADC Interrupt
     * Flag is set.
     */
    ADCSRB = (1<<ADHSM) | (0<<ACME) | (0<<MUX5) | B0000;

    sei();  // enable interrupts
    ADCSRA |= 1<<ADSC;  // Start Conversion
}

void loop() {
    if (n_buffer >= a_buffer) {
        // swap buffers
        byte* tmp = input_buffer;
        input_buffer = output_buffer;
        output_buffer = tmp;
        n_buffer = 0;

        USB_Send(CDC_TX, output_buffer, a_buffer);
    }
}

ISR(ADC_vect) {
    if (n_buffer < a_buffer) {
        input_buffer[n_buffer] = ADCH;
        n_buffer += 1;
    }
}
