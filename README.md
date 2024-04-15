# System Resource Monitoring Tool

This project provides a comprehensive toolset for monitoring various system resources such as CPU usage, memory usage, network bandwidth, and I/O operations. The tool is designed to run on Linux systems and leverages the `/proc/` directory to gather statistics. It includes a visualization component using GNUplot to plot resource usage over time.

## Features

- **CPU Monitoring**: Track CPU usage percentages over time.
- **Memory Monitoring**: Monitor the percentage of used memory.
- **Network Monitoring**: Measure bytes received and transmitted by the network interface.
- **I/O Monitoring**: Monitor disk read and write operations.
- **Graphical Visualization**: Real-time plotting of the monitored data using GNUplot.

## Requirements

- Linux OS with access to `/proc/` directory.
- GNUplot installed for generating the plots.
- C++11 standard compiler (e.g., GCC).

## Installation

1. Clone this repository or download the source code.
2. Ensure GNUplot is installed on your system. You can install it using your distribution’s package manager. For example, on Ubuntu:
`sudo apt-get install gnuplot`
3. Compile the source code using a C++ compiler:

`g++ -std=c++11 -o system_monitor main.cpp`


## Usage

Run the program from the command line:

`./system_monitor [duration]`


- `duration`: Optional. Specify the monitoring duration in seconds. If not specified, the default is 60 seconds.

The program will output real-time graphs showing the usage of each resource. Each resource is plotted in a separate window.

## Example

To monitor system resources for 120 seconds:

`./system_monitor 120`


## Output

The tool will display real-time graphs for:

- CPU Usage (%)
- Memory Usage (%)
- Bandwidth Bytes Received
- Bandwidth Bytes Transmitted
- I/O Reads
- I/O Writes

Each metric is plotted in its corresponding GNUplot window.

## Limitations

- This tool is designed specifically for Linux systems and may not function on other operating systems.
- The network interface is hardcoded to `enp0s3`. Adjust the source code for different or multiple interfaces.

## Contributing

Contributions to the project are welcome. You can contribute in several ways:

1. Reporting bugs.
2. Suggesting enhancements.
3. Submitting pull requests for new or improved features.