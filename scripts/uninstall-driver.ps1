$drivers = get-wmiobject win32_PnpsignedDriver | where {$_.DeviceName -like "*muse*"}

foreach($driverInfo in $drivers) {
    echo $driverInfo.ClassGuid
    pnputil /remove-device $driverInfo.DeviceID
    pnputil /delete-driver $driverInfo.InfName /force
}
