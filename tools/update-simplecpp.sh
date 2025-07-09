#!/usr/bin/env bash

set -e

url="https://github.com/danmar/simplecpp"

tag="$(git ls-remote --tags --sort=-v:refname  "$url" | head -1 | cut -f2)"
[ -z "$tag" ] && exit 1

echo "Latest tag is $tag"

dest_dir="$(dirname "$(realpath "${BASH_SOURCE[@]}")")/../externals/simplecpp"
files=("simplecpp.cpp" "simplecpp.h")

for file in "${files[@]}"; do
	curl -LO --output-dir "$dest_dir" "$url/$tag/$file"
done
