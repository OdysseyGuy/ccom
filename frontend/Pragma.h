#ifndef PRAGMA_H
#define PRAGMA_H

#include "SourceManager.h"

/** Describes types of pragmas #pragma, _Pragma, or __pragma. */
enum PragmaKind {
  /** #pragma */
  PK_Hash,

  /** C99 _Pragma(string-literal) */
  PK_U,

  /** Microsoft __pragma(token-string) */
  PK_UU
};

/**
 * Kind of Pragma and its location.
 */
struct Pragma {
  PragmaKind Kind;
  SourceLocation Location;
};

#endif
