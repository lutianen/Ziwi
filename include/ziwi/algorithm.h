
#pragma once

#include <imgCore/LuxDLL.h>
#include <ziwi/common.h>

#include <filesystem>
#include <iostream>
#include <string>

class DisplayUtils {
private:
    bool useFileRelay_;
    std::string relayFile_;

public:
    enum ImageFormatConversion {
        Unkow = -1,
        BayerBG2BGR = 46,
        BayerGB2BGR = 47,
        BayerRG2BGR = 48,
        BayerGR2BGR = 49,
        BayerBG2RGB = 48,
        BayerGB2RGB = 49,
        BayerRG2RGB = 46,
        BayerGR2RGB = 47,

        BayerBG2GRAY = 86,
        BayerGB2GRAY = 87,
        BayerRG2GRAY = 88,
        BayerGR2GRAY = 89,
    };

    DisplayUtils(bool useFileRelay = true, std::string relayFile = RELAY_FILE);

    unsigned char* LoadDataForDisplaySelectableMode(unsigned char workspace,
        const unsigned char* inData, int dataFormat, bool saveTiffFlag,
        std::string& tiffFileName, int mode, bool isBigEndian,
        unsigned long long width, unsigned long long height, int bitDepth,
        int channel);

private:
    bool createDirIfNot(const std::string& dirName);
};