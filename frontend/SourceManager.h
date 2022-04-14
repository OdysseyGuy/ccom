#ifndef SOURCE_MANAGER_H
#define SOURCE_MANAGER_H

#include "File.h"
#include "Mixins.h"

#include <cassert>
#include <memory>
#include <string>
#include <utility>
#include <vector>

/* ========================================================
 *  SourceLocation
 * ========================================================
 */

/**
 * Encodes a location in the source. Source location is simply an offset into
 * the manager's view of the input source.
 *
 * Stores File or macro expansion. The MacroIDBit (reserved) can be used to
 * access the information whether the location is in a file or macro expansion.
 */
class SourceLocation {
  friend class SourceManager;

  uint32_t ID = 0;

  // 1 << 31
  enum : uint32_t { IsMacroBit = 1ULL << (8 * sizeof(uint32_t) - 1) };

public:
  bool IsFileID() const { return (ID & IsMacroBit) == 0; }
  bool IsMacroID() const { return (ID & IsMacroBit) != 0; }

  bool IsValid() const { return ID != 0; }
  bool IsInvalid() const { return ID == 0; }

private:
  /** Get pure offset without the Macro test bit. */
  uint32_t GetOffset() const { return ID & ~IsMacroBit; }

  /** Creates SourceLocation from ID. */
  static SourceLocation GetFileLoc(uint32_t ID) {
    SourceLocation L;
    L.ID = ID;
    return L;
  }

  /** Creates SourceLoction from ID. Set the MacroIDBit. */
  static SourceLocation GetMacroLoc(uint32_t ID) {
    SourceLocation L;
    L.ID = IsMacroBit | ID;
    return L;
  }

public:
  /**
   * Returns a source location with the specified offset from this source
   * location.
   */
  SourceLocation GetLocWithOffset(int32_t Offset) const {
    // make sure the offset doesn't affect the IsMacroBit
    assert(((ID + Offset) & IsMacroBit) == 0 && "Offset overflow!");
    SourceLocation L;
    L.ID = ID + Offset;
    return L;
  }
};

typedef int FileID;

/* ========================================================
 *  FileInfo
 * ========================================================
 */

/** Information each corresponding to a FileID. */
class FileInfo {
  SourceLocation IncludeLocation;

public:
  /** Creates a new FileInfo object. */
  static FileInfo Create(SourceLocation IncludeLoc) {
    FileInfo FI;
    FI.IncludeLocation = IncludeLoc;
    return FI;
  }

  SourceLocation GetIncludeLocation() const { return IncludeLocation; }
};

/* ========================================================
 *  FileContentCache
 * ========================================================
 */
class FileContentCache {
  std::string Buffer;

  std::string FileName;

public:
  std::string GetFileName() const { return FileName; }

  unsigned GetSize() const { return Buffer.size(); }

  void SetBuffer(std::string &InBuffer) { Buffer = InBuffer; }
};

/* ========================================================
 *  ExpansionInfo
 * ========================================================
 */

/** Encode Expansion and Spelling location. */
class ExpansionInfo {
public:
  SourceLocation SpellingLocation;
  SourceLocation ExpansionLocStart, ExpansionLocEnd;

public:
  SourceLocation GetSpellingLoc() const {
    return SpellingLocation.IsInvalid() ? GetExpansionLocStart()
                                        : SpellingLocation;
  }

  SourceLocation GetExpansionLocStart() const { return ExpansionLocStart; }

  SourceLocation GetExpansionLocEnd() const {
    return ExpansionLocEnd.IsInvalid() ? GetExpansionLocStart()
                                       : ExpansionLocEnd;
  }

  static ExpansionInfo Create(SourceLocation SpellingLoc, SourceLocation Start,
                              SourceLocation End) {
    ExpansionInfo ExpInfo;
    ExpInfo.SpellingLocation = SpellingLoc;
    ExpInfo.ExpansionLocStart = Start;
    ExpInfo.ExpansionLocEnd = End;
    return ExpInfo;
  }
};

/* ========================================================
 *  SourceLocationEntry
 * ========================================================
 */

/**
 * 31 bits Offset.
 * 1 Bit Exansion.
 */
