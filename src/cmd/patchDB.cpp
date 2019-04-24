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

#include "libMediator/Mediator.h"
#include "libPersistence/BlockStorage.h"

using namespace std;

int main(int argc, char* argv[]) {
  vector<BlockHash> missingHashes;

  if (argc < 3) {
    cout << "Please supply the [patch DB path] [MB hashes]" << endl;
    return 1;
  }

  const string path(argv[1]);

  for (int i = 2; i < argc; i++) {
    missingHashes.push_back(BlockHash(argv[i]));
  }

  BlockStorage::GetBlockStorage().InitiateHistoricalDB(path);

  PairOfKey key;  // Dummy to initate mediator
  Peer peer;

  Mediator mediator(key, peer);
  Node node(mediator, 0, false);

  mediator.RegisterColleagues(nullptr, &node, nullptr, nullptr);

  for (const auto& blockhash : missingHashes) {
    MicroBlockSharedPtr mbptr;
    bytes serializedMB;
    if (!BlockStorage::GetBlockStorage().GetHistoricalMicroBlock(blockhash,
                                                                 mbptr)) {
      cout << blockhash << " "
           << "not available";
      continue;
    }
    mbptr->Serialize(serializedMB, 0);
    if (!BlockStorage::GetBlockStorage().PutMicroBlock(blockhash,
                                                       serializedMB)) {
      cout << "Failed to put " << blockhash << " in original DB" << endl;
    } else {
      cout << "Sucessfully Put " << blockhash << " in original DB" << endl;
    }
  }

  return 0;
}