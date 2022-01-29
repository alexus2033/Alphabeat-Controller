Set-ExecutionPolicy -ExecutionPolicy Unrestricted -Scope CurrentUser

## Change Device-Name of Arduino Pro Micro
## Version 1.0 created 29.01.2022 by Alexus2033

#try to find boards.txt in update-packages...
$file = get-Item $env:LOCALAPPDATA\Arduino*\packages\arduino\hardware\avr\*\boards.txt

If($file -eq $null){
    #... or search for the boards.txt in the program-folder
    $file = get-item ${env:ProgramFiles(x86)}\Arduino\hardware\arduino\avr\boards.txt
}

#search for the section micro.name=Arduino Micro and change the following line only: 
#-> micro.build.usb_product="Arduino Micro"

$oldName="Arduino Micro"
$newName="Alphabeat-Mixxa"

$content = Get-Content $file
if($content | Select-String ".usb_product=""$newName"""){
    "Name already exists in boads.txt"
    "nothing to do"
}
elseif($content | Select-String ".usb_product=""$oldName"""){
    #replace name in file
    $content | ForEach-Object {$_ -replace ".usb_product=""$oldName""", ".usb_product=""$newName"""} | Set-Content $file
    "name was changed successfully"
} else {
    "Error: Device-Name not found in boards.txt"
}
