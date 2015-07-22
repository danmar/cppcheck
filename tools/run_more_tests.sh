#!/bin/bash
# Script Used by generate_and_run_tests.sh

set -e

python tools/extracttests.py --code=test1 $1

cd test1

../cppcheck -q . 2> 1.txt


# (!x) => (x==0)
sed  -ri 's/([(&][ ]*)\!([a-z]+)([ ]*[&)])/\1\2==0\3/' *.cpp
../cppcheck -q . 2> 2.txt && diff 1.txt 2.txt

# (x==0) => (0==x)
sed  -ri 's/([(&][ ]*)([a-z]+)[ ]*==[ ]*0([ ]*[&)])/\10==\2\3/' *.cpp
../cppcheck -q . 2> 2.txt && diff 1.txt 2.txt

# (0==x) => (!x)
sed  -ri 's/([(&][ ]*)0[ ]*==[ ]*([a-z]+)([ ]*[&)])/\1!\2\3/' *.cpp
../cppcheck -q . 2> 2.txt && diff 1.txt 2.txt




# if (x) => (x!=0)
sed  -ri 's/(if[ ]*\([ ]*[a-z]+)([ ]*[&)])/\1!=0\2/' *.cpp
../cppcheck -q . 2> 2.txt && diff 1.txt 2.txt

# while (x) => (x!=0)
sed  -ri 's/(while[ ]*\([ ]*[a-z]+)([ ]*[&)])/\1!=0\2/' *.cpp
../cppcheck -q . 2> 2.txt && diff 1.txt 2.txt

# (x!=0) => (0!=x)
sed  -ri 's/([(&][ ]*)([a-z]+)[ ]*!=[ ]*0([ ]*[&)])/\10!=\2\3/' *.cpp
../cppcheck -q . 2> 2.txt && diff 1.txt 2.txt

# (0!=x) => (x)
sed  -ri 's/([(&][ ]*)0[ ]*!=[ ]*([a-z]+[ ]*[&)])/\1\2/' *.cpp
../cppcheck -q . 2> 2.txt && diff 1.txt 2.txt


# (x < 0) => (0 > x)
sed  -ri 's/([(&][ ]*)([a-z]+)[ ]*<[ ]*(\-?[0-9]+)([ ]*[&)])/\1\3>\2\4/' *.cpp
../cppcheck -q . 2> 2.txt && diff 1.txt 2.txt

# (x <= 0) => (0 >= x)
sed  -ri 's/([(&][ ]*)([a-z]+)[ ]*<=[ ]*(\-?[0-9]+)([ ]*[&)])/\1\3>=\2\4/' *.cpp
../cppcheck -q . 2> 2.txt && diff 1.txt 2.txt

# (x > 0) => (0 < x)
sed  -ri 's/([(&][ ]*)([a-z]+)[ ]*<=[ ]*(\-?[0-9]+)([ ]*[&)])/\1\3>=\2\4/' *.cpp
../cppcheck -q . 2> 2.txt && diff 1.txt 2.txt

# (x >= 0) => (0 <= x)
sed  -ri 's/([(&][ ]*)([a-z]+)[ ]*<=[ ]*(\-?[0-9]+)([ ]*[&)])/\1\3>=\2\4/' *.cpp
../cppcheck -q . 2> 2.txt && diff 1.txt 2.txt

# (x == 123) => (123 == x)
sed  -ri 's/([(&][ ]*)([a-z]+)[ ]*==[ ]*(\-?[0-9]+)([ ]*[&)])/\1\3==\2\4/' *.cpp
../cppcheck -q . 2> 2.txt && diff 1.txt 2.txt

# (x != 123) => (123 != x)
sed  -ri 's/([(&][ ]*)([a-z]+)[ ]*\!=[ ]*(\-?[0-9]+)([ ]*[&)])/\1\3!=\2\4/' *.cpp
../cppcheck -q . 2> 2.txt && diff 1.txt 2.txt



# (0 < x) => (x > 0)
sed  -ri 's/([(&][ ]*)(\-?[0-9]+)[ ]*<[ ]*([a-z]+)([ ]*[&)])/\1\3>\2\4/' *.cpp
../cppcheck -q . 2> 2.txt && diff 1.txt 2.txt

# (0 <= x) => (x >= 0)
sed  -ri 's/([(&][ ]*)(\-?[0-9]+)[ ]*<=[ ]*([a-z]+)([ ]*[&)])/\1\3>=\2\4/' *.cpp
../cppcheck -q . 2> 2.txt && diff 1.txt 2.txt

# (0 > x) => (x < 0)
sed  -ri 's/([(&][ ]*)(\-?[0-9]+)[ ]*<=[ ]*([a-z]+)([ ]*[&)])/\1\3>=\2\4/' *.cpp
../cppcheck -q . 2> 2.txt && diff 1.txt 2.txt

# (0 >= x) => (x <= 0)
sed  -ri 's/([(&][ ]*)(\-?[0-9]+)[ ]*<=[ ]*([a-z]+)([ ]*[&)])/\1\3>=\2\4/' *.cpp
../cppcheck -q . 2> 2.txt && diff 1.txt 2.txt

# (123 == x) => (x == 123)
sed  -ri 's/([(&][ ]*)(\-?[0-9]+)[ ]*==[ ]*([a-z]+)([ ]*[&)])/\1\3==\2\4/' *.cpp
../cppcheck -q . 2> 2.txt && diff 1.txt 2.txt

# (123 != x) => (x <= 123)
sed  -ri 's/([(&][ ]*)(\-?[0-9]+)[ ]*\!=[ ]*([a-z]+)([ ]*[&)])/\1\3!=\2\4/' *.cpp
../cppcheck -q . 2> 2.txt && diff 1.txt 2.txt


cd ..

rm -rf test1
