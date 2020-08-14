/*
 * Copyright (C) 2020 Zilliqa
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "libArchivalDB/ArchivalDB.h"
#include "libTestUtils/TestUtils.h"
#include "libUtils/Logger.h"

#define BOOST_TEST_MODULE mongodbTest
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

using namespace std;

BOOST_AUTO_TEST_SUITE(mongodbtest)

BOOST_AUTO_TEST_CASE(test_mongo) {
  const auto& txn1 = TestUtils::GenerateRandomTransaction(
      TRANSACTION_VERSION, 1, Transaction::NON_CONTRACT);
  const auto& txn1_hash = txn1.GetTranID().hex();

  const auto& txn2 = TestUtils::GenerateRandomTransaction(
      TRANSACTION_VERSION, 1, Transaction::NON_CONTRACT);

  const auto& txn2_hash = txn2.GetTranID().hex();

  ArchivalDB::GetInstance().Init();
  ArchivalDB::GetInstance().InsertTxn(txn1, 0);
  auto query_ret = ArchivalDB::GetInstance().QueryTxnHash(txn1_hash);
  cout << query_ret.toStyledString() << endl;
  ArchivalDB::GetInstance().UpdateTxn(txn1_hash, 1, true);
  query_ret = ArchivalDB::GetInstance().QueryTxnHash(txn1_hash);
  cout << query_ret.toStyledString() << endl;

  // try and insert same txn
  auto outp = ArchivalDB::GetInstance().InsertTxn(txn1, 0);
  BOOST_CHECK_EQUAL(outp, false);

  // try and query non-existent txn
  query_ret = ArchivalDB::GetInstance().QueryTxnHash("abcd");
  // a null JSON is returned
  cout << query_ret.toStyledString() << endl;

  // try and update a non-existent txn
  outp = ArchivalDB::GetInstance().UpdateTxn(txn2_hash, 0, true);
  cout << "Output: " << outp;

  query_ret = ArchivalDB::GetInstance().QueryTxnHash(txn2_hash);
  cout << query_ret << endl;
}
BOOST_AUTO_TEST_SUITE_END()
