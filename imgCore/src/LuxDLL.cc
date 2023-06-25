/**
 * @file LuxDLL.cc
 */

#include <imgCore/LuxDLL.h>
#include <stdio.h>

#include <cmath>
#include <cstring>  /// memcpy
#include <fstream>
#include <functional>
#include <iostream>  /// fflush stdout
#include <ostream>
#include <tuple>

template <typename T>
inline unsigned char LuxBound255(T src) {
    return static_cast<unsigned>(src > 255 ? 255 : src);
}

/// @brief Swap endian data in pData
/// @param pData The pointer data to swap
/// @param startIndex Strat index
/// @param length The number of bytes to swap
void LuxEndianSwap(unsigned char *pData, uint64_t startIndex, uint64_t length) {
    uint64_t cnt = length / 2;
    uint64_t start = startIndex;
    uint64_t end = startIndex + length - 1;

    uint8_t tmp;
    for (uint64_t i = 0; i < cnt; ++i) {
        tmp = pData[start + i];
        pData[start + i] = pData[end - i];
        pData[end - i] = tmp;
    }
}

/// @brief EndianRevert from @c input to @c output
/// @return The length of reverting bytes
int LuxEndianRevert(unsigned char *input, int length, int bpp,
                    unsigned char *output, bool isBig2Little) {
    int bytes = bpp / 8;
    if (input != output) {
        ::memcpy(output, input, length);
    }
    for (int kI = 0; kI < length; kI += bytes) {
        LuxEndianSwap(output, kI, bytes);
    }
    return length;
}

/// @brief clear (or flush) the output buffer and move the buffered data
///     to console (in case of stdout) or disk (in case of file output stream)
void LuxFlushStdOut() { ::fflush(stdout); }

/// @brief Get the 8-bits image data from origianl image(8-bits, 12-bits,
/// 16-bits)
/// @param orgiImg The pointor of Original image before parsing.
/// @param length The bytes number of orginal image data.
/// @param bpp (bits per pixel) Only support 8, 12, 16
/// @param highZero 0000AAAA AAAAAAAA 0000BBBB BBBBBBBB ?
/// @param outputImg The pointor of image data in memory after parsing.
/// @return unsigned long long. The number of image bytes
inline unsigned long long LuxParseImage(unsigned char *orgiImg, int length,
                                        int bpp, bool highZero,
                                        unsigned char *outputImg) {
    uint64_t k = 0;
    switch (bpp) {
        case 8:
            try {
                ::memcpy(outputImg, orgiImg, length);
                break;
            } catch (const std::exception &e) {
                std::cerr << e.what() << '\n';
                ::fflush(stderr);
            }

        case 12: {
            try {
                /// 0000AAAA AAAAAAAA 0000BBBB BBBBBBBB
                if (highZero) {
                    for (int i = 0; i < length; i += 8) {
                        outputImg[k] =
                            (orgiImg[i] << 4u) + (orgiImg[i + 1] >> 4u);
                        outputImg[k + 1] =
                            (orgiImg[i + 2] << 4u) + (orgiImg[i + 3] >> 4u);
                        outputImg[k + 2] =
                            (orgiImg[i + 4] << 4u) + (orgiImg[i + 5] >> 4u);
                        outputImg[k + 3] =
                            (orgiImg[i + 6] << 4u) + (orgiImg[i + 7] >> 4u);
                        k += 4;
                    }
                }
                /// AAAAAAAA AAAABBBB BBBBBBBB
                else {
                    for (int i = 0; i < length; i += 3) {
                        outputImg[k++] = orgiImg[i];
                        outputImg[k++] = ((orgiImg[i + 1] << 4u) & 0xFF) +
                                         (orgiImg[i + 2] >> 4u);

                        // outputImg[k + 2] = orgiImg[i + 3];
                        // outputImg[k + 3] = (orgiImg[i + 4] << 4u) +
                        // (orgiImg[i + 5] >> 4u);

                        // outputImg[k + 4] = orgiImg[i + 6];
                        // outputImg[k + 5] = (orgiImg[i + 7] << 4u) +
                        // (orgiImg[i + 8] >> 4u);

                        // outputImg[k + 6] = orgiImg[i + 9];
                        // outputImg[k + 7] = (orgiImg[i + 10] << 4u) +
                        // (orgiImg[i + 11] >> 4u);
                    }
                }
                break;
            } catch (const std::exception &e) {
                std::cerr << e.what() << '\n';
                ::fflush(stderr);
            }
        }

        case 16: {
            try {
                if (highZero) {
                    std::cerr << "The depth of bit don't match HIGH BIT!"
                              << std::endl;
                    ::fflush(stderr);
                    return 0;
                }

                /// Set the higt 8 bit into outptImg
                for (int i = 0; i < length; i += 8) {
                    outputImg[k++] = orgiImg[i + 0];
                    outputImg[k++] = orgiImg[i + 2];
                    outputImg[k++] = orgiImg[i + 4];
                    outputImg[k++] = orgiImg[i + 6];
                }
                break;
            } catch (const std::exception &e) {
                std::cerr << e.what() << '\n';
                ::fflush(stderr);
            }
        }

        default:
            std::cerr << "bpp is wrong! It only support [12, 16]" << std::endl;
            ::fflush(stderr);
            break;
    }

    return k;
}

/**
 * @brief Get type of image: CV_8UC1, CV_8UC3, ...
 *
 * @param dataFormat
 * @param bpp
 * @param channels
 * @return decltype(CV_8UC1)
 */
inline decltype(CV_8UC1) LuxGetImageType(int dataFormat, int bpp,
                                         int channels) {
    int type = -1;
    if (dataFormat == 1) {
        if ((bpp == 8) || (12 == bpp) || (16 == bpp)) {
            if (1 == channels) {
                type = CV_8UC1;
            } else if (3 == type) {
                type = CV_8UC3;
            } else {
                std::cerr << "Channels is wrong. \n"
                          << "Supported Channels: 1, 3" << std::endl;
                ::fflush(stderr);
                return -1;
            }
        }
    } else if (2 == dataFormat) {
        if (1 == channels) {
            type = CV_8UC1;
        } else if (3 == channels) {
            type = CV_8UC3;
        } else {
            std::cerr << "Channels is wrong. \n"
                      << "Supported Channels: 1, 3" << std::endl;
            ::fflush(stderr);
            return -1;
        }
    }

    return type;
}

/// @brief
/// @param imgData
/// @param outFileName
/// @param fileFormat
/// @param length
/// @return
long long LuxWriteImageIntoFile(unsigned char *imgData, const char *outFileName,
                                ImageFileType fileFormat,
                                unsigned long long length, int width,
                                int height, int type) {
    std::string outFileStr(outFileName);
    if (fileFormat == ImageFileType::raw) {
        const std::string suffix(".raw");
        std::string temp(outFileName);

        if (temp.compare(temp.length() - suffix.length(), suffix.length(),
                         suffix) != 0) {
            outFileStr += suffix;
        }
        // else {
        // }

        // Open output file stream
        std::ofstream fout(outFileStr, std::ios_base::binary);
        if (!fout.is_open()) {
            std::cerr << "Fail to open file: " << outFileName << std::endl;
            ::fflush(stderr);
            return -1;
        }

        // Write image data into file
        fout.write((const char *)(imgData), length);

        return length;
    } else if (fileFormat == ImageFileType::tiff) {
        const std::string suffix(".tiff");
        std::string temp(outFileName);

        if (temp.compare(temp.length() - suffix.length(), suffix.length(),
                         suffix) != 0) {
            outFileStr += suffix;
        }
        // else {
        // }

        /// XXX Ensure .raw and .tiff are same
        if (CV_8UC3 == type)
            cv::cvtColor(cv::Mat(height, width, type, imgData),
                         cv::Mat(height, width, type, imgData),
                         cv::COLOR_RGB2BGR);
        cv::imwrite(outFileStr, cv::Mat(height, width, type, imgData));
        return length;
    } else {
        std::cout << "Don't support file type!" << std::endl;
        return -1;
    }
}

/**
 * @brief Overload
 *
 * @param imgData
 * @param outFileName
 * @param fileFormat
 * @param length
 * @param width
 * @param height
 * @param type
 * @return long long
 */
long long LuxWriteImageIntoFile(uint16_t *imgData, const char *outFileName,
                                ImageFileType fileFormat,
                                unsigned long long length, int width,
                                int height, int type) {
    std::string outFileStr(outFileName);
    if (fileFormat == ImageFileType::raw) {
        const std::string suffix(".raw");
        std::string temp(outFileName);

        if (temp.compare(temp.length() - suffix.length(), suffix.length(),
                         suffix) != 0) {
            outFileStr += suffix;
        }
        // else {
        // }

        // Open output file stream
        std::ofstream fout(outFileStr, std::ios_base::binary);
        if (!fout.is_open()) {
            std::cerr << "Fail to open file: " << outFileName << std::endl;
            ::fflush(stderr);
            return -1;
        }

        // Write image data into file
        fout.write(reinterpret_cast<char *>(imgData), length * 2);

        return length * 2;
    } else if (fileFormat == ImageFileType::tiff) {
        const std::string suffix(".tiff");
        std::string temp(outFileName);

        if (temp.compare(temp.length() - suffix.length(), suffix.length(),
                         suffix) != 0) {
            outFileStr += suffix;
        }
        // else {
        // }

        if (CV_16UC3 == type) {
            cv::Mat ret(height, width, type);
            cv::cvtColor(cv::Mat(height, width, type, imgData), ret,
                         cv::COLOR_RGB2BGR);

            cv::imwrite(outFileStr, ret);
        } else {
            cv::imwrite(outFileStr, cv::Mat(height, width, type, imgData));
        }

        return length * 2;
    } else {
        std::cout << "Don't support file type!" << std::endl;
        return -1;
    }
}

/**
 * @brief Save 16-bits raw file with "_ext16.raw" suffix.
 *
 * @param imgData
 * @param outFileName
 * @param fileFormat
 * @param length
 * @param width
 * @param height
 * @param type
 * @return long long. The bytes number of file if successful, or -1
 */
inline long long LuxWriteImageIntoFileExternTo16(
    uint16_t *imgData, const char *outFileName, ImageFileType fileFormat,
    unsigned long long length, int width, int height, int type) {
    std::string outFileStr(outFileName);
    if (fileFormat == ImageFileType::raw) {
        const std::string suffix(".raw");
        std::string temp(outFileName);

        if (temp.compare(temp.length() - suffix.length(), suffix.length(),
                         suffix) != 0) {
            outFileStr += std::string("_ext16.raw");
        } else {
            size_t _last = outFileStr.size() - suffix.length();
            outFileStr =
                outFileStr.substr(0, _last) + std::string("_ext16.raw");
        }

        // Open output file stream
        std::ofstream fout(outFileStr, std::ios_base::binary);
        if (!fout.is_open()) {
            std::cerr << "Fail to open file: " << outFileName << std::endl;
            ::fflush(stderr);
            return -1;
        }

        // Write image data into file
        // LuxEndianRevert(reinterpret_cast<unsigned char*>(imgData), length *
        // 2, 16,
        //         reinterpret_cast<unsigned char*>(imgData), false);
        fout.write(reinterpret_cast<char *>(imgData), length * 2);

        return length * 2;
    } else {
        std::cerr << "Type Error! Only support .raw";
        ::fflush(stderr);
        return -1;
    }
}

/**
 * @brief Load image data from memory
 * @note When Python Call the Function, the ALL parameters must be SET.
 * @param imgData The poniter of image data in memory before parsing.
 * @param length The bytes number of image data in memory.
 * @param dataFormat 1: raw, 2: bayer, 3: others
 * @param width Width
 * @param height Height
 * @param bpp (Bits Per Pixel)  8, 12, 16
 * @param inChannels inChannels
 * @param outData The poniter of image data in memory after parsing.
 * @param isBigEndian Big Endian(be) ?
 * @param highZero 0000AAAA AAAAAAAA ?
 * @param code
 *  - CV_BayerBG2BGR =46,
 *  - CV_BayerGB2BGR =47,
 *  - CV_BayerRG2BGR =48,
 *  - CV_BayerGR2BGR =49,
 *  - CV_BayerBG2RGB =CV_BayerRG2BGR = 48,
 *  - CV_BayerGB2RGB =CV_BayerGR2BGR = 49,
 *  - CV_BayerRG2RGB =CV_BayerBG2BGR = 46,
 *  - CV_BayerGR2RGB =CV_BayerGB2BGR = 47,
 *  - CV_BayerRG2GRAY = 88
 *  - CV_BayerBG2GRAY = 86,
 *  - CV_BayerGB2GRAY = 87,
 *  - CV_BayerRG2GRAY = 88,
 *  - CV_BayerGR2GRAY = 89
 * @return long long
 * The number of image bytes if success.
 *  -1 : Data Format Don't Supported.
 *  -2 : Bits per pixel Don't Supported.
 *  -3 : width or height or bpp or channel are wrong.
 *  -4 : It is not reached.
 */
