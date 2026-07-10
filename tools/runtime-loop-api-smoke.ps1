param(
    [string]$BaseUrl = "http://127.0.0.1:29100"
)

$ErrorActionPreference = "Stop"

$health = Invoke-RestMethod -Method Get -Uri "$BaseUrl/api/health"
if ($health.status -ne "ok") {
    throw "Phoenix health check failed."
}

$requestId = "runtime-loop-smoke-$([DateTimeOffset]::UtcNow.ToUnixTimeMilliseconds())"
$valid = @{
    requestId = $requestId
    startTime = 1.0
    endTime = 2.0
}

$invalid = @{
    requestId = "$requestId-invalid"
    startTime = 2.0
    endTime = 1.0
}

try {
    Invoke-RestMethod -Method Put -Uri "$BaseUrl/api/runtime/loop" -ContentType "application/json" -Body ($invalid | ConvertTo-Json -Depth 4) | Out-Null
    throw "Invalid runtime loop payload was accepted."
}
catch {
    if ($_.Exception.Response.StatusCode.value__ -ne 400) {
        throw
    }
}

$result = Invoke-RestMethod -Method Put -Uri "$BaseUrl/api/runtime/loop" -ContentType "application/json" -Body ($valid | ConvertTo-Json -Depth 4)
if ($result.ok -ne $true -or $result.requestId -ne $requestId -or $result.startTime -ne 1.0 -or $result.endTime -ne 2.0) {
    throw "Valid runtime loop payload was rejected or echoed incorrectly."
}

Write-Host "Runtime loop API smoke test passed."
