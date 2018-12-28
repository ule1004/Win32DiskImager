#pragma once
#include <cstdio>
#include <cstdlib>
#include <windows.h>
#include <winioctl.h>

// Define
#ifndef WINVER
#define WINVER 0x0500
#endif

#ifndef FSCTL_IS_VOLUME_MOUNTED
#define FSCTL_IS_VOLUME_MOUNTED         CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 10, METHOD_BUFFERED, FILE_ANY_ACCESS)
#endif // FSCTL_IS_VOLUME_MOUNTED

#define IOCTL_STORAGE_QUERY_PROPERTY   CTL_CODE(IOCTL_STORAGE_BASE, 0x0500, METHOD_BUFFERED, FILE_ANY_ACCESS)

// struct
typedef struct _DEVICE_NUMBER
{
	DEVICE_TYPE  DeviceType;
	ULONG  DeviceNumber;
	ULONG  PartitionNumber;
} DEVICE_NUMBER, *PDEVICE_NUMBER;

// CDisk

class CDisk : public CWnd
{
	DECLARE_DYNAMIC(CDisk)

public:
	CDisk();
	virtual ~CDisk();

protected:
	DECLARE_MESSAGE_MAP()
public:
	HANDLE getHandleOnFile(char* filelocation, DWORD access);
	HANDLE getHandleOnDevice(int device, DWORD access);
	HANDLE getHandleOnVolume(int volume, DWORD access);
	bool getLockOnVolume(HANDLE handle);
	bool removeLockOnVolume(HANDLE handle);
	bool unmountVolume(HANDLE handle);
	bool isVolumeUnmounted(HANDLE handle);
	char* readSectorDataFromHandle(HANDLE handle, unsigned long long startsector, unsigned long long numsectors, unsigned long long sectorsize);
	bool writeSectorDataToHandle(HANDLE handle, char *data, unsigned long long startsector, unsigned long long numsectors, unsigned long long sectorsize);
	unsigned long long getNumberOfSectors(HANDLE handle, unsigned long long *sectorsize);
	unsigned long long getFileSizeInSectors(HANDLE handle, unsigned long long sectorsize);
	bool spaceAvailable(char* location, unsigned long long spaceneeded);
	CString getDriveLabel(const char *drv);
	BOOL GetDisksProperty(HANDLE hDevice, PSTORAGE_DEVICE_DESCRIPTOR pDevDesc, DEVICE_NUMBER *devInfo);
	bool slashify(char *str, char **slash, char **noSlash);
	bool checkDriveType(char *name, ULONG *pid);
};


