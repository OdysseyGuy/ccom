#ifndef TOKEN_H
#define TOKEN_H

#include "SourceManager.h"

#include <cassert>

enum TokenKind {
#define TOK(X) X,
#include "Tokens.list"
  NumTokens,
};

enum PPKeyword {
#define PPKEYWORD(X) PP_##X,
#include "Tokens.list"
  NumPPKeywords,
};

class IdentifierInfo;

/** A Lexed token. */
class Token {
  SourceLocation Location;
  TokenKind Kind;
  uint32_t Length;
  void *DataPtr;

public:
  void ResetToken() {
    Kind = Unknown;
    DataPtr = nullptr;
  }

  bool IsIdentifier() const { return Kind == Identifier; }

  bool IsStringLiteral() const {
    return Kind == StringLiteral || Kind == WideStringLiteral;
  }

  bool IsLiteral() const {
    return Kind == NumericConstant || Kind == CharacterConstant ||
           Kind == WideCharConstant || IsStringLiteral() || Kind == HeaderName;
  }

  /*====================== Getters and setters =======================*/

  TokenKind GetKind() const { return Kind; }
  void SetKind(TokenKind InKind) { Kind = InKind; }

  IdentifierInfo *GetIdentifierInfo() const {
    if (IsLiteral())
      return nullptr;
    if (Kind == Eof)
      return nullptr;
    return (IdentifierInfo *)DataPtr;
  }
  void SetIdentifierInfo(IdentifierInfo *II) { DataPtr = (void *)II; }

  const char *GetLiteralData() const {
    assert(IsLiteral() && "Cannot get literal data from non-literal");
    return reinterpret_cast<const char *>(DataPtr);
  }

  void SetLiteralData(const char *Data) {
    assert(IsLiteral() && "Cannot set literal data to non-literal");
    DataPtr = const_cast<char *>(Data);
  }

  uint32_t GetLength() { return Length; }
  void SetLength(uint32_t InLength) { Length = InLength; }

  SourceLocation GetLocation() { return Location; }
  void SetLocation(SourceLocation InLocation) { Location = InLocation; }
};

#endif
