#!/bin/bash
set -euo pipefail

LIBTORCH_VERSION="${LIBTORCH_VERSION:-2.7.0}"
INSTALL_DIR="${INSTALL_DIR:-$(pwd)/pytorch-install}"
ARCH="$(uname -m)"
OS="$(uname -s)"

# Resolve the prebuilt LibTorch download URL for known platforms.
# Returns empty when no prebuilt is available, triggering a from-source build.
resolve_libtorch_url() {
    case "${OS}-${ARCH}" in
        Darwin-arm64)
            echo "https://download.pytorch.org/libtorch/cpu/libtorch-macos-arm64-${LIBTORCH_VERSION}.zip"
            ;;
        Darwin-x86_64)
            echo "https://download.pytorch.org/libtorch/cpu/libtorch-macos-x86_64-${LIBTORCH_VERSION}.zip"
            ;;
        Linux-x86_64)
            echo "https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-${LIBTORCH_VERSION}%2Bcpu.zip"
            ;;
    esac
}

LIBTORCH_URL="${LIBTORCH_URL:-$(resolve_libtorch_url)}"

if [ -n "${LIBTORCH_URL}" ]; then
    # ── Prebuilt download path ─────────────────────────────────────────
    WORK_DIR="$(mktemp -d)"
    trap 'rm -rf "${WORK_DIR}"' EXIT

    ARCHIVE_NAME="$(basename "${LIBTORCH_URL}")"
    ARCHIVE_PATH="${WORK_DIR}/${ARCHIVE_NAME}"

    echo "Downloading LibTorch ${LIBTORCH_VERSION} for ${OS}/${ARCH}..."
    echo "Source: ${LIBTORCH_URL}"
    curl -L --fail --progress-bar "${LIBTORCH_URL}" -o "${ARCHIVE_PATH}"

    echo "Preparing install directory at ${INSTALL_DIR}..."
    rm -rf "${INSTALL_DIR}"
    mkdir -p "${INSTALL_DIR}"

    echo "Extracting LibTorch..."
    unzip -q "${ARCHIVE_PATH}" -d "${WORK_DIR}"

    if [ ! -d "${WORK_DIR}/libtorch" ]; then
        echo "Unexpected archive layout: missing libtorch/ directory"
        exit 1
    fi

    mv "${WORK_DIR}/libtorch/"* "${INSTALL_DIR}/"

    # macOS: fix OpenMP linkage and re-sign dylibs
    if [ "${OS}" = "Darwin" ]; then
        if [ -f "${INSTALL_DIR}/lib/libomp.dylib" ] && [ -f "${INSTALL_DIR}/lib/libtorch_cpu.dylib" ]; then
            echo "Rewriting LibTorch OpenMP linkage to use the bundled libomp.dylib..."
            install_name_tool -id "@rpath/libomp.dylib" "${INSTALL_DIR}/lib/libomp.dylib"
            install_name_tool \
                -change "/opt/homebrew/opt/libomp/lib/libomp.dylib" "@loader_path/libomp.dylib" \
                -change "/usr/local/opt/libomp/lib/libomp.dylib" "@loader_path/libomp.dylib" \
                "${INSTALL_DIR}/lib/libtorch_cpu.dylib"
        fi

        if compgen -G "${INSTALL_DIR}/lib/*.dylib" > /dev/null; then
            echo "Refreshing ad-hoc signatures for local LibTorch dylibs..."
            codesign --force --sign - "${INSTALL_DIR}"/lib/*.dylib
        fi
    fi

    echo "LibTorch installed successfully."
    echo "TorchConfig.cmake: ${INSTALL_DIR}/share/cmake/Torch/TorchConfig.cmake"

else
    # ── Build-from-source fallback ─────────────────────────────────────
    echo "No prebuilt LibTorch available for ${OS}/${ARCH}; building from source..."

    if [ ! -d "pytorch" ]; then
        echo "Cloning PyTorch repository..."
        git clone -b main --recurse-submodule https://github.com/pytorch/pytorch.git
    else
        echo "PyTorch repository already exists, skipping clone..."
    fi

    mkdir -p pytorch-build
    cd pytorch-build

    echo "Configuring PyTorch..."
    cmake -DBUILD_SHARED_LIBS:BOOL=ON \
          -DCMAKE_BUILD_TYPE:STRING=Release \
          -DPYTHON_EXECUTABLE:PATH="$(which python3)" \
          -DCMAKE_INSTALL_PREFIX:PATH="../pytorch-install" \
          ../pytorch

    echo "Building PyTorch (this may take a while)..."
    cmake --build . --target install -j12

    echo "PyTorch built successfully!"
    echo "Installation location: $(cd .. && pwd)/pytorch-install"
fi
