#ifndef HEADER_H
#define HEADER_H

#include <vector>

class DirectoryEntry;

class DirectoryLookup {
  DirectoryEntry &Dir;

public:
  DirectoryLookup(DirectoryEntry &InDir) : Dir(InDir) {}
};

/**
 * Information required to find the file referenced by include directive.
 */
class HeaderSearch {
  /**
   * Header search for #include "x" begins with the directory of the including
   * file, then each directory in the search directory. Search for <x> search
   * for current dir first, then each directory in the SearchDirs, starting at
   * AngledDirIndex.
   */
  std::vector<DirectoryLookup> SearchDirs;
  unsigned AngledDirIndex = 0;

public:
  void SetSearchPaths(std::vector<DirectoryLookup> &Dirs,
                      unsigned AngledDirIndex);

  /** Add an additional search path. */
  void AddSearchPath(const DirectoryLookup &Dir, bool IsAngled);
};

#endif
