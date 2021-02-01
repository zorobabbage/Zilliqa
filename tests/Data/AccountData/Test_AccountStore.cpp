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
#include <string>

#define BOOST_TEST_MODULE accountstoretest
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "libData/AccountData/AccountStore.h"
#include "libData/AccountData/AccountStoreSC.h"
#include "libData/AccountData/Address.h"
#include "libTestUtils/TestUtils.h"
#include "libUtils/Logger.h"
#include "libUtils/SysCommand.h"

#include "../ScillaTestUtil.h"

BOOST_AUTO_TEST_SUITE(accountstoretest)

BOOST_AUTO_TEST_CASE(rwtest) {
	AccountStore::GetInstance().Init();
	Address addr1, addr2;
	PairOfKey kpair1 = Schnorr::GenKeyPair();
	PairOfKey kpair2 = Schnorr::GenKeyPair();
	addr1 = Account::GetAddressFromPublicKey(kpair1.second);
	addr2 = Account::GetAddressFromPublicKey(kpair2.second);
    AccountStore::GetInstance().AddAccount(addr1,
                                           {1000, 0});
    AccountStore::GetInstance().AddAccount(addr2,
	                                       {500, 0});
    AccountStore::GetInstance().UpdateStateTrieAll();

    AccountStore::GetInstance().PrintAccountState();

	bytes _bytes;
    Transaction tx(DataConversion::Pack(CHAIN_ID, 1), 1, addr2, kpair1,
                 10, PRECISION_MIN_VALUE, 1);
    TransactionReceipt tr;
    TxnStatus error_code;
    AccountStore::GetInstance().UpdateAccountsTemp(0, 1, false, tx, tr,
	                                                 error_code);
    BOOST_CHECK(AccountStore::GetInstance().SerializeDelta());
    bytes delta;
    AccountStore::GetInstance().GetSerializedDelta(delta);
    AccountStore::GetInstance().InitTemp();
    AccountStore::GetInstance().DeserializeDeltaTemp(delta, 0);
    AccountStore::GetInstance().CommitTempRevertible();
    AccountStore::GetInstance().MoveUpdatesToDisk();
    Account* acct1 = AccountStore::GetInstance().GetAccount(addr1);
    LOG_GENERAL(INFO, "acct1: " << acct1->GetBalance());
    Account* acct2 = AccountStore::GetInstance().GetAccount(addr2);
    LOG_GENERAL(INFO, "acct2: " << acct2->GetBalance());
}

BOOST_AUTO_TEST_SUITE_END()