long long LuxLoadImageData(unsigned char *imgData, unsigned long long length,
                           int dataFormat, int width, int height, int bpp,
                           int inChannels, unsigned char *outData,
                           bool isBigEndian, bool highZero, int code) {
    if (dataFormat != 1 && dataFormat != 2 && dataFormat != 3) {
        std::cerr << "Data Format Don't Supported!!! \n"
                  << "1: raw, 2: bayer, 3: others" << std::endl;
        ::fflush(stderr);
        return -1;
    }

    float bytes = 0.0;
    switch (bpp) {
        case 8:
            bytes = 1;
            break;
        case 12:
            bytes = 1.5;
            break;
        case 16:
            bytes = 2;
            break;
        default:
            std::cerr << "bpp Don't Supported!!! \n"
                      << "Supported depth of bits: 8, 12, 16" << std::endl;
            ::fflush(stderr);
            return -2;
    }

    if (length !=
        static_cast<unsigned long long>(
            static_cast<float>(width * height * inChannels) * bytes)) {
        std::cerr << "width or height or bpp or channel are wrong!!!"
                  << "\nwidth: " << width << "\nheigth: " << height
                  << "\nbits per pixel: " << bpp
                  << "\ninChannels: " << inChannels << std::endl;
        ::fflush(stderr);
        return -3;
    }

    // Only support big endian
    if (!isBigEndian) {
        LuxEndianRevert(imgData, length, bpp, imgData, true);
    }

    // TODO 代码优化： 加入 outChannels/types
    /* raw */
    if (dataFormat == 1) {
        long long validLength = width * height;
        auto *temp = new unsigned char[validLength];
        uint64_t k = LuxParseImage(imgData, length, bpp, highZero, temp);
        (void)k;

        cv::Mat bayer8BitMat(height, width, CV_8UC1, temp);
        cv::Mat outputImg(height, width, CV_8UC1, outData);
        cv::cvtColor(bayer8BitMat, outputImg, code);

        delete[] temp;
        /* 图片大小 （字节数） */
        return outputImg.size().width * outputImg.size().height *
               outputImg.channels();
    }

    /* Bayer */
    else if (dataFormat == 2) {
        int outChannels = 3;
        long long validLength = width * height * outChannels;
        auto *temp = new unsigned char[validLength];
        uint64_t k = LuxParseImage(imgData, length, bpp, highZero, temp);
        (void)k;

        /// 16UC1 Bayer
        cv::Mat bayer8BitMat(height, width, CV_8UC1, temp);
        cv::Mat rgb8BitMat(height, width, CV_8UC3, outData);
        cv::cvtColor(bayer8BitMat, rgb8BitMat, code);

        delete[] temp;
        return rgb8BitMat.size().height * rgb8BitMat.size().width *
               rgb8BitMat.channels();
    }

    return -4;
}

/**
 * @brief Load image data from file.
 * @note When Python Call the Function, the ALL parameters must be SET.
 * @param inputFileName The file before parsing.
 * @param dataFormat 1: raw, 2: bayer, 3: others
 * @param width Width
 * @param height Height
 * @param bpp bpp(Bits Per Pixel)  8, 12, 16
 * @param channels Channels
 * @param outputRawFileName The .raw file after parsing.
 * @param outputTiffFileName The .tiff file after parsing.
 * @param isBigEndian Big Endian(be) ?
 * @param highZero 0000AAAA AAAAAAAA ?
 * @param saveTiff Save tiff ?
 * @param code
 *  - CV_BayerBG2BGR =46,
 *  - CV_BayerGB2BGR =47,
 *  - CV_BayerRG2BGR =48,
 *  - CV_BayerGR2BGR =49,
 *  - CV_BayerBG2RGB =CV_BayerRG2BGR = 48,
 *  - CV_BayerGB2RGB =CV_BayerGR2BGR = 49,
 *  - CV_BayerRG2RGB =CV_BayerBG2BGR = 46,
 *  - CV_BayerGR2RGB =CV_BayerGB2BGR = 47,
 *  - CV_BayerRG2GRAY = 88
 *  - CV_BayerBG2GRAY = 86,
 *  - CV_BayerGB2GRAY = 87,
 *  - CV_BayerRG2GRAY = 88,
 *  - CV_BayerGR2GRAY = 89
 *
 * @return long long
 * The bytes number of image file if success.
 *  -1 : Data Format Don't Supported.
 *  -2 : Bits per pixel Don't Supported.
 *  -3 : width or height or bpp or channel are wrong.
 *  -4 : DownBits Error!
 *  -5 : It is not reached.
 */
long long LuxLoadImageDataFromFile(const char *inputFileName, int dataFormat,
                                   int width, int height, int bpp, int channels,
                                   const char *outputRawFileName,
                                   const char *outputTiffFileName,
                                   bool isBigEndian, bool highZero,
                                   bool saveTiff, int code) {
    if (dataFormat != 1 && dataFormat != 2 && dataFormat != 3) {
        std::cerr << "Data Format Don't Supported!!! \n"
                  << "1: raw, 2: bayer, 3: others" << std::endl;
        ::fflush(stderr);
        return 0;
    }

    float bytes = 0.0;
    switch (bpp) {
        case 8:
            bytes = 1;
            break;
        case 12:
            bytes = 1.5;
            break;
        case 16:
            bytes = 2;
            break;
        default:
            std::cerr << "bpp Don't Supported!!! \n"
                      << "Supported depth of bits: 8, 12, 16" << std::endl;
            ::fflush(stderr);
            return 0;
    }

    std::ifstream ifstrm(reinterpret_cast<const char *>(inputFileName),
                         std::ios_base::binary);
    if (ifstrm.is_open() == false) {
        std::cerr << "Fail to read " << inputFileName << std::endl;
        ::fflush(stderr);
        return -1;
    }

    /// Check the length of file
    unsigned long long length = static_cast<unsigned long long int>(
        static_cast<float>(width * height * channels) * bytes);
    ifstrm.seekg(0, ifstrm.end);
    unsigned long long ret = ifstrm.tellg();
    ifstrm.seekg(0, ifstrm.beg);
    if (ret != length) {
        std::cerr << "The length of file is NOT right." << std::endl;
        ::fflush(stderr);
        return -1;
    }

    /// Read bytes into imgData
    auto *imgData = new unsigned char[length];
    ifstrm.read((char *)imgData, length);

    /// According to the target, Set channels, Type of outputFile
    unsigned char *outData = nullptr;
    int cvType = CV_8UC1;
    /// raw
    if (dataFormat == 1) {
        outData = new unsigned char[width * height * 1];
        cvType = CV_8UC1;
    }
    /// Bayer / others
    else {
        outData = new unsigned char[width * height * 3];
        cvType = CV_8UC3;
    }

    unsigned long long k =
        LuxLoadImageData(imgData, length, dataFormat, width, height, bpp,
                         channels, outData, isBigEndian, highZero, code);

    /// k > 0 is ok
    if (k > 0) {
        // For display
        unsigned long long _ret =
            LuxWriteImageIntoFile(outData, outputRawFileName,
                                  ImageFileType::raw, k, width, height, cvType);

        // TIFF
        if (saveTiff)
            LuxWriteImageIntoFile(outData, outputTiffFileName,
                                  ImageFileType::tiff, k, width, height,
                                  cvType);

        delete[] imgData;
        delete[] outData;
        return _ret;
    } else {
        delete[] imgData;
        delete[] outData;
        return k;
    }
}

/**
 * @brief
 *
 * @param src
 * @param length
 * @param width
 * @param height
 * @param channel
 * @param beta
 * @param dst
 * @return int
 */
int LuxAdjustBrightness(unsigned char *src, long long length, int width,
                        int height, int channel, int beta, unsigned char *dst) {
    if (width <= 0 || height <= 0 || channel <= 0) {
        perror("width or height or channel MUST be > 0!!!");
        return -1;
    }

    if (length != static_cast<long long int>(width * height * channel)) {
        perror("width or height or channel are wrong!!!");
        return -1;
    }

    if (channel == 1) {
        cv::Mat img_src(width, height, CV_8UC1, src);
        ::memcpy(dst, img_src.data, length);
        cv::Mat img_dst(width, height, CV_8UC1, dst);

        int rowNum = img_src.rows;
        int colNum = img_src.cols * img_src.channels();

        for (int i = 0; i < rowNum; ++i) {
            auto *data = img_dst.ptr<uint8_t>(i);
            for (int j = 0; j < colNum; ++j) {
                data[j] = cv::saturate_cast<uint8_t>(data[j] + beta);
            }
        }
        dst = reinterpret_cast<unsigned char *>(img_dst.data);
        return length;
    } else if (channel == 3) {
        cv::Mat img_src(width, height, CV_8UC3, src);
        ::memcpy(dst, img_src.data, length);
        cv::Mat img_dst(width, height, CV_8UC3, dst);

        int rowNum = img_src.rows;
        int colNum = img_src.cols * img_src.channels();

        for (int i = 0; i < rowNum; ++i) {
            auto *data = img_dst.ptr<uint8_t>(i);
            for (int j = 0; j < colNum; ++j) {
                data[j] = cv::saturate_cast<uint8_t>(data[j] + beta);
            }
        }
        dst = reinterpret_cast<unsigned char *>(img_dst.data);
        return length;
    } else {
        perror("Channels is not supported!!!");
        return -1;
    }
}

/**
 * @brief
 *
 * @param src
 * @param length
 * @param width
 * @param height
 * @param channel
 * @param alpha
 * @param dst
 * @return int
 */
int LuxAdjustContrast(unsigned char *src, long long length, int width,
                      int height, int channel, int alpha, unsigned char *dst) {
    if (width <= 0 || height <= 0 || channel <= 0) {
        std::cout << "width or height or channel MUST be > 0 !!!" << std::endl;
        return -1;
    }

    if (length != static_cast<long long int>(width * height * channel)) {
        std::cout << "width or height or bpp or channel are wrong!!!"
                  << std::endl;
        return -1;
    }

    if (alpha > 0 && alpha < 10) {
        std::cout << "Beta is supported with ranging from -10 to 10!!!"
                  << std::endl;
        return -1;
    }

    if (channel == 1) {
        cv::Mat img_src(width, height, CV_8UC1, src);
        ::memcpy(dst, img_src.data, length);
        cv::Mat img_dst(width, height, CV_8UC1, dst);

        int rowNum = img_src.rows;
        int colNum = img_src.cols * img_src.channels();

        for (int i = 0; i < rowNum; ++i) {
            auto *data = img_dst.ptr<uint8_t>(i);
            for (int j = 0; j < colNum; ++j) {
                data[j] = cv::saturate_cast<uint8_t>(data[j] * 0.01 * alpha);
            }
        }
        dst = reinterpret_cast<unsigned char *>(img_dst.data);
        return length;
    } else if (channel == 3) {
        cv::Mat img_src(width, height, CV_8UC3, src);
        ::memcpy(dst, img_src.data, length);
        cv::Mat img_dst(width, height, CV_8UC3, dst);

        int rowNum = img_src.rows;
        int colNum = img_src.cols * img_src.channels();

        for (int i = 0; i < rowNum; ++i) {
            auto *data = img_dst.ptr<uint8_t>(i);
            for (int j = 0; j < colNum; ++j) {
                data[j] = cv::saturate_cast<uint8_t>(data[j] * alpha);
            }
        }
        dst = reinterpret_cast<unsigned char *>(img_dst.data);
        return length;
    } else {
        std::cout << "Channel is not supported!!!" << std::endl;
        return -1;
    }
}

/**
 * @brief
 *
 * @param inputFileName_
 * @param width
 * @param height
 * @param imReadType {0, 1}
 *	- 0: Gray image.
 *	- 1: RGB image.
 * @param histImgWidth TypeValue: 256
 * @param histImgHeight TypeValue: 256
 * @param outFileName
 * @return long long int
 */
