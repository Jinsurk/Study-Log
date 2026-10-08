param(
    [string]$Port = 'COM3',
    [ValidateSet('tilt1','tilt2','tilt3')]
    [string]$Position = 'tilt1'
)
$ErrorActionPreference = 'Stop'
& (Join-Path $PSScriptRoot 'capture_accel.ps1') -Port $Port -Position $Position
$csv = Get-ChildItem -LiteralPath (Join-Path (Split-Path $PSScriptRoot -Parent) 'data') -Filter "accel_${Position}_*.csv" |
    Where-Object { $_.Name -match '^accel_tilt[123]_\d{8}_\d{6}_\d{3}\.csv$' } |
    Sort-Object Name -Descending | Select-Object -First 1
if ($null -eq $csv) { throw 'Complete validation capture not found.' }
& python (Join-Path $PSScriptRoot 'analyze_accel_validation.py') $csv.FullName
if ($LASTEXITCODE -ne 0) { throw 'Validation analysis failed.' }
