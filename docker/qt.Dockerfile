FROM debian:bookworm-slim AS build

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake ninja-build pkg-config \
    qt6-base-dev qt6-declarative-dev qt6-tools-dev qt6-tools-dev-tools \
    qt6-l10n-tools libqt6quick6 libqt6quickcontrols2-6 \
    qml6-module-qtquick qml6-module-qtquick-controls \
    qml6-module-qtquick-layouts qml6-module-qtquick-window \
    libxml2-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . .

RUN cmake -S qt -B build-qt \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build-qt --target all

FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
    libxml2 \
    libqt6core6 libqt6gui6 libqt6widgets6 libqt6qml6 \
    libqt6quick6 libqt6quickcontrols2-6 \
    qml6-module-qtquick qml6-module-qtquick-controls \
    qml6-module-qtquick-layouts qml6-module-qtquick-window \
    qml6-module-qtquick-dialogs qml6-module-qtqml-workerscript \
    qml6-module-qtquick-templates \
    libxcb-icccm4 libxcb-image0 libxcb-keysyms1 libxcb-randr0 \
    libxcb-render-util0 libxcb-xfixes0 libxcb-xinerama0 \
    libxcb-shape0 libxkbcommon-x11-0 libxcb-cursor0 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=build /src/build-qt/aris-qt /opt/aris-qt/aris-qt
COPY --from=build /src/build-qt/arisqt/ /opt/aris-qt/arisqt/

WORKDIR /opt/aris-qt
ENTRYPOINT ["./aris-qt"]
