#include "Preprocessor.h"
#include "IdentifierTable.h"

Preprocessor::Preprocessor(LanguageOptions &Options)
    : LangOptions(Options) {}

void
Preprocessor::Init() {
  // Populate the identifier info table about keywords for current language.
  Identifiers->AddKeywords(LangOptions);
}

void
Preprocessor::AdvanceToken(Token &Result) {
  bool ReturnedToken;
  do {
    switch (CurLexerKind) {
    case CLK_Lexer:
      ReturnedToken = CurLexer->AdvanceToken(Result);
      break;
    case CLK_TokenLexer:
      ReturnedToken = CurTokenLexer->AdvanceToken(Result);
      break;
    };
  } while (!ReturnedToken);

  if (Result.GetKind() == TokenKind::Unknown) {
    return;
  }
}

std::optional<FileEntry &>
LookupFile(std::string &Filename, bool bIsAngled) {}

void
Preprocessor::CheckEndOfDirective() {}

bool
Preprocessor::LexHeaderName(Token &Result) {
  if (CurLexer) {
    CurLexer->LexIncludeFilename(Result);
  } else {
    AdvanceToken(Result);
  }

  std::vector<char> FilenameBuffer(128);
  if ((Result.GetKind() == Less)) {
    FilenameBuffer.emplace_back('<');

    // Consume tokens until we find a '>'
    while (Result.GetKind() == Greater) {
      AdvanceToken(Result);
    }

    Result.ResetToken();
    Result.SetKind(HeaderName);
  } else if (Result.GetKind() == StringLiteral) {
    std::string Str = 0;
    if (Str.size() >= 2 && Str.front() == '"' && Str.back() == '"') {
      Result.SetKind(HeaderName);
    }
  }
}

/* Determine the location to use as the end of the buffer for a lexer.
 *
 * If the file ends with a newline, create the EOF token on the newline itself,
 * rather than on the following line which doesn't exist.
 */
const char *
Preprocessor::GetCurLexerEndPos() {
  const char *EndPos = CurLexer->BufferEnd;
  if (EndPos != CurLexer->BufferStart &&
      (EndPos[-1] == '\n' || EndPos[-1] == '\r')) {
    EndPos--;
  }

  // Handle \n\r \r\n
  if (EndPos != CurLexer->BufferStart &&
      (EndPos[-1] == '\n' || EndPos[-1] == '\r') && EndPos[-1] != EndPos[0]) {
    EndPos--;
  }

  return EndPos;
}

/* ==========================================================================
 *  Preprocessor Expression Evaluation.
 *
 *  Parses and evaluates integer constant
 *  expressions for #if and #elif directives.
 * ==========================================================================
 */

/**
 * Represents the subexpression of a preprocessor conditional.
 */
class PPExprResult {
  IdentifierInfo *II;

public:
  uint32_t Value;

  IdentifierInfo *GetIdentifierInfo() const { return II; }
  void SetIdentifierInfo(IdentifierInfo *II) { this->II = II; }
};

/**
 * Used to keep track of whether define(X) has been seen during expression
 * parsing.
 */
struct DefinedTracker {
  enum TrackerState {
    DefinedMacro, // defined(X)
    NotDefinedMacro, // !defined(X)
    Unknown // Something else.
  } State;

  IdentifierInfo *TheMacro;
};

static bool
EvaluateDefine(PPExprResult &Result, Token &PeekToken, DefinedTracker &DT,
               Preprocessor &PP) {
  // Lex Next
  if (PeekToken.GetKind() == LParen) {
    // Skip it
    // Lex Next
  }

  IdentifierInfo *II = PeekToken.GetIdentifierInfo();

  DT.State = DefinedTracker::DefinedMacro;
  DT.TheMacro = II;
  return false;
}

static bool
EvaluateValue(PPExprResult &Result, Token &PeekToken, DefinedTracker &DT,
              Preprocessor &PP) {
  DT.State = DefinedTracker::Unknown;

  switch (PeekToken.GetKind()) {
  default:
    if (IdentifierInfo *II = PeekToken.GetIdentifierInfo()) {
      if (II->GetName() == "defined") {
        return EvaluateDefine(Result, PeekToken, DT, PP);
      }
    }
    break;
  case Eod:
  case RParen:
    return true;
  case NumericConstant:
    return false;
  case LParen:
    // Lex next
    if (EvaluateValue(Result, PeekToken, DT, PP))
      return true;

    if (PeekToken.GetKind() == RParen) {

    } else {
      // Lex next
      return false;
    }

  case Exclaim:
    // Lex Next
    if (EvaluateValue(Result, PeekToken, DT, PP))
      return true;

    if (DT.State == DefinedTracker::DefinedMacro) {
      DT.State = DefinedTracker::NotDefinedMacro;
    } else if (DT.State == DefinedTracker::NotDefinedMacro) {
      DT.State = DefinedTracker::DefinedMacro;
    }
    return false;

  case KW_true:
  case KW_false:
    // Lex Next
    return false;
  }
}

