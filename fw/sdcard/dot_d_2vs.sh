#!/bin/sh
#set -x

if [ $# -ne 2 ]; then
    echo "Usage: dot_d_2vs.sh <vpj_file> <build_dir>"
    exit 1
fi

# Create temp file from depends files
find $2 -name "*.d" -exec cat '{}' + > e.tmp

if [ ! -s e.tmp ]; then
  find $2 -name "*.Po" -exec cat '{}' + > e.tmp
fi

# remove line continuations
cat e.tmp | sed -E 's/ *\\$//' > e1.tmp
# remove leading spaces
cat e1.tmp | sed -E 's/^ +//' > e2.tmp
# remove target files
cat e2.tmp | sed -E 's/^(.*): *//' > e3.tmp
# brake lines at spaces
cat e3.tmp | sed -E 's/([^\\]) /\1\n/g' > e4.tmp
# sort and remove dups
cat e4.tmp | sort | uniq > e5.tmp
# remove escapes on embedded spaces
cat e5.tmp | sed -E 's/\\ / /g' > e6.tmp
# add cwd to relative paths
echo -n  's!^([^/\\\])!' > e6.tmp
CWD=`pwd`
echo -n ${CWD} >> e6.tmp
echo '/\\1!g' >> e6.tmp
cat e5.tmp | sed -E -f e6.tmp > e7.tmp

# break into source and header sections
grep '\.[cs]$' e7.tmp > src.tmp
grep '\.cpp$' e7.tmp >> src.tmp
grep '\.h$' e7.tmp > hdr.tmp

cp $1 $1.bak
update_vpj.py $1 $2
rm *.tmp
echo "$1 updated"

