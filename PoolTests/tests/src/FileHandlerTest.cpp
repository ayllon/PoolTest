//
// Created by aalvarez on 03/03/2021.
//

#include "PoolTests/FileHandler.h"
#include "ElementsKernel/Temporary.h"
#include <boost/mpl/list.hpp>
#include <boost/test/unit_test.hpp>
#include <fstream>

#include "TestFileTraits.h"

using namespace SourceXtractor;

/**
 * Mock FileManager
 */
struct FileManagerMock : public FileManager {
protected:
  void notifyIntentToOpen() override {
    BOOST_CHECK_EQUAL(n_notified, n_opened);
    ++n_notified;
  }

  void notifyOpenedFile(intptr_t intptr, FileMetadata&& metadata) override {
    BOOST_CHECK_EQUAL(n_notified, n_opened + 1);
    ++n_opened;
    // TODO: Metadata
  }
  void notifyClosedFile(intptr_t intptr) override {
    BOOST_CHECK_LE(n_closed, n_opened);
    ++n_closed;
  }

public:
  FileManagerMock() : n_opened(0), n_closed(0), n_notified(0) {}

  unsigned n_opened, n_closed, n_notified;
};

/**
 * Fixture
 */
struct FileHandlerFixture {
  std::shared_ptr<FileManagerMock> m_file_manager;
  Elements::TempPath               m_path;

  FileHandlerFixture() : m_file_manager(std::make_shared<FileManagerMock>()) {}
};

/**
 * Run the tests for this set of types
 */
typedef boost::mpl::list<int, CfitsioLike*, std::fstream> file_descriptor_types;

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(FileHandlerTest)

//-----------------------------------------------------------------------------

BOOST_FIXTURE_TEST_CASE_TEMPLATE(OpenOneWriteRead, T, file_descriptor_types, FileHandlerFixture) {
  FileHandler<T> handler(m_path.path(), m_file_manager);
  std::string    write_buffer("this is a string to be written to the nice file");

  {
    auto write_accessor = handler.getAccessor(true);
    BOOST_REQUIRE(write_accessor);
    BOOST_CHECK(!handler.isReadOnly());
    BOOST_CHECK(!write_accessor->isReadOnly());
    OpenCloseTrait<T>::write(write_accessor->m_fd, write_buffer);
  }

  auto read_accessor = handler.getAccessor(false);
  BOOST_REQUIRE(read_accessor);
  BOOST_CHECK(handler.isReadOnly());
  BOOST_CHECK(read_accessor->isReadOnly());
  auto content = OpenCloseTrait<T>::read(read_accessor->m_fd);

  BOOST_CHECK_EQUAL(content, write_buffer);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()

//-----------------------------------------------------------------------------
