# UpRev-MrT

**UpRev** **M**​odule **R**​eusability and **T**​esting

MrT is a collection of reusable modules that can be easily integrated into new projects. Each module is designed and maintained according to guidelines and standards to keep consistency. This allows uniform implementation, documentation and testing.

## Installing MrT Utilities

While Mr T is a collection of modules, they are managed with a set of python tools. These can be installed using pip

```bash
pip install mrtutils
```

---
## Using Mr T for a project:

The easiest way to get started with MrT is to using the MrT config tool:

>**Note:** MrT Modules are added as git sub-modules, so make sure that you have already initialized your project as a git repo

<br>

```
cd <path/to/project>
mrt-config <relative/path/for/MrT/root>
```
This will open up the config gui and let you select modules to be imported.

---
## Contributing Modules to Mr T:
For more information on contributing a module to Mr. T, see the MrT wiki:


---
## Modules

#### Platforms

Platforms are abstractions for specific platforms. This could be an OS or an MCU family. Each platform contains abstracted interfaces such as GPIO, Uart, SPI, and I2C. This allows the device modules to have a common interface for all platforms

#### Devices
Devices are modules for supporting commonly used ICs in projects. This would include common sensors, flash/eeprom memory, displays, battery charge controllers, etc.

Deevice modules contain all the logic needed for their operation and communicate using abstracted interfaces from platform modules

#### Utilities
Utilities are modules that provide a common functionality with no need for abstraction, that is, they do not depend on any specific hardware or platform. These include Fifos, Hashing functions, encoders/decoders, and messaging protocols.

---
## Test

Automated unit testing will be performed using google test. This gives us the ability to output JUnit style test results that can be integrated with Jenkins plugins.
