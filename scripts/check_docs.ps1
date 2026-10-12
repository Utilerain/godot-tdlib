[CmdletBinding()]
param(
    [string]$ManagerDocsPath = (Join-Path $PSScriptRoot "..\doc_classes\TdJsonManager.xml"),
    [string]$ClientDocsPath = (Join-Path $PSScriptRoot "..\doc_classes\TdJsonClient.xml"),
    [string]$ManagerSourcePath = (Join-Path $PSScriptRoot "..\src\tdjson_manager.cpp"),
    [string]$ClientSourcePath = (Join-Path $PSScriptRoot "..\src\tdjson_client.cpp"),
    [string]$GeneratedPath = ""
)

function Get-PublicMethodNames([string]$XmlPath) {
    [xml]$xml = Get-Content -Raw $XmlPath
    @(
        $xml.class.methods.method |
        Where-Object { $_.name -and $_.name -notlike "_*" } |
        ForEach-Object { [string]$_.name } |
        Sort-Object -Unique
    )
}

function Get-RegisteredMethodNames([string]$SourcePath) {
    @(
        Select-String -Path $SourcePath -Pattern 'D_METHOD\(\s*"([^"]+)"' -AllMatches |
        ForEach-Object { $_.Matches } |
        ForEach-Object { $_.Groups[1].Value } |
        Where-Object { $_ -notlike "_*" } |
        Sort-Object -Unique
    )
}

$keepGeneratedDocs = -not [string]::IsNullOrWhiteSpace($GeneratedPath)
$checks = @(
    @{ Name = "TdJsonManager"; Docs = $ManagerDocsPath; Source = $ManagerSourcePath },
    @{ Name = "TdJsonClient"; Docs = $ClientDocsPath; Source = $ClientSourcePath }
)
$allPassed = $true

foreach ($check in $checks) {
    $documentedMethods = @(Get-PublicMethodNames $check.Docs)
    $registeredMethods = @(Get-RegisteredMethodNames $check.Source)

    if ($registeredMethods.Count -eq 0) {
        throw "No registered methods found in $($check.Source)"
    }

    $comparison = Compare-Object `
        -ReferenceObject $documentedMethods `
        -DifferenceObject $registeredMethods

    $missing = @($comparison |
        Where-Object SideIndicator -eq '<=' |
        ForEach-Object InputObject)
    $extra = @($comparison |
        Where-Object SideIndicator -eq '=>' |
        ForEach-Object InputObject)

    if ($missing.Count -gt 0 -or $extra.Count -gt 0) {
        $allPassed = $false
        if ($missing.Count -gt 0) {
            Write-Error ("$($check.Name): documented but not registered: " + ($missing -join ", "))
        }
        if ($extra.Count -gt 0) {
            Write-Error ("$($check.Name): registered but not documented: " + ($extra -join ", "))
        }
    } else {
        Write-Host ("$($check.Name): documentation matches ($($documentedMethods.Count) public methods).")
    }
}

if (-not $allPassed) {
    exit 1
}

if ($keepGeneratedDocs) {
    $generatedDirectory = [System.IO.Path]::GetFullPath($GeneratedPath)
    New-Item -ItemType Directory -Path $generatedDirectory -Force | Out-Null
    "TdJsonManager and TdJsonClient documentation checks passed." |
        Set-Content -Path (Join-Path $generatedDirectory "check.txt")
}
