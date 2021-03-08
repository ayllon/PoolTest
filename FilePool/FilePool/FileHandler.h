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

#ifndef POOLTESTS_FILEHANDLER_H
#define POOLTESTS_FILEHANDLER_H

#include "FileAccessor.h"
#include "FileManager.h"
#include <boost/filesystem/path.hpp>
#include <list>

namespace SourceXtractor {

/**
 * Wraps a set of file descriptors. It should rely on a FileManager implementation
 * to do the opening/closing and policy handling of lifetimes. This is,
 * the FileManager implementation decides the policy on when to close a given file descriptor
 * if the maximum is reached.
 * However, it will play "nice" and just ask the handler to please close it. The handler *must
 * not* close a file being accessed, so it should just refuse to do so and let the FileManager
 * figure it out.
 */
template <typename TFD>
class FileHandler {
public:
  typedef FileAccessor<TFD> FileAccessorType;

  enum Mode { kRead = 0b00, kWrite = 0b01, kTryRead = 0b10, kTryWrite = 0b11 };

  /**
   * Constructor
   * @param path
   *    File path
   * @param file_manager
   *    FileManager implementation responsible for opening/closing and keeping track of
   *    number of opened files. A FileHandler could survive the manager as long as no new
   *    accessors are needed.
   */
  FileHandler(const boost::filesystem::path& path, std::weak_ptr<FileManager> file_manager);

  /// Destructor
  virtual ~FileHandler();

  /**
   * Get a new FileAccessor
   * @param mode
   *    The accessor mode. TryRead and TryWrite can be used if the caller does not want to block.
   * @return
   *    A new file accessor
   * @throws
   *    If opening the file fails
   */
  std::unique_ptr<FileAccessorType> getAccessor(Mode mode = kRead);

  /// @return true if the handler is open in read-only mode (default)
  bool isReadOnly() const;

private:
  using SharedMutex = typename FileAccessorType::SharedMutex;
  using SharedLock  = typename FileAccessorType::SharedLock;
  using UniqueLock  = typename FileAccessorType::UniqueLock;

  std::mutex                         m_handler_mutex;
  boost::filesystem::path            m_path;
  std::weak_ptr<FileManager>         m_file_manager;
  SharedMutex                        m_file_mutex;
  std::map<FileManager::FileId, TFD> m_available_fd;
  bool                               m_is_readonly;

  /**
   * This is to be used by the FileManager to request the closing of a file descriptor
   * @param id
   *    ID of the file to close
   * @return
   *    false if it can not be closed (not in the available list)
   */
  bool canBeClosed(FileManager::FileId id);

  std::unique_ptr<FileAccessorType> getWriteAccessor(bool try_lock);
  std::unique_ptr<FileAccessorType> getReadAccessor(bool try_lock);
};

}  // end of namespace SourceXtractor

#define FILEHANDLER_IMPL
#include "_impl/FileHandler.icpp"
#undef FILEHANDLER_IMPL

#endif  // POOLTESTS_FILEHANDLER_H