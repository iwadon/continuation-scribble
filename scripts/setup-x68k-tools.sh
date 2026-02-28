#!/bin/sh
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
X68K_TOOLS_DIR="$PROJECT_DIR/m68k-xelf/tools"

# Create tools directories
echo "Setting up X68000 development tools..."
mkdir -p "$X68K_TOOLS_DIR/bin" "$X68K_TOOLS_DIR/lib" "$X68K_TOOLS_DIR/include" "$X68K_TOOLS_DIR/src"

# run68x
if [ ! -f "$X68K_TOOLS_DIR/src/run68x/build/run68" ]; then
	echo "Building run68x..."
	if [ ! -d "$X68K_TOOLS_DIR/src/run68x" ]; then
		git clone https://github.com/iwadon/run68x.git "$X68K_TOOLS_DIR/src/run68x"
	fi
	git -C "$X68K_TOOLS_DIR/src/run68x" checkout mod
	cmake -S "$X68K_TOOLS_DIR/src/run68x" -B "$X68K_TOOLS_DIR/src/run68x/build" -G Ninja -DCMAKE_BUILD_TYPE=Release
	ninja -C "$X68K_TOOLS_DIR/src/run68x/build"
	ln -sf "$X68K_TOOLS_DIR/src/run68x/build/run68" "$X68K_TOOLS_DIR/bin/run68"
fi

# has060x.x
if [ ! -f "$X68K_TOOLS_DIR/bin/has060x.x" ]; then
	echo "Downloading has060x.x..."
	if [ ! -f "$X68K_TOOLS_DIR/src/hasx125.zip" ]; then
	    curl -L -o /tmp/hasx125.zip https://github.com/kg68k/has060xx/releases/download/v1.2.5/hasx125.zip
		sha256sum -c - <<EOF
ef7915702063d2199ea42436ef880433f06b4b0f6f75abc7b24f9a518d5cbd67  /tmp/hasx125.zip
EOF
		mv /tmp/hasx125.zip "$X68K_TOOLS_DIR/src/"
	fi
	unzip -o "$X68K_TOOLS_DIR/src/hasx125.zip" -d "$X68K_TOOLS_DIR/src/hasx125"
	ln -sf "$X68K_TOOLS_DIR/src/hasx125/has060x.x" "$X68K_TOOLS_DIR/bin/"
fi
