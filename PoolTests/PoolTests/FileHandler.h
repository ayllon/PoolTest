//
// Created by aalvarez on 03/03/2021.
//

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
   * @param write
   *    If true, the file will be turned into write mode, so *only one accessor can be used*
   * @return
   *    A new file accessor
   * @throws
   *    If opening the file fails
   */
  std::unique_ptr<FileAccessorType> getAccessor(bool write = false);

  /**
   * Similar to getAccessor, but it will fail and return nullptr if the file is
   * write-locked by someone else
   */
  std::unique_ptr<FileAccessorType> tryGetAccessor(bool write = false);

  /// @return true if the handler is open in read-only mode (default)
  bool isReadOnly() const;

private:
  using SharedMutex = typename FileAccessorType::SharedMutex;
  using SharedLock  = typename FileAccessorType::SharedLock;
  using UniqueLock  = typename FileAccessorType::UniqueLock;

  std::mutex                 m_handler_mutex;
  boost::filesystem::path    m_path;
  std::weak_ptr<FileManager> m_file_manager;
  SharedMutex                m_file_mutex;
  std::map<intptr_t, TFD>    m_available_fd;
  bool                       m_is_readonly;

  /**
   * This is to be used by the FileManager to request the closing of a file descriptor
   * @param id
   *    ID of the file to close
   * @return
   *    false if it can not be closed (not in the available list)
   */
  bool close(intptr_t id);
};

}  // end of namespace SourceXtractor

#include "_impl/FileHandler.icpp"

#endif  // POOLTESTS_FILEHANDLER_H
