#!/usr/bin/env python3
# Copyright (C) 2019 Zilliqa
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
# Generate below values from the proposal argument in the governance portal
proposal_description = " "
proposal_question="Rock(1), Paper(2), Scissors(3) ?"
proposal_id='12345'
vote_options   = [1,2,3]
ds_epoch_start  = 0
ds_epoch_end    = 14


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
        print("Error!!! Querying to wrong node")
    elif "result" in resp:
        if resp["result"] == True:
            print("Hurray !!! vote is successfully set")
        else:
            print("Failed to set vote in the epoch.Please try again later.")
    else:
        print("Error:Response from the miner node is not recognized")
        
    
def main():
    print("\n##### Welcome To Vote On Zilliqa Governance Proposal #####\n")
    print("# Proposal Id      : ", proposal_id)
    print("# Proposal Question  : ", proposal_question)
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
                    print("Exiting the program !!!")
                    return 1
                continue 
            break
    response = get_response("GetCurrentDSEpoch")
    if response == None:
        print("Failed to receive response.Please try again.")
        return 1
    if not int(response["result"]) in range(ds_epoch_start,ds_epoch_end+1):
        print("Error:Epoch interval either not started or is expired for voting")
        return 1
    response = get_response("SetVoteInPow", [proposal_id,vote_value])
    if response == None:
        print("Error:Failed to send vote in ds epoch range")
    else:
        ProcessResponse(response)
    
if __name__ == "__main__":
    main()