long long int LuxDrawHist(const char *inputFileName_, int width, int height,
                          int imReadType, int histImgWidth, int histImgHeight,
                          const char *outFileName) {
    if (width < 0 || height < 0 || imReadType < 0 || histImgHeight < 0 ||
        histImgWidth < 0) {
        perror("imReadType or histImgWidth or histImgHeight or are wrong.");
        return 0;
    }
    const std::string suffix(".raw");
    std::string inputFileName(inputFileName_);

    cv::Mat image;
    if (inputFileName.compare(inputFileName.length() - suffix.length(),
                              suffix.length(), suffix) == 0) {
        std::ifstream fin;
        fin.open(inputFileName, std::ios::binary);
        if (!fin) {
            std::cerr << "Open failed: " << inputFileName << std::endl;
            return -1;
        }

        fin.seekg(0, fin.end);
        int length = fin.tellg();

        if (imReadType == 0) {
            if (width * height != length) {
                std::cerr << "width or height are wrong."
                          << "\r\n"
                          << " width: " << width << ", height: " << height
                          << ", length: " << length << std::endl;
                ::fflush(stderr);
                return -1;
            }
            fin.seekg(0, fin.beg);

            char *buffer = new char[length];
            fin.read(buffer, length);
            fin.close();

            image.create(width, height, CV_8UC1);
            ::memcpy(image.data, buffer, length);

            delete[] buffer;
            buffer = nullptr;
        }

        else {
            if (width * height * 3 != length) {
                std::cerr << "width or height are wrong."
                          << "\r\n"
                          << " width: " << width << ", height: " << height
                          << ", length: " << length << std::endl;
                ::fflush(stderr);
                return -1;
            }
            fin.seekg(0, fin.beg);

            char *buffer = new char[length];
            fin.read(buffer, length);
            fin.close();

            image.create(width, height, CV_8UC3);
            ::memcpy(image.data, buffer, length);

            delete[] buffer;
            buffer = nullptr;
        }
    } else {
        // IMREAD_GRAYSCALE = 0
        // IMREAD_COLOR = 1
        /// Get image Mat
        image = cv::imread(inputFileName, imReadType);
    }

    // Ensure the true image
    if (!image.data) {
        std::cerr << "Image is not found." << std::endl;
        return -1;
    }

    //
    std::vector<cv::Mat> bgr;
    cv::split(image, bgr);
    // imshow("b", bgr[0]);

    // Define the range
    int numbins = 256;
    float range[] = {0, 256};
    const float *histRange = {range};

    const int dims = image.channels();
    /// Gray
    if (1 == dims) {
        // FIXME raw 更新
        /// Define Mat to handle the channel form image
        cv::Mat hist;

        /// Calculate
        cv::calcHist(&image, 1, 0, cv::Mat(), hist, 1, &numbins, &histRange);

        ///
        int bin_w = cvRound((double)histImgWidth / numbins);

        ///
        cv::Mat histImage =
            cv::Mat::zeros(histImgHeight, histImgWidth, CV_8UC3);

        /// Normalize
        cv::normalize(hist, hist, 0, histImage.rows, cv::NORM_MINMAX, -1,
                      cv::Mat());

        /// Draw
        for (int i = 1; i < numbins; i++) {
            cv::line(histImage,
                     cv::Point(bin_w * (i - 1),
                               histImgHeight - cvRound(hist.at<float>(i - 1))),
                     cv::Point(bin_w * (i),
                               histImgHeight - cvRound(hist.at<float>(i))),
                     cv::Scalar(0, 255, 0), 2, 8, 0);
        }

        // 显示直方图
        // namedWindow("Histogram Demo", WINDOW_AUTOSIZE);
        // imshow("Histogram Demo", histImage);

        // std::cout << "height: " << histImage.size().height
        //          << " width: " << histImage.size().width
        //          << " channels: " << histImage.channels() << std::endl;

        // Write the Hist Image to file and Get the number of bytes
        // unsigned long long len =
        //         writeImageIntoFile(histImage.data, outFileName,
        //         ImageFileType::raw,
        //             histImage.size().width * histImage.size().height *
        //             histImage.channels());
        long long len = LuxWriteImageIntoFile(
            reinterpret_cast<unsigned char *>(histImage.data), outFileName,
            ImageFileType::raw,
            histImage.size().width * histImage.size().height *
                histImage.channels(),
            histImage.size().width, histImage.size().height, CV_8UC3);
        return len;
    }
    /// 3 通道图 - 彩色图
    else if (3 == dims) {
        /// Define 3 Mat to handle the channel form image
        cv::Mat b_hist, g_hist, r_hist;

        /// Calculate the Hist
        cv::calcHist(&bgr[0], 1, 0, cv::Mat(), b_hist, 1, &numbins, &histRange);
        cv::calcHist(&bgr[1], 1, 0, cv::Mat(), g_hist, 1, &numbins, &histRange);
        cv::calcHist(&bgr[2], 1, 0, cv::Mat(), r_hist, 1, &numbins, &histRange);

        /// The result - colorful Hist image
        cv::Mat histImage(histImgHeight, histImgWidth, CV_8UC3,
                          cv::Scalar(20, 20, 20));

        /// Set the height of Hist
        cv::normalize(b_hist, b_hist, 0, histImgHeight, cv::NORM_MINMAX);
        cv::normalize(g_hist, g_hist, 0, histImgHeight, cv::NORM_MINMAX);
        cv::normalize(r_hist, r_hist, 0, histImgHeight, cv::NORM_MINMAX);

        /// 通过将宽度除以区间数来计算binStep变量
        int binStep = cvRound((float)histImgWidth / (float)numbins);

        // Draw
        for (int i = 1; i < numbins; i++) {
            cv::line(
                histImage,
                cv::Point(binStep * (i - 1),
                          histImgHeight - cvRound(b_hist.at<float>(i - 1))),
                cv::Point(binStep * (i),
                          histImgHeight - cvRound(b_hist.at<float>(i))),
                cv::Scalar(255, 0, 0));
            cv::line(
                histImage,
                cv::Point(binStep * (i - 1),
                          histImgHeight - cvRound(g_hist.at<float>(i - 1))),
                cv::Point(binStep * (i),
                          histImgHeight - cvRound(g_hist.at<float>(i))),
                cv::Scalar(0, 255, 0));
            cv::line(
                histImage,
                cv::Point(binStep * (i - 1),
                          histImgHeight - cvRound(r_hist.at<float>(i - 1))),
                cv::Point(binStep * (i),
                          histImgHeight - cvRound(r_hist.at<float>(i))),
                cv::Scalar(0, 0, 255));
        }

        // cv::imshow("histImage", histImage); cv::waitKey();

        /// Debug
        // std::cout << "height: " << histImage.size().height
        //          << " width: " << histImage.size().width
        //          << " channels: " << histImage.channels() << std::endl;

        /// Write Hist image into file and Get the number of bytes
        // unsigned long long len =
        //         writeImageIntoFile(histImage.data, outFileName,
        //         ImageFileType::raw,
        //             histImage.size().width * histImage.size().height *
        //             histImage.channels());

        long long len = LuxWriteImageIntoFile(
            reinterpret_cast<unsigned char *>(histImage.data), outFileName,
            ImageFileType::raw,
            histImage.size().width * histImage.size().height *
                histImage.channels(),
            histImage.size().width, histImage.size().height, CV_8UC3);

        return len;
    }

    /// Don't support other channel
    else {
        perror("Image channel error...");
        return 0;
    }
}

/**
 * @brief Get the 16-bits image data (0000AAAA BBBBCCCC)
 * from origianl image(8-bits, 12-bits, 16-bits)
 * @param orgiImg The pointor of Original image before parsing.
 * @param length The bytes number of orginal image data.
 * @param bpp (bits per pixel) Only support 8, 12, 16
 * @param highZero 0000AAAA AAAAAAAA 0000BBBB BBBBBBBB ?
 * @param outputImg The pointor of image data in memory after parsing.
 * @return unsigned long long. The number of image bytes.
 */
inline unsigned long long LuxParseImageExtendTo16(unsigned char *orgiImg,
                                                  int length, int bpp,
                                                  bool highZero,
                                                  uint16_t *outputImg) {
    uint64_t k = 0;
    switch (bpp) {
        case 8:
            try {
                /// AAAAAAAA
                for (int i = 0; i < length; i += 4) {
                    outputImg[k++] = orgiImg[i] & 0x00FF;
                    outputImg[k++] = orgiImg[i + 1] & 0x00FF;

                    outputImg[k++] = orgiImg[i + 2] & 0x00FF;
                    outputImg[k++] = orgiImg[i + 3] & 0x00FF;
                }
                break;
            } catch (const std::exception &e) {
                std::cerr << e.what() << '\n';
                ::fflush(stderr);
            }
        case 12: {
            try {
                /// 0000AAAA AAAAAAAA 0000BBBB BBBBBBBB
                if (highZero) {
                    for (int i = 0; i < length; i += 8) {
                        outputImg[k++] = ((orgiImg[i] << 8) & 0xFF00) +
                                         (orgiImg[i + 1] & 0x00FF);
                        outputImg[k++] = ((orgiImg[i + 2] << 8) & 0xFF00) +
                                         (orgiImg[i + 3] & 0x00FF);

                        outputImg[k++] = ((orgiImg[i + 4] << 8) & 0xFF00) +
                                         (orgiImg[i + 5] & 0x00FF);
                        outputImg[k++] = ((orgiImg[i + 6] << 8) & 0xFF00) +
                                         (orgiImg[i + 7] & 0x00FF);
                    }
                }
                /// AAAAAAAA AAAABBBB BBBBBBBB
                else {
                    for (int i = 0; i < length; i += 6) {
                        outputImg[k++] =
                            (((orgiImg[i] >> 4) << 8) & 0xFF00) +
                            (((orgiImg[i] << 4) + (orgiImg[i + 1] >> 4)) &
                             0x00FF);
                        outputImg[k++] =
                            ((orgiImg[i + 1] & 0x0F) << 8) + orgiImg[i + 2];

                        outputImg[k++] =
                            (((orgiImg[i + 3] >> 4) << 8) & 0xFF00) +
                            (((orgiImg[i + 3] << 4) + (orgiImg[i + 4] >> 4)) &
                             0x00FF);
                        outputImg[k++] =
                            ((orgiImg[i + 4] & 0x0F) << 8) + orgiImg[i + 5];
                    }
                }
                break;
            } catch (const std::exception &e) {
                std::cerr << e.what() << '\n';
                ::fflush(stderr);
            }
        }

        case 16: {
            try {
                ::memcpy(outputImg, orgiImg, length);
                break;
            } catch (const std::exception &e) {
                std::cerr << e.what() << '\n';
                ::fflush(stderr);
            }
        }

        default:
            std::cerr << "bpp is wrong! It only support [12, 16]" << std::endl;
            ::fflush(stderr);
            break;
    }

    return k;
}

/**
 * @brief Save image data with extern 16-bit to @c outRawFileName.
 *
 * @param imgData
 * @param length
 * @param width
 * @param height
 * @param bpp
 * @param inChannels
 * @param outRawFileName
 * @param isBigEndian
 * @param highZero
 * @return long long.
 * The number of image bytes if success.
 *  -2 : Bits per pixel Don't Supported.
 *  -3 : width or height or bpp or channel are wrong.
 *  -4 : It is not reached.
 */
long long LuxSaveImgDatExtTo16(unsigned char *imgData,
                               unsigned long long length, int width, int height,
                               int bpp, int inChannels,
                               const char *outRawFileName, bool isBigEndian,
                               bool highZero) {
    float bytes = 0.0;
    switch (bpp) {
        case 8:
            bytes = 1;
            break;
        case 12:
            bytes = 1.5;
            break;
        case 16:
            bytes = 2;
            break;
        default:
            std::cerr << "bpp Don't Supported!!! \n"
                      << "Supported depth of bits: 8, 12, 16" << std::endl;
            ::fflush(stderr);
            return -2;
    }

    if (length !=
        static_cast<unsigned long long>(
            static_cast<float>(width * height * inChannels) * bytes)) {
        std::cerr << "width or height or bpp or channel are wrong!!!"
                  << "\nwidth: " << width << "\nheigth: " << height
                  << "\nbits per pixel: " << bpp
                  << "\ninChannels: " << inChannels << std::endl;
        ::fflush(stderr);
        return -3;
    }

    // std::cout << imgData[0] << " , " << imgData[1] << std::endl;
    // Only support big endian
    if (!isBigEndian) {
        LuxEndianRevert(imgData, length, bpp, imgData, true);
    }

    if (bpp == 8 || bpp == 12 || bpp == 16) {
        long long validLength = width * height;
        auto *temp = new uint16_t[validLength];
        uint64_t k =
            LuxParseImageExtendTo16(imgData, length, bpp, highZero, temp);

        k = k > 0 ? LuxWriteImageIntoFileExternTo16(temp, outRawFileName,
                                                    ImageFileType::raw, k,
                                                    width, height, CV_16UC1)
                  : -1;
        delete[] temp;
        return k;
    } else {
        std::cerr << "bpp Error!" << std::endl;
        ::fflush(stderr);
        return -2;
    }

    return -5;
}

/**
 * @brief
 *
 * @param orgiImg
 * @param length
 * @param bpp
 * @param highZero
 * @param outputImg
 * @return unsigned long long
 */
