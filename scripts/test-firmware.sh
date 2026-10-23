#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
build_dir=$(mktemp -d)
trap 'rm -rf "$build_dir"' EXIT
compiler=${CXX:-c++}

"$compiler" -std=c++11 -Wall -Wextra -Werror -pedantic -Isrc \
  src/lighting.cpp tests/lighting_test.cpp -o "$build_dir/lighting-test"
"$build_dir/lighting-test"

"$compiler" -std=c++11 -Wall -Wextra -Werror -pedantic -Itests/stubs -Isrc \
  tests/adapter_test.cpp -o "$build_dir/adapter-test"
"$build_dir/adapter-test"

"$compiler" -std=c++11 -Wall -Wextra -Werror -pedantic -DLIGHTING_INTEGRATION \
  -Itests/stubs -Isrc src/lighting.cpp tests/adapter_test.cpp -o "$build_dir/integration-test"
"$build_dir/integration-test"
