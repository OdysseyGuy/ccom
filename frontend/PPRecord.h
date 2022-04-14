#ifndef PP_RECORD_H
#define PP_RECORD_H

#include "SourceManager.h"

#include <string>
#include <vector>

class PPEntity;
class IdentifierInfo;

class MacroInfo {
  friend class Preprocessor;

  /** The location the macro is defined. */
  SourceLocation Location;

  /** The location of the last token in the macro. */
  SourceLocation EndLocation;

  /** List of arguments for a function-like macro. */
  IdentifierInfo **ParameterList;
  unsigned NumParameters;

  /** True if this macro is function-like, false if it is object-like. */
  bool bIsFunctionLike;

  /** True if this macro is of the form "#define X(...)" */
  bool IsC99Varargs;

  /** True if this macro requires processing before expansion. */
  bool IsBuiltinMacro;

  /** Whether this macro was used as header guard. */
  bool bUsedForHeaderGaurd;

  MacroInfo(SourceLocation DifinitionLocation);
  ~MacroInfo() = default;

public:
  SourceLocation GetDifinitionLocation() const { return Location; }

  void SetDefinitionEndLoc(SourceLocation EndLoc) { EndLocation = EndLoc; }
  SourceLocation GetDefinitionEndLoc() const { return EndLocation; }
};

class MacroArgs;

struct InclusionDirective {
  std::string FileName;
  bool bInQuotes;

public:
  std::string GetFileName() { return FileName; }
  bool WasInQuotes() const { return bInQuotes; }
};

class PPRecord {
  std::vector<PPEntity> PPEntities;
};

#endif