static bool
EvaluateDirectiveSubExpr(PPExprResult &Result, Token &PeekToken,
                         Preprocessor &PP) {}

bool
Preprocessor::EvaluateDirectiveExpression(IdentifierInfo *&IfNDefMacro) {
  Token Tok;
  AdvanceToken(Tok);

  PPExprResult Result;
  DefinedTracker DT;
  if (EvaluateValue(Result, Tok, DT, *this)) {
  }

  if (Tok.GetKind() == Eod) {
    // If the expression we just parsed was of type !define(macro), return the
    // macro in IfNDefMacro
    if (DT.State == DefinedTracker::NotDefinedMacro) {
      IfNDefMacro = DT.TheMacro;
    }
  }
}

void
Preprocessor::HandleIdentifier(Token &Identifier) {}

/*=============== Directive Handling Methods =======================*/

// This callback is invoked when the lexer sees a # token at the start of line.
void
Preprocessor::HandleDirective(Token &Result) {
  // We just parsed a # character at the start of the line, so we are in
  // directive mode. Tell the lexer this so any newlines we see will be
  // converted into Eod token (which terminates the directive).
  CurLexer->ParsingPreprocessorDirective = true;

  // Save the '#' token
  Token HashToken = Result;

  // Lex next identifier i.e. get the directive type
  LexUnexpanedToken(Result);

  switch (Result.GetKind()) {
  case Eod:
    return; // null directive
  default:
    IdentifierInfo *II = Result.GetIdentifierInfo();
    if (!II) // Not an identifier
      break;

    // Ask what the Preprocessor keyword is
    switch (II->GetPPKeyword()) {
    default:
      break;

    // Conditional Inclusion
    case PP_If:
      return HandleIfDirective(Result);
    case PP_Ifdef:
      return HandleIfdefDirective(Result, false);
    case PP_Ifndef:
      return HandleIfdefDirective(Result, true);
    case PP_Else:
      return HandleElseDirective(Result);
    case PP_Endif:
      return HandleEndifDirective(Result);

    // Source File Inclusion
    case PP_Include:
      return HandleIncludeDirective(Result);

    // Macro Replacement
    case PP_Define:
      return HandleDefineDirective();
    case PP_Undef:
      return HandleUndefDirective();

    // Line Control
    case PP_Line:
      return HandleLineDirective();

    // Error Directive
    case PP_Error:
      return HandleErrorDirective(Result);

    // Pragma Directive
    case PP_Pragma:
      return HandlePragmaDirective(PK_Hash);
    }
  }
}

bool
Preprocessor::HandleEndOfFile(Token &Result) {
  // If this is a #include'd file, pop it and continue lexing the #include'r
  // file
  if (!IncludeMacroStack.empty()) {
  }

  assert(CurLexer && "Got EOF but no current lexer set!");
  const char *EndPos = GetCurLexerEndPos();
  Result.ResetToken();
  CurLexer->BufferPtr = EndPos;
  CurLexer->CreateTokenWithChars(Result, EndPos, Eof);

  return true;
}

/*
 * Handle a #line directve
 *
 * Two acceptable forms
 *    # line digit-sequence
 *    # line digit-sequence "s-char-sequence"
 */
void
Preprocessor::HandleLineDirective() {
  Token DigitToken;
  AdvanceToken(DigitToken);

  unsigned LineNo;
}

// TODO: Implement GNU line directive
/*
 * GNU line directive
 *
 * Forms include
 *  # line-number (1 | 2)
 *  # line-number
 */
// void Preprocessor::HandleDigitDirective(Token &Result) {}

/*
 * The "#include" tokens have been just read, read the file to be include from
 * lexer, then include it.
 */
void
Preprocessor::HandleIncludeDirective(Token &Result,
                                     const DirectoryLookup *LookupFrom) {
  // Name of the file to be include
  Token FilenameToken;

  if (LexHeaderName(FilenameToken)) {
    return;
  }

  if (FilenameToken.GetKind() != HeaderName) {
    return;
  }
}

void
Preprocessor::HandleErrorDirective(Token &Result) {}

/*================= Preprocessor Conditional Directives ================*/
// Implements the #ifdef / #ifndef directive
void
Preprocessor::HandleIfdefDirective(Token &IfdefToken, bool bIsIfndef) {}

// Implements the #if directive.
void
Preprocessor::HandleIfDirective(Token &IfToken) {
  IdentifierInfo *IfNDefMacro = nullptr;
  const bool EvalResult = EvaluateDirectiveExpression(IfNDefMacro);

  if (!CurLexer)
    return;
}

// Implement the #endif directive.
void
Preprocessor::HandleEndifDirective(Token &EndifToken) {
  PPConditionalInfo CondInfo;
  CurLexer->PopConditionalLevel(CondInfo);

  // Info MI optimizer
}

// Implements the #else directive.
void
Preprocessor::HandleElseDirective(Token &ElseToken) {
  PPConditionalInfo CondInfo;
  CurLexer->PopConditionalLevel(CondInfo);

  // If this is an else with else before it error
  if (CondInfo.FoundElse) {
    // error

  }
}
