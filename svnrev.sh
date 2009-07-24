#! /bin/sh
#
a=$(svnversion -n ..)
[ -n "$a" ] || a=$(SubWCRev .. | tr -d '\n' | sed 's/[^0-9]*\([0-9]*\).*/\1/')

[ -f ../source/svnrev.h ] || touch ../source/svnrev.h

b=$(cat ../source/svnrev.h | tr -d '\n' | sed 's/[^0-9]*\([0-9]*\).*/\1/')

if [ $a -ne $b ]; then
	echo '#ifndef SVNREV_H' > ../source/svnrev.h
	echo '  #define SVNREV_H' >> ../source/svnrev.h
	echo '  #define SVN_REV "'$a'"' >> ../source/svnrev.h
	echo '#endif /* SVNREV_H */' >> ../source/svnrev.h
	echo 'svnrev.h changed' >&2
fi
echo $a
