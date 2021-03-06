#include <iostream>
#include <fstream>
#include <complex>
#include <vector>

#define cimg_use_png
#define cimg_display 0
#include "CImg.h"

// Processing buffer size
#define BUFFER_SIZE (26050)

// Returns the asked bit!
template <typename T>
inline bool getBit(T &data, int &bit)
{
    return (data >> bit) & 1;
}

int main(int argc, char *argv[])
{
    // Output and Input file
    std::ifstream data_in(argv[1], std::ios::binary);

    // Read buffer
    uint8_t *buffer = new uint8_t[BUFFER_SIZE];

    // VIRR Packet buffer
    uint16_t *virrBuffer = new uint16_t[204800];

    // Frame counter
    int frame = 0;

    // This will need some fixes
    uint16_t *imageBufferR = new uint16_t[10000 * 2048];
    uint16_t *imageBufferG = new uint16_t[10000 * 2048];
    uint16_t *imageBufferB = new uint16_t[10000 * 2048];

    // Read until EOF
    while (!data_in.eof())
    {
        // Read buffer
        data_in.read((char *)buffer, BUFFER_SIZE);

        int pos = 424; // VIRR Data position, found through a bit viewer

        // Convert into 10-bits values
        for (int i = 0; i < 20480; i += 4)
        {
            virrBuffer[i] = (buffer[pos + 0] << 2) | (buffer[pos + 1] >> 6);
            virrBuffer[i + 1] = ((buffer[pos + 1] % 64) << 4) | (buffer[pos + 2] >> 4);
            virrBuffer[i + 2] = ((buffer[pos + 2] % 16) << 6) | (buffer[pos + 3] >> 2);
            virrBuffer[i + 3] = ((buffer[pos + 3] % 4) << 8) | buffer[pos + 4];
            pos += 5;
        }

        // Channel R
        for (int i = 0; i < 2048; i++)
        {
            uint16_t pixel = virrBuffer[8 + i * 10];
            imageBufferR[frame * 2048 + i] = pixel * 60;
        }

        // Channel G
        for (int i = 0; i < 2048; i++)
        {
            uint16_t pixel = virrBuffer[0 + i * 10];
            imageBufferG[frame * 2048 + i] = pixel * 60;
        }

        // Channel B
        for (int i = 0; i < 2048; i++)
        {
            uint16_t pixel = virrBuffer[6 + i * 10];
            imageBufferB[frame * 2048 + i] = pixel * 60;
        }

        // Frame counter
        frame++;
    }

    // Print it all out into a .png
    cimg_library::CImg<uint16_t> finalImage(2048, frame, 1, 3);

    cimg_library::CImg<uint16_t> channelImageR(&imageBufferR[0], 2048, frame);
    cimg_library::CImg<uint16_t> channelImageG(&imageBufferG[0], 2048, frame);
    cimg_library::CImg<uint16_t> channelImageB(&imageBufferB[0], 2048, frame);

    channelImageR.equalize(1000);
    channelImageG.equalize(1000);
    channelImageB.equalize(1000);

    finalImage.draw_image(0, 0, 0, 0, channelImageR);
    finalImage.draw_image(0, 0, 0, 1, channelImageG);
    finalImage.draw_image(0, 0, 0, 2, channelImageB);

    finalImage.save_png(argv[2]);

    data_in.close();
}