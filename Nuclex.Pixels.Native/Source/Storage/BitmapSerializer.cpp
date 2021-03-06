#pragma region CPL License
/*
Nuclex Native Framework
Copyright (C) 2002-2019 Nuclex Development Labs

This library is free software; you can redistribute it and/or
modify it under the terms of the IBM Common Public License as
published by the IBM Corporation; either version 1.0 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
IBM Common Public License for more details.

You should have received a copy of the IBM Common Public
License along with this library
*/
#pragma endregion // CPL License

// If the library is compiled as a DLL, this ensures symbols are exported
#define NUCLEX_PIXELS_SOURCE 1

#include "Nuclex/Pixels/Storage/BitmapSerializer.h"
#include "Nuclex/Pixels/Storage/VirtualFile.h"
#include "Nuclex/Pixels/Storage/BitmapCodec.h"
#include "Nuclex/Pixels/Errors/FileFormatError.h"

#include "Utf8Fold/Utf8Fold.h"

#if defined(NUCLEX_PIXELS_HAVE_LIBPNG)
#include "Png/PngBitmapCodec.h"
#endif
#if defined(NUCLEX_PIXELS_HAVE_LIBJPEG)
#include "Jpeg/JpegBitmapCodec.h"
#endif
#if defined(NUCLEX_PIXELS_HAVE_OPENEXR)
#include "Exr/ExrBitmapCodec.h"
#endif

namespace {

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Invalid size marker for the most recent codec indices</summary>
  constexpr std::size_t InvalidIndex = std::size_t(-1);

  // ------------------------------------------------------------------------------------------- //

  /// <summary>Helper used to pass information through lambda methods</summary>
  struct FileAndBitmap {

    /// <summary>File the bitmap serializer has been tasked with reading</summary>
    public: const Nuclex::Pixels::Storage::VirtualFile *File;

    /// <summary>Container that receives the loaded bitmap if successful</summary>
    public: Nuclex::Pixels::Storage::OptionalBitmap Bitmap;

    /// <summary>Bitmap into whih the TryLoad() methods will load the pixels</summary>
    public: Nuclex::Pixels::Bitmap *TargetBitmap;

  };

  // ------------------------------------------------------------------------------------------- //

} // anonymous namespace

namespace Nuclex { namespace Pixels { namespace Storage {

  // ------------------------------------------------------------------------------------------- //

  BitmapSerializer::BitmapSerializer() :
    mostRecentCodecIndex(InvalidIndex),
    secondMostRecentCodecIndex(InvalidIndex) {
#if defined(NUCLEX_PIXELS_HAVE_LIBPNG)
    RegisterCodec(std::make_unique<Png::PngBitmapCodec>());
#endif
#if defined(NUCLEX_PIXELS_HAVE_LIBJPEG)
    RegisterCodec(std::make_unique<Jpeg::JpegBitmapCodec>());
#endif
#if defined(NUCLEX_PIXELS_HAVE_OPENEXR)
    RegisterCodec(std::make_unique<Exr::ExrBitmapCodec>());
#endif
  }

  // ------------------------------------------------------------------------------------------- //

  BitmapSerializer::~BitmapSerializer() {}

  // ------------------------------------------------------------------------------------------- //

  void BitmapSerializer::RegisterCodec(std::unique_ptr<BitmapCodec> &&codec) {
    std::size_t codecCount = this->codecs.size();

    // This should be a one-liner, but clang has a nonsensical warning then typeid()
    // is called with an expression that needs to be evaluated at runtime :-(
    const BitmapCodec &newCodec = *codec.get();
    const std::type_info &newType = typeid(newCodec);

    // Make sure this exact type isn't registered yet
    for(std::size_t index = 0; index < codecCount; ++index) {
      const BitmapCodec &checkedCodec = *this->codecs[index].get();
      const std::type_info &existingType = typeid(checkedCodec);
      if(newType == existingType) {
        throw std::runtime_error(u8"Codec already registered");
      }
    }

    const std::vector<std::string> &extensions = codec->GetFileExtensions();

    // Register the new codec into our list
    this->codecs.push_back(std::move(codec));

    // Update the extension lookup map for quick codec finding
    std::size_t extensionCount = extensions.size();
    for(std::size_t index = 0; index < extensionCount; ++index) {
      const std::string &extension = extensions[index];
      std::string::size_type extensionLength = extension.length();

      if(extensionLength > 0) {
        if(extension[0] == '.') {
          if(extensionLength > 1) {
            std::string lowerExtension = toFoldedLowercase(extension.substr(1));
            this->codecsByExtension.insert(
              ExtensionCodecIndexMap::value_type(lowerExtension, codecCount)
            );
          }
        } else {
          std::string lowerExtension = toFoldedLowercase(extension);
          this->codecsByExtension.insert(
            ExtensionCodecIndexMap::value_type(lowerExtension, codecCount)
          );
        }
      }
    }
  }

