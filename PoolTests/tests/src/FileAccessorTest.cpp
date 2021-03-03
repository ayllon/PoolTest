//
// Created by aalvarez on 03/03/2021.
//

#include "PoolTests/FileAccessor.h"
#include <boost/test/unit_test.hpp>

using namespace SourceXtractor;

// This struct can not be copied or assigned, only moved
// The code can only compile if it works properly with movable-only types
// Normally you would not expect file descriptors to be copyable
struct NonCopyableFd {
  int m_fd;

  NonCopyableFd(int fd) : m_fd(fd) {}
  NonCopyableFd(const NonCopyableFd&) = delete;
  NonCopyableFd& operator=(const NonCopyableFd&) = delete;

  NonCopyableFd(NonCopyableFd&& other) {
    m_fd       = other.m_fd;
    other.m_fd = -1;
  }
};

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(FileAccessorTest)

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(OnlyReadTest) {
  boost::shared_mutex mutex;
  int                 release_flags = 0;
  auto                callback      = [&release_flags](NonCopyableFd&& fd) { release_flags |= fd.m_fd; };

  {
    // We can have multiple readers
    FileReadAccessor<NonCopyableFd> a1(1, callback, boost::shared_lock<boost::shared_mutex>(mutex));
    FileReadAccessor<NonCopyableFd> a2(2, callback, boost::shared_lock<boost::shared_mutex>(mutex));
    FileReadAccessor<NonCopyableFd> a3(4, callback, boost::shared_lock<boost::shared_mutex>(mutex));

    BOOST_CHECK_EQUAL(a1.m_fd.m_fd, 1);
    BOOST_CHECK_EQUAL(a2.m_fd.m_fd, 2);
    BOOST_CHECK_EQUAL(a3.m_fd.m_fd, 4);
    BOOST_CHECK(a1.isReadOnly());
    BOOST_CHECK_EQUAL(release_flags, 0);
  }

  BOOST_CHECK_EQUAL(release_flags, 7);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE(WriteTest) {
  boost::shared_mutex mutex;
  int                 release_flags = 0;
  auto                callback      = [&release_flags](NonCopyableFd&& fd) { release_flags |= fd.m_fd; };

  {
    // Only one writer
    FileWriteAccessor<NonCopyableFd> a1(1, callback, boost::unique_lock<boost::shared_mutex>(mutex));

    BOOST_CHECK_EQUAL(a1.m_fd.m_fd, 1);
    BOOST_CHECK(!a1.isReadOnly());
    BOOST_CHECK_EQUAL(release_flags, 0);
    // The mutex must be acquired
    BOOST_CHECK(!mutex.try_lock());
  }

  BOOST_CHECK(mutex.try_lock());
  BOOST_CHECK_EQUAL(release_flags, 1);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()

//-----------------------------------------------------------------------------