inline unsigned long long LuxParseImageStretchTo16(unsigned char *orgiImg,
                                                   int length, int bpp,
                                                   bool highZero,
                                                   uint16_t *outputImg) {
    uint64_t k = 0;
    switch (bpp) {
        case 8:
            try {
                /// AAAAAAAA
                for (int i = 0; i < length; i += 3) {
                    // orgiImg[i][j] / 2^8 * 2^16
                    outputImg[k] = orgiImg[i] * 256;
                    outputImg[k + 1] = orgiImg[i + 1] * 256;

                    k += 2;
                }
                break;
            } catch (const std::exception &e) {
                std::cerr << e.what() << '\n';
                ::fflush(stderr);
            }
        case 12: {
            try {
                /// 0000AAAA AAAAAAAA 0000BBBB BBBBBBBB
                if (highZero) {
                    for (int i = 0; i < length; i += 4) {
                        // orgiImg[i][j] / 2^12 * 2^16
                        outputImg[k] =
                            ((orgiImg[i] << 8) + orgiImg[i + 1]) * 16;
                        outputImg[k + 1] =
                            ((orgiImg[i + 2] << 8) + orgiImg[i + 3]) * 16;
                        k += 2;
                    }
                }
                /// AAAAAAAA AAAABBBB BBBBBBBB
                else {
                    for (int i = 0; i < length; i += 3) {
                        // orgiImg[i][j] / 2^12 * 2^16
                        outputImg[k] =
                            (((orgiImg[i] >> 4) << 8) +
                             ((orgiImg[i] << 4) + (orgiImg[i + 1] >> 4))) *
                            16;
                        outputImg[k + 1] =
                            (((orgiImg[i + 1] & 0x0F) << 8) + orgiImg[i + 2]) *
                            16;

                        k += 2;
                    }
                }
                break;
            } catch (const std::exception &e) {
                std::cerr << e.what() << '\n';
                ::fflush(stderr);
            }
        }

        case 16: {
            try {
                ::memcpy(outputImg, orgiImg, length);
                break;
            } catch (const std::exception &e) {
                std::cerr << e.what() << '\n';
                ::fflush(stderr);
            }
        }

        default:
            std::cerr << "bpp is wrong! It only support [12, 16]" << std::endl;
            ::fflush(stderr);
            break;
    }

    return k;
}

/**
 * @brief
 *
 * @param imgData
 * @param length
 * @param dataFormat
 * @param width
 * @param height
 * @param bpp
 * @param inChannels
 * @param outChannels
 * @param outData
 * @param isBigEndian
 * @param highZero
 * @param code
 * @return long long
 */
long long LuxLoadImageDataStretchTo16(unsigned char *imgData,
                                      unsigned long long length, int dataFormat,
                                      int width, int height, int bpp,
                                      int inChannels, int outChannels,
                                      uint16_t *outData, bool isBigEndian,
                                      bool highZero, int code) {
    if (dataFormat != 1 && dataFormat != 2 && dataFormat != 3) {
        std::cerr << "Data Format Don't Supported!!! \n"
                  << "1: raw, 2: bayer, 3: others" << std::endl;
        ::fflush(stderr);
        return -1;
    }

    float bytes = 0.0;
    switch (bpp) {
        case 8:
            bytes = 1;
            break;
        case 12:
            bytes = 1.5;
            break;
        case 16:
            bytes = 2;
            break;
        default:
            std::cerr << "bpp Don't Supported!!! \n"
                      << "Supported depth of bits: 8, 12, 16" << std::endl;
            ::fflush(stderr);
            return -2;
    }

    if (length !=
        static_cast<unsigned long long>(
            static_cast<float>(width * height * inChannels) * bytes)) {
        std::cerr << "width or height or bpp or channel are wrong!!!"
                  << "\nwidth: " << width << "\nheigth: " << height
                  << "\nbits per pixel: " << bpp
                  << "\ninChannels: " << inChannels << std::endl;
        ::fflush(stderr);
        return -3;
    }

    // Only support big endian
    if (!isBigEndian) {
        LuxEndianRevert(imgData, length, bpp, imgData, true);
    }

    /* raw */
    if (dataFormat == 1) {
        if (bpp == 8 || bpp == 12 || bpp == 16) {
            long long validLength = width * height;
            auto *temp = new uint16_t[validLength];
            uint64_t k =
                LuxParseImageStretchTo16(imgData, length, bpp, highZero, temp);
            (void)k;

            // uint64_t k =
            // LuxParseImageExtendTo16(reinterpret_cast<uint8_t*>(imgData),
            // length, bpp, highZero,
            //              reinterpret_cast<uint16_t*>(outData));

            cv::Mat bayer16BitMat(height, width, CV_16UC1, temp);
            cv::Mat outputImg(height, width, CV_16UC1, outData);
            cv::cvtColor(bayer16BitMat, outputImg, code);

            delete[] temp;
            /* 图片大小 （字节数） */
            return k;
        } else {
            std::cerr << "bpp Error!" << std::endl;
            fflush(stderr);
            return -4;
        }
    }

    /* Bayer */
    else if (dataFormat == 2) {
        if (bpp == 8 || bpp == 12 || bpp == 16) {
            long long validLength = width * height;
            auto *temp = new uint16_t[validLength];
            uint64_t k =
                LuxParseImageStretchTo16(imgData, length, bpp, highZero, temp);
            (void)k;

            /// 16UC1 Bayer
            cv::Mat bayer16BitMat(height, width, CV_16UC1, temp);
            cv::Mat rgb16BitMat(height, width, CV_16UC3, outData);
            cv::cvtColor(bayer16BitMat, rgb16BitMat, code);

            delete[] temp;
            return rgb16BitMat.size().height * rgb16BitMat.size().width *
                   rgb16BitMat.channels();
        } else {
            std::cerr << "bpp Error!" << std::endl;
            fflush(stderr);
            return -4;
        }
    }

    return -5;
}

/**
 * @brief Load image data (Stretch to 16-bit) from file.
 * @note When Python Call the Function, the ALL parameters must be SET.
 * @param inputFileName The file before parsing.
 * @param dataFormat 1: raw, 2: bayer, 3: others
 * @param width Width
 * @param height Height
 * @param bpp bpp(Bits Per Pixel)  8, 12, 16
 * @param channels Channels
 * @param outputRawFileName The .raw file after parsing.
 * @param outputTiffFileName The .tiff file after parsing.
 * @param isBigEndian Big Endian(be) ?
 * @param highZero 0000AAAA AAAAAAAA ?
 * @param saveTiff Save tiff ?
 * @param code
 *  - CV_BayerBG2BGR =46,
 *  - CV_BayerGB2BGR =47,
 *  - CV_BayerRG2BGR =48,
 *  - CV_BayerGR2BGR =49,
 *  - CV_BayerBG2RGB =CV_BayerRG2BGR = 48,
 *  - CV_BayerGB2RGB =CV_BayerGR2BGR = 49,
 *  - CV_BayerRG2RGB =CV_BayerBG2BGR = 46,
 *  - CV_BayerGR2RGB =CV_BayerGB2BGR = 47,
 *  - CV_BayerRG2GRAY = 88
 *  - CV_BayerBG2GRAY = 86,
 *  - CV_BayerGB2GRAY = 87,
 *  - CV_BayerRG2GRAY = 88,
 *  - CV_BayerGR2GRAY = 89
 *
 * @return long long
 * The bytes number of image file if success.
 *  -1 : Data Format Don't Supported.
 *  -2 : Bits per pixel Don't Supported.
 *  -3 : width or height or bpp or channel are wrong.
 *  -4 : DownBits Error!
 *  -5 : It is not reached.
 */
long long LuxLoadImageDataFromFileStretchTo16(
    const char *inputFileName, int dataFormat, int width, int height, int bpp,
    int inChannels, int outChannels, const char *outputRawFileName,
    const char *outputTiffFileName, bool isBigEndian, bool highZero,
    bool saveTiff, int code) {
    if (dataFormat != 1 && dataFormat != 2 && dataFormat != 3) {
        std::cerr << "Data Format Don't Supported!!! \n"
                  << "1: raw, 2: bayer, 3: others" << std::endl;
        ::fflush(stderr);
        return -1;
    }

    float bytes = 0.0;
    switch (bpp) {
        case 8:
            bytes = 1;
            break;
        case 12:
            bytes = 1.5;
            break;
        case 16:
            bytes = 2;
            break;
        default:
            std::cerr << "bpp Don't Supported!!! \n"
                      << "Supported depth of bits: 8, 12, 16" << std::endl;
            ::fflush(stderr);
            return -2;
    }

    std::ifstream ifstrm(reinterpret_cast<const char *>(inputFileName),
                         std::ios_base::binary);
    if (ifstrm.is_open() == false) {
        std::cerr << "Fail to read " << inputFileName << std::endl;
        ::fflush(stderr);
        return -4;
    }

    /// Check the length of file
    unsigned long long length = static_cast<unsigned long long int>(
        static_cast<float>(width * height * inChannels) * bytes);
    ifstrm.seekg(0, ifstrm.end);
    unsigned long long ret = ifstrm.tellg();
    ifstrm.seekg(0, ifstrm.beg);
    if (ret != length) {
        std::cerr << "The length of file is NOT right." << std::endl;
        ::fflush(stderr);
        return -3;
    }

    /// Read bytes into imgData
    auto *imgData = new unsigned char[length];
    ifstrm.read((char *)imgData, length);

    /// According to the target, Set channels, Type of outputFile
    uint16_t *outData = nullptr;
    int cvType = CV_16UC1;
    /// raw
    if (dataFormat == 1) {
        outData = new uint16_t[width * height * outChannels];
        cvType = CV_16UC1;
    }
    /// Bayer / others
    else {
        outData = new uint16_t[width * height * outChannels];
        cvType = CV_16UC3;
    }

    unsigned long long k = LuxLoadImageDataStretchTo16(
        imgData, length, dataFormat, width, height, bpp, inChannels,
        outChannels, outData, isBigEndian, highZero, code);

    /// k > 0 is ok
    if (k > 0) {
        // For display
        unsigned long long _ret =
            LuxWriteImageIntoFile(outData, outputRawFileName,
                                  ImageFileType::raw, k, width, height, cvType);

        // TIFF
        if (saveTiff)
            LuxWriteImageIntoFile(outData, outputTiffFileName,
                                  ImageFileType::tiff, k, width, height,
                                  cvType);

        delete[] imgData;
        delete[] outData;
        return _ret;
    } else {
        delete[] imgData;
        delete[] outData;
        return k;
    }

    return -5;
}

/**
 * @brief Get the 8-bits image data from origianl image(8-bits, 12-bits,
 * 16-bits)
 *
 * @param orgiImg The pointor of Original image before parsing.
 * @param length The bytes number of orginal image data.
 * @param bpp (bits per pixel) Only support 8, 12, 16
 * @param highZero high 4-bit data (0000AAAA AAAAAAAA 0000BBBB BBBBBBBB) is zero
 * ?
 * @param outputImg The pointor of image data in memory after parsing.
 * @param mode {0, 1, 2, 3, 4, 5}
 *  - 0: P0P1P2P3P4P5P6P7 P8P9PaPbQ0Q1Q2Q3 Q4Q5Q6Q7Q8Q9QaQb -> P0P1P2P3P4P5P6P7
 * Q0Q1Q2Q3Q4Q5Q6Q7
 *  - 1: P0P1P2P3P4P5P6P7 P8P9PaPbQ0Q1Q2Q3 Q4Q5Q6Q7Q8Q9QaQb -> P1P2P3P4P5P6P7P8
 * Q1Q2Q3Q4Q5Q6Q7Q8
 *  - 2: P0P1P2P3P4P5P6P7 P8P9PaPbQ0Q1Q2Q3 Q4Q5Q6Q7Q8Q9QaQb -> P2P3P4P5P6P7P8P9
 * Q2Q3Q4Q5Q6Q7Q8Q9
 *  - 3: P0P1P2P3P4P5P6P7 P8P9PaPbQ0Q1Q2Q3 Q4Q5Q6Q7Q8Q9QaQb -> P3P4P5P6P7P8P9Pa
 * Q3Q4Q5Q6Q7Q8Q9Qa
 *  - 4: P0P1P2P3P4P5P6P7 P8P9PaPbQ0Q1Q2Q3 Q4Q5Q6Q7Q8Q9QaQb -> P4P5P6P7P8P9PaPb
 * Q4Q5Q6Q7Q8Q9QaQb
 *  - 5: all in 8
 * @return long long. The number of image bytes
 */
