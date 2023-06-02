
#include <ziwi/algorithm.h>

#include <fstream>

DisplayUtils::DisplayUtils(bool useFileRelay, std::string relayFile)
    : useFileRelay_(useFileRelay), relayFile_(relayFile) {
    if (useFileRelay_) {
        createDirIfNot(RELAY_DIR);
    }
}

bool DisplayUtils::createDirIfNot(const std::string& dirName) {
    std::filesystem::path path(dirName);

    if (std::filesystem::exists(path)) {  // 判断文件夹是否存在
        if (std::filesystem::is_directory(path)) {  // 文件夹已存在
            std::cout << "Directory " << dirName << " already exists."
                      << std::endl;
            return true;
        } else {  // 文件夹已存在，但是不是一个文件夹
            std::cerr << "Error: " << dirName << " is not a directory."
                      << std::endl;
            return false;
        }
    } else {                                            // 文件夹不存在
        if (std::filesystem::create_directory(path)) {  // 创建文件夹
            std::cout << "Directory " << dirName << " created successfully."
                      << std::endl;
            return true;
        } else {
            std::cerr << "Error: failed to create directory " << dirName
                      << std::endl;
            return false;
        }
    }
}

unsigned char* DisplayUtils::LoadDataForDisplaySelectableMode(
    unsigned char workspace, const unsigned char* inData, int dataFormat,
    bool saveTiffFlag, std::string& tiffFileName, int mode, bool isBigEndian,
    unsigned long long width, unsigned long long height, int bitDepth,
    int channel) {
    auto code = Unkow;
    if (dataFormat == 1)
        code = BayerRG2GRAY;
    else if (dataFormat == 2)
        code = BayerRG2RGB;
    else {
        std::cerr << "Error: unsupported data format." << std::endl;
        return nullptr;
    }

    if (useFileRelay_) {
        std::ofstream ofs(relayFile_, std::ios::binary);
        std::cout << "Length: "
                  << static_cast<size_t>(height * width * channel * bitDepth /
                                         8.0)
                  << std::endl;
        ofs.write(reinterpret_cast<const char*>(inData),
                  static_cast<size_t>(height * width * channel * bitDepth /
                                      8.0));  // NOLINT
        if (ofs.fail()) {
            std::cerr << "Error: failed to write relay file." << std::endl;
            ofs.close();
            return nullptr;
        } else {
            ofs.close();
        }

        // 0 - CE7
        long long len = INT_MIN;
        if (workspace == 0) {
            len = LuxLoadImageDataFromFileEnhanced(
                relayFile_.c_str(), dataFormat, width, height, bitDepth,
                channel, relayFile_.c_str(), tiffFileName.c_str(), isBigEndian,
                false, saveTiffFlag, mode, code);
        } else if (workspace == 1 /* 1 - TW2 */) {
            len = LuxLoadImageDataFromFileEnhanced(
                relayFile_.c_str(), dataFormat, width, height, bitDepth,
                channel, relayFile_.c_str(), tiffFileName.c_str(), isBigEndian,
                true, saveTiffFlag, mode, code);
        } else {
            std::cerr << "Error: unsupported workspace." << std::endl;
            return nullptr;
        }
        if (len < 0) return nullptr;

        auto* outData = new unsigned char[len];
        std::ifstream ifs(relayFile_, std::ios::binary);
        ifs.read(reinterpret_cast<char*>(outData), len);  // NOLINT
        ifs.close();

        return outData;
    } else {
        std::cerr << "Error: unsupported data format." << std::endl;
        return nullptr;
    }
}