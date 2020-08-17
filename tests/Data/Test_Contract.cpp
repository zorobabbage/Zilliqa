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

#include <array>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

#include <openssl/rand.h>
#include <boost/filesystem.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include <Schnorr.h>
#include "common/Constants.h"
#include "common/ErrTxn.h"
#include "depends/common/CommonIO.h"
#include "libCrypto/Sha2.h"
#include "libData/AccountData/Account.h"
#include "libData/AccountData/AccountStore.h"
#include "libData/AccountData/Transaction.h"
#include "libData/AccountData/TransactionReceipt.h"
#include "libPersistence/ContractStorage.h"
#include "libUtils/DataConversion.h"
#include "libUtils/JsonUtils.h"
#include "libUtils/Logger.h"
#include "libUtils/TimeUtils.h"

#include "ScillaTestUtil.h"

#define BOOST_TEST_MODULE contracttest
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

using namespace boost::multiprecision;
using namespace std;

PrivKey priv1, priv2, priv3, priv4;

void setup() {
  bytes priv1bytes, priv2bytes, priv3bytes;
  DataConversion::HexStrToUint8Vec(
      "1658F915F3F9AE35E6B471B7670F53AD1A5BE15D7331EC7FD5E503F21D3450C8",
      priv1bytes);
  DataConversion::HexStrToUint8Vec(
      "0FC87BC5ACF5D1243DE7301972B9649EE31688F291F781396B0F67AD98A88147",
      priv2bytes);
  DataConversion::HexStrToUint8Vec(
      "0AB52CF5D3F9A1E730243DB96419729EE31688F29B0F67AD98A881471F781396",
      priv3bytes);
  priv1.Deserialize(priv1bytes, 0);
  priv2.Deserialize(priv2bytes, 0);
  priv3.Deserialize(priv3bytes, 0);
}

BOOST_AUTO_TEST_SUITE(contracttest)

// BOOST_AUTO_TEST_CASE(simplemap) {
//   INIT_STDOUT_LOGGER();
//   LOG_MARKER();

//   PairOfKey owner = Schnorr::GenKeyPair();

//   //   PairOfKey employee1 = Schnorr::GenKeyPair();
//   //   PairOfKey employee2 = Schnorr::GenKeyPair();
//   //   PairOfKey employee3 = Schnorr::GenKeyPair();
//   vector<PairOfKey> senders;
//   unsigned int num_sender = 1;
//   for (unsigned int i = 0; i < num_sender; ++i) {
//     senders.emplace_back(Schnorr::GenKeyPair());
//   }

//   Address ownerAddr, contrAddr;

//   uint64_t nonce = 0;

//   vector<Address> senderAddrs;

//   if (SCILLA_ROOT.empty()) {
//     LOG_GENERAL(WARNING, "SCILLA_ROOT not set to run Test_Contract");
//     return;
//   }

//   AccountStore::GetInstance().Init();

//   ownerAddr = Account::GetAddressFromPublicKey(owner.second);
//   //   employee1Addr = Account::GetAddressFromPublicKey(employee1.second);
//   //   employee2Addr = Account::GetAddressFromPublicKey(employee2.second);
//   //   employee3Addr = Account::GetAddressFromPublicKey(employee3.second);

//   for (unsigned int i = 0; i < num_sender; ++i) {
//     Address senderAddr = Account::GetAddressFromPublicKey(senders[i].second);
//     senderAddrs.emplace_back(senderAddr);
//     AccountStore::GetInstance().AddAccountTemp(senderAddr,
//                                                {2000000000000, nonce});
//   }

//   AccountStore::GetInstance().AddAccountTemp(ownerAddr, {2000000000000, nonce});

//   contrAddr = Account::GetAddressForContract(ownerAddr, nonce);
//   LOG_GENERAL(INFO, "Simple-map Address: " << contrAddr);

//   std::vector<ScillaTestUtil::ScillaTest> tests;