inline long long LuxParseImageEnhanced(unsigned char *orgiImg, int length,
                                       int bpp, bool highZero,
                                       unsigned char *outputImg, int mode) {
    if (bpp != 8 && bpp != 12 && bpp != 16) {
        std::cerr << "bpp is wrong! It only support [8, 12, 16]" << std::endl;
        ::fflush(stderr);
        return -1;
    }

    if (mode != 0 && mode != 1 && mode != 2 && mode != 3 && mode != 4 &&
        mode != 5) {
        std::cerr << "Mode don't support. \n"
                  << "Mode must be in [0, 1, 2, 3, 4, 5]" << std::endl;
        ::fflush(stderr);
        return -2;
    }

    uint64_t k = 0;

    switch (mode) {
        /// P0P1P2P3P4P5P6P7 P8P9PaPbQ0Q1Q2Q3 Q4Q5Q6Q7Q8Q9QaQb
        /// P0P1P2P3P4P5P6P7         Q0Q1Q2Q3Q4Q5Q6Q7
        case 0: {
            switch (bpp) {
                case 8: {
                    try {
                        ::memcpy(outputImg, orgiImg, length);
                        return length;
                    } catch (const std::exception &e) {
                        std::cerr << e.what() << '\n';
                        ::fflush(stderr);
                        return 0;
                    }
                }

                case 12: {
                    try {
                        /// 0000AAAA AAAAAAAA 0000BBBB BBBBBBBB
                        if (highZero) {
                            // DONE Tested
                            for (int i = 0; i < length; i += 4) {
                                outputImg[k++] = ((orgiImg[i] << 4u) & 0xFF) +
                                                 (orgiImg[i + 1] >> 4u);
                                outputImg[k++] =
                                    ((orgiImg[i + 2] << 4u) & 0xFF) +
                                    (orgiImg[i + 3] >> 4u);
                            }
                        }
                        /// AAAAAAAA AAAABBBB BBBBBBBB
                        else {
                            // DONE 已测试
                            for (int i = 0; i < length; i += 3) {
                                outputImg[k++] = orgiImg[i];
                                outputImg[k++] =
                                    ((orgiImg[i + 1] << 4u) & 0xFF) +
                                    (orgiImg[i + 2] >> 4u);
                            }
                        }
                        break;
                    } catch (const std::exception &e) {
                        std::cerr << e.what() << '\n';
                        ::fflush(stderr);
                        return 0;
                    }
                }

                //
                case 16: {
                    try {
                        /// 0000AAAA AAAAAAAA
                        // DONE Tested
                        if (highZero) {
                            for (int i = 0; i < length; i += 2) {
                                outputImg[k++] = ((orgiImg[i] << 4) & 0xFF) +
                                                 (orgiImg[i + 1] >> 4);
                            }
                        }
                        /// AAAAAAAA AAAAAAAA
                        // DONE Tested
                        else {
                            /// Set the higt 8 bit into outptImg
                            for (int i = 0; i < length; i += 2) {
                                outputImg[k++] = orgiImg[i];
                            }
                        }
                        break;
                    } catch (const std::exception &e) {
                        std::cerr << e.what() << '\n';
                        ::fflush(stderr);
                        return 0;
                    }
                }

                default:
                    std::cerr << "It is never reached." << std::endl;
                    ::fflush(stderr);
                    return -1;
            }
            break;
        }

        /// P0P1P2P3P4P5P6P7 P8P9PaPbQ0Q1Q2Q3 Q4Q5Q6Q7Q8Q9QaQb
        ///   P1P2P3P4P5P6P7P8         Q1Q2Q3Q4Q5Q6Q7Q8
        case 1: {
            switch (bpp) {
                case 8: {
                    try {
                        ::memcpy(outputImg, orgiImg, length);
                        return length;
                        break;
                    } catch (const std::exception &e) {
                        std::cerr << e.what() << '\n';
                        ::fflush(stderr);
                        return 0;
                    }
                }

                case 12: {
                    try {
                        /// 0000AAAA AAAAAAAA 0000BBBB BBBBBBBB
                        if (highZero) {
                            // DONE Tested
                            for (int i = 0; i < length; i += 4) {
                                outputImg[k++] = ((orgiImg[i] & 0x07) << 5) +
                                                 ((orgiImg[i + 1] & 0xF8) >> 3);
                                outputImg[k++] =
                                    ((orgiImg[i + 2] & 0x07) << 5) +
                                    ((orgiImg[i + 3] & 0xF8) >> 3);
                            }
                        }
                        /// AAAAAAAA AAAABBBB BBBBBBBB
                        else {
                            // DONE Tested
                            for (int i = 0; i < length; i += 3) {
                                outputImg[k++] = ((orgiImg[i] & 0x7F) << 1) +
                                                 ((orgiImg[i + 1] & 0x80) >> 7);
                                outputImg[k++] =
                                    ((orgiImg[i + 1] & 0x07) << 5) +
                                    ((orgiImg[i + 2] & 0xF8) >> 3);
                            }
                        }
                        break;
                    } catch (const std::exception &e) {
                        std::cerr << e.what() << '\n';
                        ::fflush(stderr);
                        return 0;
                    }
                }

                case 16: {
                    try {
                        if (highZero) {
                            // DONE Tested
                            for (int i = 0; i < length; i += 4) {
                                outputImg[k++] = ((orgiImg[i] & 0x07) << 5) +
                                                 ((orgiImg[i + 1] & 0xF8) >> 3);
                                outputImg[k++] =
                                    ((orgiImg[i + 2] & 0x07) << 5) +
                                    ((orgiImg[i + 3] & 0xF8) >> 3);
                            }
                        } else {
                            // DONE Tested
                            for (int i = 0; i < length; i += 4) {
                                outputImg[k++] = ((orgiImg[i] & 0x7F) << 1) +
                                                 ((orgiImg[i + 1] & 0x80) >> 7);
                                outputImg[k++] =
                                    ((orgiImg[i + 2] & 0x7F) << 1) +
                                    ((orgiImg[i + 3] & 0x80) >> 7);
                            }
                        }
                        break;
                    } catch (const std::exception &e) {
                        std::cerr << e.what() << '\n';
                        ::fflush(stderr);
                        return 0;
                    }
                }

                default:
                    std::cerr << "It is never reached." << std::endl;
                    ::fflush(stderr);
                    return -1;
            }
            break;
        }

        /// P0P1P2P3P4P5P6P7 P8P9PaPbQ0Q1Q2Q3 Q4Q5Q6Q7Q8Q9QaQb
        ///     P2P3P4P5P6P7P8P9         Q2Q3Q4Q5Q6Q7Q8Q9
        case 2: {
            switch (bpp) {
                case 8: {
                    try {
                        ::memcpy(outputImg, orgiImg, length);
                        return length;
                    } catch (const std::exception &e) {
                        std::cerr << e.what() << '\n';
                        ::fflush(stderr);
                        return 0;
                    }
                }

                case 12: {
                    try {
                        /// 0000AAAA AAAAAAAA 0000BBBB BBBBBBBB
                        if (highZero) {
                            // DONE Tested
                            for (int i = 0; i < length; i += 4) {
                                outputImg[k++] = ((orgiImg[i] & 0x03) << 6) +
                                                 ((orgiImg[i + 1] & 0xFC) >> 2);
                                outputImg[k++] =
                                    ((orgiImg[i + 2] & 0x03) << 6) +
                                    ((orgiImg[i + 3] & 0xFC) >> 2);
                            }
                        }
                        /// AAAAAAAA AAAABBBB BBBBBBBB
                        else {
                            // DONE Tested
                            for (int i = 0; i < length; i += 3) {
                                outputImg[k++] = ((orgiImg[i] & 0x3F) << 2) +
                                                 ((orgiImg[i + 1] & 0xC0) >> 6);
                                //                 000000BB BBBBBB00
                                outputImg[k++] =
                                    ((orgiImg[i + 1] & 0x03) << 6) +
                                    ((orgiImg[i + 2] & 0xFC) >> 2);
                            }
                        }
                        break;
                    } catch (const std::exception &e) {
                        std::cerr << e.what() << '\n';
                        ::fflush(stderr);
                        return 0;
                    }
                }

                case 16: {
                    try {
                        if (highZero) {
                            // DONE Tested
                            for (int i = 0; i < length; i += 4) {
                                outputImg[k++] = ((orgiImg[i] & 0x03) << 6) +
                                                 ((orgiImg[i + 1] & 0xFC) >> 2);
                                outputImg[k++] =
                                    ((orgiImg[i + 2] & 0x03) << 6) +
                                    ((orgiImg[i + 3] & 0xFC) >> 2);
                            }
                        } else {
                            // DONE Tested
                            for (int i = 0; i < length; i += 4) {
                                outputImg[k++] = ((orgiImg[i] & 0x3F) << 2) +
                                                 ((orgiImg[i + 1] & 0xC0) >> 6);
                                outputImg[k++] =
                                    ((orgiImg[i + 2] & 0x3F) << 2) +
                                    ((orgiImg[i + 3] & 0xC0) >> 6);
                            }
                        }
                        break;
                    } catch (const std::exception &e) {
                        std::cerr << e.what() << '\n';
                        ::fflush(stderr);
                        return 0;
                    }
                }

                default:
                    std::cerr << "It is never reached." << std::endl;
                    ::fflush(stderr);
                    return -1;
            }
            break;
        }

        /// P0P1P2P3P4P5P6P7 P8P9PaPbQ0Q1Q2Q3 Q4Q5Q6Q7Q8Q9QaQb
        ///       P3P4P5P6P7P8P9Pa         Q3Q4Q5Q6Q7Q8Q9Qa
        case 3: {
            switch (bpp) {
                case 8: {
                    try {
                        ::memcpy(outputImg, orgiImg, length);
                        return length;
                        break;
                    } catch (const std::exception &e) {
                        std::cerr << e.what() << '\n';
                        ::fflush(stderr);
                        return 0;
                    }
                }

                case 12: {
                    try {
                        /// 0000AAAA AAAAAAAA 0000BBBB BBBBBBBB
                        if (highZero) {
                            // DONE Tested
                            for (int i = 0; i < length; i += 4) {
                                outputImg[k++] = ((orgiImg[i] & 0x01) << 7) +
                                                 ((orgiImg[i + 1] & 0xFE) >> 1);
                                outputImg[k++] =
                                    ((orgiImg[i + 2] & 0x01) << 7) +
                                    ((orgiImg[i + 3] & 0xFE) >> 1);
                            }
                        }
                        /// AAAAAAAA AAAABBBB BBBBBBBB
                        else {
                            // DONE Tested
                            for (int i = 0; i < length; i += 3) {
                                outputImg[k++] = ((orgiImg[i] & 0x1F) << 3) +
                                                 ((orgiImg[i + 1] & 0xE0) >> 5);
                                outputImg[k++] =
                                    ((orgiImg[i + 1] & 0x01) << 7) +
                                    ((orgiImg[i + 2] & 0xFE) >> 1);
                            }
                        }
                        break;
                    } catch (const std::exception &e) {
                        std::cerr << e.what() << '\n';
                        ::fflush(stderr);
                        return 0;
                    }
                }

                ///
                case 16: {
                    try {
                        if (highZero) {
                            // DONE Tested
                            for (int i = 0; i < length; i += 4) {
                                outputImg[k++] = ((orgiImg[i] & 0x01) << 7) +
                                                 ((orgiImg[i + 1] & 0xFE) >> 1);
                                outputImg[k++] =
                                    ((orgiImg[i + 2] & 0x01) << 7) +
                                    ((orgiImg[i + 3] & 0xFE) >> 1);
                            }
                        }

                        else {
                            // DONE Tested
                            for (int i = 0; i < length; i += 4) {
                                outputImg[k++] = ((orgiImg[i] & 0x1F) << 3) +
                                                 ((orgiImg[i + 1] & 0xE0) >> 5);
                                outputImg[k++] =
                                    ((orgiImg[i + 2] & 0x1F) << 3) +
                                    ((orgiImg[i + 3] & 0xE0) >> 5);
                            }
                        }
                        break;
                    } catch (const std::exception &e) {
                        std::cerr << e.what() << '\n';
                        ::fflush(stderr);
                        return 0;
                    }
                }

                default:
                    std::cerr << "bpp is wrong! It only support [12, 16]"
                              << std::endl;
                    ::fflush(stderr);
                    break;
            }
            break;
        }

        /// P0P1P2P3P4P5P6P7 P8P9PaPbQ0Q1Q2Q3 Q4Q5Q6Q7Q8Q9QaQb
        ///         P4P5P6P7P8P9PaPb          Q4Q5Q6Q7Q8Q9QaQb
        case 4: {
            switch (bpp) {
                case 8: {
                    try {
                        ::memcpy(outputImg, orgiImg, length);
                        return length;
                    } catch (const std::exception &e) {
                        std::cerr << e.what() << '\n';
                        ::fflush(stderr);
                        return 0;
                    }
                }

                case 12: {
                    try {
                        /// 0000AAAA AAAAAAAA 0000BBBB BBBBBBBB
                        if (highZero) {
                            // DONE Tested
                            for (int i = 0; i < length; i += 4) {
                                outputImg[k++] = orgiImg[i + 1];
                                outputImg[k++] = orgiImg[i + 3];
                            }
                        }
                        /// AAAAAAAA AAAABBBB BBBBBBBB
                        else {
                            // DONE Tested
                            for (int i = 0; i < length; i += 3) {
                                outputImg[k++] = ((orgiImg[i] & 0x0F) << 4) +
                                                 ((orgiImg[i + 1] & 0xF0) >> 4);
                                outputImg[k++] = orgiImg[i + 2];
                            }
                        }
                        break;
                    } catch (const std::exception &e) {
                        std::cerr << e.what() << '\n';
                        ::fflush(stderr);
                        return 0;
                    }
                }

                ///
                case 16: {
                    try {
                        if (highZero) {
                            // DONE Tested
                            for (int i = 0; i < length; i += 4) {
                                outputImg[k++] = orgiImg[i + 1];
                                outputImg[k++] = orgiImg[i + 3];
                            }
                        } else {
                            // DONE Tested
                            for (int i = 0; i < length; i += 4) {
                                outputImg[k++] = ((orgiImg[i] & 0x0F) << 4) +
                                                 ((orgiImg[i + 1] & 0xF0) >> 4);
                                outputImg[k++] =
                                    ((orgiImg[i + 2] & 0x0F) << 4) +
                                    ((orgiImg[i + 3] & 0xF0) >> 4);
                            }
                        }
                        break;
                    } catch (const std::exception &e) {
                        std::cerr << e.what() << '\n';
                        ::fflush(stderr);
                        return 0;
                    }
                }

                default:
                    std::cerr << "It is never reached." << std::endl;
                    ::fflush(stderr);
                    return -1;
            }
            break;
        }

        case 5: {
            switch (bpp) {
                case 8: {
                    try {
                        ::memcpy(outputImg, orgiImg, length);
                        return length;
                    } catch (const std::exception &e) {
                        std::cerr << e.what() << '\n';
                        ::fflush(stderr);
                        return 0;
                    }
                }

                case 12: {
                    try {
                        /// 0000AAAA AAAAAAAA 0000BBBB BBBBBBBB
                        if (highZero) {
                            int newLen = length / 2;
                            k = LuxNormalize<uint16_t, uint8_t>(
                                reinterpret_cast<uint16_t *>(orgiImg), newLen,
                                (uint8_t *)outputImg);

                            // auto max = std::get<0>(LuxFindMaxMin<uint16_t>(
                            //     reinterpret_cast<uint16_t*>(orgiImg), length
                            //     / 2));

                            // uint16_t* t =
                            // reinterpret_cast<uint16_t*>(orgiImg); for(int i =
                            // 0; i < length / 2; ++i) {
                            //     outputImg[k++] = normlize255<uint16_t,
                            //     uint8_t>(t[i], max);
                            // }
                        }

                        /// AAAAAAAA AAAABBBB BBBBBBBB
                        else {
                            uint64_t newLen = length / 3 * 4 / 2;
                            auto *temp = new uint16_t[newLen];
                            auto len = LuxParseImageExtendTo16(orgiImg, length,
                                                               12, false, temp);
                            assert(newLen == len);

                            k = LuxNormalize<uint16_t, uint8_t>(temp, newLen,
                                                                outputImg);

                            // auto max = std::get<0>(LuxFindMaxMin<uint16_t>(
                            //     temp, newLen));

                            // uint16_t* t =
                            // reinterpret_cast<uint16_t*>(orgiImg); for(int i =
                            // 0; i < length / 2; ++i) {
                            //     outputImg[k++] = normlize255<uint16_t,
                            //     uint8_t>(t[i], max);
                            // }
                            delete[] temp;
                        }
                        break;
                    } catch (const std::exception &e) {
                        std::cerr << e.what() << '\n';
                        ::fflush(stderr);
                    }
                }

                ///
                case 16: {
                    try {
                        int newLen = length / 2;
                        k = LuxNormalize<uint16_t, uint8_t>(
                            reinterpret_cast<uint16_t *>(orgiImg), newLen,
                            outputImg);
                        break;
                    } catch (const std::exception &e) {
                        std::cerr << e.what() << '\n';
                        ::fflush(stderr);
                    }
                }
                default:
                    std::cerr << "It is never reached." << std::endl;
                    ::fflush(stderr);
                    return -1;
            }
            break;
        }

        default: {
            std::cerr << "It is never reached." << std::endl;
            ::fflush(stderr);
            break;
        }
    }

    // Bytes
    return k;
}

