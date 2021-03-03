//
// Created by aalvarez on 03/03/2021.
//

#ifndef POOLTESTS_FILEACCESSOR_H
#define POOLTESTS_FILEACCESSOR_H

#include <boost/thread/shared_mutex.hpp>

namespace SourceXtractor {

/**
 * Wraps a file descriptor, so when the instance is destroyed, the callback is
 * called with the wrapped descriptor moved-in
 * @tparam TFD
 *  File descriptor type
 * @details
 *  This is a base class that hides away if the accessor is read-only or write-only,
 *  as the locking mechanisms in each case should not bother the calling code
 */
template <typename TFD>
class FileAccessor {
public:
  using ReleaseDescriptorCallback = std::function<void(TFD&&)>;

  /// The wrapped file descriptor
  TFD m_fd;

  /// Destructor
  virtual ~FileAccessor() = default;

  /// @return true if the wrapped file descriptor is read-only
  virtual bool isReadOnly() const = 0;

protected:
  FileAccessor(TFD&& fd, ReleaseDescriptorCallback release_callback);

  ReleaseDescriptorCallback m_release_callback;
};

/**
 * Wraps a file descriptor together with a shared lock, so multiple read accessors pointing
 * to the same **physical file** can exist at the same time.
 * @tparam TFD
 *  File descriptor type
 * @note
 *  The file descriptor is still unique, since normally file descriptors can not be shared
 *  between threads (shared buffers, offsets, etc.)
 *  What is shared is the *file* itself.
 */
template <typename TFD>
class FileReadAccessor : public FileAccessor<TFD> {
public:
  using ReleaseDescriptorCallback = typename FileAccessor<TFD>::ReleaseDescriptorCallback;
  using SharedLock                = boost::shared_lock<boost::shared_mutex>;

  /**
   * Constructor
   * @param fd
   *    File descriptor to own
   * @param release_callback
   *    Callback to be called at destruction
   * @param lock
   *    Shared lock
   */
  FileReadAccessor(TFD&& fd, ReleaseDescriptorCallback release_callback, SharedLock lock);

  /// It can not be copied
  FileReadAccessor(const FileReadAccessor&) = delete;

  /// But it can be moved
  FileReadAccessor(FileReadAccessor&&) = default;

  /// Destructor
  virtual ~FileReadAccessor();

  /// @return Always true
  bool isReadOnly() const final;

private:
  SharedLock m_shared_lock;
};

/**
 * Wraps a file descriptor together with an exclusive lock, so there can only be one
 * accessor (no simultaneous reads!)
 * @tparam TFD
 *  File descriptor type
 */
template <typename TFD>
class FileWriteAccessor : public FileAccessor<TFD> {
public:
  using ReleaseDescriptorCallback = typename FileAccessor<TFD>::ReleaseDescriptorCallback;
  using UniqueLock                = boost::unique_lock<boost::shared_mutex>;

  /**
   * Constructor
   * @param fd
   *    File descriptor to own
   * @param release_callback
   *    Callback to be called at destruction
   * @param lock
   *    Unique lock to the underlying file
   */
  FileWriteAccessor(TFD&& fd, ReleaseDescriptorCallback release_callback, UniqueLock lock);

  /// Destructor
  virtual ~FileWriteAccessor();

  /// @return Always true
  bool isReadOnly() const final;

private:
  UniqueLock m_unique_lock;
};

}  // end of namespace SourceXtractor

#include "_impl/FileAccessor.icpp"

#endif  // POOLTESTS_FILEACCESSOR_H