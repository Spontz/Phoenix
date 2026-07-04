param(
    [string]$BaseUrl = "http://127.0.0.1:29100"
)

$ErrorActionPreference = "Stop"

$health = Invoke-RestMethod -Method Get -Uri "$BaseUrl/api/health"
if ($health.status -ne "ok") {
    throw "Phoenix health check failed."
}

$current = Invoke-RestMethod -Method Get -Uri "$BaseUrl/api/graphics"
if ($current.ok -ne $true -or -not $current.config) {
    throw "Phoenix returned an invalid graphics config."
}

$config = $current.config
$config.requestId = "graphics-smoke-$([DateTimeOffset]::UtcNow.ToUnixTimeMilliseconds())"

$invalid = $config | ConvertTo-Json -Depth 8 | ConvertFrom-Json
$invalid.fbos[0].format = "INVALID"

try {
    Invoke-RestMethod -Method Put -Uri "$BaseUrl/api/graphics" -ContentType "application/json" -Body ($invalid | ConvertTo-Json -Depth 8) | Out-Null
    throw "Invalid graphics payload was accepted."
}
catch {
    if ($_.Exception.Response.StatusCode.value__ -ne 400) {
        throw
    }
}

$result = Invoke-RestMethod -Method Put -Uri "$BaseUrl/api/graphics" -ContentType "application/json" -Body ($config | ConvertTo-Json -Depth 8)
if ($result.ok -ne $true) {
    throw "Valid graphics payload was rejected."
}

Write-Host "Graphics API smoke test passed."
