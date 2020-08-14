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

#ifndef ZILLIQA_SRC_LIBARCHIVALDB_ARCHIVALDB_H_
#define ZILLIQA_SRC_LIBARCHIVALDB_ARCHIVALDB_H_

#include "libData/AccountData/Account.h"
#include "libData/AccountData/Transaction.h"

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/pool.hpp>

class ArchivalDB : public Singleton<ArchivalDB> {
  std::unique_ptr<mongocxx::pool> m_pool;
  std::unique_ptr<mongocxx::instance> m_inst;
  bool m_initialized;
  const std::string m_dbName;
  const std::string m_txnCollectionName;

 public:
  ArchivalDB(std::string txnCollectionName = "TransactionStatus")
      : m_initialized(false),
        m_dbName(DB_NAME),
        m_txnCollectionName(txnCollectionName) {}

  void Init(unsigned int port = 27017, bool configure = true);
  bool InsertJson(const Json::Value& _json, const std::string& collectionName);
  bool InsertTxn(const Transaction& txn, const unsigned int status,
                 const bool success = false);
  bool UpdateTxn(const std::string& txnhash, const unsigned int status,
                 const bool success);
  Json::Value QueryTxnHash(const std::string& txnhash);

  static ArchivalDB& GetInstance() {
    static ArchivalDB aDB;
    return aDB;
  }
};

#endif  // ZILLIQA_SRC_LIBARCHIVALDB_ARCHIVALDB_H_