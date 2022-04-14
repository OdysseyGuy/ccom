#ifndef PREPROCESSOR_LEXER_H
#define PREPROCESSOR_LEXER_H

#include "Mixins.h"
#include "PPConditionalDirective.h"
#include "SourceManager.h"

#include <cassert>
#include <map>
#include <vector>

class FileEntry;
class Preprocessor;

class Token;

class PreprocessorLexer : private NonCopyable<PreprocessorLexer> {
  friend class Preprocessor;
  friend class Lexer;

  Preprocessor *OwnerPP = nullptr;

  /** FileID corresponding to the file being lexed. */
  const FileID FID;

  /** True when parsing #X; turns '\\n' into a Eod token. */
  bool ParsingPreprocessorDirective = false;

  /** True after #include; turns <foo> or "foo" into HeaderName token. */
  bool ParsingFilename = false;

  /**
   * Information about the set of #if / #ifdef / #ifndef blocks we are
   * currently in.
   */
  std::vector<PPConditionalInfo> ConiditionalStack;

  struct IncludeEntry {
    const FileEntry *File;
    SourceLocation Location;
  };

  /** A complete history of all the files included by the current file. */
  std::map<std::string, IncludeEntry> IncludeHistory;

  PreprocessorLexer(Preprocessor *InOwnerPP, FileID InFid);

public:
  void LexIncludeFilename(Token &FilenameToken);

  Preprocessor *GetOwnerPP() { return OwnerPP; }

  FileID GetFileID() const {
    assert(OwnerPP);
    return FID;
  }

  using ConditionalIterator = std::vector<PPConditionalInfo>::const_iterator;

  ConditionalIterator ConditionalBegin() const {
    return ConiditionalStack.begin();
  }

  ConditionalIterator ConditionalEnd() const { return ConiditionalStack.end(); }

  /** Insert into the list of  */
  void AddInclude(const std::string &Filename, const FileEntry &InFile,
                  SourceLocation InSourceLocation) {
    IncludeHistory.insert({Filename, {&InFile, InSourceLocation}});
  }

  std::map<std::string, IncludeEntry> &GetIncludeHistory() {
    return IncludeHistory;
  }
};

#endif
