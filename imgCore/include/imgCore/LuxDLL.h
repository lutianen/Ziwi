/**
 * @file LuxDLL.h
 *
 * @link https://github.com/lutianen/LuxDLL
 */

#ifndef LUXTW2_H
#define LUXTW2_H

#include <cstdint>
#include <opencv2/opencv.hpp>

template <typename T>
inline std::tuple<T, T> LuxFindMaxMin(const T *img, int length);

template <typename TSrc, typename TDst = uint8_t>
inline TDst normlize255(TSrc src, TSrc max);

// template <typename TSrc, typename TDst>
// inline unsigned long long LuxNormalize(TSrc* orgiImg, int orgiImgLen, TDst*
// outputImg);

template <typename TSrc, typename TDst>
inline TDst normlize255(TSrc src, TSrc max) {
    // UINT8_MAX
    return static_cast<TDst>((static_cast<float>(src) / max) * UINT8_MAX);
};

template <typename T>
inline std::tuple<T, T> LuxFindMaxMin(const T *img, int length) {
    T max = img[0];
    T min = img[0];
    for (int i = 0; i < length; ++i) {
        if (img[i] > max) {
            max = img[i];
        }
        if (img[i] < min) {
            min = img[i];
        }
    }

    return std::make_tuple(max, min);
}

template <typename TSrc, typename TDst>
inline uint64_t LuxNormalize(TSrc *orgiImg, int orgiImgLen, TDst *outputImg) {
    uint64_t k = 0;
    TSrc maxOfOrgiImg = std::get<0>(LuxFindMaxMin<TSrc>(orgiImg, orgiImgLen));

    for (int i = 0; i < orgiImgLen; ++i) {
        outputImg[k++] = normlize255<TSrc, TDst>(orgiImg[i], maxOfOrgiImg);
    }

    return k;
}

