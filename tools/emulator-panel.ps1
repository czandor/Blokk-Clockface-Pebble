param([switch]$Check)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Windows.Forms, System.Drawing
[Windows.Forms.Application]::EnableVisualStyles()

$form = [Windows.Forms.Form]::new()
$form.Text = "Blokk – Pebble Time 2 emulátor"
$form.ClientSize = [Drawing.Size]::new(422, 370)
$form.FormBorderStyle = 'FixedDialog'
$form.MaximizeBox = $false
$form.StartPosition = 'CenterScreen'
$form.Font = [Drawing.Font]::new('Segoe UI', 10)

$commands = [Windows.Forms.Panel]::new()
$commands.SetBounds(0, 0, 422, 300)
$form.Controls.Add($commands)

$status = [Windows.Forms.Label]::new()
$status.SetBounds(20, 308, 382, 48)
$status.Text = 'Emery · válassz egy eseményt.'
$form.Controls.Add($status)
$script:pending = $null

function Start-EmulatorCommand {
    param([string]$Title, [string[]]$CommandArgs)
    if ($script:pending) { return }
    try {
        $start = [Diagnostics.ProcessStartInfo]::new()
        $start.FileName = 'wsl.exe'
        # Values come only from buttons, numeric fields and fixed SDK enums.
        $wslArgs = @('-d', 'NineTimeSDK', '-u', 'root', '--', '/usr/bin/env', 'DISPLAY=:0')
        if ($CommandArgs[0] -eq 'pulse-series') {
            $wslArgs += @('/bin/bash', '../tools/emulator-pulse.sh', $CommandArgs[1], $CommandArgs[2])
        } else {
            $wslArgs += @('/opt/ninetime-cli/bin/pebble', $CommandArgs[0], '--emulator', 'emery')
            $wslArgs += ($CommandArgs | Select-Object -Skip 1)
        }
        $start.Arguments = $wslArgs -join ' '
        $start.WorkingDirectory = (Join-Path $PSScriptRoot '..\pebble')
        $start.UseShellExecute = $false
        $start.CreateNoWindow = $true
        $start.RedirectStandardOutput = $true
        $start.RedirectStandardError = $true
        $process = [Diagnostics.Process]::Start($start)
        $script:pending = @{
            Process = $process; Output = $process.StandardOutput.ReadToEndAsync()
            Error = $process.StandardError.ReadToEndAsync(); Title = $Title; Started = [DateTime]::UtcNow
        }
        $commands.Enabled = $false
        $status.Text = "Küldés: $Title"
    } catch {
        $status.Text = "Indítási hiba: $($_.Exception.Message)"
    }
}

function Add-Heading([string]$Text, [int]$Y) {
    $label = [Windows.Forms.Label]::new()
    $label.SetBounds(20, $Y, 380, 24)
    $label.Text = $Text
    $label.Font = [Drawing.Font]::new($form.Font, [Drawing.FontStyle]::Bold)
    $commands.Controls.Add($label)
}

function New-PanelButton([string]$Text, [int]$X, [int]$Y, [int]$Width) {
    $button = [Windows.Forms.Button]::new()
    $button.SetBounds($X, $Y, $Width, 36)
    $button.Text = $Text
    $commands.Controls.Add($button)
    return $button
}

function Add-CommandButton([string]$Text, [int]$X, [int]$Y, [int]$Width, [string[]]$CommandArgs) {
    $button = New-PanelButton $Text $X $Y $Width
    $button.Tag = $CommandArgs
    $button.Add_Click({ param($sender, $eventArgs)
        Start-EmulatorCommand -Title $sender.Text -CommandArgs $sender.Tag
    })
}

Add-Heading 'Óragombok' 16
Add-CommandButton 'Vissza' 20 44 90 @('emu-button', 'click', 'back')
Add-CommandButton 'Fel' 118 44 90 @('emu-button', 'click', 'up')
Add-CommandButton 'Kiválasztás' 216 44 90 @('emu-button', 'click', 'select')
Add-CommandButton 'Le' 314 44 90 @('emu-button', 'click', 'down')

