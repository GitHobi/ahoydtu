#pragma once

#include <LittleFS.h>

#include <vector>

// template <class T>
class DataStorage {
   public:
    DataStorage(const String aFileName = "/data.bin", uint16_t aBufferSize = 24)
        : fileName(aFileName), bufferSize(aBufferSize) {
        data = readDataFromFile();


    }

    void pushData(float newData);
    void persistData();
    std::vector<float> getData() {
        return data;
    }

   private:
    std::vector<float> data;
    String fileName;
    uint16_t bufferSize;

    void writeDataToFile(const std::vector<float> &arr);
    std::vector<float> readDataFromFile();
};