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
#define SENSOR_BUF_SIZE 250 // 1) here should be 256 since says 256 readings but NOT critical

class SensorReader {
public:
    SensorReader() {
        // Allocate readings array (each entry is an integer reading)
        dataBuffer = new int[SENSOR_BUF_SIZE]; 
        bufferSizeBytes = SENSOR_BUF_SIZE; 
        // Initialize with a sentinel
        for (size_t i = 0; i < bufferSizeBytes; ++i) { //2) starts from 0 up to 250 should be up to 249 
                                                        // so should use < instead of <=
            dataBuffer[i] = -1;
        }
        reading.store(false);
    }

    ~SensorReader() {
        delete[] dataBuffer; //4) dataBuffer seems list so has to be delete[] dataBuffer or delete dataBuffer[]
                           // not sure on the deletion syntax but is a list so have to delete a list not an element
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
    }// hmm not sure but maybe it would be better if there is a reading.store(false)in the beginning 
    //so before joining thethread it actually stops the reading no? even though it is after reading the 
    //sensor data, hmm mamma mia not sure don't know this!!!

    void printData() {
        for (size_t i = 0; i < bufferSizeBytes; ++i) { // 3)same logic as at 2) use have to use < instead of <= it will through out of bound
            std::cout << "Data[" << i << "]: " << dataBuffer[i] << std::endl;
        }
    }

private:
    void readSensorData() {
        // Each sample should be sample_index * 8 (per spec)
        for (size_t i = 0; i < SENSOR_BUF_SIZE; ++i) {
            dataBuffer[i] = static_cast<int>(i * 8); //5) here should be i*8
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
