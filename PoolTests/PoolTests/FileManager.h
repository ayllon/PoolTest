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
  /// Opaque structure, its members can only be used by FileManager and derived classes
  struct FileMetadata;

  /// Opaque FileId, its concrete type should only be assumed to be copyable and hashable
  using FileId = FileMetadata*;

  /// Destructor
  virtual ~FileManager();

  /**
   * Open a file
   * @tparam TFD
   *    File descriptor type.
   * @param path
   *    File path
   * @param write
   *    True if the file is to be opened in write mode
   * @param request_close
   *    The manager will call this function when it needs to close the file descriptor,
   *    so whoever called open can put everything in order. The callback can return "false" if the given
   *    FileId can not be closed (i.e. it is still in use). The callback is responsible for calling close.
   * @return
   *    A pair FileId, FileDescriptor
   * @note
   *    An specialization of OpenCloseTrait must exists for TFD.
   */
  template <typename TFD>
  std::pair<FileId, TFD> open(const boost::filesystem::path& path, bool write, std::function<bool(FileId)> request_close);

  /**
   * Close a file
   * @param id
   *    The id returned by open
   */
  template <typename TFD>
  void close(FileId id, TFD& fd);

  /**
   * Notify that the given file has been/is going to be used. This will update
   * the book-keeping data used to decide what to close when.
   */
  virtual void notifyUsed(FileId id);

protected:
  using Timestamp           = std::chrono::steady_clock::time_point;
  static constexpr auto Now = std::chrono::steady_clock::now;

  std::mutex m_mutex;
  // The standard guarantees that iterators to a list remain valid even after a sort,
  // so the list can be safely sorted by the strategy implementations
  std::map<FileId, std::unique_ptr<FileMetadata>> m_files;

  virtual void notifyIntentToOpen(bool write) = 0;
  virtual void notifyOpenedFile(FileId)       = 0;
  virtual void notifyClosedFile(FileId)       = 0;
};

}  // end of namespace SourceXtractor

#include "_impl/FileManager.icpp"

#endif  // POOLTESTS_FILEMANAGER_H
