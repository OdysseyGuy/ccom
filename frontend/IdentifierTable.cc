#include "IdentifierTable.h"
#include "Options.h"

#include <string.h>

// Keyword flags
namespace {
enum {
  KEYC99 = 0x1,
  KEYCXX = 0x2,
  KEYCXX11 = 0x4,
  KEYALL = 0x1ffffff,
};

enum KeywordAvaibility {
  KA_Available,
  KA_NotAvailable,
};
} // namespace

/** Check if the laguage options allows all the keyword flags. */
static KeywordAvaibility
GetKeywordFlagAvailibility(const LanguageOptions &LangOptions, unsigned Flags) {
  if (Flags == KEYALL)
    return KA_Available;
  if (LangOptions.CPlusPlus && (Flags & KEYCXX))
    return KA_Available;
  if (LangOptions.CPlusPlus11 && (Flags & KEYCXX11))
    return KA_Available;
  if (LangOptions.C99 && (Flags & KEYC99))
    return KA_Available;
  return KA_NotAvailable;
}

/** Adds reserved language specific keywords to identifier info table. */
static void
AddKeyword(std::string Keyword, TokenKind Code,
           const LanguageOptions &LangOptions, unsigned Flags,
           IdentifierInfoTable &Table) {
  KeywordAvaibility AddResult = GetKeywordFlagAvailibility(LangOptions, Flags);

  if (AddResult != KA_Available)
    return;

  IdentifierInfo &Info = Table.GetOrCreate(Keyword);
}

/** Check if the keyword flags are available. */
static KeywordAvaibility
GetKwTokenAvaibility(const LanguageOptions &LangOptions, TokenKind Kind) {
  switch (Kind) {
#define KEYWORD(NAME, FLAGS)                                                   \
  case KW_##NAME:                                                              \
    return GetKeywordAvailibility(LangOptions, FLAGS);
#include "Tokens.list"
  default:
    return KA_NotAvailable;
  }
}

bool
IdentifierInfo::IsKeyword(const LanguageOptions &LangOptions) const {
  return GetKwTokenAvaibility(LangOptions, Kind) == KA_Available ? true : false;
}

PPKeyword
IdentifierInfo::GetPPKeyword() const {
#define HASH(LEN, FIRST, THIRD)                                                \
  (LEN << 5) + (((FIRST - 'a') + (THIRD - 'a')) & 31)

#define CASE(LEN, FIRST, THIRD, NAME, KWNAME)                                  \
  case HASH(LEN, FIRST, THIRD):                                                \
    return memcmp(Name, #NAME, LEN) ? PP_Invalid : PP_##KWNAME

  unsigned Length = GetName().length();
  if (Length < 2)
    return PP_Invalid;
  const char *Name = GetName().c_str();
  switch (HASH(Length, Name[0], Name[2])) {
    CASE(2, 'i', '\0', if, If);

    CASE(4, 'e', 'i', elif, Elif);
    CASE(4, 'e', 's', else, Else);
    CASE(4, 'l', 'n', line, Line);

    CASE(5, 'i', 'd', ifdef, Ifdef);
    CASE(5, 'e', 'd', endif, Endif);
    CASE(5, 'u', 'd', undef, Undef);
    CASE(5, 'e', 'r', error, Error);

    CASE(6, 'i', 'n', ifndef, Ifndef);
    CASE(6, 'd', 'f', define, Define);
    CASE(6, 'p', 'a', pragma, Pragma);

    CASE(7, 'i', 'c', include, Include);
  default:
    return PP_Invalid;
  }
#undef CASE
#undef HASH
}

void
IdentifierInfoTable::AddKeywords(const LanguageOptions &LangOptions) {
#define KEYWORD(NAME, FLAGS)                                                   \
  AddKeyword(std::string(#NAME), KW_##NAME, LangOptions, FLAGS, *this);
#include "Tokens.list"
}