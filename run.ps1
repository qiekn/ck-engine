$PRESET = if ($env:PRESET) { $env:PRESET } else { "debug" }
$TARGET = "editor"
$BIN_DIR = "build/$PRESET"

# Auto-configure if build directory doesn't exist
if (-not (Test-Path $BIN_DIR)) {
  Write-Host "Build directory not found, running cmake configure with preset $PRESET..."
  cmake --preset $PRESET
}

cmake --build --preset $PRESET
if ($LASTEXITCODE -eq 0) {
  & ".\$BIN_DIR\$TARGET.exe"
}
