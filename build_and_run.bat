@echo off
setlocal enabledelayedexpansion
rem Change to the directory containing this script so all relative paths work
pushd "%~dp0"
echo Building SFML game...
rem Prepare bin directory so we can compile directly into it
set "BIN_DIR=bin"
set "OBJ_DIR=bin\obj"
if not exist "!BIN_DIR!" mkdir "!BIN_DIR!"
if not exist "!OBJ_DIR!" mkdir "!OBJ_DIR!"

rem Compiler settings
set "CXX=C:\mingw64\bin\g++.exe"
set "CXXFLAGS=-std=c++17 -O2 -I"SFML-2.5.1/include" -I"src/core" -I"src/entities" -I"src/systems" -I"src/world" -I"src/rendering" -I"src/data_structures""
set "LDFLAGS=-L"SFML-2.5.1/lib" -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio"
set "NUM_JOBS=4"

rem List of source files (relative paths without .cpp extension)
set "SOURCES=main src/core/Game src/core/Session src/core/GameState src/core/GameMaster src/entities/Animation src/entities/Entity src/entities/Character src/entities/Projectile src/entities/Player src/entities/Enemy src/entities/Boss src/entities/FireProjectile src/entities/IceProjectile src/entities/HPPotion src/entities/Meteor src/systems/ResourceManager src/systems/UISystem src/systems/InputManager src/systems/CombatSystem src/systems/CameraController src/systems/PhysicsManager src/systems/SkillTree src/systems/KeyBindingManager src/systems/SaveGameManager src/world/FloatingText src/world/GameWorld src/world/Platform src/rendering/DebugRenderer"

rem Compile each source file to object file (only if changed)
set "OBJECTS="
set "NEED_LINK=0"
set "TO_COMPILE="
set "COMPILE_COUNT=0"

rem First pass: determine what needs compilation
for %%S in (!SOURCES!) do (
    set "SRC=%%S.cpp"
    for %%F in ("%%S.cpp") do set "BASENAME=%%~nF"
    set "OBJ=!OBJ_DIR!\!BASENAME!.o"
    set "OBJECTS=!OBJECTS! !OBJ!"
    set "SHOULD_COMPILE=0"
    
    rem Check if object file doesn't exist
    if not exist "!OBJ!" (
        set "SHOULD_COMPILE=1"
    ) else (
        rem Use PowerShell to compare file timestamps reliably
        powershell -NoProfile -Command "if ((Get-Item '!SRC!').LastWriteTime -gt (Get-Item '!OBJ!').LastWriteTime) { exit 0 } else { exit 1 }" >nul 2>&1
        if not errorlevel 1 (
            set "SHOULD_COMPILE=1"
        )
    )
    
    if "!SHOULD_COMPILE!"=="1" (
        set "TO_COMPILE=!TO_COMPILE! %%S"
        set /a "COMPILE_COUNT+=1"
        set "NEED_LINK=1"
    )
)

rem Second pass: compile in parallel batches
if !COMPILE_COUNT! GTR 0 (
    echo Compiling !COMPILE_COUNT! file^(s^) with !NUM_JOBS! parallel jobs...
    set "JOB_COUNT=0"
    for %%S in (!TO_COMPILE!) do (
        set "SRC=%%S.cpp"
        for %%F in ("%%S.cpp") do set "BASENAME=%%~nF"
        set "OBJ=!OBJ_DIR!\!BASENAME!.o"
        echo   Compiling %%S.cpp...
        start /B "compile_!BASENAME!" cmd /c "!CXX! !CXXFLAGS! -c "!SRC!" -o "!OBJ!" 2>&1 && exit 0 || exit 1"
        set /a "JOB_COUNT+=1"
        
        rem Wait if we've reached max parallel jobs
        if !JOB_COUNT! GEQ !NUM_JOBS! (
            call :wait_for_jobs
            set "JOB_COUNT=0"
        )
    )
    
    rem Wait for remaining jobs
    call :wait_for_jobs
    
    rem Check for compilation errors
    for %%S in (!TO_COMPILE!) do (
        for %%F in ("%%S.cpp") do set "BASENAME=%%~nF"
        set "OBJ=!OBJ_DIR!\!BASENAME!.o"
        if not exist "!OBJ!" (
            echo Error compiling %%S.cpp
            pause
            exit /b 1
        )
    )
)
goto :after_compile

:wait_for_jobs
timeout /t 1 /nobreak >nul 2>&1
goto :eof

:after_compile

rem Link all object files into executable
if !NEED_LINK! EQU 1 (
    echo Linking executable...
    !CXX! !OBJECTS! !LDFLAGS! -o "!BIN_DIR!\main.exe"
    if errorlevel 1 (
        echo Linking failed!
        pause
        exit /b 1
    )
) else if not exist "!BIN_DIR!\main.exe" (
    echo Linking executable...
    !CXX! !OBJECTS! !LDFLAGS! -o "!BIN_DIR!\main.exe"
    if errorlevel 1 (
        echo Linking failed!
        pause
        exit /b 1
    )
) else (
    echo All files up to date, skipping compilation.
)

if exist "!BIN_DIR!\main.exe" (
    echo Build successful!
    rem Create bin directory next to this script if it doesn't exist
    rem (compiled directly into bin)

    rem List of DLLs to copy. Adjust names if your SFML DLLs differ.
    set "DLLS=sfml-graphics-2.dll sfml-window-2.dll sfml-system-2.dll sfml-audio-2.dll openal32.dll"

    rem Copy DLLs from workspace root and SFML bin if present
    for %%D in (!DLLS!) do (
        if exist "%~dp0%%D" (
            copy /Y "%~dp0%%D" "!BIN_DIR!" >nul
        ) else if exist "%~dp0SFML-2.5.1\bin\%%D" (
            copy /Y "%~dp0SFML-2.5.1\bin\%%D" "!BIN_DIR!" >nul
        )
    )

    rem Copy font and assets so the game can find resources when running from bin
    rem Prefer font inside assets folder (assets\arial.ttf), fall back to root arial.ttf
    if exist "%~dp0assets\\arial.ttf" (
        copy /Y "%~dp0assets\\arial.ttf" "!BIN_DIR!" >nul
    ) else if exist "%~dp0arial.ttf" (
        copy /Y "%~dp0arial.ttf" "!BIN_DIR!" >nul
    )

    if exist "%~dp0assets" (
        xcopy "%~dp0assets" "!BIN_DIR!\assets" /E /I /Y >nul
    )

    rem Check if --no-run parameter was passed (used by VS Code)
    if "%1"=="--no-run" (
        echo Build complete. Skipping auto-run.
    ) else (
        echo Starting game from "!BIN_DIR!"...
        pushd "!BIN_DIR!"
        main.exe
        popd
    )
) else (
    echo Build failed!
    pause
)

