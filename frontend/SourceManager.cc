#include "SourceManager.h"

SourceManager::SourceManager(bool _Dummy) { Reset(); }

SourceManager::~SourceManager() {}

/* Create a new FileID for specified include position. */
FileID
SourceManager::CreateFileID(FileContentCache &File, SourceLocation IncludePos,
                            int LoadedID) {
  // Loaded entry
  if (LoadedID < 0) {
    // Loaded FileID
    assert(LoadedID != -1 && "Loading sentinel FileID");
    unsigned Index = unsigned(-LoadedID) - 2;
    assert(Index < LoadedSrcLocEntryTable.size() && "FileID out of range");
    assert(!SLocEntryLoaded[Index] && "FileID already loaded");
    // TODO : work this out
    // LoadedSrcLocEntryTable[Index] = SourceLocationEntry::Create(LoadedO)
    SLocEntryLoaded[Index] = true;
  }

  // Local entry
  LocalSrcLocEntryTable.emplace_back(SourceLocationEntry::Create(
      NextLocalOffset, FileInfo::Create(IncludePos)));

  // We do a +1 here because we want a SourceLocation that means "the end of the
  // file"
  NextLocalOffset += File.GetSize() + 1;

  // FileID is just the last insert index
  FileID FID = LocalSrcLocEntryTable.size() - 1;
  return LastFileIDLookup = FID;
}

FileID
SourceManager::GetFileID_CM(uint32_t SLocOffset) const {
  if (SLocOffset < NextLocalOffset) {
    return GetFileID_CM_Local(SLocOffset);
  }
  return GetFileID_CM_Loaded(SLocOffset);
}

FileID
SourceManager::GetFileID_CM_Local(uint32_t SLocOffset) const {
  assert(SLocOffset < NextLocalOffset && "Bad function choice");

  SourceLocationEntryTable::const_iterator I;

  // First we are going to lookup for offset in the LastFileIDLookup
  if (LastFileIDLookup < 0 ||
      LocalSrcLocEntryTable[LastFileIDLookup].GetOffset() < SLocOffset) {
    // set this to end we are going to search by decrementing the pointer
    I = LocalSrcLocEntryTable.end();
  } else {
    // start from LastFileIDLookup
    I = (LocalSrcLocEntryTable.begin() + LastFileIDLookup);
  }

  // decrement the Iterator to find the FileID we are looking for
  unsigned NumProbes = 0;
  for (; NumProbes < 8; NumProbes++, --I) {
    if (I->GetOffset() <= SLocOffset) {
      FileID ResultIndex = (I - LocalSrcLocEntryTable.begin());
      LastFileIDLookup = ResultIndex; // Save it to cache.
      return ResultIndex;
    }
  }

  unsigned GreaterIndex = I - LocalSrcLocEntryTable.begin();
  NumProbes = 0;
  while (true) {
    break;
  }
}

FileID
SourceManager::GetFileID_CM_Loaded(uint32_t SLocOffset) const {
  assert(SLocOffset < CurrentLoadedOffset && "Bad function choice");

  unsigned I;
  if (LastFileIDLookup >= 0 ||
      GetSLocEntryByID(LastFileIDLookup).GetOffset() < SLocOffset) {
    I = 0;
  } else {
    I = (-LastFileIDLookup + 2) + 1;
  }

  unsigned NumProbes = 0;
  for (NumProbes = 0; NumProbes < 8; NumProbes++, ++I) {
    const SourceLocationEntry &Entry = GetLoadedSLocEntry(I);
    if (Entry.GetOffset() <= SLocOffset) {
      FileID Result = (-I - 2);
      LastFileIDLookup = Result;
      return Result;
    }
  }
}

std::pair<FileID, unsigned>
SourceManager::GetDecomposedExpansionLocSlow(
    const SourceLocationEntry *Entry) const {
  FileID FID;
  SourceLocation Loc;
  unsigned Offset;
  do {
    Loc = Entry->GetExpansion().GetExpansionLocStart();
    FID = GetFileID(Loc);

    Entry = &GetSLocEntryByID(FID);
    Offset = Loc.GetOffset() - Entry->GetOffset();
  } while (!Loc.IsFileID());

  return std::make_pair(FID, Offset);
}

std::pair<FileID, unsigned>
SourceManager::GetDecomposedSpellingLocSlow(const SourceLocationEntry *Entry,
                                            uint32_t Offset) const {
  FileID FID;
  SourceLocation Loc;
  do {
    Loc = Entry->GetExpansion().GetSpellingLoc();
    Loc = Loc.GetLocWithOffset(Offset);
    FID = GetFileID(Loc);

    Entry = &GetSLocEntryByID(FID);
    Offset = Loc.GetOffset() - Entry->GetOffset();
  } while (!Loc.IsFileID());

  return std::make_pair(FID, Offset);
}

void
SourceManager::Reset() {
  MainFileID = 0;
  LastFileIDLookup = 0;

  LoadedSrcLocEntryTable.clear();
  LocalSrcLocEntryTable.clear();
}

FileContentCache &
SourceManager::CreateContentCache(std::string &Buf) {
  FileContentCache *Entry = new FileContentCache();
  Entry->SetBuffer(Buf);
  return *Entry;
}

FileID
SourceManager::TranslateFile(const FileEntry *SourceFile) {
  assert(SourceFile && "Null source file!");

  // Look through all the local source locations
  for (unsigned Index = 0; Index < LocalSrcLocEntryTable.size(); ++Index) {
    const SourceLocationEntry &Entry = GetLocalSLocEntry(Index);
    if (Entry.IsFile()) {
      return Index;
    }
  }

  for (unsigned Index = 0; Index < LoadedSrcLocEntryTable.size(); ++Index) {
    const SourceLocationEntry &Entry = GetLoadedSLocEntry(Index);
    if (Entry.IsFile()) {
      return Index;
    }
  }
}
