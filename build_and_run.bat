@echo off
setlocal enabledelayedexpansion
echo Building SFML game...
rem Prepare bin directory so we can compile directly into it
set "BIN_DIR=%~dp0bin"
set "OBJ_DIR=%~dp0bin\obj"
if not exist "!BIN_DIR!" mkdir "!BIN_DIR!"
if not exist "!OBJ_DIR!" mkdir "!OBJ_DIR!"

rem Compiler settings
set "CXX=C:\mingw730\mingw64\bin\g++.exe"
set "CXXFLAGS=-std=c++17 -O2 -I"%~dp0SFML-2.5.1/include""
set "LDFLAGS=-L"%~dp0SFML-2.5.1/lib" -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio"
set "NUM_JOBS=4"

rem List of source files (without .cpp extension)
set "SOURCES=main Game Session Animation Entity Character Projectile Player Enemy Boss Platform FireProjectile IceProjectile HPPotion Meteor ResourceManager UISystem InputManager CombatSystem FloatingText GameWorld CameraController PhysicsManager SkillTree KeyBindingManager EntityRenderer DebugRenderer GameMaster SaveGameManager"

rem Compile each source file to object file (only if changed)
set "OBJECTS="
set "NEED_LINK=0"
set "TO_COMPILE="
set "COMPILE_COUNT=0"

rem First pass: determine what needs compilation
for %%S in (!SOURCES!) do (
    set "SRC=%%S.cpp"
    set "OBJ=!OBJ_DIR!\%%S.o"
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
        set "OBJ=!OBJ_DIR!\%%S.o"
        echo   Compiling %%S.cpp...
        start /B "compile_%%S" cmd /c "!CXX! !CXXFLAGS! -c "!SRC!" -o "!OBJ!" 2>&1 && exit 0 || exit 1"
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
        set "OBJ=!OBJ_DIR!\%%S.o"
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