//   for (unsigned int i = 1; i <= 2; i++) {
//     ScillaTestUtil::ScillaTest test;
//     BOOST_CHECK_MESSAGE(ScillaTestUtil::GetScillaTest(test, "simple-map", i),
//                         "Unable to fetch test simple-map_" << i << ".");

//     test.message["_sender"] = "0x" + ownerAddr.hex();

//     tests.emplace_back(test);
//   }

//   //   tests[1].message["params"][0]["value"] = "0x" + employee1Addr.hex();
//   //   tests[2].message["params"][0]["value"] = "0x" + employee2Addr.hex();
//   //   tests[3].message["params"][0]["value"] = "0x" + employee3Addr.hex();
//   //   tests[4].message["params"][0]["value"] = "0x" + employee1Addr.hex();

//   for (const auto& test : tests) {
//     LOG_GENERAL(INFO, "message: " << JSONUtils::GetInstance().convertJsontoStr(
//                           test.message));
//   }

//   // Replace owner address in init.json
//   for (auto& it : tests[0].init) {
//     if (it["vname"] == "owner") {
//       it["value"] = "0x" + ownerAddr.hex();
//     }
//   }

//   // and remove _creation_block (automatic insertion later).
//   ScillaTestUtil::RemoveCreationBlockFromInit(tests[0].init);
//   ScillaTestUtil::RemoveThisAddressFromInit(tests[0].init);

//   bool deployed = false;

//   uint64_t bnum = 1;

//   uint64_t stressAmount = 0;
//   bytes stressData;

//   for (unsigned int i = 0; i < tests.size();) {
//     bool deploy = i == 0 && !deployed;

//     // uint64_t bnum =
//     // ScillaTestUtil::GetBlockNumberFromJson(tests[0].blockchain);
//     std::string initStr =
//         JSONUtils::GetInstance().convertJsontoStr(tests[0].init);
//     bytes data;
//     uint64_t amount = 0;
//     Address recipient;
//     bytes code;
//     if (deploy) {
//       data = bytes(initStr.begin(), initStr.end());
//       recipient = Address();
//       code = tests[0].code;
//       deployed = true;
//     } else {
//       amount = ScillaTestUtil::PrepareMessageData(tests[i].message, data);
//       if (i == 0) {
//         stressAmount = amount;
//         stressData = data;
//       }
//       recipient = contrAddr;
//       i++;
//     }

//     Transaction tx(DataConversion::Pack(CHAIN_ID, 1), nonce, recipient, owner,
//                    amount, PRECISION_MIN_VALUE, 20000, code, data);
//     TransactionReceipt tr;
//     ErrTxnStatus error_code;
//     AccountStore::GetInstance().UpdateAccountsTemp(bnum, 1, true, tx, tr,
//                                                    error_code);
//     nonce++;
//   }

//   LOG_GENERAL(INFO, "STRESS TEST START");

//   // std::chrono::system_clock::time_point tpStart;

//   for (unsigned int i = 0; i < num_sender; ++i) {
//     LOG_GENERAL(INFO, "iteration " << i + 1);

//     Transaction tx(DataConversion::Pack(CHAIN_ID, 1), 0, contrAddr, senders[i],
//                    stressAmount, PRECISION_MIN_VALUE, 20000, {}, stressData);
//     TransactionReceipt tr;

//     // tpStart = r_timer_start();

//     ErrTxnStatus error_code;
//     AccountStore::GetInstance().UpdateAccountsTemp(bnum, 1, true, tx, tr,
//                                                    error_code);

//     // LOG_GENERAL(DEBUG, "Parse Transaction (microseconds) = "
//     //                        << r_timer_end(tpStart));
//   }

//   LOG_GENERAL(INFO, "STRESS TEST END");

//   //   Account* e2 = AccountStore::GetInstance().GetAccountTemp(employee2Addr);
//   //   Account* e3 = AccountStore::GetInstance().GetAccountTemp(employee3Addr);

//   //   BOOST_CHECK_MESSAGE(e2 != nullptr && e3 != nullptr,
//   //                       "employee2 or 3 are not existing");

