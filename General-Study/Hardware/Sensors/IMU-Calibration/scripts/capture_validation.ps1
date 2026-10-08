param(
    [Parameter(Mandatory = $true)]
    [string]$Port
)

$ErrorActionPreference = 'Stop'
$culture = [System.Globalization.CultureInfo]::InvariantCulture
$outputDir = Join-Path (Split-Path $PSScriptRoot -Parent) 'data'
$null = New-Item -ItemType Directory -Path $outputDir -Force
$stamp = Get-Date -Format 'yyyyMMdd_HHmmss_fff'
$partialPath = Join-Path $outputDir "gyro_validation_$stamp.partial.csv"
$csvPath = Join-Path $outputDir "gyro_validation_$stamp.csv"
$summaryPath = Join-Path $outputDir "gyro_validation_$stamp.summary.csv"
$header = 'sample,t_us,gx_deg_s,gy_deg_s,gz_deg_s,gx_corrected_deg_s,gy_corrected_deg_s,gz_corrected_deg_s'
$serial = [System.IO.Ports.SerialPort]::new($Port, 115200)
$serial.ReadTimeout = 1000
$serial.DtrEnable = $true
$serial.NewLine = "`n"
$writer = $null
$count = 0
$started = $false
$done = $false

try {
    $serial.Open()
    Start-Sleep -Seconds 2
    $serial.DiscardInBuffer()
    $writer = [System.IO.StreamWriter]::new($partialPath, $false, [System.Text.UTF8Encoding]::new($false))
    $writer.WriteLine($header)
    Write-Host 'Keep the board still. Waiting for 1000 samples...'
    $serial.Write('S')
    $deadline = [DateTime]::UtcNow.AddSeconds(60)

    while ([DateTime]::UtcNow -lt $deadline) {
        try { $line = $serial.ReadLine().Trim() }
        catch [System.TimeoutException] { continue }

        if ($line.StartsWith('ERROR:')) { throw $line }
        if ($line -eq $header) { $started = $true; continue }
        if ($line -eq 'DONE') { $done = $true; break }
        if (!$started) { continue }
        if ($line -notmatch '^\d+,\d+(,-?\d+\.\d+){6}$') {
            throw "Invalid data row: $line"
        }
        $fields = $line.Split(',')
        if ([int]$fields[0] -ne ($count + 1)) { throw 'Sample sequence mismatch.' }
        $writer.WriteLine($line)
        $count++
        if ($count % 100 -eq 0) { Write-Host "Collected: $count / 1000" }
    }

    if (!$done -or $count -ne 1000) {
        throw "Capture incomplete: $count rows. Reset the board before retrying."
    }
}
finally {
    if ($null -ne $writer) { $writer.Dispose() }
    if ($serial.IsOpen) { $serial.Close() }
    $serial.Dispose()
}

# Only a complete capture receives the final CSV name.
Move-Item -LiteralPath $partialPath -Destination $csvPath
$rows = @(Import-Csv -LiteralPath $csvPath)
$stats = foreach ($axis in @('gx_deg_s', 'gy_deg_s', 'gz_deg_s', 'gx_corrected_deg_s', 'gy_corrected_deg_s', 'gz_corrected_deg_s')) {
    $values = @($rows | ForEach-Object { [double]::Parse($_.$axis, $culture) })
    $mean = ($values | Measure-Object -Average).Average
    $squaredSum = 0.0
    foreach ($value in $values) { $squaredSum += [Math]::Pow($value - $mean, 2) }
    $std = [Math]::Sqrt($squaredSum / ($values.Count - 1))
    [PSCustomObject]@{
        axis = $axis
        samples = $values.Count
        mean_deg_s = $mean.ToString('F8', $culture)
        sample_std_deg_s = $std.ToString('F8', $culture)
    }
}
$stats | Export-Csv -LiteralPath $summaryPath -NoTypeInformation -Encoding UTF8
$stats | Format-Table -AutoSize
Write-Host "Raw CSV: $csvPath"
Write-Host "Summary: $summaryPath"
