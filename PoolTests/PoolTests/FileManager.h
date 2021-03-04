//
// Created by aalvarez on 04/03/2021.
//

#ifndef POOLTESTS_FILEMANAGER_H
#define POOLTESTS_FILEMANAGER_H

#include <boost/filesystem/path.hpp>

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
  template <typename TFD>
  std::pair<intptr_t, TFD> open(const boost::filesystem::path& path, bool write, std::function<bool(intptr_t)> request_close);

  template <typename TFD>
  void close(intptr_t id, TFD& fd);

protected:
  struct FileMetadata {
    boost::filesystem::path   m_path;
    bool                      m_write;
    std::function<bool(void)> m_request_close;
  };

  virtual void notifyIntentToOpen()                       = 0;
  virtual void notifyOpenedFile(intptr_t, FileMetadata&&) = 0;
  virtual void notifyClosedFile(intptr_t)                 = 0;
};

}  // end of namespace SourceXtractor

#include "_impl/FileManager.icpp"

#endif  // POOLTESTS_FILEMANAGER_H
