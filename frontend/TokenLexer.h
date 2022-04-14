#ifndef TOKEN_LEXER_H
#define TOKEN_LEXER_H

#include "Mixins.h"
#include "PPRecord.h"
#include "SourceManager.h"
#include "Token.h"

class Preprocessor;

class TokenLexer : private NonCopyable<TokenLexer> {
  friend class Preprocessor;

  SourceLocation ExpandLocStart, ExpandLocEnd;

  /** Location of the macro definition. */
  SourceLocation MacroDefStart;
  unsigned MacroDefLength;

public:
  TokenLexer(MacroInfo *MI, Preprocessor &PP) { Init(MI); }

  /** Initialize a Token lexer to expand a macro. */
  void Init(MacroInfo *MI);

  /** Lex and return next token from this macro stream. */
  bool AdvanceToken(Token &Result);
};

#endif
