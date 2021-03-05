//
// Created by aalvarez on 04/03/2021.
//

#ifndef POOLTESTS_FILEMANAGER_H
#define POOLTESTS_FILEMANAGER_H

#include <boost/filesystem/path.hpp>
#include <list>

namespace SourceXtractor {

/**
 * This trait has to be implemented for all supported file descriptor types.
 * @tparam TFD
 *  File descriptor type
 */
template <typename TFD>
struct OpenCloseTrait {
  static TFD open(const boost::filesystem::path&, bool /*write*/) {
    static_assert(!std::is_same<TFD, TFD>::value, "Specialization of OpenCloseTrait required");
  }
  static void close(TFD& /*fd*/) {
    static_assert(!std::is_same<TFD, TFD>::value, "Specialization of OpenCloseTrait required");
  }
};

/**
 * Provide an open/close interface to FileHandler. Concrete policies must inherit
 * this interface and implement the notify* methods.
 */
class FileManager {
public:
  struct FileMetadata;
  using FileId = FileMetadata*;

  template <typename TFD>
  std::pair<FileId, TFD> open(const boost::filesystem::path& path, bool write, std::function<bool(FileId)> request_close);

  template <typename TFD>
  void close(FileId id, TFD& fd);

  virtual void notifyUsed(FileId id);

protected:
  using Timestamp           = std::chrono::steady_clock::time_point;
  static constexpr auto Now = std::chrono::steady_clock::now;

  std::mutex                                                           m_mutex;
  std::list<std::unique_ptr<FileMetadata>>                             m_files;
  std::map<FileId, std::list<std::unique_ptr<FileMetadata>>::iterator> m_files_iter;

  virtual void notifyIntentToOpen(bool write) = 0;
  virtual void notifyOpenedFile(FileId)       = 0;
  virtual void notifyClosedFile(FileId)       = 0;
};

}  // end of namespace SourceXtractor

#include "_impl/FileManager.icpp"

#endif  // POOLTESTS_FILEMANAGER_H
