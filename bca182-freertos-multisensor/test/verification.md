# Static Analysis and Functional Verification

## PlatformIO Check

Command: `pio check -e bluepill_f103c8`

The medium- and high-severity scans reported no findings. The full scan reported low-severity style notices in CubeMX integration code and the FreeRTOS heap implementation:

| Finding | File/Line | Cause | Resolution |
|---|---|---|---|
| `constParameterPointer` | `src/cubemx/stm32f1xx_hal_msp.c`: 91, 124, 152, 187, 218, 258 | Cppcheck suggests making callback parameters pointer-to-const because the local implementation does not modify them. | Retained: these functions implement STM32 HAL callback signatures and the HAL expects the declared non-const signatures. |
| `unusedFunction` | `src/cubemx/stm32f1xx_hal_msp.c`: 62, 91, 124, 152, 187, 218, 258 | The static analyzer cannot see calls made indirectly by the HAL/framework. | Retained: these are HAL MSP entry points called through the framework. |
| `unusedFunction` | `src/cubemx/stm32f1xx_hal_timebase_tim.c`: 42, 122, 134 | The static analyzer cannot see HAL timebase hooks called through the framework. | Retained: these are HAL timebase entry points. |
| `unusedFunction` | `src/cubemx/stm32f1xx_it.c`: 70, 85, 100, 115, 130, 145, 165 | Cppcheck does not follow the startup vector table's references to interrupt handlers. | Retained: exception/interrupt handlers are entry points referenced outside normal C/C++ calls. |
| `unusedFunction` | `src/heap_4.c`: 115, 266, 315, 321, 327, 442 | The analyzer does not see allocator calls made through the FreeRTOS kernel; some heap-statistics APIs are optional and unused by this application. | Retained: `pvPortMalloc()`/`vPortFree()` support kernel allocation; optional heap APIs are part of the supplied allocator implementation. |
| `variableScope` | `src/heap_4.c`: 269 | Cppcheck suggests narrowing the scope of an allocator implementation variable. | Retained: style-only change in the FreeRTOS heap implementation with no behavioral defect. |

These are low-severity style/integration notices; the scan found no medium- or high-severity defects. Do not change HAL callback signatures or interrupt-handler linkage just to silence notices caused by indirect framework entry points.

## Functional Test Record

Run each test in Wokwi and record what actually happened before marking PASS or FAIL. Temperature changes may require stopping the simulation, changing the DHT22 `temperature` attribute in `diagram.json`, rebuilding if needed, and restarting. Change humidity similarly; adjust the photoresistor light control during the run.

| Test ID | Input/Stimulus | Expected | Actual | Result |
|---|---|---|---|---|
| FT-01 | Change DHT22 temperature | Temperature page updates to the new reading. |  |  |
| FT-02 | Change DHT22 humidity | Humidity page updates to the new reading. |  |  |
| FT-03 | Change photoresistor light input | Light page value changes. |  |  |
| FT-04 | Rotate encoder clockwise | Advances Temperature → Humidity → Light → Motion → Temperature. |  |  |
| FT-05 | Rotate encoder counterclockwise | Moves to the previous page; Temperature wraps to Motion. |  |  |
| FT-06 | Set DHT22 temperature above 30 C | AlarmTask reports HIGH TEMPERATURE and buzzer activates. |  |  |
| FT-07 | Return temperature to normal range (18–30 C) | AlarmTask reports normal and buzzer stops. |  |  |
| FT-08 | Trigger PIR while system is active | PIR is detected; system remains or becomes ACTIVE. |  |  |
| FT-09 | Leave PIR clear for at least 15 seconds | System becomes INACTIVE and OLED turns off. |  |  |
| FT-10 | Trigger PIR while system is inactive | System returns to ACTIVE and OLED turns on. |  |  |

Do not mark any functional test PASS until its actual observed behavior is recorded.