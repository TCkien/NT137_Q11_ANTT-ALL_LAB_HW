
#https://learn.microsoft.com/en-us/powershell/scripting/learn/deep-dives/everything-about-arrays?view=powershell-7.5
$Registry_path = @(
    'HKLM:\Software\Microsoft\Windows\CurrentVersion\Run'
    'HKCU:\Software\Microsoft\Windows\CurrentVersion\Run'
    'HKLM:\Software\Microsoft\Windows\CurrentVersion\RunOnce'
    'HKCU:\Software\Microsoft\Windows\CurrentVersion\RunOnce'
)

Write-Host "1.Listing key" -ForegroundColor Blue
#https://stackoverflow.com/questions/66528639/how-to-iterate-through-an-array-of-objects-in-powershell
ForEach ($Path in $Registry_path){
    #https://stackoverflow.com/questions/2038181/how-to-output-something-in-powershell
    Write-Host "Listing all key name and value in $Path" -ForegroundColor Yellow
    Write-Host "Start---------------------------------------------"
    try{
        #https://www.reddit.com/r/PowerShell/comments/9lpsmo/how_to_use_invokecommand_for_getitemproperty/
        Get-ItemProperty -Path $Path
    }
    catch{
        Write-Host "Path $Path not found"
    }
    Write-Host "End---------------------------------------------"
}

Write-Host "2.ScheduleTasks" -ForegroundColor Blue
#Check Schedule Task
Get-ScheduledTask | Format-Table

Write-Host "3.Service" -ForegroundColor Blue
#Check Service
Get-Service | Format-Table

Write-Host "4.WMI" -ForegroundColor Blue
Write-Host "WMI filters" -ForegroundColor Yellow
# List all WMI event filters
Get-CimInstance -Namespace root\Subscription -ClassName __EventFilter

Write-Host "WMI consumers" -ForegroundColor Yellow
# List all WMI event consumers
Get-CimInstance -Namespace root\Subscription -ClassName __EventConsumer

Write-Host "WMI consumers to filterss" -ForegroundColor Yellow
# List all WMI filter-to-consumer bindings
Get-CimInstance -Namespace root\Subscription -ClassName __FilterToConsumerBinding

Write-Host "5.All startup folder" -ForegroundColor Blue
#All user startup folders
Get-ChildItem -Path "C:\ProgramData\Microsoft\Windows\Start Menu\Programs\StartUp"

Write-Host "6.Local user logons" -ForegroundColor Blue
Get-LocalUser

Write-Host "7.All change file " -ForegroundColor Blue
Get-ChildItem -Path "C:\" -File | Where-Object {
  $_.LastWriteTime -gt (Get-Date).AddDays(-1) 
}
