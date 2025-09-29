param(
    [switch]$Clean,
    [ValidateSet('x86','x64')]
    [string]$Arch = 'x64'
)

if ($Clean) {
    Get-ChildItem -Path . -Recurse -File -Include finder.exe,*.obj,*.pdb,*.ilk,*.exp,*.lib,*.res,*.idb,*.pch,*.iobj,*.ipdb |
        ForEach-Object { Remove-Item $_ -Force -ErrorAction SilentlyContinue }
    Write-Host "Cleaned build artifacts."
    exit 0
}

$cl = Get-Command cl -ErrorAction SilentlyContinue
if (-not $cl) {
    Write-Error "MSVC 'cl' 未找到。请在 x64 Native Tools Command Prompt 或设置好环境变量后运行。"
    exit 1
}

$machine = if ($Arch -eq 'x86') { 'X86' } else { 'X64' }

# 提示当前工具链目标架构（VS 开发者命令行会设置该变量）
if ($env:VSCMD_ARG_TGT_ARCH -and ($env:VSCMD_ARG_TGT_ARCH.ToLower() -ne $Arch)) {
    Write-Warning "当前工具链为 $($env:VSCMD_ARG_TGT_ARCH)，与 -Arch $Arch 不一致。请在相应的 Native Tools 命令行中运行以确保成功编译。"
}

$srcFiles = (Get-ChildItem -Path .\src -Filter *.cpp | ForEach-Object { $_.FullName }) -join ' '
$cmd = "cl /nologo /EHsc /std:c++17 /O2 /DNDEBUG /Fe:finder.exe main.cpp $srcFiles /link /MACHINE:$machine /DEBUG:NONE /PDB:NONE /INCREMENTAL:NO /OPT:REF /OPT:ICF /Brepro"
Write-Host $cmd
& cmd /c $cmd

if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
Write-Host "Build succeeded: finder.exe"

# 删除中间产物，仅保留 finder.exe（需要 -Recurse/-Path 搭配 -Include）
Get-ChildItem -Path . -Recurse -File -Include *.obj,*.pdb,*.ilk,*.exp,*.lib,*.res,*.idb,*.pch,*.iobj,*.ipdb |
    ForEach-Object { Remove-Item $_ -Force -ErrorAction SilentlyContinue }
Write-Host "Cleaned intermediates, kept finder.exe only."


