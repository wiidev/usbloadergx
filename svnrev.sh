#! /bin/sh
#
a=$(svnversion -n . 2> /dev/null)
[ -n "$a" ] || a=$(SubWCRev . 2> /dev/null| tr -d '\n' | sed 's/[^0-9]*[0-9]*[^0-9]*\([0-9]*\).*/\1/')

[ -f ./source/svnrev.c ] || touch ./source/svnrev.c

b=$(cat ./source/svnrev.c | tr -d '\n' | sed 's/[^0-9]*\([0-9]*\).*/\1/')

if [ "$a" != "$b" ] || [ ! -n "$a" ]; then
	[ -n "$a" ] || a='unknow'
	echo '#define SVN_REV "'$a'"' > ./source/svnrev.c
	echo '' >> ./source/svnrev.c
	echo 'const char *GetRev()' >> ./source/svnrev.c
	echo '{ ' >> ./source/svnrev.c
	echo '  return SVN_REV;' >> ./source/svnrev.c
	echo '}' >> ./source/svnrev.c
	echo '' >> ./source/svnrev.c
	echo "Changed Rev $b to $a" >&2
	echo >&2
fi
echo $a
