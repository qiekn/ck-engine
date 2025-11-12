cmake --build build -j10;
if ($LASTEXITCODE -eq 0) {
  ./build/sandbox.exe
}
