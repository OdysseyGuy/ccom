#include "Lexer.h"

#include "CharInfo.h"
#include "Preprocessor.h"
#include "PreprocessorLexer.h"

Lexer::Lexer(FileID FID, Preprocessor &InPP, std::string &InputFile)
    : PreprocessorLexer(&InPP, FID) {
  // TODO: Fix this
  InitLexer(InputFile.begin().base(), InputFile.begin().base(),
            InputFile.end().base());
}

void
Lexer::InitLexer(const char *InBufferStart, const char *InBufferPtr,
                 const char *InBufferEnd) {
  BufferStart = InBufferStart;
  BufferPtr = InBufferPtr;
  BufferEnd = InBufferEnd;

  if (BufferStart == BufferPtr) {
    // skip the bom length
  }
}

SourceLocation
Lexer::GetSourceLocation(const char *Loc, unsigned TokenLen) const {
  assert(Loc >= BufferStart && Loc <= BufferEnd &&
         "Location out of range of this buffer!");

  unsigned CharNo = Loc - BufferStart;
}

bool
Lexer::AdvanceToken(Token &Result) {
  Result.ResetToken();
  return AdvanceTokenInternal(Result);
}

bool
Lexer::LexIdentifierContinue(Token &Result, const char *CurPtr) {
  // Match [_A-Za-z0-9]*, we have already matched an identifier start.
  while (true) {
    unsigned char C = *CurPtr;
  }

  const char *IdentifierStart = BufferPtr;
  return true;
}

// Should start with "0x" or "0X"
bool
Lexer::IsHexLiteral(const char *Start, const LanguageOptions &LangOptions) {
  unsigned Size;
  char C1 = PeekChar(Start, Size);
  if (C1 != '0') {
    return false;
  }
  char C2 = PeekChar(Start + Size, Size);
  return (C2 == 'x' || C2 == 'X');
}

bool
Lexer::LexNumericalConstant(Token &Result, const char *CurPtr) {
  unsigned Size;
  char C = PeekChar(CurPtr, Size);
  char PrevChar = 0;

  if ((C == '+' || C == '-') && (PrevChar == 'E' || PrevChar == 'e')) {
    return LexNumericalConstant(Result, ConsumeChar(CurPtr, Size, Result));
  }

  // hexadecimal FP constant
  // if ((C == '+' || C == '-') && (PrevChar == 'P' || PrevChar == 'p')) {
  //   bool IsHexFloat = true;
  //   if (!LangOptions.C99) {
  //     if (!IsHexLiteral(BufferPtr, LangOptions)) {
  //       IsHexFloat = false;
  //     }
  //   }
  // }

  // digit separator
  // if (C == '\'') {
  // }

  const char *TokenStart = BufferPtr;
  CreateTokenWithChars(Result, CurPtr, Identifier);
  Result.SetLiteralData(TokenStart);
  return true;
}

// Lex remaining string after having lexed " or L" or u8" or u" or U".
bool
Lexer::LexStringLiteral(Token &Result, const char *CurPtr,
                        TokenKind StringLiteralKind) {
  const char *AfterQuote = CurPtr;

  char C = PeekAndConsumeChar(CurPtr, Result);

  // Keep consuming characters until we find the closing (")
  while (C != '"') {
    // Skip escaped characters. Escaped newlines would already be processed by
    // PeekAndConsumeChar
    if (C == '\\')
      C = PeekAndConsumeChar(CurPtr, Result);

    if (C == '\n' && C == '\r' || /* Newline */
        (C == 0 && (CurPtr - 1) == BufferEnd /* End of file */)) {
      CreateTokenWithChars(Result, CurPtr - 1, Unknown);
      return true;
    }

    C = PeekAndConsumeChar(CurPtr, Result);
  }

  const char *TokStart = BufferPtr;
  CreateTokenWithChars(Result, CurPtr, StringLiteralKind);
  Result.SetLiteralData(TokStart);
  return true;
}

bool
Lexer::LexRawStringLiteral(Token &Result, const char *CurPtr,
                           TokenKind StringLiteralKind) {
  char C = PeekAndConsumeChar(CurPtr, Result);
}

