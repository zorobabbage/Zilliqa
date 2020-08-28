#!/usr/bin/env python3
# Copyright (C) 2020 Zilliqa
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

import json
import socket
import argparse
# Generate below values from the proposal argument in the governance portal


def send_packet_tcp(data, host ="localhost", port=4301):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        sock.connect((host, port))
        data = data + "\n"
        sock.sendall((data.encode()))
        received = sock.recv(4096).decode()
    except socket.error:
        print("Socket error")
        sock.close()
        return None
    sock.close()
    return received

def generate_payload(params, methodName):
    payload = {"method": methodName,
               "params": params,
               "jsonrpc": "2.0",
               "id": 1.0
               }
    return payload

def get_response(methodName,params=None):
    data = json.dumps(generate_payload(params, methodName))
    print("Request:\n\t"+data)
    recv = send_packet_tcp(data)
    if not recv:
        return None
    response = json.loads(recv)
    print("Response:\n\t"+recv)
    return response

def ProcessResponse(resp):
    if "error" in resp:
        print("Error!!! Invalid request.Please send valid request parameter")
    elif "result" in resp:
        if resp["result"] == True:
            print("Hurray !!! vote is successfully set")
        else:
            print("Request parameter either empty or invalid")
    else:
        print("Error:Response from the miner node is not recognized")

def parse_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument("--proposal_description","-pd",help="proposal description of the governance proposal, default=Zilliqa proposed your vote on the game Rock, paper and Scissors.Please select options mentioned in the proposal question", default="Zilliqa proposed your vote on the game Rock, paper and Scissors.Please select options mentioned in the proposal question")
    parser.add_argument("--proposal_id","-pi",help="proposal id of the governance proposal, default=12345", default=12345)
    parser.add_argument("--proposal_question","-pq",help="proposal question of the governance proposal, default=Rock(1), Paper(2), Scissors(3) ?", default="Rock(1), Paper(2), Scissors(3) ?")
    parser.add_argument("--vote_options","-vo",help="Voting options to vote, default=[range(1,999)]",default=[*range(1,31)],type=int, nargs='*')
    parser.add_argument("--max_vote_attempt","-mv",help="Number of times to send vote in pows upon failure to be DS or shard member, default=5", default=5)
    parser.add_argument("--remaining_vote_count","-rv",help="Number of multiple votes, default=2", default=2)
    parser.add_argument("--start_epoch","-se",help="Starting epoch for voting, default=1", default=1)
    parser.add_argument("--end_epoch","-ee",help="Ending epoch for voting, default=1500", default=1500)
    args = parser.parse_args()
    return args

def main():
    args = parse_arguments()
    proposal_description = args.proposal_description
    proposal_id = args.proposal_id
    proposal_question = args.proposal_question
    vote_options = args.vote_options
    max_vote_attempt    = args.max_vote_attempt
    remaining_vote_count    = args.remaining_vote_count
    ds_epoch_start  = args.start_epoch
    ds_epoch_end    = args.end_epoch

    print("\n##### Welcome To Vote On Zilliqa Governance Proposal #####\n")
    print("# Proposal Description   : ", proposal_description)
    print("# Proposal Id            : ", proposal_id)
    print("# Proposal Question      : ", proposal_question)
    print("# Vote Options           : ", vote_options)
    print("# Max Vote Attempt       : ", max_vote_attempt)
    print("# Remaining Vote Count   : ", remaining_vote_count)
    print("# Start Epoch            : ", ds_epoch_start)
    print("# End Epoch              : ", ds_epoch_end)
    max_count = 3
    count = 0
    while True:
        try:
            vote_value = int(input("Enter your vote from the options in proposal question:"))
        except ValueError:
            print("Sorry, Couldn't understand the input.")
            continue
        else:
            if vote_value not in vote_options:
                print("Try again !!! Please provide valid voting input from the proposal question")
                count+=1
                if count >= max_count :
                    print("Exiting the program after entering incorrect vote option for {} times !!!".format(max_count))
                    return 1
                continue 
            break
    response = get_response("SetVoteInPow", [proposal_id,vote_value, max_vote_attempt, remaining_vote_count, ds_epoch_start, ds_epoch_end])
    if response == None:
        print("Error:Failed to send vote in ds epoch range")
    else:
        ProcessResponse(response)

if __name__ == "__main__":
    main()

