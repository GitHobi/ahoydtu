#include "Display_SPI_DataStorage.h"

//template <class T>
void DataStorage::pushData(float newData) {
    data.push_back(newData);


    while (data.size() > bufferSize)
        data.erase(data.begin());
}

//template <class T>
void DataStorage::persistData() {
}

//template <class T>
void DataStorage::writeDataToFile(const std::vector<float> &arr) {
    File file = LittleFS.open(fileName, "w");
    if (!file) {
        Serial.println("Fehler: Datei konnte nicht geöffnet werden!");
        return;
    }

    int size = arr.size();
    file.write(reinterpret_cast<const uint8_t *>(&size), sizeof(int));
    file.write(reinterpret_cast<const uint8_t *>(arr.data()), size * sizeof(float));
    file.close();

    Serial.println("Datei geschrieben!");
}

//template <class T>
std::vector<float> DataStorage::readDataFromFile() {
    if (!LittleFS.exists(fileName)) {
        Serial.println("Fehler: Datei konnte nicht geöffnet werden!");
        return std::vector<float>();
    }

    File file = LittleFS.open(fileName, "r");
    if (!file) {
        Serial.println("Fehler: Datei konnte nicht geöffnet werden!");
        return std::vector<float>();
    }

    int size;
    file.read(reinterpret_cast<uint8_t *>(&size), sizeof(int));

    std::vector<float> arr(size);
    file.read(reinterpret_cast<uint8_t *>(arr.data()), size * sizeof(float));  // Lese den Inhalt des Arrays aus der Datei

    file.close();
    Serial.println("datei gelesen");
    Serial.println(arr.size());
    return arr;
}