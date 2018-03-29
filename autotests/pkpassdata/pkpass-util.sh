#!/bin/bash

command=$1
pkpassFile=$2

if [ -z "${command}" ] || ! [ -f "${pkpassFile}" ]; then
	echo "Usage: $0 (unpack|repack) <pkpassFile>"
	exit 1
fi

if [ "${command}" == "unpack" ]; then
	mkdir -p ${pkpassFile}.dir
	pushd ${pkpassFile}.dir > /dev/null
	unzip -o ../${pkpassFile}
	rm -f *.png
	rm -f manifest.json
	rm -f signature
	rm -rf *.lproj
	mv pass.json pass.raw.json
	jq '.' pass.raw.json > pass.json
	rm pass.raw.json
	popd > /dev/null
elif [ "${command}" == "repack" ]; then
	pushd ${pkpassFile}.dir > /dev/null
	if ! [ -f "pass.json" ]; then
		echo "Invalid extracted pass!"
		exit 1
	fi
	rm  -f ../${pkpassFile}
	zip -r ../${pkpassFile} .
	popd > /dev/null
	rm -rf ${pkpassFile}.dir
else
	exit 1
fi

