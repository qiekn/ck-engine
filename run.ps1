$BUILD = "build"
$TARGET = "editor"

# Auto-configure if build directory doesn't exist
if (-not (Test-Path $BUILD)) {
  Write-Host "Build directory not found, running cmake configure..."
  cmake -B $BUILD
}

cmake --build $BUILD -j
if ($LASTEXITCODE -eq 0) {
  & ".\$BUILD\$TARGET.exe"
}
