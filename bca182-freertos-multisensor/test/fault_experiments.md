# Part XVII: Deliberate FreeRTOS Fault Experiments

All experiment switches are in `include/fault_experiments.h` and default to `0`. Enable only one experiment at a time, rebuild with `pio run`, then restart Wokwi so it loads the rebuilt firmware. Record observations and restore the switch to `0` immediately after each experiment. Rebuild once more with all switches at `0` before normal use or submission.

## FT-49: Remove the SensorTask Delay

Set `FAULT_EXPERIMENT_REMOVE_SENSOR_DELAY` to `1`. Build and restart Wokwi. SensorTask then samples and publishes continuously instead of waiting for its 2-second period. Observe whether display, motion, and alarm work become less responsive or Wokwi slows/stalls. This custom port does not switch tasks from the timer ISR, so a continuously running task may monopolize execution. Stop or reset Wokwi if it becomes unresponsive. Restore the switch to `0` and rebuild.

## FT-50: Raise a Frequent Task's Priority

Set `FAULT_EXPERIMENT_HIGH_INPUT_PRIORITY` to `1`. InputTask polls the encoder every 10 ms and receives the highest configured task priority (`configMAX_PRIORITIES - 1`). Build and restart Wokwi; observe the responsiveness of sensor, alarm, and display work. The input task blocks every 10 ms, so it may not starve other tasks, but it will always run first when it is Ready. Restore the switch to `0` and rebuild.

## FT-51: Remove the Serial Mutex

Set `FAULT_EXPERIMENT_REMOVE_SERIAL_MUTEX` to `1`. Build and restart Wokwi, then watch for interleaved task messages. In this firmware, `Serial_WriteRaw()` transmits a complete string synchronously and does not yield mid-message; this custom scheduler also does not switch tasks from the tick ISR. Therefore output may remain intact even without the mutex. Record that result and explain that the mutex protects against interleaving if serial writes later become blocking/asynchronous or messages are emitted in multiple chunks. Restore the switch to `0` and rebuild.

| Experiment | Switch enabled | Observation | Restored and rebuilt |
|---|---|---|---|
| FT-49 Remove delay |  |  |  |
| FT-50 Raise priority |  |  |  |
| FT-51 Remove mutex |  |  |  |