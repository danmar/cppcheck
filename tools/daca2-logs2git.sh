#!/bin/bash

#just in case...
rm /tmp/daca_msg /tmp/daca_tmp_diff /tmp/daca_tmp_plus /tmp/daca_tmp_minus >& /dev/null

base_url="http://cppcheck.sourceforge.net/devinfo/daca2-report/"

echo ${base_url}daca2.html


for site in `curl -s --compressed ${base_url}daca2.html | grep "^<tr" | grep -o "\".*\"" | sed s/\"//g` ; do
    echo ${site} ;  curl -s --compressed ${base_url}${site} | sort -n --parallel=4 | sed 's/^ *//; s/ *$//; /^$/d; s/^M$//; s/\r$//'  | sed -e 's/\[[0-9][0-9]:[0-9][0-9]\]$//' | sed '/^$/d' | sed 's/\&amp\;/\&/g; s/&lt;/</g; s/&gt;/>/g;' > ${site}
done


curl -s --compressed ${base_url}daca2.html > index.html


git diff > /tmp/daca_tmp_diff

grep "^+.*" /tmp/daca_tmp_diff > /tmp/daca_tmp_plus

grep "^-.*" /tmp/daca_tmp_diff > /tmp/daca_tmp_minus



plus_glob=`wc -l /tmp/daca_tmp_plus | cut -d' ' -f1`

plus_error=`grep -c ":\ \(inconclusive\ \)\?error: "  /tmp/daca_tmp_plus`
plus_warning=`grep -c ":\ \(inconclusive\ \)\?warning: "  /tmp/daca_tmp_plus`
plus_style=`grep -c ":\ \(inconclusive\ \)\?style: "  /tmp/daca_tmp_plus`
plus_performance=`grep -c ":\ \(inconclusive\ \)\?performance: "  /tmp/daca_tmp_plus`
plus_portability=`grep -c ":\ \(inconclusive\ \)\?portability: "   /tmp/daca_tmp_plus`
#plus_information=`grep -c "]: (information)"   /tmp/daca_tmp_plus`
plus_crash=`grep -c "\ Crash?$" /tmp/daca_tmp_plus`
plus_varid=`grep -c "called with varid 0\." /tmp/daca_tmp_plus`


minus_glob=`wc -l /tmp/daca_tmp_minus | cut -d' ' -f1`

minus_error=`grep -c ":\ \(inconclusive\ \)\?error: "  /tmp/daca_tmp_minus`
minus_warning=`grep -c ":\ \(inconclusive\ \)\?warning: "  /tmp/daca_tmp_minus`
minus_style=`grep -c ":\ \(inconclusive\ \)\?style: "  /tmp/daca_tmp_minus`
minus_performance=`grep -c ":\ \(inconclusive\ \)\?performance: "  /tmp/daca_tmp_minus`
minus_portability=`grep -c ":\ \(inconclusive\ \)\?portability: "   /tmp/daca_tmp_minus`
#minus_information=`grep -c "]: (information)"   /tmp/daca_tmp_minus`
minus_crash=`grep -c "\ Crash?$" /tmp/daca_tmp_minus`
minus_varid=`grep -c "called with varid 0\." /tmp/daca_tmp_minus`



files=`git ls-files`

ID_stats=`awk '{ print $NF }' $files | grep "^\[.*\]$"  | sort -n | uniq --count | sort -n`

echo "Update `date`" >> /tmp/daca_msg
echo "Updated: `git status  --porcelain   | grep daca | cut -d' ' -f3 | sed s/daca2-// | sed s/\.html// | tr '\n' ' '`"  >> /tmp/daca_msg
echo "all:           new: $plus_glob   gone: $minus_glob =   $((plus_glob-minus_glob))" >> /tmp/daca_msg
echo "error:         new: $plus_error   gone: $minus_error =   $((plus_error-minus_error))" >> /tmp/daca_msg
echo "warning:       new: $plus_warning   gone: $minus_warning =   $((plus_warning-minus_warning))" >> /tmp/daca_msg
echo "style:         new: $plus_style   gone: $minus_style =   $((plus_style-minus_style))" >> /tmp/daca_msg
echo "performance:   new: $plus_performance   gone: $minus_performance =   $((plus_performance-minus_performance))" >> /tmp/daca_msg
echo "portability:   new: $plus_portability   gone: $minus_portability =   $((plus_portability-minus_portability))" >> /tmp/daca_msg
#echo "information:   new: $plus_information   gone: $minus_information =   $((plus_information-minus_information))" >> /tmp/daca_msg
echo "crashes:       new: $plus_crash   gone: $minus_crash =   $((plus_crash-minus_crash))" >> /tmp/daca_msg
echo "varids:        new: $plus_varid   gone: $minus_varid =   $((plus_varid-minus_varid))" >> /tmp/daca_msg
echo "ID stats:" >> /tmp/daca_msg
echo "${ID_stats}" >> /tmp/daca_msg

cat /tmp/daca_msg

git add -A
git commit -F /tmp/daca_msg

rm /tmp/daca_msg /tmp/daca_tmp_diff /tmp/daca_tmp_plus /tmp/daca_tmp_minus

notify-send "daca logs done"
