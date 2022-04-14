#ifndef FILE_H
#define FILE_H

#include "Mixins.h"

#include <string>

class FileEntry : private NonCopyable<FileEntry> {
  friend class FileManager;

  std::string RealPathName;
  off_t Size;

  bool bIsValid = false;

public:
  FileEntry() {}
  ~FileEntry() = default;

  std::string GetRealPathName() const { return RealPathName; }
  bool IsValid() const { return bIsValid; }
  off_t GetSize() const { return Size; }
};

#endif