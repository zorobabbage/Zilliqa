/*
 * Copyright (C) 2019 Zilliqa
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

#ifndef ZILLIQA_SRC_LIBPERSISTENCE_CONTRACTSTORAGE2_H_
#define ZILLIQA_SRC_LIBPERSISTENCE_CONTRACTSTORAGE2_H_

#include <json/json.h>
#include <leveldb/db.h>
#include <shared_mutex>
#include <type_traits>

#include "common/Constants.h"
#include "common/Singleton.h"
#include "depends/libDatabase/LevelDB.h"
#include "libUtils/DataConversion.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "depends/libDatabase/OverlayDB.h"
#pragma GCC diagnostic pop

#include "depends/libTrie/TrieDB.h"

class ProtoScillaVal;

namespace Contract {

enum TERM { TEMPORARY, SHORTTERM, LONGTERM };

Index GetIndex(const dev::h160& address, const std::string& key);

//-----------------------------------------------------------------------//

// template<size_t idx, typename T>
// struct GetHelper;

struct MapBase {
  virtual bool exists(dev::h256 const& _h) const = 0;
  virtual std::string lookup(dev::h256 const& _h) const = 0;
  virtual void insert(dev::h256 const& _h, dev::bytesConstRef _v) = 0;
  virtual bool kill(dev::h256 const& _h) = 0;
};

template <typename ADDS, typename DELETES>
struct AddDeleteMap : private MapBase {
  AddDeleteMap(std::shared_ptr<ADDS> adds, std::shared_ptr<DELETES> deletes)
      : m_adds(adds), m_deletes(deletes) {}
  bool exists(dev::h256 const& _h) const {
    if (m_deletes->find(_h.hex()) != m_deletes->end()) {
      return false;
    }
    return m_adds->find(_h.hex()) != m_adds->end();
  }
  std::string lookup(dev::h256 const& _h) const {
    if (m_deletes->find(_h.hex()) != m_deletes->end()) {
      return "";
    }
    auto find = m_adds->find(_h.hex());
    if (find != m_adds->end()) {
      return DataConversion::CharArrayToString(find->second);
    }
    return "";
  }
  void insert(dev::h256 const& _h, dev::bytesConstRef _v) {
    auto find = m_deletes->find(_h.hex());
    if (find != m_deletes->end()) {
      m_deletes->erase(find);
    }
    (*m_adds)[_h.hex()] = DataConversion::StringToCharArray(_v.toString());
  }
  bool kill(dev::h256 const& _h) {
    m_deletes->emplace(_h.hex());
    return true;
  }

 private:
  std::shared_ptr<ADDS> m_adds;
  std::shared_ptr<DELETES> m_deletes;
};

struct LevelDBMap : private MapBase {
  LevelDBMap(std::shared_ptr<LevelDB> db) : m_db(db) {}
  bool exists(dev::h256 const& _h) const { return m_db->Exists(_h); }
  std::string lookup(dev::h256 const& _h) const { return m_db->Lookup(_h); }
  void insert([[gnu::unused]] dev::h256 const& _h,
              [[gnu::unused]] dev::bytesConstRef _v) {
    // do nothing
  }
  bool kill([[gnu::unused]] dev::h256 const& _h) {
    // do nothing
    return true;
  }

 private:
  std::shared_ptr<LevelDB> m_db;
};

template <class... T>
struct OverlayMap {};

// template<class T>
// struct MapUnion{
//   MapUnion(T t) : t(t) {}
//   bool exists(dev::h256 const& _h) const {
//     return t.exists(_h);
//   }

//   std::string lookup(dev::h256 const& _h) const {
//     return t.lookup(_h);
//   }

//   void insert(dev::h256 const& _h, dev::bytesConstRef _v) {
//     t.insert(_h, _v);
//   }

//   bool kill(dev::h256 const& _h) {
//     t.kill(_h);

//     return true;
//   }
// private:
//   T t;
// };

template <class Head, class... Tail>
struct OverlayMap<Head, Tail...> {
  OverlayMap(Head head, Tail... tail) : head(head), tail(tail...) {}

  bool exists(dev::h256 const& _h) const {
    if (head.exists(_h)) {
      return true;
    } else if (!sizeof...(Tail)) {
      return false;
    }

    return tail.exists(_h);
  }

  std::string lookup(dev::h256 const& _h) const {
    std::string ret = head.lookup(_h);
    if (!ret.empty()) {
      return ret;
    } else if (!sizeof...(Tail)) {
      return "";
    }

    return tail.lookup(_h);
  }

  void insert(dev::h256 const& _h, dev::bytesConstRef _v) {
    head.insert(_h, _v);
  }

  bool kill(dev::h256 const& _h) {
    head.kill(_h);

    return true;
  }

 private:
  Head head;
  OverlayMap<Tail...> tail;
};

template <>
struct OverlayMap<> {
  bool exists([[gnu::unused]] dev::h256 const& _h) const { return false; }

  std::string lookup([[gnu::unused]] dev::h256 const& _h) const { return ""; }

  void insert([[gnu::unused]] dev::h256 const& _h,
              [[gnu::unused]] dev::bytesConstRef _v) {}

  bool kill([[gnu::unused]] dev::h256 const& _h) { return false; }
};

using DefaultAddDeleteMap =
    AddDeleteMap<std::map<std::string, bytes>, std::set<std::string>>;

using PermOverlayMap = OverlayMap<DefaultAddDeleteMap, LevelDBMap>;
using TempOverlayMap =
    OverlayMap<DefaultAddDeleteMap, DefaultAddDeleteMap, LevelDBMap>;

class ContractStorage2 : public Singleton<ContractStorage2> {
  LevelDB m_codeDB;
  LevelDB m_initDataDB;

  LevelDB m_stateDataDB;
  //*
  std::shared_ptr<LevelDB> mp_stateDataDB;
  //*

  // Used by AccountStore
  std::map<std::string, bytes> m_stateDataMap;
  std::set<std::string> m_indexToBeDeleted;
  //*
  std::shared_ptr<std::map<std::string, bytes>> mp_stateDataMap;
  std::shared_ptr<std::set<std::string>> mp_indexToBeDeleted;
  //*

  // Used by AccountStoreTemp for StateDelta
  std::map<std::string, bytes> t_stateDataMap;
  std::set<std::string> t_indexToBeDeleted;
  //*
  std::shared_ptr<std::map<std::string, bytes>> tp_stateDataMap;
  std::shared_ptr<std::set<std::string>> tp_indexToBeDeleted;
  //*

  // Used for revert state due to failure in chain call
  std::map<std::string, bytes> p_stateDataMap;
  std::set<std::string> p_indexToBeDeleted;

  // Used for RevertCommitTemp
  std::unordered_map<std::string, bytes> r_stateDataMap;
  // value being true for newly added, false for newly deleted
  std::unordered_map<std::string, bool> r_indexToBeDeleted;

  // Permanent State
  //*
  PermOverlayMap m_permOM;
  TempOverlayMap m_tempOM;
  dev::GenericTrieDB<PermOverlayMap> m_permTrie;
  dev::GenericTrieDB<TempOverlayMap> m_tempTrie;
  //*

  mutable std::mutex m_codeMutex;
  mutable std::mutex m_initDataMutex;
  mutable std::mutex m_stateDataMutex;

  void DeleteByPrefix(const std::string& prefix);

  void DeleteByIndex(const std::string& index);

  void UpdateStateData(const std::string& key, const bytes& value,
                       bool cleanEmpty = false);

  bool CleanEmptyMapPlaceholders(const std::string& key);

  dev::h256 GetContractStateHashCore(const dev::h160& address, bool temp);

  void InitTempStateCore();

  ContractStorage2();

  ~ContractStorage2() = default;

 public:
  /// Returns the singleton ContractStorage instance.
  static ContractStorage2& GetContractStorage() {
    static ContractStorage2 cs;
    return cs;
  }

  /// Adds a contract code to persistence
  bool PutContractCode(const dev::h160& address, const bytes& code);

  /// Adds contract codes to persistence in batch
  bool PutContractCodeBatch(
      const std::unordered_map<std::string, std::string>& batch);

  /// Get the desired code from persistence
  bytes GetContractCode(const dev::h160& address);

  /// Delete the contract code in persistence
  bool DeleteContractCode(const dev::h160& address);

  /////////////////////////////////////////////////////////////////////////////
  bool PutInitData(const dev::h160& address, const bytes& initData);

  bool PutInitDataBatch(
      const std::unordered_map<std::string, std::string>& batch);

  bytes GetInitData(const dev::h160& address);

  bool DeleteInitData(const dev::h160& address);

  /////////////////////////////////////////////////////////////////////////////
  static std::string GenerateStorageKey(
      const dev::h160& addr, const std::string& vname,
      const std::vector<std::string>& indices);

  bool FetchStateValue(const dev::h160& addr, const bytes& src,
                       unsigned int s_offset, bytes& dst, unsigned int d_offset,
                       bool& foundVal);

  bool FetchContractFieldsMapDepth(const dev::h160& address,
                                   Json::Value& map_depth_json, bool temp);

  void InsertValueToStateJson(Json::Value& _json, std::string key,
                              std::string value, bool unquote = true,
                              bool nokey = false);

  bool FetchStateJsonForContract(Json::Value& _json, const dev::h160& address,
                                 const std::string& vname = "",
                                 const std::vector<std::string>& indices = {},
                                 bool temp = false);

  void FetchStateDataForKey(std::map<std::string, bytes>& states,
                            const std::string& key, bool temp);

  void FetchStateDataForContract(std::map<std::string, bytes>& states,
                                 const dev::h160& address,
                                 const std::string& vname = "",
                                 const std::vector<std::string>& indices = {},
                                 bool temp = true);

  void FetchUpdatedStateValuesForAddress(
      const dev::h160& address, std::map<std::string, bytes>& t_states,
      std::vector<std::string>& toDeletedIndices, bool temp = false);

  bool UpdateStateValue(const dev::h160& addr, const bytes& q,
                        unsigned int q_offset, const bytes& v,
                        unsigned int v_offset);

  void UpdateStateDatasAndToDeletes(
      const dev::h160& addr, const std::map<std::string, bytes>& t_states,
      const std::vector<std::string>& toDeleteIndices, dev::h256& stateHash,
      bool temp, bool revertible);

  /// Buffer the current t_map into p_map
  void BufferCurrentState();

  /// Revert the t_map from the p_map just buffered
  void RevertPrevState();

  /// Put the in-memory m_map into database
  bool CommitStateDB();

  /// Clean t_maps
  void InitTempState(bool callFromExternal = false);

  /// Get the state hash of a contract account
  dev::h256 GetContractStateHash(const dev::h160& address, bool temp,
                                 bool callFromExternal = false);

  /// Clean the databases
  void Reset();

  /// Revert m_map with r_map
  void RevertContractStates();

  /// Clean r_map
  void InitRevertibles();

  /// Refresh all DB
  bool RefreshAll();
};

}  // namespace Contract

#endif  // ZILLIQA_SRC_LIBPERSISTENCE_CONTRACTSTORAGE2_H_
