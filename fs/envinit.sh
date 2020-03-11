#!/bin/bash
chunks=(
f.4K.d
f.8K.d
f.16K.d
f.32K.d
f.64K.d
f.128K.d
f.256K.d
f.512K.d
f.1M.d
f.2M.d
f.4M.d
f.8M.d
f.16M.d
f.32M.d
f.64M.d
f.128M.d
f.256M.d
f.512M.d
f.1024M.d)
config="config"

os=$(uname -s)
blksize=0
if [ $os == Darwin ]; then
    blksize=$(diskutil info disk1s1 | grep 'Device Block' | tr -s " " | rev | cut -d' ' -f2 | rev)
    echo "Block size: $blksize, please set in \"fs.c\""
fi

echo $blksize > $config
for c in ${chunks[@]}; do
    echo "preparing $c"
    echo $c >> $config
    size=$(echo $c | cut -d. -f2)
    [ ! -f $c ] && dd if=/dev/zero of=$c bs=$size count=0 seek=1 &> /dev/null
done

exit 0