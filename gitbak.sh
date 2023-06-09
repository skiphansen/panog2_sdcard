#!/bin/sh

#set -x

if [ $# -ne 1 ]; then
   echo "tarball filename ?"
   exit 1
fi
git status --short -uno --ignore-submodules | cut -c4- > ~/e

SUBMODULES=`git submodule status | cut -f3 -d' '`
for sub in ${SUBMODULES} ; do
  (cd ${sub};git status --short -uno --ignore-submodules | cut -c4- |  sed "s#^#${sub}/#" >> ~/e)
done

tar czf $1.tgz `cat ~/e`
echo "Backed up:"
tar tzf $1.tgz
mv $1.tgz ~/zips

