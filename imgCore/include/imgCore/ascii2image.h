#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>

namespace Lux {
namespace ziwi {

using namespace cv;
using namespace std;

inline int ascii2Image() {
    ifstream infile("/home/lux/Desktop/Ziwi/icon/ascii.txt");
    if (!infile.is_open()) {
        std::cerr << "Error opening input file" << std::endl;
        return 1;
    }

    std::string line;
    std::vector<std::string> lines;
    while (std::getline(infile, line)) {
        lines.push_back(line);
    }

    for (const auto &line : lines) {
        std::cout << line << std::endl;
    }

    int width = lines[0].size();
    int height = lines.size();
    cv::Mat img(height, width, CV_8UC1);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            unsigned char pixel = lines[y][x];
            img.at<unsigned char>(y, x) = pixel;
        }
    }

    cv::imwrite("/home/lux/Desktop/Ziwi/icon/ascii_image.png", img);
    std::cout << "Image saved as output.png" << std::endl;

    return 0;
}

}  // namespace ziwi
}  // namespace Lux