  // ------------------------------------------------------------------------------------------- //

  bool BitmapSerializer::CanLoad(
    const VirtualFile &file, const std::string &extensionHint /* = std::string() */
  ) const {
    FileAndBitmap fileProvider;
    fileProvider.File = &file;

    return tryCodecsInOptimalOrder<FileAndBitmap>(
      extensionHint,
      [](const BitmapCodec &codec, const std::string &extension, FileAndBitmap &fileAndBitmap) {
        return codec.CanLoad(*fileAndBitmap.File, extension);
      },
      fileProvider
    );
  }

  // ------------------------------------------------------------------------------------------- //

  bool BitmapSerializer::CanLoad(const std::string &path) const {
    std::string::size_type extensionDotIndex = path.find_last_of('.');
#if defined(NUCLEX_PIXELS_WIN32)
    std::string::size_type lastPathSeparatorIndex = path.find_last_of('\\');
#else
    std::string::size_type lastPathSeparatorIndex = path.find_last_of('/');
#endif

    // Check if the provided path contains a file extension and if so, pass it along to
    // the CanLoad() method as a hint (this speeds up codec search)
    if(extensionDotIndex != std::string::npos) {
      bool dotBelongsToFilename = (
        (lastPathSeparatorIndex == std::string::npos) ||
        (extensionDotIndex > lastPathSeparatorIndex)
      );
      if(dotBelongsToFilename) {
        std::unique_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(path, true);
        return CanLoad(*file.get(), path.substr(extensionDotIndex + 1));
      }
    }

    // The specified file has no extension, so do not provide the extension hint
    {
      std::unique_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(path, true);
      return CanLoad(*file.get());
    }
  }

  // ------------------------------------------------------------------------------------------- //

  Bitmap BitmapSerializer::Load(
    const VirtualFile &file, const std::string &extensionHint /* = std::string() */
  ) const {
    FileAndBitmap fileProvider;
    fileProvider.File = &file;

    bool wasLoaded = tryCodecsInOptimalOrder<FileAndBitmap>(
      extensionHint,
      [](const BitmapCodec &codec, const std::string &extension, FileAndBitmap &fileAndBitmap) {
        OptionalBitmap loadedBitmap = codec.TryLoad(*fileAndBitmap.File, extension);
        if(loadedBitmap.HasValue()) {
          fileAndBitmap.Bitmap = std::move(loadedBitmap);
          return true;
        } else {
          return false;
        }
      },
      fileProvider
    );
    if(wasLoaded) {
      return fileProvider.Bitmap.Take();
    } else {
      throw Errors::FileFormatError("File format not supported by any registered codec");
    }
  }

  // ------------------------------------------------------------------------------------------- //

