//
// Created by aalvarez on 05/03/2021.
//

#include "PoolTests/FileManager.h"

namespace SourceXtractor {

FileManager::~FileManager() {}

void FileManager::notifyUsed(FileId id) {
  // In principle a FileId should only be hold by a single thread, so no need to lock here
  id->m_last_used = Now();
  ++id->m_used_count;
}

}  // end of namespace SourceXtractor
