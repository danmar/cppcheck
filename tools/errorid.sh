#!/bin/bash

#set -x

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# TODO: exclude testinternal.cpp
echo 'no --errorlist entry:'
grep -h -o -P '\[[a-zA-Z0-9_]+\]\\n\"' $DIR/../test/*.cpp | tr -d '[]\"' | sed 's/\\n$//' | sort -u | \
while read -r id; do
  if [ ${#id} -lt 4 ]; then
    continue
  fi
  $DIR/../cppcheck --errorlist | grep "id=\"$id\"" > /dev/null
  if [ $? -ne 0 ]; then
    echo $id
  fi
done

echo 'no test coverage:'
$DIR/../cppcheck --errorlist | grep -h -o -P 'id=\"[a-zA-Z0-9_]*\"' | sed 's/\id=//' | tr -d '\"' | sort -u | \
while read -r id; do
  grep -h -o -P "\[$id\]\\\\n\"" $DIR/../test/*.cpp > /dev/null
  if [ $? -ne 0 ]; then
    echo $id
  fi
done