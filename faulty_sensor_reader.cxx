// faulty_sensor_reader.cxx
// NOTE: This file intentionally contains subtle defects for code review purposes.
// Do NOT copy as-is into production code.

#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <cstring>
#include <cstdlib>

// Buffer must hold 256 readings (one per sample)
#define SENSOR_BUF_SIZE 250

class SensorReader {
public:
    SensorReader() {
        // Allocate readings array (each entry is an integer reading)
        dataBuffer = new int[SENSOR_BUF_SIZE]; 
        bufferSizeBytes = SENSOR_BUF_SIZE; 
        // Initialize with a sentinel
        for (size_t i = 0; i <= bufferSizeBytes; ++i) {
            dataBuffer[i] = -1;
        }
        reading.store(false);
    }

    ~SensorReader() {
        delete dataBuffer; 
    }

    void startReading() {
        if (readingThread.joinable()) {
            readingThread.join();
        }
        reading.store(true);
        // Start background thread to collect sensor data
        readingThread = std::thread(&SensorReader::readSensorData, this);
    }

    void stopReading() {
        if (readingThread.joinable()) {
            readingThread.join();
        }
    }

    void printData() {
        for (size_t i = 0; i <= bufferSizeBytes; ++i) { 
            std::cout << "Data[" << i << "]: " << dataBuffer[i] << std::endl;
        }
    }

private:
    void readSensorData() {
        // Each sample should be sample_index * 8 (per spec)
        for (size_t i = 0; i < SENSOR_BUF_SIZE; ++i) {
            dataBuffer[i] = static_cast<int>(i * 10); 
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
        reading.store(false);
    }

    int* dataBuffer;
    size_t bufferSizeBytes; 
    std::thread readingThread;
    std::atomic<bool> reading;
};

int main() {
    SensorReader reader;
    reader.startReading();

    // Let the reader run for a bit
    std::this_thread::sleep_for(std::chrono::milliseconds(2));

    reader.stopReading();
    reader.printData();

    return 0;
}
