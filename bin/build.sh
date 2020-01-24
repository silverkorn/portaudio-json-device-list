#!/usr/bin/env bash
CURRENT_SCRIPT_DIRPATH=$(dirname $(realpath "$0"))
#CURRENT_SCRIPT_DIRPATH_DOCKER_FIX=${CURRENT_SCRIPT_DIRPATH}
PROJECT_ROOT_DIRPATH=$(realpath "${CURRENT_SCRIPT_DIRPATH}/..")
#PROJECT_ROOT_DIRPATH_FIX=${PROJECT_ROOT_DIRPATH}

if [ "$1" != "" ]; then
	mkdir build 2> /dev/null

	MSYS_NO_PATHCONV=0 docker run \
		--rm \
		-v ${PROJECT_ROOT_DIRPATH}/src:/work/src \
		-v ${PROJECT_ROOT_DIRPATH}/bin:/work/bin \
		-v ${PROJECT_ROOT_DIRPATH}/build:/work/build \
		dockcross/${1,,} \
		bash ./bin/compile.sh
else
	echo
	echo "You must specify the \"dockcross\" image to use for the build."
	echo
	echo "Supported list:"
	echo "  linux-x64"
	echo "  linux-x86"
	echo "  windows-static-x86"
	echo "  windows-static-x64"
fi