Add-Heading 'Események' 93
Add-CommandButton 'Rázás' 20 121 118 @('emu-tap')
Add-CommandButton 'Bluetooth be' 153 121 118 @('emu-bt-connection', '--connected', 'yes')
Add-CommandButton 'Bluetooth ki' 286 121 118 @('emu-bt-connection', '--connected', 'no')

Add-Heading 'Pulzus (BPM)' 170
$pulse = [Windows.Forms.NumericUpDown]::new()
$pulse.SetBounds(20, 204, 90, 30)
$pulse.Maximum = 255
$pulse.Value = 72
$commands.Controls.Add($pulse)
$quality = [Windows.Forms.ComboBox]::new()
$quality.SetBounds(118, 204, 188, 30)
$quality.DropDownStyle = 'DropDownList'
$quality.Items.AddRange(@('Kiváló jel', 'Gyenge jel', 'Levett óra'))
$quality.SelectedIndex = 0
$commands.Controls.Add($quality)
$pulseButton = New-PanelButton 'BPM küldése' 309 198 95
$pulseButton.Add_Click({
    $signal = @('excellent', 'poor', 'off-wrist')[$quality.SelectedIndex]
    $bpm = if ($signal -eq 'off-wrist') { '0' } else { [string][int]$pulse.Value }
    Start-EmulatorCommand -Title "Pulzus $bpm" -CommandArgs @('pulse-series', $signal, $bpm)
})

$stepsLabel = [Windows.Forms.Label]::new()
$stepsLabel.SetBounds(20, 252, 98, 30)
$stepsLabel.Text = 'Lépésszám'
$commands.Controls.Add($stepsLabel)
$steps = [Windows.Forms.NumericUpDown]::new()
$steps.SetBounds(118, 250, 188, 30)
$steps.Maximum = 1000000
$steps.Value = 1234
$commands.Controls.Add($steps)
$stepsButton = New-PanelButton 'Küldés' 309 244 95
$stepsButton.Add_Click({
    $count = [string][int]$steps.Value
    Start-EmulatorCommand -Title "Lépésszám $count" -CommandArgs @('emu-steps', $count)
})

$timer = [Windows.Forms.Timer]::new()
$timer.Interval = 100
$timer.Add_Tick({
    $job = $script:pending
    if (!$job) { return }
    if (!$job.Process.HasExited) {
        if (([DateTime]::UtcNow - $job.Started).TotalSeconds -le 60) { return }
        $job.Process.Kill()
        $status.Text = 'Az emulátor nem válaszolt 60 másodpercen belül.'
    } elseif ($job.Process.ExitCode -eq 0) {
        $status.Text = "Elküldve: $($job.Title)"
    } else {
        $message = ($job.Error.Result + "`n" + $job.Output.Result) -split '\r?\n' |
            Where-Object { $_.Trim() } | Select-Object -First 1
        $status.Text = "Hiba: $message"
    }
    $job.Process.Dispose()
    $script:pending = $null
    $commands.Enabled = $true
})

if ($Check) {
    Start-EmulatorCommand -Title 'Emery kapcsolat' -CommandArgs @('screenshot', '--no-open', 'build/panel-check.png')
    $timer.Start()
    while ($script:pending) {
        [Windows.Forms.Application]::DoEvents()
        Start-Sleep -Milliseconds 60
    }
    $timer.Dispose()
    $result = $status.Text
    $form.Dispose()
    if ($result -ne 'Elküldve: Emery kapcsolat') { throw $result }
    Write-Output 'Emery panel ready. SDK screenshot passed.'
    return
}
$timer.Start()
$form.Add_Shown({ $form.Activate() })
try {
    [void]$form.ShowDialog()
} finally {
    $timer.Dispose()
    if ($script:pending) { $script:pending.Process.Dispose() }
    $form.Dispose()
}

