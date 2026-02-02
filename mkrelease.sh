#!/usr/bin/env bash

if test "$#" -ne 1; then
    echo "usage: $0 tagname"
    exit 1
fi

TAGNAME=$1

tagmatch="mclog\-[0-9]+\.[0-9]+\.[0-9]+"

TagExists () {
    git describe --tags --exact-match "${TAGNAME}" > /dev/null 2>&1
}

EchoOrExec () {
    if [[ $1 -eq 0 ]]; then
	echo "Will execute the following commands:"
	echo -e '\techo '${TAGNAME} '> latest_release'
	echo -e '\t'git add latest_release
	echo -e '\t'git commit -m \"Tagging as ${TAGNAME}\"
	echo -e '\t'git tag -a "${TAGNAME}" -m \"Tagged as ${TAGNAME}\"
	echo -e '\t'${pfx}git push --follow-tags
    else
	echo ${TAGNAME} > latest_release
	git add latest_release
	git commit -m "Tagging as ${TAGNAME}"
	git tag -a "${TAGNAME}" -m "Tagged as ${TAGNAME}"
	git push --follow-tags
    fi
}

TagExists
if [[ $? -eq 0 ]]; then
    echo "Tag '"${TAGNAME}"' already exists!!!"
    exit 1
fi

if [[ ! "${TAGNAME}" =~ $tagmatch ]]; then
    echo "bad tag name ${TAGNAME}, must match pattern '"${tagmatch}"'"
    exit 1
fi

EchoOrExec 0
echo -n "Proceed? [y/N]: "
read -n1 proceed
echo
if [[ ${proceed} == 'y' || ${proceed} == 'Y' ]]; then
    EchoOrExec 1
fi

exit 1
