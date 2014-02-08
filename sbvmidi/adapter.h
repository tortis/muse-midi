#include "portcls.h"
#include "ksdebug.h"
#include "dmusicks.h"


#define MAX_MINIPORTS 1
NTSTATUS
CreateMiniportDMusUART
(
OUT     PUNKNOWN *  Unknown,
IN      REFCLSID,
IN      PUNKNOWN    UnknownOuter    OPTIONAL,
_When_((PoolType & NonPagedPoolMustSucceed) != 0,
__drv_reportError("Must succeed pool allocations are forbidden. "
"Allocation failures cause a system crash"))
IN      POOL_TYPE   PoolType
);

DRIVER_ADD_DEVICE AddDevice;

NTSTATUS
StartDevice
(
IN      PDEVICE_OBJECT  pDeviceObject,  // Context for the class driver.
IN      PIRP            pIrp,           // Context for the class driver.
IN      PRESOURCELIST   ResourceList    // List of hardware resources.
);

NTSTATUS
InstallSubdevice
(
_In_        PVOID               Context1,
_In_        PVOID               Context2,
_In_        PWSTR               Name,
_In_        REFGUID             PortClassId,
_In_        REFGUID             MiniportClassId,
_In_opt_    PUNKNOWN            UnknownAdapter,     //not used - null
_In_        PRESOURCELIST       ResourceList,       //not optional, but can be EMPTY!
_In_        REFGUID             PortInterfaceId,
_Out_opt_   PUNKNOWN *          OutPortInterface,   //not used - null
_Out_opt_   PUNKNOWN *          OutPortUnknown      //not used - null
);
NTSTATUS InstallSubdeviceVirtual(
	_In_        PVOID               Context1,
	_In_        PVOID               Context2,
	_In_        PWSTR               Name,
	_In_        REFGUID             PortClassId, //DirectMusic or MIDI
	_In_        PRESOURCELIST       ResourceList
	);
#define VLOG(...) { DbgPrint(__VA_ARGS__); DbgPrint("\n");}
