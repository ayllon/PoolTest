//
// Created by aalvarez on 05/03/2021.
//

#ifndef POOLTESTS_LRUFILEMANAGER_H
#define POOLTESTS_LRUFILEMANAGER_H

#include "FileManager.h"

namespace SourceXtractor {

/**
 * Least Recently Used strategy for the FileManager
 */
class LRUFileManager final : public FileManager {
public:
  /**
   * Constructor
   * @param limit
   *    Limit on the number of open files. If 0, it will query the system to obtain the configured limit.
   */
  LRUFileManager(unsigned limit = 0);
  virtual ~LRUFileManager();

  void notifyUsed(FileId id) override;

  unsigned limit() const;
  unsigned used() const;
  unsigned available() const;

protected:
  void notifyIntentToOpen(bool write) override;
  void notifyOpenedFile(FileId id) override;
  void notifyClosedFile(FileId id) override;

private:
  unsigned                                      m_limit;
  std::list<FileId>                             m_sorted_ids;
  std::map<FileId, std::list<FileId>::iterator> m_current_pos;
};

}  // end of namespace SourceXtractor

#endif  // POOLTESTS_LRUFILEMANAGER_H
