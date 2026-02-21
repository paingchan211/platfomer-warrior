# Fast parallel build script for SFML game
param(
    [switch]$Clean,
    [switch]$NoRun,
    [int]$Jobs = 8
)

$ErrorActionPreference = "Stop"

# Configuration
$CXX = "C:\mingw64\bin\g++.exe"
$CXXFLAGS = "-std=c++17 -O2 -I`"$PSScriptRoot\SFML-2.5.1\include`" -I`"$PSScriptRoot\src\core`" -I`"$PSScriptRoot\src\entities`" -I`"$PSScriptRoot\src\systems`" -I`"$PSScriptRoot\src\world`" -I`"$PSScriptRoot\src\rendering`" -I`"$PSScriptRoot\src\data_structures`""
$LDFLAGS = "-L`"$PSScriptRoot\SFML-2.5.1\lib`" -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio"

$BIN_DIR = "$PSScriptRoot\bin"
$OBJ_DIR = "$BIN_DIR\obj"
$TARGET = "$BIN_DIR\main.exe"

# Source files (relative paths without .cpp extension)
$SOURCES = @(
    "main",
    "src/core/Game", "src/core/Session", "src/core/GameState", "src/core/GameMaster",
    "src/entities/Animation", "src/entities/Entity", "src/entities/Character",
    "src/entities/Projectile", "src/entities/Player", "src/entities/Enemy",
    "src/entities/Boss", "src/entities/FireProjectile", "src/entities/IceProjectile",
    "src/entities/HPPotion", "src/entities/Meteor",
    "src/systems/ResourceManager", "src/systems/UISystem", "src/systems/InputManager",
    "src/systems/CombatSystem", "src/systems/CameraController", "src/systems/PhysicsManager",
    "src/systems/SkillTree", "src/systems/KeyBindingManager", "src/systems/SaveGameManager",
    "src/world/FloatingText", "src/world/GameWorld", "src/world/Platform",
    "src/rendering/DebugRenderer"
)

# Create directories
if (-not (Test-Path $BIN_DIR)) { New-Item -ItemType Directory -Path $BIN_DIR | Out-Null }
if (-not (Test-Path $OBJ_DIR)) { New-Item -ItemType Directory -Path $OBJ_DIR | Out-Null }

# Clean if requested
if ($Clean) {
    Write-Host "Cleaning build artifacts..." -ForegroundColor Yellow
    Remove-Item "$OBJ_DIR\*.o" -ErrorAction SilentlyContinue
    Remove-Item $TARGET -ErrorAction SilentlyContinue
    Write-Host "Clean complete!" -ForegroundColor Green
    exit 0
}

# Determine what needs compilation
$toCompile = @()
foreach ($src in $SOURCES) {
    $srcFile = "$PSScriptRoot\$src.cpp"
    $basename = [System.IO.Path]::GetFileName($src)
    $objFile = "$OBJ_DIR\$basename.o"
    
    if (-not (Test-Path $objFile)) {
        $toCompile += $src
    }
    elseif ((Get-Item $srcFile).LastWriteTime -gt (Get-Item $objFile).LastWriteTime) {
        $toCompile += $src
    }
}

# Compile in parallel
if ($toCompile.Count -gt 0) {
    Write-Host "Compiling $($toCompile.Count) file(s) with $Jobs parallel jobs..." -ForegroundColor Cyan
    
    $processes = @()
    foreach ($src in $toCompile) {
        $srcFile = "$PSScriptRoot\$src.cpp"
        $objFile = "$OBJ_DIR\$src.o"
        
        # Wait if we've reached max parallel jobs
        while (($processes | Where-Object { -not $_.HasExited }).Count -ge $Jobs) {
            Start-Sleep -Milliseconds 100
        }
        
        $basename = [System.IO.Path]::GetFileName($src)
        $srcFile = "$PSScriptRoot\$src.cpp"
        $objFile = "$OBJ_DIR\$basename.o"
        Write-Host "  Compiling $src.cpp..." -ForegroundColor Gray
        $psi = New-Object System.Diagnostics.ProcessStartInfo
        $psi.FileName = $CXX
        $psi.Arguments = "$CXXFLAGS -c `"$srcFile`" -o `"$objFile`""
        $psi.UseShellExecute = $false
        $psi.RedirectStandardError = $true
        $psi.RedirectStandardOutput = $true
        $psi.WorkingDirectory = $PSScriptRoot
        
        $process = [System.Diagnostics.Process]::Start($psi)
        $processes += $process
    }
    
    # Wait for all jobs to complete
    $processes | ForEach-Object { $_.WaitForExit() }
    
    # Check for errors
    $failed = $false
    foreach ($proc in $processes) {
        if ($proc.ExitCode -ne 0) {
            $stderr = $proc.StandardError.ReadToEnd()
            Write-Host $stderr -ForegroundColor Red
            $failed = $true
        }
    }
    
    if ($failed) {
        Write-Host "Compilation failed!" -ForegroundColor Red
        exit 1
    }
    
    Write-Host "Compilation complete!" -ForegroundColor Green
}
else {
    Write-Host "All files up to date." -ForegroundColor Green
}

# Link
$needsLink = $toCompile.Count -gt 0 -or -not (Test-Path $TARGET)
if ($needsLink) {
    Write-Host "Linking executable..." -ForegroundColor Cyan
    
    $objFiles = ($SOURCES | ForEach-Object { "`"$OBJ_DIR\$([System.IO.Path]::GetFileName($_)).o`"" }) -join ' '
    $linkArgs = "$objFiles $LDFLAGS -o `"$TARGET`""
    
    $psi = New-Object System.Diagnostics.ProcessStartInfo
    $psi.FileName = $CXX
    $psi.Arguments = $linkArgs
    $psi.UseShellExecute = $false
    $psi.RedirectStandardError = $true
    $psi.WorkingDirectory = $PSScriptRoot
    
    $process = [System.Diagnostics.Process]::Start($psi)
    $process.WaitForExit()
    
    if ($process.ExitCode -ne 0) {
        $stderr = $process.StandardError.ReadToEnd()
        Write-Host $stderr -ForegroundColor Red
        Write-Host "Linking failed!" -ForegroundColor Red
        exit 1
    }
    
    Write-Host "Build successful!" -ForegroundColor Green
}

# Copy DLLs and assets
$dlls = @("sfml-graphics-2.dll", "sfml-window-2.dll", "sfml-system-2.dll", "sfml-audio-2.dll", "openal32.dll")
foreach ($dll in $dlls) {
    $dllPath = "$PSScriptRoot\SFML-2.5.1\bin\$dll"
    if (Test-Path $dllPath) {
        Copy-Item $dllPath $BIN_DIR -Force
    }
}

if (Test-Path "$PSScriptRoot\assets") {
    Copy-Item "$PSScriptRoot\assets" $BIN_DIR -Recurse -Force
}

# Run by default unless -NoRun is specified
if (-not $NoRun) {
    Write-Host "Starting game..." -ForegroundColor Cyan
    Push-Location $BIN_DIR
    & ".\main.exe"
    Pop-Location
}

