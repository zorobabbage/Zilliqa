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

#define BOOST_TEST_MODULE test_state
#include <boost/test/included/unit_test.hpp>

#include "common/Messages.h"
#include "common/Serializable.h"
#include "depends/safeserver/safehttpserver.h"
#include "libCrypto/Schnorr.h"
#include "libMediator/Mediator.h"
#include "libNetwork/P2PComm.h"
#include "libServer/LookupServer.h"

using namespace std;

BOOST_AUTO_TEST_SUITE(test_state)

BOOST_AUTO_TEST_CASE(test_state) {
  INIT_STDOUT_LOGGER();

  LOG_MARKER();

  PairOfKey key;
  Peer peer;

  Mediator mediator(key, peer);
  Node node(mediator, 0, false);
  auto vd = make_shared<Validator>(mediator);

  uint num_txns;
  cin >> num_txns;

  mediator.RegisterColleagues(nullptr, &node, nullptr, vd.get());

  AccountStore::GetInstance().Init();

  auto ud_from_addr = Address("DFA89866ae86632b36361d53b76c1373448C28fa");
  auto ud_contract_one = Address("a11de7664F55F5bDf8544a9aC711691D01378b4c");
  auto ud_contract_two = Address("9611c53BE6d1b32058b2747bdeCECed7e1216793");

  node.AddBalanceToGenesisAccount();

  std::list<TxBlockSharedPtr> txblocks;
  if (!BlockStorage::GetBlockStorage().GetAllTxBlocks(txblocks)) {
    LOG_GENERAL(WARNING, "Failed to get Tx Blocks");
    return;
  }

  txblocks.sort([](const TxBlockSharedPtr& a, const TxBlockSharedPtr& b) {
    return a->GetHeader().GetBlockNum() < b->GetHeader().GetBlockNum();
  });

  vector<TxBlock> txBlocks;

  for (const auto& txblock : txblocks) {
    txBlocks.emplace_back(*txblock);
  }

  uint global_counter = 0;
  uint counter = 0;

  for (uint i = 1; i < txBlocks.size(); i++) {
    if (counter > num_txns) {
      break;
    }
    const auto& blocknum = txBlocks.at(i).GetHeader().GetBlockNum();
    if (i % 100 == 0) {
      LOG_GENERAL(INFO, "On Tx block " << blocknum);
    }
    auto microblockInfos = txBlocks.at(i).GetMicroBlockInfos();

    uint numShards = 0;
    for (const auto& mbInfo : microblockInfos) {
      if (numShards < mbInfo.m_shardId) {
        numShards = mbInfo.m_shardId;
      }
    }

    for (const auto& mbInfo : microblockInfos) {
      MicroBlockSharedPtr mbptr;
      if (counter > num_txns) {
        break;
      }

      if (mbInfo.m_txnRootHash == TxnHash()) {
        continue;
      }

      if (!BlockStorage::GetBlockStorage().GetMicroBlock(
              mbInfo.m_microBlockHash, mbptr)) {
        LOG_GENERAL(WARNING, "Unable to get MB: " << mbInfo.m_microBlockHash);
        continue;
      }

      for (const auto& tranHash : mbptr->GetTranHashes()) {
        TxBodySharedPtr tx;
        if (!BlockStorage::GetBlockStorage().GetTxBody(tranHash, tx)) {
          LOG_GENERAL(WARNING, "Missing Tx: " << tranHash);
          continue;
        }
        bool process = false;
        if (tx->GetTransaction().GetSenderAddr() == ud_from_addr) {
          process = true;
        } else if (tx->GetTransaction().GetToAddr() == ud_contract_one) {
          process = true;
          counter++;
        } else if (tx->GetTransaction().GetToAddr() == ud_contract_two) {
          process = true;
        }
        if (!process) {
          continue;
        }
        if (counter > num_txns) {
          break;
        }
        global_counter++;
        LOG_GENERAL(INFO, "Process txn " << global_counter);
        bool isDS = false;
        if (mbInfo.m_shardId == numShards) {
          isDS = true;
        }
        TransactionReceipt txreceipt;
        AccountStore::GetInstance().UpdateAccountsTemp(
            blocknum, numShards, isDS, tx->GetTransaction(), txreceipt);
      }
    }
  }

  AccountStore::GetInstance().SerializeDelta();
  AccountStore::GetInstance().CommitTemp();

  auto lookupServerConnector =
      make_unique<jsonrpc::SafeHttpServer>(LOOKUP_RPC_PORT);
  auto lookupServer =
      make_shared<LookupServer>(mediator, *lookupServerConnector);

  if (!(lookupServer->StartListening())) {
    LOG_GENERAL(WARNING, "Failed to start server");
  }

  while (true) {
    ;
  }
}

BOOST_AUTO_TEST_SUITE_END()
