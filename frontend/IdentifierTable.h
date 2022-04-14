#ifndef IDENDTIFIER_TABLE_H
#define IDENDTIFIER_TABLE_H

#include "Mixins.h"
#include "SourceManager.h"
#include "Token.h"

#include <map>
#include <string>

struct LanguageOptions;

class alignas(8) IdentifierInfo
    : private NonCopyableAndMovable<IdentifierInfo> {
  friend class IdentifierInfoTable;

  TokenKind Kind;

  /** True if there is a #define for this. */
  unsigned bHasMacro : 1;

  /** True if there was a #define for this. */
  unsigned bHadMacro : 1;

  SourceLocation Location;

  std::pair<const std::string, IdentifierInfo *> *Entry = nullptr;

  IdentifierInfo() : bHasMacro(false), bHadMacro(false) {}

public:
  std::string GetName() const { return Entry->first; }

  bool GetHasMacroDefinition() const { return bHadMacro; }
  void SetHasMacroDefinition(bool Value) {
    if (Value == bHasMacro)
      return;

    bHasMacro = Value;
    if (Value) {
    }
  }

  /**
   * Returns the preprocessor keyword for this identifier.
   * Returns directive type else returns PP_Invalid if not a preprocessor
   * keyword.
   *
   * For example, "define" would return PP_Define.
   */
  PPKeyword GetPPKeyword() const;

  /** Return true if this token is a keyword in the specified language. */
  bool IsKeyword(const LanguageOptions &LangOptions) const;
};

/** Provides an interface for Identifier lookup. */
class IIdentifierInfoLookup {
public:
  virtual ~IIdentifierInfoLookup();

  virtual IdentifierInfo *Get(std::string &Name) = 0;
};

/** Hash table holding all the user-defined identifiers.  */
class IdentifierInfoTable {
  /** Hash table that stores identifier name as its key. */
  using HashTable_T = std::map<std::string, IdentifierInfo *>;
  HashTable_T HashTable;

  IIdentifierInfoLookup *ExternalLookup;

public:
  IdentifierInfo &GetOrCreate(std::string &Name) {
    auto &Entry = *HashTable.insert({Name, nullptr}).first;

    IdentifierInfo *&II = Entry.second;
    if (II)
      return *II;

    // No entry check for exteranal lookups
    if (ExternalLookup) {
      II = ExternalLookup->Get(Name);
      if (II)
        return *II;
    }

    // Lookups failed, make a new IdentifierInfo.
    II = new IdentifierInfo();
    II->Entry = &Entry;
  }

  using Iterator = HashTable_T::const_iterator;
  using Iterator = HashTable_T::const_iterator;

  Iterator begin() const { return HashTable.begin(); }
  Iterator end() const { return HashTable.end(); }
  unsigned size() const { return HashTable.size(); }

  /**
   * Fill the Identifier table with keywords for language specified by
   * LangOptions.
   */
  void AddKeywords(const LanguageOptions &LangOptions);
};

#endif
