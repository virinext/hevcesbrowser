#!/bin/bash

version_file=$1
out_file=$2


major=`sed -n '1p' $version_file`
minor=`sed -n '2p' $version_file`

commit=`git rev-list --count HEAD`
date=`date +'%d%m'%y`


echo "#ifndef VERSION_INFO_H_"                                      > $out_file
echo "#define VERSION_INFO_H_"                                      >> $out_file
echo ""                                                             >> $out_file
echo "#define VERSION_STR \"$major.$minor.$commit.$date\""          >> $out_file
echo ""                                                             >> $out_file
echo "#endif"                                                       >> $out_file