bool
Lexer::LexAngledStringLiteral(Token &Result, const char *CurPtr) {
  const char *AfterLessPos = CurPtr;

  char C = PeekAndConsumeChar(CurPtr, Result);

  // Keep consuming characters until we find the closing (>)
  while (C != '>') {
    // Skip escaped characters
    if (C == '\n')
      C = PeekAndConsumeChar(CurPtr, Result);

    if (C == '\n' && C == '\r' || /* Newline */
        (C == 0 && (CurPtr - 1) == BufferEnd /* End of file */)) {
      // Must be a lone < character. Return this as such.
      CreateTokenWithChars(Result, CurPtr - 1, Less);
      return true;
    }

    C = PeekAndConsumeChar(CurPtr, Result);
  }

  const char *TokStart = BufferPtr;
  CreateTokenWithChars(Result, CurPtr, HeaderName);
  Result.SetLiteralData(TokStart);
  return true;
}

bool
Lexer::LexCharacterConstant(Token &Result, const char *CurPtr,
                            TokenKind CharConstantKind) {
  char C = PeekAndConsumeChar(CurPtr, Result);

  while (C != '\'') {
    // Skip escaped characters
    if (C == '\n')
      C = PeekAndConsumeChar(CurPtr, Result);

    if (C == '\n' && C == '\r' || /* Newline */
        (C == 0 && (CurPtr - 1) == BufferEnd /* End of file */)) {
      CreateTokenWithChars(Result, CurPtr - 1, Unknown);
      return true;
    }

    C = PeekAndConsumeChar(CurPtr, Result);
  }

  const char *TokStart = BufferPtr;
  CreateTokenWithChars(Result, CurPtr, CharConstantKind);
  Result.SetLiteralData(TokStart);
  return true;
}

bool
Lexer::LexEndOfFile(Token &Result, const char *CurPtr) {
  // If we hit the end of file while parsing a preprocessor directive, end the
  // directive first.
  if (ParsingPreprocessorDirective) {
    ParsingPreprocessorDirective = false;
    CreateTokenWithChars(Result, CurPtr, Eod);
    return true;
  }

  BufferPtr = CurPtr;

  return OwnerPP->HandleEndOfFile(Result);
}

