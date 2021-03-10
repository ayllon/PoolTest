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

#include "ElementsKernel/Temporary.h"
#include "FilePool/FileHandler.h"
#include "FilePool/LRUFileManager.h"
#include <boost/random.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>

#include "TestFileTraits.h"

using namespace SourceXtractor;

static int randInt(int min, int max) {
  boost::random::mt19937                    s_rng(time(NULL));
  boost::random::uniform_int_distribution<> dist(min, max);
  return dist(s_rng);
}

static void testThread(FileHandler* handler) {
  int niter = randInt(10, 50);
  for (int i = 0; i < niter; ++i) {
    try {
      {
        auto write_acc = handler->getAccessor<std::fstream>(FileHandler::kWrite);
        write_acc->m_fd.clear();
        write_acc->m_fd.seekp(0);
        write_acc->m_fd << boost::this_thread::get_id() << " writing something to this file time " << i << std::endl;
      }
      {
        std::string line;
        auto        read_acc = handler->getAccessor<std::fstream>(FileHandler::kRead);
        read_acc->m_fd.clear();
        read_acc->m_fd.seekg(0);
        std::getline(read_acc->m_fd, line);
      }
    } catch (std::ios_base::failure& e) {
      char buffer[512];
      BOOST_ERROR(e.what() << ": " << strerror_r(errno, buffer, sizeof(buffer)));
    } catch (const Elements::Exception& e) {
      // If we get unlucky, we can run out of file descriptors and have all used by others threads
      // That's ok, it is part of what's supposed to happen!
      BOOST_WARN(e.what());
    }
    boost::this_thread::sleep(boost::posix_time::milliseconds(randInt(50, 200)));
  }
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE(MultithreadTest)

//-----------------------------------------------------------------------------

// Note that this is not a test per-se, but just a mechanism to exercise the file manager
// and handlers to see if they work well under concurrency
BOOST_AUTO_TEST_CASE(MultithreadTest) {
  auto                                    manager       = std::make_shared<LRUFileManager>(10);
  int                                     n_files       = randInt(2, 5);
  int                                     total_threads = 0;
  std::list<Elements::TempPath>           temp_files;
  std::list<std::shared_ptr<FileHandler>> handlers;
  boost::thread_group                     thread_group;

  BOOST_TEST_MESSAGE("Running with " << n_files << " unique files");
  for (int i = 0; i < n_files; ++i) {
    temp_files.emplace_back();
    auto path = temp_files.back().path();
    handlers.emplace_back(manager->getFileHandler(path));
    auto handler = handlers.back().get();

    int n_threads = randInt(1, 5);
    total_threads += n_threads;

    for (int j = 0; j < n_threads; ++j) {
      thread_group.create_thread([handler]() { testThread(handler); });
    }
  }
  BOOST_TEST_MESSAGE("Waiting for " << total_threads << " threads");
  thread_group.join_all();
}

//-----------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE_END()
