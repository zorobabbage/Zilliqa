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

#include "ArchivalDB.h"
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/stdx/make_unique.hpp>
#include "libServer/JSONConversion.h"
#include "libUtils/HashUtils.h"

using namespace std;
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

pair<string, string> getCreds() {
  string username, password;
  if (const char* env_p = getenv("ZIL_DB_USERNAME")) {
    username = move(env_p);
  }
  if (const char* env_p = getenv("ZIL_DB_PASSWORD")) {
    password = move(env_p);
  }

  return make_pair(username, password);
}

void ArchivalDB::Init(unsigned int port, bool configure) {
  auto instance = bsoncxx::stdx::make_unique<mongocxx::instance>();
  try {
    m_inst = move(instance);
    auto creds = getCreds();
    string uri;
    if (creds.first.empty() || creds.second.empty()) {
      uri = "mongodb://" + DB_HOST + ":" + to_string(port);
    } else {
      uri = "mongodb://" + creds.first + ":" + creds.second + "@" + DB_HOST +
            ":" + to_string(port);
    }
    mongocxx::uri URI(uri);
    m_pool = bsoncxx::stdx::make_unique<mongocxx::pool>(move(URI));
    if (configure) {
      auto c = m_pool->acquire();
      (*c)[m_dbName].drop();
      mongocxx::options::index index_options;
      index_options.unique(true);
      auto mongoDB = c->database(m_dbName);
      // ID is unique in txn and from is also an index but not unique
      mongoDB[m_txnCollectionName].create_index(make_document(kvp("ID", 1)),
                                                index_options);
      mongoDB[m_txnCollectionName].create_index(make_document(kvp("toAddr", 1)),
                                                {});
    }
    m_initialized = true;
  } catch (exception& e) {
    LOG_GENERAL(WARNING, "Failed to initialized DB " << e.what());
  }
}

bool ArchivalDB::InsertTxn(const Transaction& txn, const unsigned int status,
                           const bool success) {
  if (!m_initialized) {
    LOG_GENERAL(WARNING, "DB not initialized");
    return false;
  }
  Json::Value tx_json = JSONConversion::convertTxtoJson(txn);
  tx_json["status"] = status;
  tx_json["success"] = success;
  return InsertJson(tx_json, m_txnCollectionName);
}

bool ArchivalDB::UpdateTxn(const string& txnhash, const unsigned int status,
                           const bool success) {
  if (!m_initialized) {
    LOG_GENERAL(WARNING, "DB not initialized");
    return false;
  }
  try {
    auto mongoClient = m_pool->acquire();
    auto mongoDB = mongoClient->database(m_dbName);
    const auto& res = mongoDB[m_txnCollectionName].update_one(
        make_document(kvp("ID", txnhash)),
        make_document(
            kvp("$set", make_document(kvp("status", static_cast<int>(status)),
                                      kvp("success", success)))));

    if (res.value().matched_count() == 0) {
      LOG_GENERAL(INFO, "Could not find " << txnhash << " to update");
      return false;
    }

  } catch (exception& e) {
    LOG_GENERAL(WARNING, "Failed to update " << txnhash << " " << e.what());
    return false;
  }
  return true;
}

bool ArchivalDB::InsertJson(const Json::Value& _json,
                            const string& collectionName) {
  if (!m_initialized) {
    LOG_GENERAL(WARNING, "DB not initialized");
    return false;
  }
  try {
    auto mongoClient = m_pool->acquire();
    auto mongoDB = mongoClient->database(m_dbName);
    bsoncxx::document::value doc_val =
        bsoncxx::from_json(_json.toStyledString());
    auto res = mongoDB[collectionName].insert_one(move(doc_val));
    return true;
  } catch (exception& e) {
    LOG_GENERAL(WARNING, "Failed to Insert " << _json.toStyledString() << endl
                                             << e.what());
    return false;
  }
}

Json::Value ArchivalDB::QueryTxnHash(const std::string& txnhash) {
  Json::Value _json;
  if (!m_initialized) {
    LOG_GENERAL(WARNING, "DB not initialized");
    return false;
  }

  try {
    auto mongoClient = m_pool->acquire();
    auto mongoDB = mongoClient->database(m_dbName);
    auto cursor =
        mongoDB[m_txnCollectionName].find(make_document(kvp("ID", txnhash)));
    for (auto&& doc : cursor) {
      _json.append(bsoncxx::to_json(doc));
    }

  } catch (std::exception& e) {
    LOG_GENERAL(WARNING, "Failed to query" << txnhash << " " << e.what());
    _json["error"] = true;
  }

  return _json;
}