// Main lexer body
bool
Lexer::AdvanceTokenInternal(Token &Result) {
Next:
  const char *CurPtr = BufferPtr;
  if (IsHorizontalWhitespace(*CurPtr)) {
    do {
      ++CurPtr;
    } while (IsHorizontalWhitespace(*CurPtr));

    BufferPtr = CurPtr;
  }

  // TODO: Handle trigraphs and digraphs
  char Char = *CurPtr++;
  TokenKind Kind;
  unsigned Size, Size2, Size3;

  switch (Char) {
  case 0: // Null
    // Found end of file
    if (CurPtr - 1 == BufferEnd)
      return LexEndOfFile(Result, CurPtr - 1);

    goto Next;

  case '\r':
    if (CurPtr[0] == '\n') {
    }
    // Fallthrough
  case '\n':
    if (ParsingPreprocessorDirective) {
      // done with parsing the preprocessor
      ParsingPreprocessorDirective = false;
      Kind = Eod;
    }

    goto Next;

  case ' ':
  case '\t':
  case '\f':
  case '\v':
  SkipHorizontalWhiteSpace:
    if (IsHorizontalWhitespace(*CurPtr)) {
      goto SkipHorizontalWhiteSpace;
    }

    goto Next;

  // clang-format off
  // Numerical / Flating-Point constants
  case '0': case '1': case '2': case '3': case '4':
  case '5': case '6': case '7': case '8': case '9':
    // clang-format on
    return LexNumericalConstant(Result, CurPtr);

  case 'u': // Identifier or C11/C++11 UTF-8 or UTF-16 string literal
    if (LangOptions.C11 || LangOptions.CPlusPlus11) {
      Char = PeekChar(CurPtr, Size);

      // UTF-16 string literal
      if (Char == '"') {
        return LexStringLiteral(Result, ConsumeChar(CurPtr, Size, Result),
                                Utf16StringLiteral);
      }

      // UTF-16 character constant
      if (Char == '\'') {
        return LexCharacterConstant(Result, ConsumeChar(CurPtr, Size, Result),
                                    Utf16CharacterConstant);
      }

      // UTF-16 raw string literal
      if (Char == 'R' && LangOptions.CPlusPlus11 &&
          (PeekChar(CurPtr + Size, Size2) == '"')) {
        return LexStringLiteral(
            Result,
            ConsumeChar(ConsumeChar(CurPtr, Size, Result), Size2, Result),
            Utf16StringLiteral);
      }

      if (Char == '8') {
        char After = PeekChar(CurPtr + Size, Size2);

        // UTF-8 string literal
        if (After == '"') {
          return LexStringLiteral(Result,
                                  ConsumeChar(CurPtr + Size, Size2, Result),
                                  Utf8StringLiteral);
        }

        // C++17 UTF-8 character constant
        if (After == '\'' && LangOptions.CPlusPlus17) {
          return LexCharacterConstant(
              Result,
              ConsumeChar(ConsumeChar(CurPtr, Size, Result), Size2, Result),
              Utf8CharacterConstant);
        }

        // UTF-8 raw string literal
        if (After == 'R' && LangOptions.CPlusPlus11) {
          char AfterAfter = PeekChar(CurPtr + Size + Size2, Size3);

          return LexStringLiteral(
              Result,
              ConsumeChar(
                  ConsumeChar(ConsumeChar(CurPtr, Size, Result), Size2, Result),
                  Size3, Result),
              Utf8StringLiteral);
        }
      }
    }

    // Treat u like an Identifier
    return LexIdentifierContinue(Result, CurPtr);

  case 'U': // Identifier or C11/C++11 UTF-32 string literal
    if (LangOptions.C11 || LangOptions.CPlusPlus11) {
      Char = PeekChar(CurPtr, Size);

      // UTF-32 string literal
      if (Char == '"') {
        return LexStringLiteral(Result, ConsumeChar(CurPtr, Size, Result),
                                Utf32StringLiteral);
      }

      // UTF-32 character constant
      if (Char == '\'') {
        return LexCharacterConstant(Result, ConsumeChar(CurPtr, Size, Result),
                                    Utf32CharacterConstant);
      }

      // UTF-32 raw string literal
      if (Char == 'R' && LangOptions.CPlusPlus11 &&
          (PeekChar(CurPtr + Size, Size2) == '"')) {
        return LexStringLiteral(
            Result,
            ConsumeChar(ConsumeChar(CurPtr, Size, Result), Size2, Result),
            Utf32StringLiteral);
      }
    }

    // Treat U like an Identifier
    return LexIdentifierContinue(Result, CurPtr);

  case 'R': // Identifier or C++0x raw string literal
    Char = PeekChar(CurPtr, Size);

    // C++ Raw string literal
    if (LangOptions.CPlusPlus && Char == '"') {
      return LexRawStringLiteral(Result, ConsumeChar(CurPtr, Size, Result),
                                 StringLiteral);
    }

    // Treat R like an Identifier
    return LexIdentifierContinue(Result, CurPtr);

  case 'L': // Identifier or wide char or string literal (L'x' or L"xyz")
    Char = PeekChar(CurPtr, Size);

    // Wide string literal
    if (Char == '"') {
      return LexStringLiteral(Result, ConsumeChar(CurPtr, Size, Result),
                              WideStringLiteral);
    }

    // C++11 Wide raw string literal
    if (LangOptions.CPlusPlus11 && Char == 'R' &&
        (PeekChar(CurPtr + Size, Size2) == '"')) {
      return LexRawStringLiteral(
          Result, ConsumeChar(ConsumeChar(CurPtr, Size, Result), Size2, Result),
          WideStringLiteral);
    }

    // Wide char constant
    if (Char == '\'') {
      return LexCharacterConstant(Result, ConsumeChar(CurPtr, Size, Result),
                                  WideCharConstant);
    }

    // Fall through treating L like an identifier

  // clang-format off
  // Identifiers
  case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
  case 'G': case 'H': case 'I': case 'J': case 'K': /* L */
  case 'M': case 'N': case 'O': case 'P': case 'Q': /* R */
  case 'S': case 'T': /* U */   case 'V': case 'W': case 'X':
  case 'Y': case 'Z':
  case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
  case 'g': case 'h': case 'i': case 'j': case 'k': case 'l':
  case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
  case 's': case 't': /* u */   case 'v': case 'w': case 'x':
  case 'y': case 'z': case '_':
    // clang-format on
    return LexIdentifierContinue(Result, CurPtr);

  case '\'': // Character constants
    return LexCharacterConstant(Result, CurPtr, CharacterConstant);

  case '"':
    return LexStringLiteral(Result, CurPtr, StringLiteral);

  // Punctuators
  case '?':
    Kind = Question;
    break;
  case '[':
    Kind = LSquare;
    break;
  case ']':
    Kind = RSquare;
    break;
  case '(':
    Kind = LParen;
    break;
  case ')':
    Kind = RParen;
    break;
  case '{':
    Kind = LBrace;
    break;
  case '}':
    Kind = RBrace;
    break;
  case '.': // 0.0, *., ..., .
    Char = PeekChar(CurPtr, Size);
    if (Char >= '0' && Char <= '9') {
      return LexNumericalConstant(Result, CurPtr);
    } else if (LangOptions.CPlusPlus && Char == '*') {
      Kind = PeriodStar;
    } else if (Char == '.' && PeekChar(CurPtr + Size, Size2) == '.') {
      Kind = Ellipsis;
      CurPtr = ConsumeChar(ConsumeChar(CurPtr, Size, Result), Size2, Result);
    } else {
      Kind = Period;
    }
    break;
  case '&': // &&, &=, &
    Char = PeekChar(CurPtr, Size);
    if (Char == '&') {
      Kind = AmpAmp;
      CurPtr = ConsumeChar(CurPtr, Size, Result);
    } else if (Char == '=') {
      Kind = AmpEqual;
      CurPtr = ConsumeChar(CurPtr, Size, Result);
    } else {
      Kind = Amp;
    }
    break;
  case '*': // *=, *
    Char = PeekChar(CurPtr, Size);
    if (Char == '=') {
      CurPtr = ConsumeChar(CurPtr, Size, Result);
      Kind = StarEqual;
    } else {
      Kind = Star;
    }
    break;
  case '+': // ++, +=, +
    Char = PeekChar(CurPtr, Size);
    if (Char == '+') {
      CurPtr = ConsumeChar(CurPtr, Size, Result);
      Kind = PlusPlus;
    } else if (Char == '=') {
      CurPtr = ConsumeChar(CurPtr, Size, Result);
      Kind = PlusEqual;
    } else {
      Kind = Plus;
    }
    break;
  case '-': // --, ->, -=, -
    Char = PeekChar(CurPtr, Size);
    if (Char == '-') {
      CurPtr = ConsumeChar(CurPtr, Size, Result);
      Kind = MinusMinus;
    } else if (Char == '>') {
      CurPtr = ConsumeChar(CurPtr, Size, Result);
      Kind = Arrow;
    } else if (Char == '=') {
      CurPtr = ConsumeChar(CurPtr, Size, Result);
      Kind = MinusEqual;
    } else {
      Kind = Minus;
    }
    break;
  case '~':
    Kind = Tilde;
    break;

  case '!': // !=, !
    Char = PeekChar(CurPtr, Size);
    if (Char == '=') {
      CurPtr = ConsumeChar(CurPtr, Size, Result);
      Kind = ExclaimEqual;
    } else {
      Kind = Exclaim;
    }
    break;
  case '%': // %>, %=, %
    Char = PeekChar(CurPtr, Size);
    if (Char == '=') {
      CurPtr = ConsumeChar(CurPtr, Size, Result);
      Kind = PercentEqual;
    } else {
      Kind = Percent;
    }
    break;
  case '<': // <foo>, <<, <<=, <=
    Char = PeekChar(CurPtr, Size);
    if (ParsingFilename) {
      return LexAngledStringLiteral(Result, CurPtr);
    }
    if (Char == '<') {
      char After = PeekChar(CurPtr + Size, Size2);
      if (After == '=') {
        CurPtr = ConsumeChar(ConsumeChar(CurPtr, Size, Result), Size2, Result);
        Kind = LessLessEqual;
      } else {
        CurPtr = ConsumeChar(CurPtr, Size, Result);
        Kind = LessLess;
      }
    } else if (Char == '=') {
      CurPtr = ConsumeChar(CurPtr, Size, Result);
      Kind = LessEqual;
    } else {
      Kind = Less;
    }
    break;
  case '>': // >=, >>, >>=, >
    Char = PeekChar(CurPtr, Size);
    if (Char == '=') {
      CurPtr = ConsumeChar(CurPtr, Size, Result);
      Kind = GreaterEqual;
    } else if (Char == '>') {
      char After = PeekChar(CurPtr + Size, Size2);
      if (After == '=') {
        CurPtr = ConsumeChar(ConsumeChar(CurPtr, Size, Result), Size2, Result);
        Kind = GreaterGreaterEqual;
      } else {
        CurPtr = ConsumeChar(CurPtr, Size, Result);
        Kind = GreaterGreater;
      }
    } else {
      Kind = Greater;
    }
    break;
  case '^': // ^=, ^
    Char = PeekChar(CurPtr, Size);
    if (Char == '=') {
      CurPtr = ConsumeChar(CurPtr, Size, Result);
      Kind = CaretEqual;
    } else {
      Kind = Caret;
    }
    break;
  case '|': // ||, |=, |
    Char = PeekChar(CurPtr, Size);
    if (Char == '|') {
      CurPtr = ConsumeChar(CurPtr, Size, Result);
      Kind = PipePipe;
    } else if (Char == '=') {
      CurPtr = ConsumeChar(CurPtr, Size, Result);
      Kind = PipeEqual;
    } else {
      Kind = Pipe;
    }
    break;
  case ':': // ::, :
    Char = PeekChar(CurPtr, Size);

    // Scope operator
    if (LangOptions.CPlusPlus && Char == ':') {
      Kind = ColonColon;
      CurPtr = ConsumeChar(CurPtr, Size, Result);
    } else if (LangOptions.Digraphs && Char == '>') {
      Kind = RSquare; // ':>' becomes ']'
      CurPtr = ConsumeChar(CurPtr, Size, Result);
    } else {
      Kind = Colon;
    }

    break;
  case ';':
    Kind = Semi;
    break;
  case '=': // ==, =
    Char = PeekChar(CurPtr, Size);
    if (Char == '=') {
      CurPtr = ConsumeChar(CurPtr, Size, Result);
      Kind = EqualEqual;
    } else {
      Kind = Equal;
    }
    break;
  case ',':
    Kind = Comma;
    break;
  case '#':
    Char = PeekChar(CurPtr, Size);
    if (Char == '#') {
      Kind = HashHash;
      CurPtr = ConsumeChar(CurPtr, Size, Result);
    } else {
      // TODO: Handle Preprocessor directive.

      // We parsed a # at the start of line, it's actually a preprocessor
      // directive. Callback to the preprocessor to handle it.
      if (/* Token at physical start of line */ 1) {
        goto HandlePPDirective;
      }

      Kind = Hash;
    }
    break;

  default:
    if (IsAscii(Char)) {
      Kind = Unknown;
      break;
    }

    goto Next;
  }

  // Create a token and update location of BufferPtr
  CreateTokenWithChars(Result, CurPtr, Kind);
  return true;

HandlePPDirective:
  // We parsed a # character and its the start of a preprocessing directive.
  CreateTokenWithChars(Result, CurPtr, Hash);
  OwnerPP->HandleDirective(Result);

  return false;
}
