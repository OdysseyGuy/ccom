#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include "Header.h"
#include "Lexer.h"
#include "PPRecord.h"
#include "Pragma.h"
#include "Token.h"
#include "TokenLexer.h"

#include <memory>
#include <optional>
#include <vector>

class FileEntry;
class IdentifierInfoTable;

/* ========================================================
 *  Preprocessor
 * ========================================================
 */

class Preprocessor {
  LanguageOptions &LangOptions;

  HeaderSearch *HS;

  /** Value of __COUNTER__. */
  unsigned CounterValue = 0;

  /** True if macro expansion is disabled. */
  bool DisableMacroExpansion : 1;

  /**
   * Keeps information of all identifiers in the program, including language
   * keywords.
   */
  IdentifierInfoTable *Identifiers;

  /** The files that have been included. */
  std::vector<const FileEntry *> IncludedFiles;

  std::unique_ptr<Lexer> CurLexer;
  std::unique_ptr<TokenLexer> CurTokenLexer;

  /** Current type of lexer we are working with. */
  enum CurLexerKind {
    CLK_Lexer,
    CLK_TokenLexer,
  } CurLexerKind = CLK_Lexer;

  /** Linked-list of Macro Infos. */
  struct MacroInfoList {
    MacroInfo MInfo;
    MacroInfoList *Next;
  };

  MacroInfoList *MacroInfoListHead;

  struct IncludeStackInfo {
    enum CurLexerKind CurLexerKind;
    std::unique_ptr<Lexer> TheLexer;
    const DirectoryLookup *TheDirLookup;
  };

  std::vector<IncludeStackInfo> IncludeMacroStack;

public:
  Preprocessor(LanguageOptions &Options);
  ~Preprocessor();

  void Init();

  bool EnterSourceFile(FileID FID);
  void EnterMacro(MacroInfo *MI);

private:
  /** Lex the next token for this preprocessor. */
  void AdvanceToken(Token &Result);

  /** Same as advance token by prevent macro expansion for identifiers. */
  void LexUnexpanedToken(Token &Result) {
    bool Backup = DisableMacroExpansion;
    DisableMacroExpansion = false;

    AdvanceToken(Result);

    DisableMacroExpansion = Backup;
  }

  /** Lex a token, forming a HeaderName token if possible. */
  bool LexHeaderName(Token &Result);

  const char *GetCurLexerEndPos();

  /**
   * Evaluate an integer constant expression that may occur after a #if or
   * #elif directive.
   *
   * If the expression is equivalent to "!defined(X)" return X in IfNDefMacro
   */
  bool EvaluateDirectiveExpression(IdentifierInfo *&IfNDefMacro);

public:
  /** Given a "foo" or <foo> reference, lookup the indicated file. */
  std::optional<FileEntry &> LookupFile(std::string &Filename, bool bIsAngled);

public:
  /**
   * Callback when the lexer lexes an identifier. This callback looksup the
   * identifier in the map and/or potentially macro expands it or turns it into
   * a named token (like 'for').
   */
  void HandleIdentifier(Token &Identifier);

  /*=============== Directive Handling Methods =======================*/
  /**
   * Callback when the lexer sees a # token at the start of a line.
   *
   * Consumes the directive, modifies lexer/preprocessor state, advances the
   * lexer so that next token read is the correct one.
   */
  void HandleDirective(Token &Result);

  /**
   * Callback when lexer hits the end of current file.
   */
  bool HandleEndOfFile(Token &Result);

  /** Handle GNU line marker directive. */
  // void HandleDigitDirective(Token &Result);

  /** Ensure that the next token is Eod token. */
  void CheckEndOfDirective();

  /*=============== Conditional Directives ============================*/
  void HandleIfDirective(Token &Result);
  void HandleIfdefDirective(Token &Result, bool bIsIfndef);
  void HandleElseDirective(Token &Result);
  void HandleEndifDirective(Token &Result);

  /*=============== Macro Replacement Directives =======================*/
  void HandleDefineDirective();
  void HandleUndefDirective();

  /*====================== Error Directives ============================*/
  void HandleErrorDirective(Token &Result);

  /*====================== Line Directives ============================*/
  void HandleLineDirective();

  /*====================== Pragma Directives ============================*/
  void HandlePragmaDirective(PragmaKind Kind);

  void HandleIncludeDirective(Token &Result, const DirectoryLookup *LookupFrom);
};

#endif