#ifdef __cplusplus
extern "C" {
#endif

// 在 Windows 平台上使用 __declspec
#ifdef _WIN32
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

enum class ImageFileType { UnknowType = -1, raw = 0, tiff };

/**
 * C Type                   ctypes Type
 * ---------------------------------------
 * char                     c_char
 * wchar_r                  c_wchar
 * char                     c_byte
 * char                     c_ubyte
 * short                    c_short
 * unsigned short           c_ushort
 * int                      c_int
 * unsigned int             c_uint
 * long                     c_long
 * unsigned long            c_ulong
 * long long                c_longlong
 * unsigned long long       c_ulonglong
 * float                    c_float
 * double                   c_double
 * char* (NULL terminated)  c_char_p
 * wchar* (NULL terminated) c_wchar_p
 * void*                    c_void_p
 */

/*************************************************************************************************/
/*                                     Function Declare */
/*************************************************************************************************/

void LuxEndianSwap(char *pData, uint64_t startIndex, uint64_t length);

DLL_EXPORT
int LuxEndianRevert(unsigned char *input, int length, int bpp,
                    unsigned char *output, bool isBig2Little);

DLL_EXPORT
void LuxFlushStdOut();

inline unsigned long long LuxParseImage(unsigned char *orgiImg, int length,
                                        int bpp, bool highZero,
                                        unsigned char *outputImg);

inline unsigned long long LuxParseImageExtendTo16(unsigned char *orgiImg,
                                                  int length, int bpp,
                                                  bool highZero,
                                                  uint16_t *outputImg);

inline unsigned long long LuxParseImageStretchTo16(unsigned char *orgiImg,
                                                   int length, int bpp,
                                                   bool highZero,
                                                   uint16_t *outputImg);

inline decltype(CV_8UC1) LuxGetImageType(int dataFormat, int bpp, int channels);

long long LuxWriteImageIntoFile(unsigned char *imgData, const char *outFileName,
                                ImageFileType fileFormat,
                                unsigned long long length, int width,
                                int height, int type);

DLL_EXPORT
long long LuxLoadImageData(unsigned char *imgData, unsigned long long length,
                           int dataFormat, int width, int height, int bpp,
                           int channels, unsigned char *outData,
                           bool isBigEndian, bool highZero,
                           //  int code = CV_BayerRG2RGB);
                           int code = cv::COLOR_BayerRG2RGB);

DLL_EXPORT
long long LuxLoadImageDataFromFile(const char *inputFileName, int dataFormat,
                                   int width, int height, int bpp, int channels,
                                   const char *outputRawFileName,
                                   const char *outputTiffFileName,
                                   bool isBigEndian, bool highZero,
                                   bool saveTiff,
                                   int code = cv::COLOR_BayerRG2RGB);

DLL_EXPORT
long long LuxLoadImageDataStretchTo16(unsigned char *imgData,
                                      unsigned long long length, int dataFormat,
                                      int width, int height, int bpp,
                                      int inChannels, int outChannels,
                                      uint16_t *outData, bool isBigEndian,
                                      bool highZero, int code);

DLL_EXPORT
long long LuxLoadImageDataFromFileStretchTo16(
    const char *inputFileName, int dataFormat, int width, int height, int bpp,
    int inChannels, int outChannels, const char *outputRawFileName,
    const char *outputTiffFileName, bool isBigEndian, bool highZero,
    bool saveTiff, int code = cv::COLOR_BayerRG2RGB);

DLL_EXPORT
int LuxAdjustBrightness(unsigned char *src, long long length, int width,
                        int height, int channel, int beta, unsigned char *dst);

DLL_EXPORT
int LuxAdjustContrast(unsigned char *src, long long length, int width,
                      int height, int channel, int alpha, unsigned char *dst);

DLL_EXPORT
long long int LuxDrawHist(const char *inputFileName_, int width, int height,
                          int imReadType, int histImgWidth, int histImgHeight,
                          const char *outFileName);

DLL_EXPORT
long long LuxSaveImgDatExtTo16(unsigned char *imgData,
                               unsigned long long length, int width, int height,
                               int bpp, int inChannels,
                               const char *outRawFileName, bool isBigEndian,
                               bool highZero);

inline long long LuxParseImageEnhanced(unsigned char *orgiImg, int bytes,
                                       int bpp, bool highZero,
                                       unsigned char *outputImg, int mode = 0);

DLL_EXPORT
long long LuxLoadImageDataEnhanced(unsigned char *imgData,
                                   unsigned long long length, int dataFormat,
                                   int width, int height, int bpp,
                                   int inChannels, unsigned char *outData,
                                   bool isBigEndian, bool highZero, int mode,
                                   int code = cv::COLOR_BayerRG2RGB);

DLL_EXPORT
long long LuxLoadImageDataFromFileEnhanced(
    const char *inputFileName, int dataFormat, int width, int height, int bpp,
    int channels, const char *outputRawFileName, const char *outputTiffFileName,
    bool isBigEndian, bool highZero, bool saveTiff, int mode,
    int code = cv::COLOR_BayerRG2RGB);

DLL_EXPORT
int LuxGetBayerRawChanenls(unsigned char *src, int width, int height,
                           int bayerMode, unsigned char *RDst,
                           unsigned char *G1Dst, unsigned char *G2Dst,
                           unsigned char *BDst);

DLL_EXPORT
long long LuxSetChannelFactors(unsigned char *src, int width, int height,
                               int mode, float r, float g, float b);

DLL_EXPORT
long long LuxLoadImageDataFromFileEnhanced2(
    const char *inputFileName, int dataFormat, int width, int height, int bpp,
    int channels, const char *outputRawFileName, const char *outputTiffFileName,
    bool isBigEndian, bool highZero, bool saveTiff, int mode, int code,
    bool eaf, int bayerType, float r, float g, float b);

DLL_EXPORT
long long LuxLoadImageDataEnhanced2(unsigned char *imgData,
                                    unsigned long long length, int dataFormat,
                                    int width, int height, int bpp,
                                    int inChannels, unsigned char *outData,
                                    bool isBigEndian, bool highZero, int mode,
                                    int code, bool eaf, int bayerType, float r,
                                    float g, float b);

#ifdef __cplusplus
}
#endif  /// __cplusplus
#endif