/**
 * @brief Load image data from memory
 * @note When Python Call the Function, the ALL parameters must be SET.
 * @param imgData The poniter of image data in memory before parsing.
 * @param length The bytes number of image data in memory.
 * @param dataFormat 1: raw, 2: bayer, 3: others
 * @param width Width
 * @param height Height
 * @param bpp (Bits Per Pixel)  8, 12, 16
 * @param inChannels inChannels
 * @param outData The poniter of image data in memory after parsing.
 * @param isBigEndian Big Endian(be) ?
 * @param highZero 0000AAAA AAAAAAAA ?
 * @param mode [0, 1, 2, 3, 4]
 * 0 - P0P1P2P3P4P5P6P7 P8P9PaPbQ0Q1Q2Q3 Q4Q5Q6Q7Q8Q9QaQb -> P0P1P2P3P4P5P6P7
 * Q0Q1Q2Q3Q4Q5Q6Q7 1 - P0P1P2P3P4P5P6P7 P8P9PaPbQ0Q1Q2Q3 Q4Q5Q6Q7Q8Q9QaQb ->
 * P1P2P3P4P5P6P7P8 Q1Q2Q3Q4Q5Q6Q7Q8 2 - P0P1P2P3P4P5P6P7 P8P9PaPbQ0Q1Q2Q3
 * Q4Q5Q6Q7Q8Q9QaQb -> P2P3P4P5P6P7P8P9 Q2Q3Q4Q5Q6Q7Q8Q9 3 - P0P1P2P3P4P5P6P7
 * P8P9PaPbQ0Q1Q2Q3 Q4Q5Q6Q7Q8Q9QaQb -> P3P4P5P6P7P8P9Pa Q3Q4Q5Q6Q7Q8Q9Qa 4 -
 * P0P1P2P3P4P5P6P7 P8P9PaPbQ0Q1Q2Q3 Q4Q5Q6Q7Q8Q9QaQb -> P4P5P6P7P8P9PaPb
 * Q4Q5Q6Q7Q8Q9QaQb
 * @param code
 *  - CV_BayerBG2BGR =46,
 *  - CV_BayerGB2BGR =47,
 *  - CV_BayerRG2BGR =48,
 *  - CV_BayerGR2BGR =49,
 *  - CV_BayerBG2RGB =CV_BayerRG2BGR = 48,
 *  - CV_BayerGB2RGB =CV_BayerGR2BGR = 49,
 *  - CV_BayerRG2RGB =CV_BayerBG2BGR = 46,
 *  - CV_BayerGR2RGB =CV_BayerGB2BGR = 47,
 *  - CV_BayerRG2GRAY = 88
 *  - CV_BayerBG2GRAY = 86,
 *  - CV_BayerGB2GRAY = 87,
 *  - CV_BayerRG2GRAY = 88,
 *  - CV_BayerGR2GRAY = 89
 * @return long long
 * The bytes number of image file if success.
 *  -1 : Data Format Don't Supported.
 *  -2 : Bits per pixel Don't Supported.
 *  -4 : width or height or bpp or channel are wrong.
 *  -5 : It is not reached.
 */
long long LuxLoadImageDataEnhanced(unsigned char *imgData,
                                   unsigned long long length, int dataFormat,
                                   int width, int height, int bpp,
                                   int inChannels, unsigned char *outData,
                                   bool isBigEndian, bool highZero, int mode,
                                   int code) {
    if (dataFormat != 1 && dataFormat != 2 && dataFormat != 3) {
        std::cerr << "Data Format Don't Supported!!! \n"
                  << "1: raw, 2: bayer, 3: others" << std::endl;
        ::fflush(stderr);
        return -1;
    }

    float bytes = 0.0;
    switch (bpp) {
        case 8:
            bytes = 1;
            break;

        case 12:
            bytes = 1.5;
            break;
        case 16:
            bytes = 2;
            break;

        default:
            std::cerr << "bpp Don't Supported!!! \n"
                      << "Supported depth of bits: 8, 12, 16" << std::endl;
            ::fflush(stderr);
            return -2;
    }

    if (length !=
        static_cast<unsigned long long>(
            static_cast<float>(width * height * inChannels) * bytes)) {
        std::cerr << "width or height or bpp or channel are wrong!!!"
                  << "\nlenght: " << length << "\nwidth: " << width
                  << "\nheigth: " << height << "\nbits per pixel: " << bpp
                  << "\ninChannels: " << inChannels << std::endl;
        ::fflush(stderr);
        return -4;
    }

    // Only support big endian
    if (!isBigEndian) {
        LuxEndianRevert(imgData, length, bpp, imgData, true);
    }

    /// bind mode
    auto parseImage =
        std::bind(&LuxParseImageEnhanced, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3,
                  std::placeholders::_4, std::placeholders::_5, mode);

    // TODO 代码优化： 加入 outChannels/types
    /* raw */
    if (dataFormat == 1) {
        long long validLength = width * height;
        auto *temp = new unsigned char[validLength];
        // uint64_t k = LuxParseImage(imgData, length, bpp, highZero, temp);
        uint64_t k = parseImage(imgData, length, bpp, highZero, temp);
        (void)k;

        cv::Mat bayer8BitMat(height, width, CV_8UC1, temp);
        cv::Mat outputImg(height, width, CV_8UC1, outData);
        cv::cvtColor(bayer8BitMat, outputImg, code);

        delete[] temp;
        /* 图片大小 （字节数） */
        return outputImg.size().width * outputImg.size().height *
               outputImg.channels();
    }

    /* Bayer */
    else if (dataFormat == 2) {
        int outChannels = 3;
        long long validLength = width * height * outChannels;
        auto *temp = new unsigned char[validLength];
        // uint64_t k = LuxParseImage(imgData, length, bpp, highZero, temp);
        uint64_t k = parseImage(imgData, length, bpp, highZero, temp);
        (void)k;

        /// 16UC1 Bayer
        cv::Mat bayer8BitMat(height, width, CV_8UC1, temp);
        cv::Mat rgb8BitMat(height, width, CV_8UC3, outData);
        cv::cvtColor(bayer8BitMat, rgb8BitMat, code);

        // cv::imshow("LuxTW2", rgb8BitMat);
        // cv::waitKey();

        delete[] temp;
        return rgb8BitMat.size().height * rgb8BitMat.size().width *
               rgb8BitMat.channels();
    }

    return -5;
}

/**
 * @brief Load image data from file.
 * @note When Python Call the Function, the ALL parameters must be SET.
 * @param inputFileName The file before parsing.
 * @param dataFormat 1: raw, 2: bayer, 3: others
 * @param width Width
 * @param height Height
 * @param bpp bpp(Bits Per Pixel)  8, 12, 16
 * @param channels Channels
 * @param outputRawFileName The .raw file after parsing.
 * @param outputTiffFileName The .tiff file after parsing.
 * @param isBigEndian Big Endian(be) ?
 * @param highZero 0000AAAA AAAAAAAA ?
 * @param saveTiff Save tiff ?
 * @param mode [0, 1, 2, 3, 4]
 * 0 - P0P1P2P3P4P5P6P7 P8P9PaPbQ0Q1Q2Q3 Q4Q5Q6Q7Q8Q9QaQb -> P0P1P2P3P4P5P6P7
 * Q0Q1Q2Q3Q4Q5Q6Q7 1 - P0P1P2P3P4P5P6P7 P8P9PaPbQ0Q1Q2Q3 Q4Q5Q6Q7Q8Q9QaQb ->
 * P1P2P3P4P5P6P7P8 Q1Q2Q3Q4Q5Q6Q7Q8 2 - P0P1P2P3P4P5P6P7 P8P9PaPbQ0Q1Q2Q3
 * Q4Q5Q6Q7Q8Q9QaQb -> P2P3P4P5P6P7P8P9 Q2Q3Q4Q5Q6Q7Q8Q9 3 - P0P1P2P3P4P5P6P7
 * P8P9PaPbQ0Q1Q2Q3 Q4Q5Q6Q7Q8Q9QaQb -> P3P4P5P6P7P8P9Pa Q3Q4Q5Q6Q7Q8Q9Qa 4 -
 * P0P1P2P3P4P5P6P7 P8P9PaPbQ0Q1Q2Q3 Q4Q5Q6Q7Q8Q9QaQb -> P4P5P6P7P8P9PaPb
 * Q4Q5Q6Q7Q8Q9QaQb
 * @param code
 *  - CV_BayerBG2BGR =46,
 *  - CV_BayerGB2BGR =47,
 *  - CV_BayerRG2BGR =48,
 *  - CV_BayerGR2BGR =49,
 *  - CV_BayerBG2RGB =CV_BayerRG2BGR = 48,
 *  - CV_BayerGB2RGB =CV_BayerGR2BGR = 49,
 *  - CV_BayerRG2RGB =CV_BayerBG2BGR = 46,
 *  - CV_BayerGR2RGB =CV_BayerGB2BGR = 47,
 *  - CV_BayerRG2GRAY = 88
 *  - CV_BayerBG2GRAY = 86,
 *  - CV_BayerGB2GRAY = 87,
 *  - CV_BayerRG2GRAY = 88,
 *  - CV_BayerGR2GRAY = 89
 *
 * @return long long
 * The bytes number of image file if success.
 *  -1 : Data Format Don't Supported.
 *  -2 : Bits per pixel Don't Supported.
 *  -3 : File open failed.
 *  -4 : width or height or bpp or channel are wrong.
 *  -5 : It is not reached.
 */
