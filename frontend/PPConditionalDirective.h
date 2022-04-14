#ifndef PP_CONDITIONAL_DIRECTIVE_H
#define PP_CONDITIONAL_DIRECTIVE_H

#include "SourceManager.h"

/**
 * Information about the conditional stack (#if directives) currently active.
 */
struct PPConditionalInfo {
  /** Location where the conditional started. */
  SourceLocation IfLocation;

  /** True if we've seen a #else in this block. */
  bool FoundElse;
};

#endif
