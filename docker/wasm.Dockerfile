# ── Stage 1: cross-compile libxml2 for wasm ──────────────────────────
FROM emscripten/emsdk:3.1.25 AS build-libxml2

RUN apt-get update && apt-get install -y --no-install-recommends \
    git \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build
RUN git clone --depth 1 --branch v2.10.4 \
        https://gitlab.gnome.org/GNOME/libxml2.git /libxml2-src \
    && emcmake cmake -S /libxml2-src -B /build/libxml2-build \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=OFF \
        -DLIBXML2_WITH_HTTP=OFF \
        -DLIBXML2_WITH_FTP=OFF \
        -DLIBXML2_WITH_PYTHON=OFF \
        -DLIBXML2_WITH_THREADS=OFF \
        -DLIBXML2_WITH_LZMA=OFF \
        -DLIBXML2_WITH_ZLIB=OFF \
        -DLIBXML2_WITH_ICONV=OFF \
    && emmake cmake --build /build/libxml2-build -j"$(nproc)"

RUN mkdir -p /libxml2-out \
    && cp /build/libxml2-build/libxml2.a /libxml2-out/ \
    && cp -r /libxml2-src/include/libxml /libxml2-out/ \
    && cp /build/libxml2-build/libxml/xmlversion.h /libxml2-out/libxml/

# ── Stage 2: install Qt 6.5 wasm SDK + build aris-qt ─────────────────
FROM emscripten/emsdk:3.1.25 AS build-aris

RUN apt-get update && apt-get install -y --no-install-recommends \
    make python3 python3-pip python3-dev \
    && pip3 install --no-cache-dir aqtinstall \
    && aqt install-qt linux desktop 6.5.3 gcc_64 \
        --outputdir /opt/qt \
    && aqt install-qt linux desktop 6.5.3 wasm_singlethread \
        --outputdir /opt/qt \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . .
COPY --from=build-libxml2 /libxml2-out/ /src/libxml2/

RUN /opt/qt/6.5.3/wasm_singlethread/bin/qmake qt/aris-qt.pro -spec wasm-emscripten \
    && make -j"$(nproc)"

# ── Stage 3: serve ───────────────────────────────────────────────────
FROM python:3.12-slim

WORKDIR /app
COPY --from=build-aris /src/*.html /src/*.js /src/*.wasm /app/

EXPOSE 8000
CMD ["python", "-m", "http.server", "8000"]
