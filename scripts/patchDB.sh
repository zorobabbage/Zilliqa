#!/bin/bash
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

# [MUST BE FILLED IN] User configuration settings


binaryPath=""
patchDBPath="patchDB"
mbHash="" ##Fill in
tarFileName="" ##Fill in

echo -n "Enter the full path of your zilliqa source code directory: " && read path_read && [ -n "$path_read" ] && binaryPath=$path_read

if [ -z "$binaryPath" ] || ([ ! -x $binaryPath/build/src/cmd/patchDB ] && [ ! -x $binaryPath/src/cmd/patchDB ]); then
    echo "Cannot find patchDB binary on the path you specified"
    exit 1
fi


##to do fetch from s3

(mkdir "${patchDBPath}" ; cd "${patchDBPath}"; tar -xvzf ../"${tarFileName}")



if [ -x $binaryPath/build/src/cmd/patchDB ]; then
    $binaryPath/build/src/cmd/patchDB "${patchDBPath}/persistence" "$mbHash"
elif [ -x  $binaryPath/src/cmd/patchDB ]; then
   $binaryPath/src/cmd/patchDB "${patchDBPath}/persistence" "$mbHash"
fi

