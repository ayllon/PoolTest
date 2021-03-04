//
// Created by aalvarez on 04/03/2021.
//

#ifndef POOLTESTS_TESTFILETRAITS_H
#define POOLTESTS_TESTFILETRAITS_H

#include "ElementsKernel/Exception.h"
#include <fcntl.h>
#include <string>

namespace SourceXtractor {

/**
 * An opaque non copyable used as a file handler (intended to mick cfitsio handlers)
 */
struct CfitsioLike {
  int   fd;
  char* buffer;
};

/**
 * Trait for low-level syscalls (file descriptor is an int, which is copyable, movable, etc.)
 */
template <>
struct OpenCloseTrait<int> {
  static int open(const boost::filesystem::path& path, bool write) {
    int fd = ::open(path.native().c_str(), write * (O_TRUNC | O_CREAT | O_RDWR), 0700);
    if (fd < 0) {
      throw Elements::Exception() << strerror(errno);
    }
    return fd;
  }

  static void close(int fd) {
    if (::close(fd) < 0) {
      BOOST_ERROR(strerror(errno));
    }
  }

  // This two are not part of the original trait! They are here for convenience
  static void write(int& fd, const std::string& buf) {
    if (::write(fd, buf.c_str(), buf.size()) < buf.size()) {
      BOOST_ERROR(strerror(errno));
    }
  }

  static std::string read(int& fd) {
    char buffer[1024] = {0};
    if (::read(fd, buffer, sizeof(buffer)) <= 0) {
      BOOST_ERROR(strerror(errno));
    }
    return buffer;
  }
};

/**
 * Trait for a C-like file handler, so a pointer to something
 */
template <>
struct OpenCloseTrait<CfitsioLike*> {
  static CfitsioLike* open(const boost::filesystem::path& path, bool write) {
    int fd = ::open(path.native().c_str(), write * (O_TRUNC | O_CREAT | O_RDWR), 0700);
    if (fd < 0) {
      throw Elements::Exception() << strerror(errno);
    }
    return new CfitsioLike{fd, new char[1024]};
  }

  static void close(CfitsioLike* ptr) {
    if (::close(ptr->fd) < 0) {
      BOOST_ERROR(strerror(errno));
    }
    delete ptr->buffer;
    delete ptr;
  }

  // This two are not part of the original trait! They are here for convenience
  static void write(CfitsioLike* ptr, const std::string& buf) {
    if (::write(ptr->fd, buf.c_str(), buf.size()) < buf.size()) {
      BOOST_ERROR(strerror(errno));
    }
  }

  static std::string read(CfitsioLike* ptr) {
    if (::read(ptr->fd, ptr->buffer, 1024) <= 0) {
      BOOST_ERROR(strerror(errno));
    }
    return ptr->buffer;
  }
};

/**
 * Trait for a C++ file stream, which is movable but *not* copyable
 */
template <>
struct OpenCloseTrait<std::fstream> {
  static std::fstream open(const boost::filesystem::path& path, bool write) {
    auto mode = std::ios_base::in;
    if (write)
      mode = std::ios_base::out;
    std::fstream stream(path.native(), mode);
    stream.exceptions(std::fstream::failbit | std::fstream::badbit);
    return stream;
  }

  static void close(std::fstream& stream) {
    stream.close();
  }

  // This two are not part of the original trait! They are here for convenience
  static void write(std::fstream& stream, const std::string& buf) {
    stream << buf << std::endl;
  }

  static std::string read(std::fstream& stream) {
    std::string content;
    std::getline(stream, content);
    return content;
  }
};
}  // namespace SourceXtractor

#endif  // POOLTESTS_TESTFILETRAITS_H
