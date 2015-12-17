#!/bin/bash

cat $1 | sed s@\"@\\\\\"@g | sed s@^@\"@ | sed s@\$@\\\\n\"@ | sed 's@\t@    @g'
