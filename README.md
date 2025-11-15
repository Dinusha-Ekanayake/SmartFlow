# SmartFlow

A human-centric cooling solution.

## Overview
SmartFlow is a C++ project that implements algorithms and controls for a human-centric cooling system. It focuses on energy-efficient localized cooling, sensor-driven control loops, and easy integration with embedded controllers and HVAC systems.

## Features
- Sensor-driven, feedback-controlled cooling logic
- Modular C++ components for sensing, control, and actuation
- Designed for embedded and edge deployment

## Requirements
- C++17 (or newer) compatible compiler (g++, clang++)
- CMake 3.10+
- Linux or other POSIX-compatible systems for development and most deployments
- Optional: hardware sensors and actuators for full end-to-end testing

## Build
```bash
# clone the repository
git clone https://github.com/Dinusha-Ekanayake/SmartFlow.git
cd SmartFlow

# create build directory and compile
mkdir -p build && cd build
cmake ..
cmake --build . -- -j$(nproc)

# resulting binaries will be placed in the build/ directory (or as configured by CMake)
```

If the project uses a different build system or specific targets, adjust the commands above accordingly.

## Configuration & Running
- Review the config/ or docs/ directory (if present) for runtime configuration options.
- Typical run command (replace with the actual binary name once built):
```bash
./build/smartflow [--config path/to/config.yaml]
```

## Tests
If tests are present, run them from the build directory, for example:
```bash
ctest --output-on-failure
```

## Contributing
Contributions are welcome. Please follow these guidelines:
- Open an issue to discuss larger changes before implementing them.
- Fork the repository and open a pull request against the main branch.
- Follow the existing C++ style in the codebase.

## License
See the LICENSE file in the repository for licensing information. If there is no LICENSE file, please contact the repository owner to clarify licensing.

## Contact
Repository owner: @Dinusha-Ekanayake