  Bitmap BitmapSerializer::Load(const std::string &path) const {
    std::string::size_type extensionDotIndex = path.find_last_of('.');
#if defined(NUCLEX_PIXELS_WIN32)
    std::string::size_type lastPathSeparatorIndex = path.find_last_of('\\');
#else
    std::string::size_type lastPathSeparatorIndex = path.find_last_of('/');
#endif

    // Check if the provided path contains a file extension and if so, pass it along to
    // the CanLoad() method as a hint (this speeds up codec search)
    if(extensionDotIndex != std::string::npos) {
      bool dotBelongsToFilename = (
        (lastPathSeparatorIndex == std::string::npos) ||
        (extensionDotIndex > lastPathSeparatorIndex)
      );
      if(dotBelongsToFilename) {
        std::unique_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(path, true);
        return Load(*file.get(), path.substr(extensionDotIndex + 1));
      }
    }

    // The specified file has no extension, so do not provide the extension hint
    {
      std::unique_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(path, true);
      return Load(*file.get());
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void BitmapSerializer::Reload(
    Bitmap &exactFittingBitmap,
    const VirtualFile &file, const std::string &extensionHint /* = std::string() */
  ) const {
    FileAndBitmap fileProvider;
    fileProvider.File = &file;
    fileProvider.TargetBitmap = &exactFittingBitmap;

    bool wasLoaded = tryCodecsInOptimalOrder<FileAndBitmap>(
      extensionHint,
      [](const BitmapCodec &codec, const std::string &extension, FileAndBitmap &fileAndBitmap) {
        if(codec.TryReload(*fileAndBitmap.TargetBitmap, *fileAndBitmap.File, extension)) {
          return true;
        } else {
          return false;
        }
      },
      fileProvider
    );

    if(!wasLoaded) {
      throw Errors::FileFormatError("File format not supported by any registered codec");
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void BitmapSerializer::Reload(Bitmap &exactFittingBitmap, const std::string &path) const {
    std::string::size_type extensionDotIndex = path.find_last_of('.');
#if defined(NUCLEX_PIXELS_WIN32)
    std::string::size_type lastPathSeparatorIndex = path.find_last_of('\\');
#else
    std::string::size_type lastPathSeparatorIndex = path.find_last_of('/');
#endif

    // Check if the provided path contains a file extension and if so, pass it along to
    // the CanLoad() method as a hint (this speeds up codec search)
    if(extensionDotIndex != std::string::npos) {
      bool dotBelongsToFilename = (
        (lastPathSeparatorIndex == std::string::npos) ||
        (extensionDotIndex > lastPathSeparatorIndex)
      );
      if(dotBelongsToFilename) {
        std::unique_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(path, true);
        Reload(exactFittingBitmap, *file.get(), path.substr(extensionDotIndex + 1));
        return;
      }
    }

    // The specified file has no extension, so do not provide the extension hint
    {
      std::unique_ptr<const VirtualFile> file = VirtualFile::OpenRealFileForReading(path, true);
      Reload(exactFittingBitmap, *file.get());
      return;
    }
  }

  // ------------------------------------------------------------------------------------------- //

  void BitmapSerializer::Save(
    const Bitmap &bitmap, VirtualFile &file, const std::string &extension
  ) const {
    (void)bitmap;
    (void)file;
    (void)extension;
    throw std::runtime_error("Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  void BitmapSerializer::Save(
    const Bitmap &bitmap, const std::string &path, const std::string &extension
  ) const {
    (void)bitmap;
    (void)path;
    (void)extension;
    throw std::runtime_error("Not implemented yet");
  }

  // ------------------------------------------------------------------------------------------- //

  template<typename TOutput>
  bool BitmapSerializer::tryCodecsInOptimalOrder(
    const std::string &extension,
    bool (*tryCodecCallback)(
      const BitmapCodec &codec, const std::string &extension, TOutput &result
    ),
    TOutput &result
  ) const {
    std::size_t hintCodecIndex;

    // If an extension hint was provided, try the codec registered for the extension first
    if(extension.empty()) {
      hintCodecIndex = InvalidIndex;
    } else {
      std::string foldedLowercaseExtension = toFoldedLowercase(extension);
      ExtensionCodecIndexMap::const_iterator iterator = (
        this->codecsByExtension.find(foldedLowercaseExtension)
      );
      if(iterator == this->codecsByExtension.end()) {
        hintCodecIndex = InvalidIndex;
      } else {
        hintCodecIndex = iterator->second;
        if(tryCodecCallback(*this->codecs[hintCodecIndex].get(), extension, result)) {
          updateMostRecentCodecIndex(hintCodecIndex);
          return true;
        }
      }
    }

    // Look up the two most recently used codecs (we don't care about race conditions here,
    // in the rare case of one occurring, we'll simple be a little less efficient and not
    // have the right codec in the MRU list...
    std::size_t mostRecent = this->mostRecentCodecIndex;
    std::size_t secondMostRecent = this->secondMostRecentCodecIndex;

    // Try the most recently used codec. It may be set to 'InvalidIndex' if this
    // is the first call to Load(). Don't try if it's the same as the extension hint.
    if((mostRecent != InvalidIndex) && (mostRecent != hintCodecIndex)) {
      if(tryCodecCallback(*this->codecs[mostRecentCodecIndex].get(), extension, result)) {
        updateMostRecentCodecIndex(mostRecent);
        return true;
      }
    }

    // Try the second most recently used logic. It, too, may be set to 'InvalidIndex'.
    // Also avoid retrying codecs we already tried.
    if(
      (secondMostRecent != InvalidIndex) &&
      (secondMostRecent != mostRecent) &&
      (secondMostRecent != hintCodecIndex)
    ) {
      if(tryCodecCallback(*this->codecs[secondMostRecent].get(), extension, result)) {
        updateMostRecentCodecIndex(secondMostRecent);
        return true;
      }
    }

    // Hint was not provided or wrong, most recently used codecs didn't work,
    // so go through all remaining codecs.
    std::size_t codecCount = this->codecs.size();
    for(std::size_t index = 0; index < codecCount; ++index) {
      if((index == mostRecent) || (index == secondMostRecent) || (index == hintCodecIndex)) {
        continue;
      }

      if(tryCodecCallback(*this->codecs[index].get(), extension, result)) {
        updateMostRecentCodecIndex(secondMostRecent);
        return true;
      }
    }

    // No codec can load the file, we give up
    return false;
  }

  // ------------------------------------------------------------------------------------------- //

  void BitmapSerializer::updateMostRecentCodecIndex(std::size_t codecIndex) const {
    this->secondMostRecentCodecIndex = this->mostRecentCodecIndex;
    this->mostRecentCodecIndex = codecIndex;
  }

  // ------------------------------------------------------------------------------------------- //

}}} // namespace Nuclex::Pixels::Storage
