#!/bin/sh

# The rules of thumb, when dealing with these values are:
#
#     - Always increase the revision value.
#     - Increase the current value whenever an interface has been added,
#       removed or changed.
#     - Increase the age value only if the changes made to the ABI are
#       backward compatible.
#
# Format is "current:revision:age"

SHARED_LIB_VERSION="1:1:0"

RELEASE_NAME_FILE=`dirname $0`/latest_release
RELEASE_NAME=`cat ${RELEASE_NAME_FILE} 2>/dev/null`

GIT_TAG=""
GIT_VERSION=""
SVN_TAG=""
SVN_VERSION=""
DWM_TAG=""
DWM_VERSION=""
DWM_STATUS='DWM_WHAT_STATUS_DEV'

DwmGetGitTag() {
    local gittag=`git describe --tags --dirty 2>/dev/null`
    if test -z "${gittag}"; then
	GIT_TAG="${RELEASE_NAME}"
	DWM_STATUS='DWM_WHAT_STATUS_REL'
	GIT_VERSION=`echo ${GIT_TAG} | cut -d'-' -f2`
    else
	dirty=`echo "${gittag}" | awk -F '-' '{ if (NF > 2) { print "dirty"; } }'`
    
	if test -z "${dirty}"; then
	    GIT_TAG="${gittag}"
	    DWM_STATUS='DWM_WHAT_STATUS_REL'
	    GIT_VERSION=`echo "${gittag}" | awk -F '-' '{print $NF}'`
	else
	    fakevers=`date +%Y%m%d`
	    GIT_TAG="$1-0.0.${fakevers}"
	    GIT_VERSION="0.0.${fakevers}"
	fi
    fi
}

DwmGetSvnTag() {
    svntag=`svn info . 2>/dev/null | grep ^URL | grep 'tags/' | awk -F 'tags/' '{print $2}' | awk -F '/' '{ print $1}'`
    if test -n "${svntag}"; then
	SVN_TAG="${svntag}"
	SVN_VERSION=`echo "${svntag}" | awk -F '-' '{print $NF}'`
    else
	svn_vers=`svnversion . | sed 's/[[0-9]]*://g'`
	if test "${svn_vers}" = "Unversioned directory"; then
	    SVN_VERSION="0.0.0"
	else
	    SVN_VERSION="0.0.${svn_vers}"
	fi
	SVN_TAG="$1-${SVN_VERSION}"
    fi
}

DwmGetTag() {
    DwmGetGitTag $1
    if test -n "${GIT_TAG}"; then
	DWM_TAG="${GIT_TAG}"
	DWM_VERSION="${GIT_VERSION}"
    else
	DwmGetSvnTag $1
	if test -n "${SVN_TAG}" ; then
	    DWM_TAG="${SVN_TAG}"
	    DWM_VERSION="${SVN_VERSION}"
	fi
    fi
    if test -z "${DWM_TAG}" ; then
	DWM_TAG="$1-0.0.0"
	DWM_VERSION="0.0.0"
    fi
}

DwmGetTag mclog

args=`getopt Ssv $*`
set -- $args
for i; do
    case "$i" in
	-s)
	    echo "${SHARED_LIB_VERSION}"
	    exit 0
	    break;;
	-v)
	    echo "${DWM_VERSION}"
	    exit 0
	    break;;
	-S)
	    echo "${DWM_STATUS}"
	    exit 0
	    break;;	
    esac
done

echo "${DWM_TAG}"
