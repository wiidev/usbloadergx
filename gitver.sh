#! /bin/bash

# Need to use GNU sed on Mac OS X
if [[ $(uname -s) == Darwin ]]; then
	SED=gsed
else
	SED=sed
fi

# Correct the path on Windows so that git works correctly
if [[ $(uname -s) == MSYS* ]]; then
	winpath=$(cmd //c REG QUERY "HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" -v PATH | grep '^    PATH'| sed 's/    PATH    REG_SZ    //')
	winpath=$(echo /$winpath | sed 's/://g' | sed -e 's/;/:\//g' | sed 's/\\/\//g' | sed 's/:\/$//')
	PATH=$PATH:$winpath
fi

commit_id=$(git rev-parse HEAD 2>/dev/null | head -c 7)
[ -z "$commit_id" ] && commit_id="0000001"
commit_message=$(git show -s --format="%<(52,trunc)%s" $commit_id 2>/dev/null | $SED -e 's/[[:space:]]*$//')
[ -z "$commit_message" ] && commit_message="unable to get the commit message"
commit_id_old=$(cat ./source/gitver.c 2>/dev/null | tr -d '\n' | $SED -r 's/.*([0-9a-f]{7}).*/\1/')

if [ "$commit_id" != "$commit_id_old" ] || [ ! -f ./source/gitver.c ]; then

cat <<EOF > ./source/gitver.c
#define GIT_HASH "$commit_id"

const char *commitID()
{
	return GIT_HASH;
}
EOF

	if [ -z "$commit_id_old" ]; then
		echo "Created gitver.c and set the commit ID to $commit_id ($commit_message)" >&2
	else
		echo "Changed the commit ID from $commit_id_old to $commit_id ($commit_message)" >&2
	fi
	echo >&2
fi
