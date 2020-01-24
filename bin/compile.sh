#!/usr/bin/env bash
apt-get -y install upx-ucl
mkdir -p "build/${CROSS_TRIPLE}" 2> /dev/null
if [[ ${CROSS_TRIPLE} =~ mingw ]]; then 
	pushd /tmp > /dev/null
	curl -L -o rapidjson.tar.gz https://github.com/Tencent/rapidjson/archive/v1.1.0.tar.gz
	tar -xzvf rapidjson.tar.gz
	rm -f rapidjson.tar.gz
	mv rapidjson* rapidjson
	popd > /dev/null
	pushd /usr/src/mxe/ > /dev/null
	make MXE_TARGETS="${CROSS_TRIPLE}" portaudio
	popd > /dev/null
	$CXX -s -Os -ffunction-sections -fdata-sections -Wl,--gc-sections -o "build/${CROSS_TRIPLE}/pa_devs_json.exe" "src/pa_devs_json.cpp" -mthreads -Wl,--start-group -lwinmm -lm -ldsound -lole32 -luuid -lsetupapi -l:libportaudio.a -Wl,--end-group -I/usr/src/mxe/usr/${CROSS_TRIPLE}/include/ -I/tmp/rapidjson/include/
	strip --strip-all "build/${CROSS_TRIPLE}/pa_devs_json.exe"
else
	PACKAGE_ARCH=
	[[ ${CROSS_TRIPLE} =~ x86_64-linux ]] && PACKAGE_ARCH=amd64
	[[ ${CROSS_TRIPLE} =~ i.86-linux ]] && PACKAGE_ARCH=i386
	[[ ${CROSS_TRIPLE} =~ arm-.+hf ]] && PACKAGE_ARCH=armhf
	[[ ${CROSS_TRIPLE} =~ arm-.+el ]] && PACKAGE_ARCH=armel
	[[ ${CROSS_TRIPLE} =~ aarch64- ]] && PACKAGE_ARCH=aarch64
	echo "Target architecture: ${PACKAGE_ARCH}"
	apt-get update -y
	dpkg --add-architecture ${PACKAGE_ARCH}
	apt-get update -y
	apt-get -y install rapidjson-dev libjack-dev:${PACKAGE_ARCH} libasound2-dev:${PACKAGE_ARCH} portaudio19-dev:${PACKAGE_ARCH}
	$CXX -s -Os -fpermissive -ffunction-sections -fdata-sections -Wl,--gc-sections -o "build/${CROSS_TRIPLE}/pa_devs_json" "src/pa_devs_json.cpp" -pthread -Wl,--start-group -l:libjack.a -l:libasound.so -l:libportaudio.a -Wl,--end-group -I/usr/include/ -I/usr/include/$CROSS_TRIPLE/
	strip --strip-all "build/${CROSS_TRIPLE}/pa_devs_json"
fi
