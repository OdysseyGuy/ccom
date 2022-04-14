#ifndef LEXER_H
#define LEXER_H

#include "Options.h"
#include "PreprocessorLexer.h"
#include "Token.h"

/* ========================================================
 *  Lexer
 * ========================================================
 */

class Lexer
    : public PreprocessorLexer
    , private NonCopyable<Lexer> {
  friend class Preprocessor;

  const char *BufferStart;
  const char *BufferEnd;

  LanguageOptions LangOptions;

  /** Location of the start of the file. */
  SourceLocation FileLocation;

  /** Current pointer into buffer. Points to the next character to be read. */
  const char *BufferPtr;

  bool IsAtPhysicalStartOfLine;

public:
  Lexer(FileID FID, Preprocessor &InPP, std::string &InputFile);

private:
  void InitLexer(const char *InBufferStart, const char *InBufferPtr,
                 const char *InBufferEnd);

  /** Return the next token in the file. */
  bool AdvanceToken(Token &Result);
  bool AdvanceTokenInternal(Token &Result);

public:
  /** Source code buffer. */
  std::string GetBuffer() const {
    return std::string(BufferStart, BufferEnd - BufferStart);
  }

  const char *GetBufferLocation() const { return BufferPtr; }
  unsigned GetBufferOffset() const { return BufferPtr - BufferStart; }

  /** Return a source location for the specified offset in the current file. */
  SourceLocation GetSourceLocation(const char *Loc, unsigned TokLen = 1) const;

private:
  /** Creates a token */
  void CreateTokenWithChars(Token &Result, const char *TokenEndPtr,
                            TokenKind Kind) {
    unsigned TokenLength = TokenEndPtr - BufferPtr;
    Result.SetLength(TokenLength);
    Result.SetLocation(GetSourceLocation(BufferPtr, TokenLength));
    Result.SetKind(Kind);
    BufferPtr = TokenEndPtr;
  }

  /*==================== Lexer character reading ====================*/
  /**
   * Reads character after Ptr and returns its size.
   */
  inline char PeekChar(const char *Ptr, unsigned &Size) {
    // if simple character return immediately
    if (!IsSpecialCharacter(Ptr[0])) {
      Size = 1;
      return *Ptr;
    }
  }

  const char *ConsumeChar(const char *Ptr, unsigned Size, Token &Tok) {
    if (Size == 1) {
      return Ptr + Size;
    }
  }

  /**
   * Reads a character and advances (increments) the character pointer.
   */
  inline char PeekAndConsumeChar(const char *Ptr, Token &Tok) {
    if (!IsSpecialCharacter(Ptr[0])) {
      return *Ptr++;
    }
  }

  /** Is some kind of special character */
  inline bool IsSpecialCharacter(unsigned char C) {
    return ((C != '?') && (C != '\\'));
  }

  /**
   * Slower alternative for inlined PeekChar. Works on trigraphs and escape
   * sequences.
   */
  void PeekCharSlow(const char *Ptr, unsigned &Size);

  bool IsHexLiteral(const char *Start, const LanguageOptions &LangOptions);

  /*==================== Lexer Methods ================================*/

  // The lexer identifies every letter sequence as identifiers. In the next
  // stage it identifies the keywords.
  bool LexIdentifierContinue(Token &Result, const char *CurPtr);

  bool LexNumericalConstant(Token &Result, const char *CurPtr);

  bool LexStringLiteral(Token &Result, const char *CurPtr,
                        TokenKind StringLiteralKind);
  bool LexRawStringLiteral(Token &Result, const char *CurPtr,
                           TokenKind StringLiteralKind);

  bool LexAngledStringLiteral(Token &Result, const char *CurPtr);

  bool LexCharacterConstant(Token &Result, const char *CurPtr,
                            TokenKind CharConstantKind);

  bool LexEndOfFile(Token &Result, const char *CurPtr);
};

#endif