//   //   BOOST_CHECK_MESSAGE(e2->GetBalance() == 11000 && e3->GetBalance() ==
//   //   12000,
//   //                       "multi message failed");
// }

BOOST_AUTO_TEST_CASE(ud) {
  INIT_STDOUT_LOGGER();
  LOG_MARKER();

  PairOfKey owner = Schnorr::GenKeyPair();
  
  vector<PairOfKey> senders;
  unsigned int num_sender = 1;
  for (unsigned int i = 0; i < num_sender; ++i) {
    senders.emplace_back(Schnorr::GenKeyPair());
  }

  Address ownerAddr, contrAddr1, contrAddr2;

  uint64_t nonce = 0;

  vector<Address> senderAddrs;

  if (SCILLA_ROOT.empty()) {
    LOG_GENERAL(WARNING, "SCILLA_ROOT not set to run Test_Contract");
    return;
  }

  AccountStore::GetInstance().Init();

  ownerAddr = Account::GetAddressFromPublicKey(owner.second);

  for (unsigned int i = 0; i < num_sender; ++i) {
    Address senderAddr = Account::GetAddressFromPublicKey(senders[i].second);
    senderAddrs.emplace_back(senderAddr);
    AccountStore::GetInstance().AddAccountTemp(senderAddr,
                                               {2000000000000, nonce});
  }

  AccountStore::GetInstance().AddAccountTemp(ownerAddr, {2000000000000, nonce});

  contrAddr1 = Account::GetAddressForContract(ownerAddr, nonce);
  LOG_GENERAL(INFO, "ud-registry address: " << contrAddr1);

  std::vector<ScillaTestUtil::ScillaTest> tests1;

  for (unsigned int i = 1; i <= 2; i++) {
    ScillaTestUtil::ScillaTest test;
    BOOST_CHECK_MESSAGE(ScillaTestUtil::GetScillaTest(test, "ud-registry", i),
                        "Unable to fetch test simple-map_" << i << ".");

    test.message["_sender"] = "0x" + ownerAddr.hex();

    tests1.emplace_back(test);
  }

  std::vector<ScillaTestUtil::ScillaTest> tests2;
  for (unsigned int i = 1; i <= 1; i++) {
    ScillaTestUtil::ScillaTest test;
    BOOST_CHECK_MESSAGE(ScillaTestUtil::GetScillaTest(test, "ud-proxy", i),
                        "Unable to fetch test simple-map_" << i << ".");

    test.message["_sender"] = "0x" + ownerAddr.hex();

    tests2.emplace_back(test);
  }

  // Replace owner address in init.json
  for (auto& it : tests1[0].init) {
    if (it["vname"] == "initialOwner") {
      it["value"] = "0x" + ownerAddr.hex();
    }
  }

  for (auto& it : tests2[0].init) {
    if (it["vname"] == "initialAdmin") {
      it["value"] = "0x" + ownerAddr.hex();
    } else if (it["vname"] == "registry") {
      it["value"] = "0x" + contrAddr1.hex();
    }
  }

  // and remove _creation_block (automatic insertion later).
  ScillaTestUtil::RemoveCreationBlockFromInit(tests1[0].init);
  ScillaTestUtil::RemoveThisAddressFromInit(tests1[0].init);
  ScillaTestUtil::RemoveCreationBlockFromInit(tests2[0].init);
  ScillaTestUtil::RemoveThisAddressFromInit(tests2[0].init);

  // deploy ud-registry
  {
    std::string initStr =
          JSONUtils::GetInstance().convertJsontoStr(tests1[0].init);
    uint64_t amount = 0;
    bytes code = tests1[0].code;
    bytes data = bytes(initStr.begin(), initStr.end());;
    Address recipient = Address();

    Transaction tx(DataConversion::Pack(CHAIN_ID, 1), nonce, recipient, owner,
                   amount, PRECISION_MIN_VALUE, 20000, code, data);
    TransactionReceipt tr;
    ErrTxnStatus error_code;
    AccountStore::GetInstance().UpdateAccountsTemp(1, 1, true, tx, tr,
                                                   error_code);
    nonce++;
  }

  contrAddr2 = Account::GetAddressForContract(ownerAddr, nonce);
  LOG_GENERAL(INFO, "ud-proxy address: " << contrAddr2);

  // deploy ud-proxy
  {
    std::string initStr =
          JSONUtils::GetInstance().convertJsontoStr(tests2[0].init);
    uint64_t amount = 0;
    bytes code = tests2[0].code;
    bytes data = bytes(initStr.begin(), initStr.end());;
    Address recipient = Address();

    Transaction tx(DataConversion::Pack(CHAIN_ID, 1), nonce, recipient, owner,
                   amount, PRECISION_MIN_VALUE, 20000, code, data);
    TransactionReceipt tr;
    ErrTxnStatus error_code;
    AccountStore::GetInstance().UpdateAccountsTemp(1, 1, true, tx, tr,
                                                   error_code);
    nonce++;
  }

  // replace ud-registry message
  // setRegistrar
  for (auto& it : tests1[0].message["params"]) {
    if (it["vname"] == "address") {
      it["value"] = "0x" + ownerAddr.hex();
    }
  }
  // setAdmin
  for (auto& it : tests1[1].message["params"]) {
    if (it["vname"] == "address") {
      it["value"] = "0x" + contrAddr2.hex();
    }
  }

  // replace ud-proxy message
  // bestow
  for (auto& it : tests2[0].message["params"]) {
    if (it["vname"] == "owner") {
      it["value"] = "0x" + ownerAddr.hex();
    } else if (it["vname"] == "resolver") {
      it["value"] = "0x" + ownerAddr.hex();
    }
  }

  // setRegistrar
  {
    bytes data;
    uint64_t amount = ScillaTestUtil::PrepareMessageData(tests1[0].message, data);
    Address recipient = contrAddr1;

    Transaction tx(DataConversion::Pack(CHAIN_ID, 1), nonce, recipient, owner,
                   amount, PRECISION_MIN_VALUE, 20000, {}, data);
    TransactionReceipt tr;
    ErrTxnStatus error_code;
    AccountStore::GetInstance().UpdateAccountsTemp(1, 1, true, tx, tr,
                                                   error_code);
    nonce++;
  }

  // setAdmin
  {
    bytes data;
    uint64_t amount = ScillaTestUtil::PrepareMessageData(tests1[1].message, data);
    Address recipient = contrAddr1;

    Transaction tx(DataConversion::Pack(CHAIN_ID, 1), nonce, recipient, owner,
                   amount, PRECISION_MIN_VALUE, 20000, {}, data);
    TransactionReceipt tr;
    ErrTxnStatus error_code;
    AccountStore::GetInstance().UpdateAccountsTemp(1, 1, true, tx, tr,
                                                   error_code);
    nonce++;
  }

  const Account* proxy = AccountStore::GetInstance().GetAccountTemp(contrAddr2);
  Json::Value state_proxy;
  if (!proxy->FetchStateJson(state_proxy, "", {}, true)) {
    return;
  }

  LOG_GENERAL(INFO, "ud-proxy states: " << JSONUtils::GetInstance().convertJsontoStr(state_proxy));

  // bestow
  {
    bytes data;
    uint64_t amount = ScillaTestUtil::PrepareMessageData(tests2[0].message, data);
    Address recipient = contrAddr2;

    LOG_GENERAL(INFO, "bestow: " << DataConversion::CharArrayToString(data));

    Transaction tx(DataConversion::Pack(CHAIN_ID, 1), nonce, recipient, owner,
                   amount, PRECISION_MIN_VALUE, 20000, {}, data);
    TransactionReceipt tr;
    ErrTxnStatus error_code;
    AccountStore::GetInstance().UpdateAccountsTemp(1, 1, true, tx, tr,
                                                   error_code);
    nonce++;
  }
}



BOOST_AUTO_TEST_SUITE_END()
