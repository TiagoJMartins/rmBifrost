#include "bifrost/bifrost_client.h"
#include <iostream>

typedef unsigned int Rgb;                       
inline constexpr Rgb Rgba(int r, int g, int b, int a)
{ return ((a & 0xffu) << 24) | ((r & 0xffu) << 16) | ((g & 0xffu) << 8) | (b & 0xffu); }

int main()
{
    bifrost_client client("example_app", "Example App", false, 1);
    client.start();

    auto [image_index, image_ptr] = client.acquire_swapchain_image();
    auto image = reinterpret_cast<Rgb*>(image_ptr);

    auto extent = client.get_swapchain_extent();
    std::cout << "Swapchain extent: " << extent.first << "x" << extent.second << std::endl;

    for (int y = 0; y < extent.second; y++) {
        for (int x = 0; x < extent.first; x++) {
            image[y * extent.first + x] = Rgba(255, 255, 255, 255);
        }
    }

    for (int y = 0; y < 100; y++) {
        for (int x = 0; x < 100; x++) {
            image[y * extent.first + x] = Rgba(255, 255, 0, 255);
        }
    }

    std::cout << "Submitting frame" << std::endl;
    client.submit_frame(image_index, 0, 0, 100, 100, COLOR_FAST);
    std::cout << "Submitted frame" << std::endl;

    // std::cout << "Stopping client" << std::endl;
    // client.stop();
    return 0;
}
