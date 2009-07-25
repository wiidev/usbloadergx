#! /bin/sh
#
a=$(svnversion -n ..)
[ -n "$a" ] || a=$(SubWCRev .. | tr -d '\n' | sed 's/[^0-9]*\([0-9]*\).*/\1/')

[ -f ../source/svnrev.c ] || touch ../source/svnrev.c

b=$(cat ../source/svnrev.c | tr -d '\n' | sed 's/[^0-9]*\([0-9]*\).*/\1/')

if [ "$a" != "$b" ]; then
	echo '#define SVN_REV "'$a'"' > ../source/svnrev.c
	echo '' >> ../source/svnrev.c
	echo 'const char *GetRev()' >> ../source/svnrev.c
	echo '{ ' >> ../source/svnrev.c
	echo '  return SVN_REV;' >> ../source/svnrev.c
	echo '}' >> ../source/svnrev.c
	echo '' >> ../source/svnrev.c
	echo 'svnrev changed' >&2
fi
echo $a
