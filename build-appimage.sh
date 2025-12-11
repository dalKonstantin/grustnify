#!/usr/bin/env bash
set -e

# ---- настройки, под себя поправь при необходимости ----

APP_NAME="grustnify"
APP_EXECUTABLE="grustnify"
APP_ICON="assets/icons/Icon-iOS-Dark-256x256@1x.png"

TOOLS_DIR="$HOME/tools"
LINUXDEPLOY="$TOOLS_DIR/linuxdeploy-x86_64.AppImage"
LINUXDEPLOY_QT="$TOOLS_DIR/linuxdeploy-plugin-qt-x86_64.AppImage"

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$ROOT_DIR/build-appimage"
APPDIR="$BUILD_DIR/AppDir"

if [ ! -f "$LINUXDEPLOY" ]; then
  echo "ОШИБКА: не найден $LINUXDEPLOY"
  exit 1
fi

if [ ! -f "$LINUXDEPLOY_QT" ]; then
  echo "ОШИБКА: не найден $LINUXDEPLOY_QT"
  exit 1
fi

chmod +x "$LINUXDEPLOY" "$LINUXDEPLOY_QT"

echo "==> Чистим и создаём build-dir: $BUILD_DIR"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "==> Конфигурация CMake (Release)"
cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr \
  "$ROOT_DIR"

echo "==> Сборка"
cmake --build . --config Release -- -j"$(nproc)"

echo "==> Установка в AppDir"
DESTDIR="$APPDIR" cmake --install . --config Release

if [ ! -x "$APPDIR/usr/bin/$APP_EXECUTABLE" ]; then
  echo "ОШИБКА: не найден исполняемый файл $APPDIR/usr/bin/$APP_EXECUTABLE"
  exit 1
fi

DESKTOP_FILE="$BUILD_DIR/$APP_NAME.desktop"
echo "==> Генерируем .desktop файл: $DESKTOP_FILE"

cat >"$DESKTOP_FILE" <<EOF
[Desktop Entry]
Type=Application
Name=$APP_NAME
Comment=Make your favourite sad tracks even sadder
Exec=$APP_EXECUTABLE
Icon=$APP_NAME
Categories=AudioVideo;Utility;
Terminal=false
EOF

ICON_TARGET="$APPDIR/usr/share/icons/hicolor/256x256/apps/$APP_NAME.png"
ICON_SRC="$ROOT_DIR/$APP_ICON"

if [ -f "$ICON_SRC" ]; then
  echo "==> Копируем иконку"
  mkdir -p "$(dirname "$ICON_TARGET")"
  cp "$ICON_SRC" "$ICON_TARGET"
else
  echo "==> ВНИМАНИЕ: иконка $ICON_SRC не найдена, пропускаем."
  ICON_TARGET=""
fi

echo "==> Запускаем linuxdeploy + Qt plugin"

ICON_ARGS=()
if [ -n "$ICON_TARGET" ]; then
  ICON_ARGS=(-i "$ICON_TARGET")
fi

echo "==> Добавляем ~/tools в PATH для плагина Qt"
export PATH="$TOOLS_DIR:$PATH"
export QMAKE="/usr/bin/qmake6"

echo "==> Запускаем linuxdeploy + Qt plugin"

NO_STRIP=1 "$LINUXDEPLOY" \
  --appimage-extract-and-run \
  --appdir "$APPDIR" \
  -e "$APPDIR/usr/bin/$APP_EXECUTABLE" \
  -d "$DESKTOP_FILE" \
  "${ICON_ARGS[@]}" \
  --plugin qt \
  --output appimage

echo "==> Готово. Ищи *.AppImage в $BUILD_DIR"
