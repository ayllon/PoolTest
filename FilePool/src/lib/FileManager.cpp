/*
 * Copyright (C) 2012-2021 Euclid Science Ground Segment
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 3.0 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "FilePool/FileManager.h"
#include "FilePool/FileHandler.h"
#include <boost/filesystem/operations.hpp>

namespace SourceXtractor {

FileManager::FileManager() {}

FileManager::~FileManager() {}

void FileManager::notifyUsed(FileId id) {
  // In principle a FileId should only be hold by a single thread, so no need to lock here
  id->m_last_used = Now();
  ++id->m_used_count;
}

void FileManager::closeAll() {
  m_handlers.clear();
}

std::shared_ptr<FileHandler> FileManager::getFileHandler(const boost::filesystem::path& path) {
  auto canonical = weakly_canonical(path);

  std::lock_guard<std::mutex>  lock(m_mutex);
  auto                         i = m_handlers.find(canonical);
  std::shared_ptr<FileHandler> handler_ptr;

  if (i != m_handlers.end()) {
    handler_ptr = i->second.lock();
  }
  // Either didn't exist or it is gone
  if (!handler_ptr) {
    handler_ptr           = std::shared_ptr<FileHandler>(new FileHandler(canonical, this), [this, canonical](FileHandler* obj) {
      {
        std::lock_guard<std::mutex> manager_lock(m_mutex);
        m_handlers.erase(canonical);
      }
      delete obj;
    });
    m_handlers[canonical] = handler_ptr;
  }
  return handler_ptr;
}

bool FileManager::hasHandler(const boost::filesystem::path& path) const {
  std::lock_guard<std::mutex> this_lock(m_mutex);
  auto                        canonical = weakly_canonical(path);
  auto                        iter      = m_handlers.find(canonical);
  return iter != m_handlers.end();
}

}  // end of namespace SourceXtractor