long long LuxLoadImageDataFromFileEnhanced(
    const char *inputFileName, int dataFormat, int width, int height, int bpp,
    int channels, const char *outputRawFileName, const char *outputTiffFileName,
    bool isBigEndian, bool highZero, bool saveTiff, int mode, int code) {
    if (dataFormat != 1 && dataFormat != 2 && dataFormat != 3) {
        std::cerr << "Data Format Don't Supported!!! \n"
                  << "1: raw, 2: bayer, 3: others" << std::endl;
        ::fflush(stderr);
        return -1;
    }

    float bytes = 0.0;
    switch (bpp) {
        case 8:
            bytes = 1;
            break;
        case 12:
            bytes = 1.5;
            break;
        case 16:
            bytes = 2;
            break;
        default:
            std::cerr << "bpp Don't Supported!!! \n"
                      << "Supported depth of bits: 8, 12, 16" << std::endl;
            ::fflush(stderr);
            return -2;
    }

    std::ifstream ifstrm(reinterpret_cast<const char *>(inputFileName),
                         std::ios_base::binary);
    if (ifstrm.is_open() == false) {
        std::cerr << "Fail to read " << inputFileName << std::endl;
        ::fflush(stderr);
        return -3;
    }

    /// Check the length of file
    unsigned long long length = static_cast<unsigned long long int>(
        static_cast<float>(width * height * channels) * bytes);
    ifstrm.seekg(0, ifstrm.end);
    unsigned long long ret = ifstrm.tellg();
    ifstrm.seekg(0, ifstrm.beg);
    if (ret != length) {
        std::cerr << "The length of file is NOT right." << std::endl
                  << "width: " << width << " height: " << height
                  << " channels: " << channels << " bpp: " << bpp << std::endl
                  << "Target length: " << length << " File length: " << ret
                  << std::endl;
        ::fflush(stderr);
        return -4;
    }

    /// Read bytes into imgData
    auto *imgData = new unsigned char[length];
    ifstrm.read((char *)imgData, length);

    /// According to the target, Set channels, Type of outputFile
    unsigned char *outData = nullptr;
    int cvType = CV_8UC1;
    /// raw
    if (dataFormat == 1) {
        outData = new unsigned char[width * height * 1];
        cvType = CV_8UC1;
    }
    /// Bayer / others
    else {
        outData = new unsigned char[width * height * 3];
        cvType = CV_8UC3;
    }

    // unsigned long long k = LuxLoadImageData(imgData, length, dataFormat,
    // width, height, bpp, channels, outData, isBigEndian, highZero, code);
    long long k = LuxLoadImageDataEnhanced(imgData, length, dataFormat, width,
                                           height, bpp, channels, outData,
                                           isBigEndian, highZero, mode, code);

    /// k > 0 is ok
    if (k > 0) {
        // For display
        unsigned long long _ret =
            LuxWriteImageIntoFile(outData, outputRawFileName,
                                  ImageFileType::raw, k, width, height, cvType);

        // TIFF
        if (saveTiff)
            LuxWriteImageIntoFile(outData, outputTiffFileName,
                                  ImageFileType::tiff, k, width, height,
                                  cvType);

        delete[] imgData;
        delete[] outData;
        return _ret;
    } else {
        delete[] imgData;
        delete[] outData;
        return k;
    }

    return -5;
}

/**
 * @brief Get G1/G2/R/B data into @c G1Dst / @c G2Dst / @c RDst / @c BDst from
 * 8-bit Byaer raw image.
 *
 * @param src 8-bit Bayer raw image
 * @param width width
 * @param height height
 * @param bayerMode {0, 1, 2, 3}
 *  - 0: GBRG
 *  - 1: GRBG
 *  - 2: BGGR
 *  - 3: RGGB
 * @param RDst Red channel data
 * @param G1Dst Green1 channel data
 * @param G2Dst Green2 channel data
 * @param BDst Blue channel data
 * @return 0 if success else < 0
 */
int LuxGetBayerRawChanenls(unsigned char *src, int width, int height,
                           int bayerMode, unsigned char *RDst,
                           unsigned char *G1Dst, unsigned char *G2Dst,
                           unsigned char *BDst) {
    if (nullptr == src) {
        std::cerr << "Input image(src) is nullptr" << std::endl;
        return -1;
    }

    if (width < 0 || height < 0) {
        std::cerr << "Width or Height < 0" << std::endl;
        return -2;
    }

    if (bayerMode != 0 && bayerMode != 1 && bayerMode != 2 && bayerMode != 3) {
        std::cerr << "bayerMode selection is wrong. Support is [0, 1, 2, 3]\n"
                     "* 0: GBRG\n"
                     "* 1: GRBG\n"
                     "* 2: BGGR\n"
                     "* 3: RGGB"
                  << std::endl;

        return -3;
    }

    int row, col;
    unsigned int runCount = width * height;
    int step = 2;
    int iOff = 0;

    // Seclect byaer mode
    switch (bayerMode) {
        // GBRG
        case 0: {
            for (unsigned int rPtr = 0; rPtr < runCount; rPtr += step) {
                row = (rPtr * 2 / step) / width;
                col = (rPtr * 2 / step) - width * row;

                // Even
                if (row % 2 == 0) {
                    row = row / 2;
                    col = col / 2;
                    iOff = row * (width / 2) + col;
                    G1Dst[iOff] = src[rPtr];
                    BDst[iOff] = src[rPtr + 1];
                    // Odd
                } else {
                    row = row / 2;
                    col = col / 2;
                    iOff = row * (width / 2) + col;
                    RDst[iOff] = src[rPtr];
                    G2Dst[iOff] = src[rPtr + 1];
                }
            }
            break;
        }

        // GRBG
        case 1: {
            for (unsigned int rPtr = 0; rPtr < runCount; rPtr += step) {
                row = (rPtr * 2 / step) / width;
                col = (rPtr * 2 / step) - width * row;

                // Even
                if (row % 2 == 0) {
                    row = row / 2;
                    col = col / 2;
                    iOff = row * (width / 2) + col;
                    G1Dst[iOff] = src[rPtr];
                    RDst[iOff] = src[rPtr + 1];
                    // Odd
                } else {
                    row = row / 2;
                    col = col / 2;
                    iOff = row * (width / 2) + col;
                    BDst[iOff] = src[rPtr];
                    G2Dst[iOff] = src[rPtr + 1];
                }
            }
            break;
        }

        // BGGR
        case 2: {
            for (unsigned int rPtr = 0; rPtr < runCount; rPtr += step) {
                row = (rPtr * 2 / step) / width;
                col = (rPtr * 2 / step) - width * row;

                // Even
                if (row % 2 == 0) {
                    row = row / 2;
                    col = col / 2;
                    iOff = row * (width / 2) + col;
                    BDst[iOff] = src[rPtr];
                    G1Dst[iOff] = src[rPtr + 1];
                    // Odd
                } else {
                    row = row / 2;
                    col = col / 2;
                    iOff = row * (width / 2) + col;
                    G2Dst[iOff] = src[rPtr];
                    RDst[iOff] = src[rPtr + 1];
                }
            }
            break;
        }

        // RGGB
        case 3: {
            for (unsigned int rPtr = 0; rPtr < runCount; rPtr += step) {
                row = (rPtr * 2 / step) / width;
                col = (rPtr * 2 / step) - width * row;

                // Even
                if (row % 2 == 0) {
                    row = row / 2;
                    col = col / 2;
                    iOff = row * (width / 2) + col;
                    RDst[iOff] = src[rPtr];
                    G1Dst[iOff] = src[rPtr + 1];
                    // Odd
                } else {
                    row = row / 2;
                    col = col / 2;
                    iOff = row * (width / 2) + col;
                    G2Dst[iOff] = src[rPtr];
                    BDst[iOff] = src[rPtr + 1];
                }
            }
            break;
        }
    }

    return 0;
}

/**
 * @brief Set R/G/B channel factor, with only 8-bit raw bayer.
 *
 * @param src 8-bit raw bayer
 * @param width The width of bayer image.
 * @param height The width of bayer image.
 * @param mode The type of bayer {0, 1, 2, 3}.
 *  - GBRG 0
 *  - GRBG 1
 *  - BGGR 2
 *  - RGGB 3
 * @param r [0, 10]
 * @param g [0, 10]
 * @param b [0, 10]
 * @return #bytes of bayer image.
 */
long long LuxSetChannelFactors(unsigned char *src, int width, int height,
                               int mode, float r, float g, float b) {
    if (nullptr == src) {
        std::cerr << "Input image(src) is nullptr" << std::endl;
        return -1;
    }

    if (width < 0 || height < 0) {
        std::cerr << "Width or Height < 0" << std::endl;
        return -2;
    }

    if (mode != 0 && mode != 1 && mode != 2 && mode != 3) {
        std::cerr << "Mode selection is wrong. Support is [0, 1, 2, 3]\n"
                     "* 0: GBRG\n"
                     "* 1: GRBG\n"
                     "* 2: BGGR\n"
                     "* 3: RGGB"
                  << std::endl;

        return -3;
    }

    if (std::abs(r) > 10 || std::abs(g) > 10 || std::abs(b) > 10) {
        std::cerr << "The factors(r/g/b) is wrong. It should be in [0, 10]."
                  << std::endl;
        return -4;
    }

    int row, col;
    unsigned int runCount = width * height;
    int step = 2;
    int k = 0;

    // Seclect byaer mode
    switch (mode) {
        // GBRG
        case 0: {
            for (unsigned int rPtr = 0; rPtr < runCount; rPtr += step) {
                row = (rPtr * 2 / step) / width;
                col = (rPtr * 2 / step) - width * row;

                // Even
                if (row % 2 == 0) {
                    row = row / 2;
                    col = col / 2;
                    // iOff = row * (width / 2) + col;
                    src[rPtr] = LuxBound255<float>(src[rPtr] * g);
                    src[rPtr + 1] = LuxBound255<float>(src[rPtr + 1] * b);
                    k++;
                    // Odd
                } else {
                    row = row / 2;
                    col = col / 2;
                    // iOff = row * (width / 2) + col;
                    src[rPtr] = LuxBound255<float>(src[rPtr] * r);
                    src[rPtr + 1] = LuxBound255<float>(src[rPtr + 1] * g);
                    k++;
                }
            }
            break;
        }

        // GRBG
        case 1: {
            for (unsigned int rPtr = 0; rPtr < runCount; rPtr += step) {
                row = (rPtr * 2 / step) / width;
                col = (rPtr * 2 / step) - width * row;

                // Even
                if (row % 2 == 0) {
                    row = row / 2;
                    col = col / 2;
                    // iOff = row * (width / 2) + col;
                    src[rPtr] = LuxBound255<float>(src[rPtr] * g);
                    src[rPtr + 1] = LuxBound255<float>(src[rPtr + 1] * r);
                    k++;
                    // Odd
                } else {
                    row = row / 2;
                    col = col / 2;
                    // iOff = row * (width / 2) + col;
                    src[rPtr] = LuxBound255<float>(src[rPtr] * b);
                    src[rPtr + 1] = LuxBound255<float>(src[rPtr + 1] * g);
                    k++;
                }
            }
            break;
        }

        // BGGR
        case 2: {
            for (unsigned int rPtr = 0; rPtr < runCount; rPtr += step) {
                row = (rPtr * 2 / step) / width;
                col = (rPtr * 2 / step) - width * row;

                // Even
                if (row % 2 == 0) {
                    row = row / 2;
                    col = col / 2;
                    // iOff = row * (width / 2) + col;
                    src[rPtr] = LuxBound255<float>(src[rPtr] * b);
                    src[rPtr + 1] = LuxBound255<float>(src[rPtr + 1] * g);
                    k++;
                    // Odd
                } else {
                    row = row / 2;
                    col = col / 2;
                    // iOff = row * (width / 2) + col;
                    src[rPtr] = LuxBound255<float>(src[rPtr] * g);
                    src[rPtr + 1] = LuxBound255<float>(src[rPtr + 1] * r);
                    k++;
                }
            }
            break;
        }

        // RGGB
        case 3: {
            for (unsigned int rPtr = 0; rPtr < runCount; rPtr += step) {
                row = (rPtr * 2 / step) / width;
                col = (rPtr * 2 / step) - width * row;

                // Even
                if (row % 2 == 0) {
                    row = row / 2;
                    col = col / 2;
                    // iOff = row * (width / 2) + col;
                    src[rPtr] = LuxBound255<float>(src[rPtr] * r);
                    src[rPtr + 1] = LuxBound255<float>(src[rPtr + 1] * g);
                    k++;
                    // Odd
                } else {
                    row = row / 2;
                    col = col / 2;
                    // iOff = row * (width / 2) + col;
                    src[rPtr] = LuxBound255<float>(src[rPtr] * g);
                    src[rPtr + 1] = LuxBound255<float>(src[rPtr + 1] * b);
                    k++;
                }
            }
            break;
        }
    }

    // Bytes
    return k;
}

