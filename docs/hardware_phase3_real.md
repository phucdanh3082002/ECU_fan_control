# Phase 3 Real Hardware Wiring

This document describes the current real-hardware setup for the 2-wire DC fan. The fan has no tachometer wire, so GPIO27 is not connected and real RPM/fan-stall diagnostics are not available unless an external tach sensor is added later.

## ESP32 Pin Map

| ESP32 Pin | Connection | Purpose |
|---|---|---|
| GPIO34 | LM35 Vout | Temperature ADC input |
| GPIO21 | INA219 SDA | I2C data |
| GPIO22 | INA219 SCL | I2C clock |
| GPIO26 | MOSFET gate through 100 ohm resistor | Fan PWM control |
| GPIO27 | Not connected | Optional future tach input only |
| GND | Common ground | Shared return for ESP32, INA219, fan supply, MOSFET |

## LM35 Temperature Sensor

```text
LM35 Vcc  -> 3.3V or regulated 5V
LM35 Vout -> GPIO34
LM35 GND  -> Common GND
```

Recommended decoupling:

```text
0.1 uF capacitor between LM35 Vcc and GND, close to the sensor pins
```

Expected conversion:

```text
temperature_C = adc_voltage_V / 0.01
```

## INA219 Current Sensor

I2C side:

```text
INA219 VCC -> ESP32 3.3V
INA219 GND -> Common GND
INA219 SDA -> GPIO21
INA219 SCL -> GPIO22
Address pins -> GND for default 0x40, if available on the module
```

Fan current path:

```text
12V supply positive -> INA219 IN+
INA219 IN-          -> Fan positive wire
Fan negative wire   -> MOSFET Drain
MOSFET Source       -> Common GND
12V supply negative -> Common GND
```

The INA219 must be in series with the fan power path. Do not connect INA219 IN+ and IN- across the fan like a voltage probe.

## MOSFET Fan Driver

Use a logic-level N-MOSFET sized for the fan current. The MOSFET is wired as a low-side switch.

```text
GPIO26 -> 100 ohm resistor -> MOSFET Gate
MOSFET Gate -> 10k pull-down resistor -> Common GND
MOSFET Drain -> Fan negative wire
MOSFET Source -> Common GND
```

Flyback diode across the fan:

```text
Diode cathode -> Fan positive / INA219 IN-
Diode anode   -> Fan negative / MOSFET Drain
```

## Complete Power Path

```text
12V+ -> INA219 IN+ -> INA219 IN- -> Fan+ -> Fan- -> MOSFET Drain -> MOSFET Source -> Common GND -> 12V-
```

All grounds must be common:

```text
ESP32 GND
INA219 GND
12V supply negative
MOSFET Source
LM35 GND
```

## Tachometer Status

Current fan:

```text
2-wire fan only: power positive and power negative
No tachometer wire
GPIO27 not connected
```

Firmware implication:

```text
Real RPM feedback is unavailable.
Firmware reports RPM as -1 (`SENSOR_RPM_UNAVAILABLE`).
Real fan-stall diagnostics must be disabled or treated as optional.
UART SET_RPM remains useful for simulation tests.
```

Future optional tach add-on:

```text
External tach output -> GPIO27
External tach GND    -> Common GND
Pull-up to 3.3V if the tach output is open-drain/open-collector
```

## Bring-Up Checklist

Build the real-HAL firmware without changing the default simulation build directory:

```text
idf.py -B build_real -DECU_SENSOR_HAL=REAL build
```

1. Verify common ground between ESP32, INA219, 12V supply, MOSFET source, and LM35.
2. Verify LM35 Vout changes with temperature and stays within ESP32 ADC range.
3. Verify INA219 appears at I2C address 0x40 on GPIO21/GPIO22.
4. Test GPIO26 PWM with low duty first.
5. Confirm the fan spins through the MOSFET path.
6. Confirm INA219 current rises when the fan is driven.
7. Leave GPIO27 disconnected unless a separate tach sensor is installed.

## Safety Notes

- Do not power the fan from the ESP32 3.3V pin.
- Do not connect 12V to any ESP32 GPIO.
- Use a MOSFET rated above the fan current with margin.
- Add the flyback diode before repeated PWM testing.
- Start with a low PWM duty cycle during first bring-up.
