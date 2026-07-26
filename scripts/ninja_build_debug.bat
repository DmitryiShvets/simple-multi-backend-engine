call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
:: Явно прокидываем Vulkan SDK в переменные для CMake и компилятора
if defined VULKAN_SDK (
    set "INCLUDE=%INCLUDE%;%VULKAN_SDK%\Include"
    set "LIB=%LIB%;%VULKAN_SDK%\Lib"
    echo [INFO] Vulkan SDK found at: %VULKAN_SDK%
) else (
    echo [WARNING] VULKAN_SDK environment variable is not set!
)

cd build/debug
ninja
