@echo off
setlocal enabledelayedexpansion

echo ==========================================
echo   Building and installing libraries
echo   (one time, or when libraries change)
echo ==========================================

set "INSTALL_DIR=%CD%\build\libs"

:: Удаляем старую папку установки
if exist "%INSTALL_DIR%" rmdir /s /q "%INSTALL_DIR%"
mkdir "%INSTALL_DIR%"

:: Папка для сборки
set "BUILD_DIR=%CD%\build\libs_build"
if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
mkdir "%BUILD_DIR%"

:: Вызываем vcvars64.bat для настройки компиляторов Visual Studio
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

:: Явно прокидываем Vulkan SDK в переменные для CMake и компилятора
if defined VULKAN_SDK (
    set "INCLUDE=%INCLUDE%;%VULKAN_SDK%\Include"
    set "LIB=%LIB%;%VULKAN_SDK%\Lib"
    echo [INFO] Vulkan SDK found at: %VULKAN_SDK%
) else (
    echo [WARNING] VULKAN_SDK environment variable is not set!
)

:: Переходим в папку сборки
cd /d "%BUILD_DIR%"

:: Запускаем CMake с генератором Ninja
cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="%INSTALL_DIR%" -B . -S ..\..\external

:: Сборка и установка
ninja
ninja install

echo.
echo ==========================================
echo   Libraries installed: %INSTALL_DIR%
echo ==========================================
echo.

cd /d "%~dp0"
endlocal