class SourceLocationEntry {
  static constexpr int OffsetBits = 8 * sizeof(uint32_t) - 1;
  uint32_t Offset : OffsetBits;
  uint32_t bIsExpansion : 1;
  union {
    FileInfo File;
    ExpansionInfo Expansion;
  };

public:
  SourceLocationEntry()
      : Offset()
      , bIsExpansion()
      , File() {}

public:
  /** Creates a FileInfo type SourceLocationEntry. */
  static SourceLocationEntry Create(uint32_t InOffset, const FileInfo &FI) {
    assert(!(InOffset & (1 << OffsetBits)) && "Offset size overflow!");
    SourceLocationEntry Entry;
    Entry.Offset = InOffset;
    Entry.File = FI;
    Entry.bIsExpansion = false;
    return Entry;
  }

  /** Creates a ExapansionInfo type SourceLocationEntry. */
  static SourceLocationEntry Create(uint32_t InOffset,
                                    const ExpansionInfo &EI) {
    assert(!(InOffset & (1 << OffsetBits)) && "Offset size overflow!");
    SourceLocationEntry Entry;
    Entry.Offset = InOffset;
    Entry.Expansion = EI;
    Entry.bIsExpansion = true;
    return Entry;
  }

public:
  uint32_t GetOffset() const { return Offset; }

  bool IsExpansion() const { return bIsExpansion; }
  bool IsFile() const { return !IsExpansion(); }

  const FileInfo &GetFile() const {
    assert(IsFile() && "Not a file SourceLocationEntry!");
    return File;
  }

  const ExpansionInfo &GetExpansion() const {
    assert(IsFile() && "Not a macro expansion SourceLocationEntry!");
    return Expansion;
  }
};

/* ========================================================
 *  SourceManager
 * ========================================================
 */

class SourceManager : private NonCopyable<SourceManager> {
  using SourceLocationEntryTable = std::vector<SourceLocationEntry>;

  /** Table of SourceLocationEntries that are local to this module. */
  SourceLocationEntryTable LocalSrcLocEntryTable;

  /** Table of SourceLocationEntries that were loaded from other modules. */
  SourceLocationEntryTable LoadedSrcLocEntryTable;

  // TODO: replace with a bit vector for better effeciency
  /** A Boolean vector that indicates whether the entries of
   * LoadedSLocEntryTable have already been loaded from external source. */
  std::vector<bool> SLocEntryLoaded;

  /** The starting offset of the next local SourceLocationEntry. */
  uint32_t NextLocalOffset;

  /** The starting offset of the next loaded SourceLocationEntry. */
  uint32_t CurrentLoadedOffset;

  /**
   * Single-entry cache to speed up GetFileID. On cache miss fallback to
   * GetFileID_CM method.
   *
   * One entry cache. Store the last FileID looked up, or created.
   */
  mutable FileID LastFileIDLookup;

  /** The File ID for the main source file of the translation unit.  */
  FileID MainFileID;

public:
  SourceManager(bool _Dummy);
  ~SourceManager();

  FileID CreateFileID(FileContentCache &File, SourceLocation IncludePos,
                      int LoadedID);

  std::string GetFilename(SourceLocation Location) const;
  const char *GetCharacterData(SourceLocation SL);

  FileID GetFileID(SourceLocation Loc) const {
    // fast path; look up single-entry cache
    unsigned SourceLocOffset = Loc.GetOffset();

    if (IsOffsetInFileID(LastFileIDLookup, SourceLocOffset)) {
      uint32_t SourceLocOffset = Loc.GetOffset();
      return 0;
    }

    return GetFileID_CM(SourceLocOffset);
  }

  /** Form source location from a FileID and Offset pair. */
  SourceLocation GetComposedLoc(FileID FID, unsigned Offset) {
    auto *Entry = GetSLocEntryOrNull(FID);
    if (!Entry) {
      return SourceLocation();
    }

    // FileID offset + Offset of loc
    uint32_t GlobalOffset = Entry->GetOffset() + Offset;
    return Entry->IsFile() ? SourceLocation::GetFileLoc(GlobalOffset)
                           : SourceLocation::GetMacroLoc(GlobalOffset);
  }

