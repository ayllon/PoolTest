//
// Created by aalvarez on 05/03/2021.
//

#include "PoolTests/LRUFileManager.h"
#include "ElementsKernel/Temporary.h"
#include <boost/test/unit_test.hpp>

#include "TestFileTraits.h"

using namespace SourceXtractor;

struct LRUFixture {
  static constexpr int            NFILES = 5;
  std::vector<Elements::TempPath> paths;

  LRUFixture() : paths(NFILES) {
    for (auto& path : paths) {
      std::ofstream stream(path.path().native());
      stream << "THIS IS FILE " << path.path().native();
    }
  }
};

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(LRUFileManagerTest)

//-----------------------------------------------------------------------------

BOOST_FIXTURE_TEST_CASE(TestLRU, LRUFixture) {
  constexpr int LIMIT = 3;

  LRUFileManager                     manager(LIMIT);
  std::map<FileManager::FileId, int> descriptors;
  std::vector<FileManager::FileId>   order_closed;

  auto close_callback = [&](FileManager::FileId id) mutable {
    order_closed.push_back(id);
    auto iter = descriptors.find(id);
    manager.close(iter->first, iter->second);
    descriptors.erase(iter);
    return true;
  };

  // Open all files
  std::vector<FileManager::FileId> order_opened;
  for (auto& path : paths) {
    auto pair = manager.open<int>(paths[0].path(), false, close_callback);
    descriptors.emplace(pair);
    order_opened.push_back(pair.first);
  }

  // There are more files than the maximum, so the first opened should have been closed
  BOOST_REQUIRE_EQUAL(order_closed.size(), NFILES - LIMIT);
  for (int i = 0; i < NFILES - LIMIT; ++i) {
    BOOST_CHECK_EQUAL(order_opened[i], order_closed[i]);
  }
}

//-----------------------------------------------------------------------------

BOOST_FIXTURE_TEST_CASE(TestLRUMultiple, LRUFixture) {
  constexpr int LIMIT = 3;

  LRUFileManager                     manager(LIMIT);
  std::map<FileManager::FileId, int> descriptors;
  std::vector<FileManager::FileId>   order_closed;

  auto close_callback = [&](FileManager::FileId id) mutable {
    order_closed.push_back(id);
    auto iter = descriptors.find(id);
    manager.close(iter->first, iter->second);
    descriptors.erase(iter);
    return true;
  };

  // Open first three
  std::vector<FileManager::FileId> order_opened;
  for (auto i = paths.begin(); i != paths.end() && order_opened.size() < 3; ++i) {
    auto pair = manager.open<int>(i->path(), false, close_callback);
    descriptors.emplace(pair);
    order_opened.push_back(pair.first);
  }
  BOOST_CHECK_EQUAL(order_closed.size(), 0);
  BOOST_CHECK_EQUAL(manager.limit(), 3);
  BOOST_CHECK_EQUAL(manager.used(), 3);
  BOOST_CHECK_EQUAL(manager.available(), 0);

  // Re-use two and three
  manager.notifyUsed(order_opened[0]);
  manager.notifyUsed(order_opened[1]);

  // Open two more
  for (auto i = paths.begin(); i != paths.end() && order_opened.size() < 5; ++i) {
    auto pair = manager.open<int>(i->path(), false, close_callback);
    descriptors.emplace(pair);
    order_opened.push_back(pair.first);
  }

  BOOST_CHECK_EQUAL(order_closed.size(), 2);
  BOOST_CHECK_EQUAL(manager.used(), 3);
  BOOST_CHECK_EQUAL(manager.available(), 0);

  // Since file 0 and 1 were re-used, the third opened should be gone first
  BOOST_CHECK_EQUAL(order_closed[0], order_opened[2]);
  BOOST_CHECK_EQUAL(order_closed[1], order_opened[0]);
}

//-----------------------------------------------------------------------------

BOOST_FIXTURE_TEST_CASE(TestLruMixed, LRUFixture) {
  constexpr int LIMIT = 3;

  LRUFileManager                     manager(LIMIT);
  std::map<FileManager::FileId, int> descriptors;

  auto close_callback = [&](FileManager::FileId id) mutable {
    auto iter = descriptors.find(id);
    manager.close(iter->first, iter->second);
    return true;
  };

  auto fd1 = manager.open<int>(paths[0].path(), false, close_callback);
  auto fd2 = manager.open<CfitsioLike*>(paths[1].path(), false, close_callback);
  auto fd3 = manager.open<std::fstream>(paths[2].path(), false, close_callback);

  BOOST_CHECK_EQUAL(manager.limit(), 3);
  BOOST_CHECK_EQUAL(manager.used(), 3);
  BOOST_CHECK_EQUAL(manager.available(), 0);
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()

//-----------------------------------------------------------------------------
