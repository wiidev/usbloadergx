#! /bin/bash
#
# Automatic resource file list generation
# Created by Dimok

outFile="./source/themes/filelist.h"
count_old=$(cat $outFile 2>/dev/null | tr -d '\n\n' | sed 's/[^0-9]*\([0-9]*\).*/\1/')

count=0
for i in $(find ./data/images/ ./data/sounds/ ./data/fonts/ ./data/binary/ -maxdepth 1 -type f  \( ! -printf "%f\n" \))
do
	files[count]=$i
	count=$((count+1))
done

if [ "$count_old" != "$count" ] || [ ! -f $outFile ]
then

echo "Generating filelist.h for $count files." >&2
cat <<EOF > $outFile
/****************************************************************************
 * USB Loader GX resource files.
 * This file is generated automatically.
 * Includes $count files.
 *
 * NOTE:
 * Any manual modification of this file will be overwriten by the generation.
 ****************************************************************************/
#ifndef _FILELIST_H_
#define _FILELIST_H_

#include <gctypes.h>

EOF

for i in ${files[@]}
do
	filename=${i%.*}
	extension=${i##*.}
	echo 'extern const u8 '$filename'_'$extension'[];' >> $outFile
	echo 'extern const u32 '$filename'_'$extension'_size;' >> $outFile
	echo '' >> $outFile
done

echo 'RecourceFile Resources::RecourceFiles[] =' >> $outFile
echo '{' >> $outFile

for i in ${files[@]}
do
	filename=${i%.*}
	extension=${i##*.}
	echo -e '\t{"'$i'", '$filename'_'$extension', '$filename'_'$extension'_size, NULL, 0},' >> $outFile
done

echo -e '\t{"listBackground.png", NULL, 0, NULL, 0},\t// Optional' >> $outFile
echo -e '\t{"carouselBackground.png", NULL, 0, NULL, 0},\t// Optional' >> $outFile
echo -e '\t{"gridBackground.png", NULL, 0, NULL, 0},\t// Optional' >> $outFile
echo -e '\t{NULL, NULL, 0, NULL, 0}' >> $outFile
echo '};' >> $outFile

echo '' >> $outFile
echo '#endif' >> $outFile

fi