  /**
   * Decomposes the specified location into a raw FileID + offset pair.
   *
   * The first element is the FileID, the second is the offset from the start of
   * the buffer of the location.
   */
  std::pair<FileID, unsigned> GetDecomposedLoc(SourceLocation Loc) const {
    FileID FID = GetFileID(Loc);
    auto *Entry = GetSLocEntryOrNull(FID);
    if (!Entry) {
      return std::make_pair(0, 0);
    }
    return std::make_pair(FID, Loc.GetOffset() - Entry->GetOffset());
  }

  std::pair<FileID, unsigned>
  GetDecomposedExpansionLoc(SourceLocation Loc) const {
    FileID FileIndex = GetFileID(Loc);
    auto *Entry = GetSLocEntryOrNull(FileIndex);
    if (!Entry) {
      return std::make_pair(0, 0);
    }

    unsigned Offset = Loc.GetOffset() - Entry->GetOffset();
    return Loc.IsFileID() ? std::make_pair(FileIndex, Offset)
                          : GetDecomposedExpansionLocSlow(Entry);
  }

  std::pair<FileID, unsigned>
  GetDecomposedSpellingLoc(SourceLocation Loc) const {
    FileID FID = GetFileID(Loc);
    auto *Entry = GetSLocEntryOrNull(FID);
    if (!Entry) {
      return std::make_pair(0, 0);
    }

    unsigned Offset = Loc.GetOffset() - Entry->GetOffset();
    return Loc.IsFileID() ? std::make_pair(FID, Offset)
                          : GetDecomposedSpellingLocSlow(Entry, Offset);
  }

  /**
   * Returns the offset of the start of the file that the specified
   * SourceLocation represents.
   */
  unsigned GetFileOffset(SourceLocation Loc) const {
    return GetDecomposedLoc(Loc).second;
  }

  bool IsOffsetInFileID(SourceLocation Loc, FileID FID) { return false; }

  /** Returns true if the specified FileID contains the specifier offset. */
  inline bool IsOffsetInFileID(FileID FID, uint32_t Offset) const {
    return false;
  }

  const SourceLocationEntry &GetSLocEntryByID(int ID) const {
    // Load either form loaded or local entry table
    if (ID < 0)
      return GetLoadedSLocEntry(static_cast<unsigned>(-ID - 2));
    return GetLocalSLocEntry(static_cast<unsigned>(ID));
  }

  const SourceLocationEntry &GetLoadedSLocEntry(int Index) const {
    assert(Index < LoadedSrcLocEntryTable.size() && "Invalid Index");
    // TODO: check whether the entry has been loaded else load it
    // if (SLocEntryLoaded[Index]) {
    // }
    return LoadedSrcLocEntryTable[Index];
    // else load the SLocEntry
  }

  const SourceLocationEntry &GetLocalSLocEntry(int Index) const {
    assert(Index < LocalSrcLocEntryTable.size() && "Invalid Index");
    return LocalSrcLocEntryTable[Index];
  }

  const SourceLocationEntry *GetSLocEntryOrNull(FileID FID) const {
    return &GetSLocEntryByID(FID);
  }

  /** Fallback path in case GetFileID cache miss. */
  FileID GetFileID_CM(uint32_t SLocOffset) const;
  FileID GetFileID_CM_Local(uint32_t SLocOffset) const;
  FileID GetFileID_CM_Loaded(uint32_t SLocOffset) const;

  std::pair<FileID, unsigned>
  GetDecomposedExpansionLocSlow(const SourceLocationEntry *Entry) const;

  std::pair<FileID, unsigned>
  GetDecomposedSpellingLocSlow(const SourceLocationEntry *Entry,
                               uint32_t Offset) const;

  FileID GetMainFileID() const { return MainFileID; }
  void SetMainFileID(FileID FID) { MainFileID = FID; }

  FileContentCache &CreateContentCache(std::string &Buf);

  /**
   * Given a source file return the FileID for it.
   *
   * If the source file is included multiple times, the FileID will be the first
   * inclusion.
   */
  FileID TranslateFile(const FileEntry *SourceFile);

  void Reset();
};

#endif