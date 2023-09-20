# EDK2 UEFI Firmware For HTC HD2

## Status 

| Function      | Notes                                   | status |
|---------------|-----------------------------------------|--------|
| GPIO          | Based on cLK driver                     |   ✅   |
| SD Card       | Based on cLK driver                     |   ✅   |
| I2C           | Driver exists in cLK                    |   ❌   |
| Panel         | Driver exists in cLK                    |   ❌   |
| Charging      | Supported in cLK since 1.5.x            |   ❌   |
| Battery Gauge | Supported in cLK since 1.5.x (needs i2c)|   ❌   |
| Keypad        | Loosely based on cLK driver             |   ✅   |

## Loading
UEFI can be either chainloaded as a kernel from cLK, as well as flashed to boot directly from HSPL.

## To-Do
Test/fix loading linux
Get to load ReactOS

## Credits
 - Cotulla and DFT for the work on HD2
 - cedesmith for creating the lk port for Leo, kokotas on further work
 - n0d3 for the sdcard driver in cLK
 - imbushuo for creating PrimeG2Pkg
 - ivoszbg for Msm8916Pkg
 - winocm for the iPhone4Pkg

## License
All code except drivers in `GplDrivers` directory is licensed under BSD 2-Clause. 
GPL Drivers are licensed under GPLv2 license.
