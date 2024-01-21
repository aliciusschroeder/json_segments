# JSON Segmentation and Reassembly Library

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://opensource.org/licenses/MIT)

This C library offers a robust framework for managing large JSON objects. It efficiently splits these objects into smaller segments, optimizing them for transmission and processing. Whether you're dealing with network data transfer, IoT devices, or resource-constrained environments, this library can help you manage JSON data effectively.

## Features

- **JSON Segmentation**: Split large JSON objects into smaller segments, allowing for efficient transmission over networks or processing in resource-constrained environments.

- **Segment Reassembly**: Reassemble received JSON segments into complete JSON objects for further processing.

- **Error Resilience**: Gracefully handles errors, timeouts, and incomplete segments, safeguarding data integrity.

- **Memory and CPU Efficiency**: Designed for resource-constrained environments, the library minimizes memory usage and processing overhead.

- **User-Defined Processing**: Define your JSON processing function to handle reassembled JSON objects.

## Use Cases

The `json_segments` library is tailored to address the inherent limitations of IoT communication, where stringent data size constraints often pose challenges for data transmission. In scenarios like ESP-NOW, which allows only 250 bytes per message, transmitting larger JSON objects becomes a complex task. Key use cases illustrating the json_segments library's importance in overcoming these limitations include:

1. **IoT Configuration**: IoT devices frequently require configuration updates, but these updates may involve larger JSON objects that exceed the size limits of communication protocols like ESP-NOW. With `json_segments`, you can efficiently transmit configuration data in segmented form, ensuring that even devices with constrained data transfer capabilities can receive and apply configurations.

2. **Sensor Data Segmentation**: IoT sensors often generate substantial data, but transmitting complete datasets can be problematic due to size restrictions. The library enables you to segment sensor readings into manageable JSON segments, allowing you to transmit and process data while adhering to protocol-imposed limits.

3. **Custom IoT Protocols**: In cases where you are working with custom IoT communication protocols that impose specific data size limitations, `json_segments` offers a versatile solution to ensure that your JSON data is transmitted and received accurately, regardless of the protocol's constraints.

The `json_segments` library simplifies the task of transmitting and reassembling large JSON objects within the confines of IoT communication protocols. It is an essential tool for IoT developers seeking efficient data management solutions in environments where data size limitations are a primary concern.


## Getting Started

```c
#include <cJSON.h>
#include "json_segments.h"

// Define your custom JSON processing function.
void process_json(cJSON *json) {
    // Your logic for processing the JSON object
}

int main() {
    // Set the JSON processing function.
    current_json_processing_function = process_json;

    // Your application logic here.
}
```

## Usage

1. **Segment JSON Data**:

   ```c
   // Split a string into JSON segments.
   cJSON **segments = json_segments_split_string(your_json_data, unique_id, max_segment_length);
   ```

2. **Receive JSON Segment Data**:

   ```c
   // Handle received segments:
   json_segments_parse_input(payload);
   ```

3. **Custom JSON Processing**:

   Define your JSON processing function and set it using `current_json_processing_function` to handle reassembled JSON objects.

3. **Error Handling**:

   Use `json_segments_check_timeout` to check for and handle timeouts, and `json_segments_delete_segments` to delete segments associated with a unique ID.

## Contributing

Contributions are welcome! If you wish to suggest improvements, report bugs, or add new features, please feel free to open an issue or submit a pull request.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- This library uses cJSON for JSON parsing and generation. Visit the [cJSON repository](https://github.com/DaveGamble/cJSON) for more information.

---

**Note**: This README provides a brief overview of the library's features and usage. For detailed documentation and function descriptions, refer to the [json_segments.h](json_segments.h) header file.
