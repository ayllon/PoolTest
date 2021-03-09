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
#include "ElementsKernel/Temporary.h"
#include <boost/test/unit_test.hpp>

#include "TestFileTraits.h"

using namespace SourceXtractor;

/**
 * Dummy policy, since we are only interested on the methods implemented by the parent class
 */
struct FileManagerFixture : public FileManager {
public:
  FileManagerFixture() {}

protected:
  void notifyIntentToOpen(bool) final {}
  void notifyOpenedFile(FileId) final {}
  void notifyClosedFile(FileId) final {}
};

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(FileManagerTest)

//-----------------------------------------------------------------------------

BOOST_FIXTURE_TEST_CASE(DifferentFilesTest, FileManagerFixture) {
  Elements::TempPath temp1, temp2;

  auto handler1 = getFileHandler<std::fstream>(temp1.path());
  auto handler2 = getFileHandler<std::fstream>(temp2.path());

  BOOST_CHECK(handler1 != nullptr & handler2 != nullptr);
  BOOST_CHECK_NE(handler1, handler2);
}

//-----------------------------------------------------------------------------

BOOST_FIXTURE_TEST_CASE(SameHandlerTest, FileManagerFixture) {
  Elements::TempPath temp;

  auto handler1 = getFileHandler<std::fstream>(temp.path());
  auto handler2 = getFileHandler<std::fstream>(temp.path());

  BOOST_CHECK(handler1);
  BOOST_CHECK_EQUAL(handler1, handler2);
}

//-----------------------------------------------------------------------------

BOOST_FIXTURE_TEST_CASE(SamePathDifferentTypeTest, FileManagerFixture) {
  Elements::TempPath temp;
  auto               handler1 = getFileHandler<std::fstream>(temp.path());
  BOOST_CHECK_THROW(getFileHandler<int>(temp.path()), Elements::Exception);
}

//-----------------------------------------------------------------------------

BOOST_FIXTURE_TEST_CASE(RelativeSameHandlerTest, FileManagerFixture) {
  Elements::TempPath temp;
  auto               name   = temp.path().filename();
  auto               parent = temp.path().parent_path();

  // Build something like /tmp/../tmp/blah
  auto alternative = parent / ".." / parent.filename() / name;
  BOOST_REQUIRE_NE(temp.path(), alternative);
  BOOST_TEST_MESSAGE(alternative.native());

  auto handler1 = getFileHandler<std::fstream>(temp.path());
  auto handler2 = getFileHandler<std::fstream>(alternative);

  BOOST_CHECK(handler1);
  BOOST_CHECK_EQUAL(handler1, handler2);
}

//----------------------------------------------------------------------------

BOOST_FIXTURE_TEST_CASE(SymlinkSameHandlerTest, FileManagerFixture) {
  Elements::TempFile temp;
  Elements::TempPath symlink;

  BOOST_TEST_MESSAGE(temp.path() << " -> " << symlink.path());
  create_symlink(temp.path(), symlink.path());

  auto handler1 = getFileHandler<std::fstream>(temp.path());
  auto handler2 = getFileHandler<std::fstream>(symlink.path());

  BOOST_CHECK(handler1);
  BOOST_CHECK_EQUAL(handler1, handler2);
}

//----------------------------------------------------------------------------

BOOST_FIXTURE_TEST_CASE(NewHandlerTest, FileManagerFixture) {
  Elements::TempFile temp;
  {
    auto handler1 = getFileHandler<std::fstream>(temp.path());
    BOOST_CHECK(handler1);
  }
  // Note that we use a different descriptor type, which should be fine if handler1 is gone
  // SamePathDifferentTypeTest checks this should not work if handler1 were alive
  auto handler2 = getFileHandler<int>(temp.path());
  BOOST_CHECK(handler2);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()

//-----------------------------------------------------------------------------
