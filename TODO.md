# TODO

- scale out color values from 0x00-0x1F range to 0x00-0xFF range

## Major
- implement mbc5
  ```
  In Double Speed Mode the following will operate twice as fast as normal:
  
    The CPU (2.10 MHz, 1 Cycle = approx. 0.5us)
    Timer and Divider Registers
    Serial Port (Link Cable)
    DMA Transfer to OAM
  
  And the following will keep operating as usual:
  
    LCD Video Controller
    HDMA Transfer to VRAM
    All Sound Timings and Frequencies
  ```