/**
 * @brief Load image data from memory
 * @note When Python Call the Function, the ALL parameters must be SET.
 * @param imgData The poniter of image data in memory before parsing.
 * @param length The bytes number of image data in memory.
 * @param dataFormat 1: raw, 2: bayer, 3: others
 * @param width Width
 * @param height Height
 * @param bpp (Bits Per Pixel)  8, 12, 16
 * @param inChannels inChannels
 * @param outData The poniter of image data in memory after parsing.
 * @param isBigEndian Big Endian(be) ?
 * @param highZero 0000AAAA AAAAAAAA ?
 * @param mode [0, 1, 2, 3, 4]
 * 0 - P0P1P2P3P4P5P6P7 P8P9PaPbQ0Q1Q2Q3 Q4Q5Q6Q7Q8Q9QaQb -> P0P1P2P3P4P5P6P7
 * Q0Q1Q2Q3Q4Q5Q6Q7 1 - P0P1P2P3P4P5P6P7 P8P9PaPbQ0Q1Q2Q3 Q4Q5Q6Q7Q8Q9QaQb ->
 * P1P2P3P4P5P6P7P8 Q1Q2Q3Q4Q5Q6Q7Q8 2 - P0P1P2P3P4P5P6P7 P8P9PaPbQ0Q1Q2Q3
 * Q4Q5Q6Q7Q8Q9QaQb -> P2P3P4P5P6P7P8P9 Q2Q3Q4Q5Q6Q7Q8Q9 3 - P0P1P2P3P4P5P6P7
 * P8P9PaPbQ0Q1Q2Q3 Q4Q5Q6Q7Q8Q9QaQb -> P3P4P5P6P7P8P9Pa Q3Q4Q5Q6Q7Q8Q9Qa 4 -
 * P0P1P2P3P4P5P6P7 P8P9PaPbQ0Q1Q2Q3 Q4Q5Q6Q7Q8Q9QaQb -> P4P5P6P7P8P9PaPb
 * Q4Q5Q6Q7Q8Q9QaQb
 * @param code
 *  - CV_BayerBG2BGR =46,
 *  - CV_BayerGB2BGR =47,
 *  - CV_BayerRG2BGR =48,
 *  - CV_BayerGR2BGR =49,
 *  - CV_BayerBG2RGB =CV_BayerRG2BGR = 48,
 *  - CV_BayerGB2RGB =CV_BayerGR2BGR = 49,
 *  - CV_BayerRG2RGB =CV_BayerBG2BGR = 46,
 *  - CV_BayerGR2RGB =CV_BayerGB2BGR = 47,
 *  - CV_BayerRG2GRAY = 88
 *  - CV_BayerBG2GRAY = 86,
 *  - CV_BayerGB2GRAY = 87,
 *  - CV_BayerRG2GRAY = 88,
 *  - CV_BayerGR2GRAY = 89
 * @param eaf Enable Adjust Factors
 * @param bayerType The type of bayer {0, 1, 2, 3}.
 *  - GBRG 0
 *  - GRBG 1
 *  - BGGR 2
 *  - RGGB 3
 * @param r [0, 10]
 * @param g [0, 10]
 * @param b [0, 10]
 * @return long long
 * The bytes number of image file if success.
 *  -1 : Data Format Don't Supported.
 *  -2 : Bits per pixel Don't Supported.
 *  -4 : width or height or bpp or channel are wrong.
 *  -5 : It is not reached.
 */
long long LuxLoadImageDataEnhanced2(unsigned char *imgData,
                                    unsigned long long length, int dataFormat,
                                    int width, int height, int bpp,
                                    int inChannels, unsigned char *outData,
                                    bool isBigEndian, bool highZero, int mode,
                                    int code, bool eaf, int bayerType, float r,
                                    float g, float b) {
    if (dataFormat != 1 && dataFormat != 2 && dataFormat != 3) {
        std::cerr << "Data Format Don't Supported!!! \n"
                  << "1: raw, 2: bayer, 3: others" << std::endl;
        ::fflush(stderr);
        return -1;
    }

    float bytes = 0.0;
    switch (bpp) {
        case 8:
            bytes = 1;
            break;

        case 12:
            bytes = 1.5;
            break;
        case 16:
            bytes = 2;
            break;

        default:
            std::cerr << "bpp Don't Supported!!! \n"
                      << "Supported depth of bits: 8, 12, 16" << std::endl;
            ::fflush(stderr);
            return -2;
    }

    if (length !=
        static_cast<unsigned long long>(
            static_cast<float>(width * height * inChannels) * bytes)) {
        std::cerr << "width or height or bpp or channel are wrong!!!"
                  << "\nwidth: " << width << "\nheigth: " << height
                  << "\nbits per pixel: " << bpp
                  << "\ninChannels: " << inChannels << std::endl;
        ::fflush(stderr);
        return -4;
    }

    // Only support big endian
    if (!isBigEndian) {
        LuxEndianRevert(imgData, length, bpp, imgData, true);
    }

    /// bind mode
    auto parseImage =
        std::bind(&LuxParseImageEnhanced, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3,
                  std::placeholders::_4, std::placeholders::_5, mode);

    // TODO 代码优化： 加入 outChannels/types
    /* raw */
    if (dataFormat == 1) {
        long long validLength = width * height;
        auto *temp = new unsigned char[validLength];
        // uint64_t k = LuxParseImage(imgData, length, bpp, highZero, temp);
        uint64_t k = parseImage(imgData, length, bpp, highZero, temp);
        (void)k;

        // Adjust r/g/b
        if (eaf) {
            std::cout << "With DataFormat = 1, Adjust R/G/B Factors is not "
                         "supported. Please SET \"eaf\" TO \"false\"."
                      << std::endl;
            std::cout << "Please Consider to use \"LuxAdjustBrightness()\"."
                      << std::endl;
        }

        cv::Mat bayer8BitMat(height, width, CV_8UC1, temp);
        cv::Mat outputImg(height, width, CV_8UC1, outData);
        cv::cvtColor(bayer8BitMat, outputImg, code);

        delete[] temp;
        /* 图片大小 （字节数） */
        return outputImg.size().width * outputImg.size().height *
               outputImg.channels();
    }

    /* Bayer */
    else if (dataFormat == 2) {
        int outChannels = 3;
        long long validLength = width * height * outChannels;
        auto *temp = new unsigned char[validLength];
        // uint64_t k = LuxParseImage(imgData, length, bpp, highZero, temp);
        uint64_t k = parseImage(imgData, length, bpp, highZero, temp);

        // Adjust r/g/b
        if (eaf)
            k = LuxSetChannelFactors(temp, width, height, bayerType, r, g, b);
        else
            (void)k;

        /// 16UC1 Bayer
        cv::Mat bayer8BitMat(height, width, CV_8UC1, temp);
        cv::Mat rgb8BitMat(height, width, CV_8UC3, outData);
        cv::cvtColor(bayer8BitMat, rgb8BitMat, code);

        // cv::imshow("LuxTW2", rgb8BitMat);
        // cv::waitKey();

        delete[] temp;
        return rgb8BitMat.size().height * rgb8BitMat.size().width *
               rgb8BitMat.channels();
    }

    return -5;
}

/**
 * @brief Load image data from file.
 * @note When Python Call the Function, the ALL parameters must be SET.
 * @param inputFileName The file before parsing.
 * @param dataFormat 1: raw, 2: bayer, 3: others
 * @param width Width
 * @param height Height
 * @param bpp bpp(Bits Per Pixel)  8, 12, 16
 * @param channels Channels
 * @param outputRawFileName The .raw file after parsing.
 * @param outputTiffFileName The .tiff file after parsing.
 * @param isBigEndian Big Endian(be) ?
 * @param highZero 0000AAAA AAAAAAAA ?
 * @param saveTiff Save tiff ?
 * @param mode [0, 1, 2, 3, 4]
 * 0 - P0P1P2P3P4P5P6P7 P8P9PaPbQ0Q1Q2Q3 Q4Q5Q6Q7Q8Q9QaQb -> P0P1P2P3P4P5P6P7
 * Q0Q1Q2Q3Q4Q5Q6Q7 1 - P0P1P2P3P4P5P6P7 P8P9PaPbQ0Q1Q2Q3 Q4Q5Q6Q7Q8Q9QaQb ->
 * P1P2P3P4P5P6P7P8 Q1Q2Q3Q4Q5Q6Q7Q8 2 - P0P1P2P3P4P5P6P7 P8P9PaPbQ0Q1Q2Q3
 * Q4Q5Q6Q7Q8Q9QaQb -> P2P3P4P5P6P7P8P9 Q2Q3Q4Q5Q6Q7Q8Q9 3 - P0P1P2P3P4P5P6P7
 * P8P9PaPbQ0Q1Q2Q3 Q4Q5Q6Q7Q8Q9QaQb -> P3P4P5P6P7P8P9Pa Q3Q4Q5Q6Q7Q8Q9Qa 4 -
 * P0P1P2P3P4P5P6P7 P8P9PaPbQ0Q1Q2Q3 Q4Q5Q6Q7Q8Q9QaQb -> P4P5P6P7P8P9PaPb
 * Q4Q5Q6Q7Q8Q9QaQb
 * @param code
 *  - CV_BayerBG2BGR =46,
 *  - CV_BayerGB2BGR =47,
 *  - CV_BayerRG2BGR =48,
 *  - CV_BayerGR2BGR =49,
 *  - CV_BayerBG2RGB =CV_BayerRG2BGR = 48,
 *  - CV_BayerGB2RGB =CV_BayerGR2BGR = 49,
 *  - CV_BayerRG2RGB =CV_BayerBG2BGR = 46,
 *  - CV_BayerGR2RGB =CV_BayerGB2BGR = 47,
 *  - CV_BayerRG2GRAY = 88
 *  - CV_BayerBG2GRAY = 86,
 *  - CV_BayerGB2GRAY = 87,
 *  - CV_BayerRG2GRAY = 88,
 *  - CV_BayerGR2GRAY = 89
 * @param eaf Enable Adjust Factors
 * @param bayerType The type of bayer {0, 1, 2, 3}.
 *  - GBRG 0
 *  - GRBG 1
 *  - BGGR 2
 *  - RGGB 3
 * @param r [0, 10]
 * @param g [0, 10]
 * @param b [0, 10] *
 *
 * @return long long
 * The bytes number of image file if success.
 *  -1 : Data Format Don't Supported.
 *  -2 : Bits per pixel Don't Supported.
 *  -3 : File open failed.
 *  -4 : width or height or bpp or channel are wrong.
 *  -5 : It is not reached.
 */
long long LuxLoadImageDataFromFileEnhanced2(
    const char *inputFileName, int dataFormat, int width, int height, int bpp,
    int channels, const char *outputRawFileName, const char *outputTiffFileName,
    bool isBigEndian, bool highZero, bool saveTiff, int mode, int code,
    bool eaf, int bayerType, float r, float g, float b) {
    if (dataFormat != 1 && dataFormat != 2 && dataFormat != 3) {
        std::cerr << "Data Format Don't Supported!!! \n"
                  << "1: raw, 2: bayer, 3: others" << std::endl;
        ::fflush(stderr);
        return -1;
    }

    float bytes = 0.0;
    switch (bpp) {
        case 8:
            bytes = 1;
            break;
        case 12:
            bytes = 1.5;
            break;
        case 16:
            bytes = 2;
            break;
        default:
            std::cerr << "bpp Don't Supported!!! \n"
                      << "Supported depth of bits: 8, 12, 16" << std::endl;
            ::fflush(stderr);
            return -2;
    }

    std::ifstream ifstrm(reinterpret_cast<const char *>(inputFileName),
                         std::ios_base::binary);
    if (ifstrm.is_open() == false) {
        std::cerr << "Fail to read " << inputFileName << std::endl;
        ::fflush(stderr);
        return -3;
    }

    /// Check the length of file
    unsigned long long length = static_cast<unsigned long long int>(
        static_cast<float>(width * height * channels) * bytes);
    ifstrm.seekg(0, ifstrm.end);
    unsigned long long ret = ifstrm.tellg();
    ifstrm.seekg(0, ifstrm.beg);
    if (ret != length) {
        std::cerr << "The length of file is NOT right." << std::endl;
        ::fflush(stderr);
        return -4;
    }

    /// Read bytes into imgData
    auto *imgData = new unsigned char[length];
    ifstrm.read((char *)imgData, length);

    /// According to the target, Set channels, Type of outputFile
    unsigned char *outData = nullptr;
    int cvType = CV_8UC1;
    /// raw
    if (dataFormat == 1) {
        outData = new unsigned char[width * height * 1];
        cvType = CV_8UC1;
    }
    /// Bayer / others
    else {
        outData = new unsigned char[width * height * 3];
        cvType = CV_8UC3;
    }

    // unsigned long long k = LuxLoadImageData(imgData, length, dataFormat,
    // width, height, bpp, channels, outData, isBigEndian, highZero, code);
    long long k = LuxLoadImageDataEnhanced2(
        imgData, length, dataFormat, width, height, bpp, channels, outData,
        isBigEndian, highZero, mode, code, eaf, bayerType, r, g, b);

    /// k > 0 is ok
    if (k > 0) {
        // For display
        unsigned long long _ret =
            LuxWriteImageIntoFile(outData, outputRawFileName,
                                  ImageFileType::raw, k, width, height, cvType);

        // TIFF
        if (saveTiff)
            LuxWriteImageIntoFile(outData, outputTiffFileName,
                                  ImageFileType::tiff, k, width, height,
                                  cvType);

        delete[] imgData;
        delete[] outData;
        return _ret;
    } else {
        delete[] imgData;
        delete[] outData;
        return k;
    }

    return -5;
}
