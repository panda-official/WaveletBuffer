// Copyright 2020-2022 PANDA GmbH

#ifndef WAVELET_BUFFER_IMG_WAVELET_IMAGE_H_
#define WAVELET_BUFFER_IMG_WAVELET_IMAGE_H_

#include <memory>
#include <string>

#include "wavelet_buffer/img/image_codec.h"
#include "wavelet_buffer/wavelet_buffer.h"

namespace drift::img {

/**
 * A class to load/save images of different formats and store them
 * as list of wavelet subbands
 */
class WaveletImage {
 public:
  using Buffer = WaveletBuffer;
  using Denoiser = DenoiseAlgorithm<DataType>;
  using Codec = IImageCodec;

  /**
   * Statuses for error handling
   */
  // NOLINTNEXTLINE
  enum [[nodiscard]] Status{
      kOk = 0,                  // No errors
      kWrongSize = 1,           // the image has wrong size
      kDecompositionError = 2,  // Failed to decompose
      kCompositionError = 3,    // Failed to compose
      kIOError = 4,             // Failed open, write or read a file
      kWrongData = 5            // Bad input data
  };

  /**
   * Creates a wavelet image without wavlete compression
   * @param w
   * @param h
   */
  WaveletImage(size_t channel_number, size_t w, size_t h);

  /**
   * Creates an empty image
   * @param parameters Parameters fo wavelet decomposition
   */
  explicit WaveletImage(WaveletParameters parameters);

  /**
   * Creates an image from WaveletBuffer
   */
  explicit WaveletImage(Buffer buffer);

  /**
   * Destructor
   */
  ~WaveletImage();

  /**
   * Imports an image from a file
   * @param file_path the path to he file
   * @param image_type the image type
   * @return
   */
  Status ImportFromFile(const std::string& file_path, const Denoiser& denoiser,
                        const Codec& codec, size_t start_channel = 0);

  /**
   * Exports an image to a file
   * @param file_path the path to he file
   * @param image_type the image type
   * @return
   */
  Status ExportToFile(const std::string& file_path, const Codec& codec,
                      size_t start_channel = 0) const;

  /**
   * Imports an image from a string blob
   * @param data the blob
   * @param image_type the image type
   * @return
   */
  Status ImportFromString(const std::string& data, const Denoiser& denoiser,
                          const Codec& codec, size_t start_channel = 0);

  /**
   * Exports an image to a string blob
   * @param data the blob
   * @param image_type the image type
   * @return
   */
  Status ExportToString(std::string* data, const Codec& codec,
                        size_t start_channel = 0) const;

  /**
   * Loads and creates an image from a binary archive
   * @param file_path the path to the archive
   * @return nullptr if it failed to creates an image
   */
  static std::unique_ptr<WaveletImage> Load(const std::string& file_path);

  /**
   * Saves an image to a binary archive
   * @param file_path the path to the archive
   * @return
   */
  Status Save(const std::string& file_path) const;

  /**
   * Compares to another image
   * @param other
   * @return distance value 0 both images the same, 1 both buffers absolutely
   * different
   */
  [[nodiscard]] double CompareTo(const WaveletImage& other) const;

  [[nodiscard]] const Buffer& const_buffer() const;

  [[nodiscard]] Buffer& buffer();

 private:
  class WaveletImageImpl;
  std::unique_ptr<WaveletImageImpl> impl_;
};
}  // namespace drift::img

#endif  // WAVELET_BUFFER_IMG_WAVELET_IMAGE_H_
