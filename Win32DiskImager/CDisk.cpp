﻿// CDisk.cpp: 구현 파일
//

#include "stdafx.h"
#include "Win32DiskImager.h"
#include "CDisk.h"


// CDisk

IMPLEMENT_DYNAMIC(CDisk, CWnd)

CDisk::CDisk()
{

}

CDisk::~CDisk()
{
}


BEGIN_MESSAGE_MAP(CDisk, CWnd)
END_MESSAGE_MAP()



// CDisk 메시지 처리기




HANDLE CDisk::getHandleOnFile(char* filelocation, DWORD access)
{
	HANDLE hFile;
	hFile = CreateFile(filelocation, access, 0, NULL, (access == GENERIC_READ) ? OPEN_EXISTING : CREATE_ALWAYS, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		wchar_t *errormessage = NULL;
		::FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0,
			(LPWSTR)&errormessage, 0, NULL);
		CString errText(errormessage);
		MessageBox(errText, _T("File Error"), MB_ICONEXCLAMATION | MB_ICONWARNING);
		// QMessageBox::critical(NULL, CObject::tr("File Error"), QObject::tr("An error occurred when attempting to get a handle on the file.\n Error %1: %2").arg(GetLastError()).arg(errText));
		LocalFree(errormessage);
	}
	return hFile;
}


HANDLE CDisk::getHandleOnDevice(int device, DWORD access)
{
	HANDLE hDevice;
	CString devicename;
	devicename.Format(_T("\\\\.\\PhysicalDrive%d"), device);
	hDevice = CreateFile(devicename, access, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		wchar_t *errormessage = NULL;
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPWSTR)&errormessage, 0, NULL);
		CString errText(errormessage);
		MessageBox(errText, _T("Device Error"), MB_ICONEXCLAMATION | MB_ICONWARNING);
		// QMessageBox::critical(NULL, QObject::tr("Device Error"), QObject::tr("An error occurred when attempting to get a handle on the device.\n Error %1: %2").arg(GetLastError()).arg(errText));
		LocalFree(errormessage);
	}
	return hDevice;
}


HANDLE CDisk::getHandleOnVolume(int volume, DWORD access)
{
	HANDLE hVolume;
	char volumename[] = "\\\\.\\A:";
	volumename[4] += volume;

	hVolume = CreateFile(volumename, access, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hVolume == INVALID_HANDLE_VALUE)
	{
		wchar_t *errormessage = NULL;
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPWSTR)&errormessage, 0, NULL);
		CString errText(errormessage);
		MessageBox(errText, _T("Volume Error"), MB_ICONEXCLAMATION | MB_ICONWARNING);
		// QMessageBox::critical(NULL, QObject::tr("Volume Error"), QObject::tr("An error occurred when attempting to get a handle on the volume.\n Error %1: %2").arg(GetLastError()).arg(errText));
		LocalFree(errormessage);
	}
	return hVolume;
}


bool CDisk::getLockOnVolume(HANDLE handle)
{
	DWORD bytesreturned;
	BOOL bResult;
	bResult = DeviceIoControl(handle, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &bytesreturned, NULL);
	if (!bResult)
	{
		wchar_t *errormessage = NULL;
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPWSTR)&errormessage, 0, NULL);
		CString errText(errormessage);;
		MessageBox(errText, _T("Lock Error"), MB_ICONEXCLAMATION | MB_ICONWARNING);
		// QMessageBox::critical(NULL, QObject::tr("Lock Error"), QObject::tr("An error occurred when attempting to lock the volume.\n Error %1: %2").arg(GetLastError()).arg(errText));
		LocalFree(errormessage);
	}
	return (bResult);
}


bool CDisk::removeLockOnVolume(HANDLE handle)
{
	DWORD junk;
	BOOL bResult;
	bResult = DeviceIoControl(handle, FSCTL_UNLOCK_VOLUME, NULL, 0, NULL, 0, &junk, NULL);
	if (!bResult)
	{
		wchar_t *errormessage = NULL;
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPWSTR)&errormessage, 0, NULL);
		CString errText(errormessage);
		MessageBox(errText, _T("Unlock Error"), MB_ICONEXCLAMATION | MB_ICONWARNING);
		// QMessageBox::critical(NULL, QObject::tr("Unlock Error"), QObject::tr("An error occurred when attempting to unlock the volume.\n Error %1: %2").arg(GetLastError()).arg(errText));
		LocalFree(errormessage);
	}
	return (bResult);
}


bool CDisk::unmountVolume(HANDLE handle)
{
	DWORD junk;
	BOOL bResult;
	bResult = DeviceIoControl(handle, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &junk, NULL);
	if (!bResult)
	{
		wchar_t *errormessage = NULL;
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPWSTR)&errormessage, 0, NULL);
		CString errText(errormessage);
		MessageBox(errText, _T("Dismount Error"), MB_ICONEXCLAMATION | MB_ICONWARNING);
		// QMessageBox::critical(NULL, QObject::tr("Dismount Error"), QObject::tr("An error occurred when attempting to dismount the volume.\n Error %1: %2").arg(GetLastError()).arg(errText));
		LocalFree(errormessage);
	}
	return (bResult);
}


bool CDisk::isVolumeUnmounted(HANDLE handle)
{
	DWORD junk;
	BOOL bResult;
	bResult = DeviceIoControl(handle, FSCTL_IS_VOLUME_MOUNTED, NULL, 0, NULL, 0, &junk, NULL);
	return (!bResult);
}


char* CDisk::readSectorDataFromHandle(HANDLE handle, unsigned long long startsector, unsigned long long numsectors, unsigned long long sectorsize)
{
	unsigned long bytesread;
	char *data = new char[sectorsize * numsectors];
	LARGE_INTEGER li;
	li.QuadPart = startsector * sectorsize;
	SetFilePointer(handle, li.LowPart, &li.HighPart, FILE_BEGIN);
	if (!ReadFile(handle, data, sectorsize * numsectors, &bytesread, NULL))
	{
		wchar_t *errormessage = NULL;
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPWSTR)&errormessage, 0, NULL);
		CString errText(errormessage);
		MessageBox(errText, _T("Read Error"), MB_ICONEXCLAMATION | MB_ICONWARNING);
		// QMessageBox::critical(NULL, QObject::tr("Read Error"), QObject::tr("An error occurred when attempting to read data from handle.\n Error %1: %2").arg(GetLastError()).arg(errText));
		LocalFree(errormessage);
		delete data;
		data = NULL;
	}
	return data;
}


bool CDisk::writeSectorDataToHandle(HANDLE handle, char *data, unsigned long long startsector, unsigned long long numsectors, unsigned long long sectorsize)
{
	unsigned long byteswritten;
	BOOL bResult;
	LARGE_INTEGER li;
	li.QuadPart = startsector * sectorsize;
	SetFilePointer(handle, li.LowPart, &li.HighPart, FILE_BEGIN);
	bResult = WriteFile(handle, data, sectorsize * numsectors, &byteswritten, NULL);
	if (!bResult)
	{
		wchar_t *errormessage = NULL;
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPWSTR)&errormessage, 0, NULL);
		CString errText(errormessage);
		MessageBox(errText, _T("Write Error"), MB_ICONEXCLAMATION | MB_ICONWARNING);
		// QMessageBox::critical(NULL, QObject::tr("Write Error"), QObject::tr("An error occurred when attempting to write data to handle.\n Error %1: %2").arg(GetLastError()).arg(errText));
		LocalFree(errormessage);
	}
	return (bResult);
}


unsigned long long CDisk::getNumberOfSectors(HANDLE handle, unsigned long long *sectorsize)
{
	DWORD junk;
	DISK_GEOMETRY_EX diskgeometry;
	BOOL bResult;
	bResult = DeviceIoControl(handle, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0, &diskgeometry, sizeof(diskgeometry), &junk, NULL);
	if (!bResult)
	{
		wchar_t *errormessage = NULL;
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPWSTR)&errormessage, 0, NULL);
		CString errText(errormessage);
		MessageBox(errText, _T("Device Error"), MB_ICONEXCLAMATION | MB_ICONWARNING);
		// QMessageBox::critical(NULL, QObject::tr("Device Error"), QObject::tr("An error occurred when attempting to get the device's geometry.\n Error %1: %2").arg(GetLastError()).arg(errText));
		LocalFree(errormessage);
		return 0;
	}
	if (sectorsize != NULL)
	{
		*sectorsize = (unsigned long long)diskgeometry.Geometry.BytesPerSector;
	}
	return (unsigned long long)diskgeometry.DiskSize.QuadPart / (unsigned long long)diskgeometry.Geometry.BytesPerSector;
}


unsigned long long CDisk::getFileSizeInSectors(HANDLE handle, unsigned long long sectorsize)
{
	unsigned long long retVal = 0;
	LARGE_INTEGER filesize;
	if (GetFileSizeEx(handle, &filesize) == 0)
	{
		// error
		wchar_t *errormessage = NULL;
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPWSTR)&errormessage, 0, NULL);
		CString errText(errormessage);
		MessageBox(errText, _T("Lock Error"), MB_ICONEXCLAMATION | MB_ICONWARNING);
		// QMessageBox::critical(NULL, QObject::tr("File Error"), QObject::tr("An error occurred while getting the file size.\n Error %1: %2").arg(GetLastError()).arg(errText));
		LocalFree(errormessage);
		retVal = 0;
	}
	else
	{
		retVal = ((unsigned long long)filesize.QuadPart / sectorsize);
	}
	return(retVal);
}


bool CDisk::spaceAvailable(char* location, unsigned long long spaceneeded)
{
	ULARGE_INTEGER freespace;
	BOOL bResult;
	bResult = GetDiskFreeSpaceEx(location, NULL, NULL, &freespace);
	// bResult = GetDiskFreeSpaceEx("C:\\", NULL, NULL, &freespace);
	if (!bResult)
	{
		wchar_t *errormessage = NULL;
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPWSTR)&errormessage, 0, NULL);
		CString errText(errormessage);
		MessageBox(errText, _T("Free Space Error"), MB_ICONEXCLAMATION | MB_ICONWARNING);
		// QMessageBox::critical(NULL, QObject::tr("Free Space Error"), QObject::tr("Failed to get the free space on drive %1.\n Error %2: %3\n Checking of free space will be skipped.").arg(location).arg(GetLastError()).arg(errText));
		return false;
	}
	return (spaceneeded <= freespace.QuadPart);
}


// given a drive letter (ending in a slash), return the label for that drive
// TODO make this more robust by adding input verification
CString CDisk::getDriveLabel(const char *drv)
{
	CString retVal;
	int szNameBuf = MAX_PATH + 1;
	char *nameBuf = NULL;
	if ((nameBuf = (char *)calloc(szNameBuf, sizeof(char))) != 0)
	{
		::GetVolumeInformationA(drv, nameBuf, szNameBuf, NULL,
			NULL, NULL, NULL, 0);
	}

	// if malloc fails, nameBuf will be NULL.
	//   if GetVolumeInfo fails, nameBuf will contain empty string
	//   if all succeeds, nameBuf will contain label
	if (nameBuf == NULL)
	{
		retVal = CString("");
	}
	else
	{
		retVal = CString(nameBuf);
		free(nameBuf);
	}

	return(retVal);
}


BOOL CDisk::GetDisksProperty(HANDLE hDevice, PSTORAGE_DEVICE_DESCRIPTOR pDevDesc, DEVICE_NUMBER *devInfo)
{
	STORAGE_PROPERTY_QUERY Query; // input param for query
	DWORD dwOutBytes; // IOCTL output length
	BOOL bResult; // IOCTL return val
	BOOL retVal = true;
	DWORD cbBytesReturned;

	// specify the query type
	Query.PropertyId = StorageDeviceProperty;
	Query.QueryType = PropertyStandardQuery;

	// Query using IOCTL_STORAGE_QUERY_PROPERTY 
	bResult = ::DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
		&Query, sizeof(STORAGE_PROPERTY_QUERY), pDevDesc,
		pDevDesc->Size, &dwOutBytes, (LPOVERLAPPED)NULL);
	if (bResult)
	{
		bResult = ::DeviceIoControl(hDevice, IOCTL_STORAGE_GET_DEVICE_NUMBER,
			NULL, 0, devInfo, sizeof(DEVICE_NUMBER), &dwOutBytes,
			(LPOVERLAPPED)NULL);
		if (!bResult)
		{
			retVal = false;
			wchar_t *errormessage = NULL;
			FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPWSTR)&errormessage, 0, NULL);
			CString errText(errormessage);
			MessageBox(errText, _T("File Error"), MB_ICONEXCLAMATION | MB_ICONWARNING);
			// QMessageBox::critical(NULL, QObject::tr("File Error"), QObject::tr("An error occurred while getting the device number.\n This usually means something is currently accessing the device; please close all applications and try again.\n\nError %1: %2").arg(GetLastError()).arg(errText));
			LocalFree(errormessage);
		}
	}
	else
	{
		if (DeviceIoControl(hDevice, IOCTL_STORAGE_CHECK_VERIFY2, NULL, 0, NULL, 0, &cbBytesReturned,
			(LPOVERLAPPED)NULL))
		{
			wchar_t *errormessage = NULL;
			FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPWSTR)&errormessage, 0, NULL);
			CString errText(errormessage);
			MessageBox(errText, _T("File Error"), MB_ICONEXCLAMATION | MB_ICONWARNING);
			// QMessageBox::critical(NULL, QObject::tr("File Error"), QObject::tr("An error occurred while querying the properties.\n This usually means something is currently accessing the device; please close all applications and try again.\n\nError %1: %2").arg(GetLastError()).arg(errText));
			LocalFree(errormessage);
		}
		retVal = false;
	}

	return(retVal);
}


// some routines fail if there's no trailing slash in a name,
// 		others fail if there is.  So this routine takes a name (trailing
// 		slash or no), and creates 2 versions - one with the slash, and one w/o
//
// 		CALLER MUST FREE THE 2 RETURNED STRINGS
bool CDisk::slashify(char *str, char **slash, char **noSlash)
{
	bool retVal = false;
	int strLen = strlen(str);
	if (strLen > 0)
	{
		if (*(str + strLen - 1) == '\\')
		{
			// trailing slash exists
			if (((*slash = (char *)calloc((strLen + 1), sizeof(char))) != NULL) &&
				((*noSlash = (char *)calloc(strLen, sizeof(char))) != NULL))
			{
				strncpy(*slash, str, strLen);
				strncpy(*noSlash, *slash, (strLen - 1));
				retVal = true;
			}
		}
		else
		{
			// no trailing slash exists
			if (((*slash = (char *)calloc((strLen + 2), sizeof(char))) != NULL) &&
				((*noSlash = (char *)calloc((strLen + 1), sizeof(char))) != NULL))
			{
				strncpy(*noSlash, str, strLen);
				sprintf(*slash, "%s\\", *noSlash);
				retVal = true;
			}
		}
	}
	return(retVal);
}


bool CDisk::checkDriveType(char *name, ULONG *pid)
{
	HANDLE hDevice;
	PSTORAGE_DEVICE_DESCRIPTOR pDevDesc;
	DEVICE_NUMBER deviceInfo;
	bool retVal = false;
	char *nameWithSlash;
	char *nameNoSlash;
	int driveType;
	DWORD cbBytesReturned;

	// some calls require no tailing slash, some require a trailing slash...
	if (!(slashify(name, &nameWithSlash, &nameNoSlash)))
	{
		return(retVal);
	}

	driveType = GetDriveType(nameWithSlash);
	switch (driveType)
	{
	case DRIVE_REMOVABLE: // The media can be removed from the drive.
	case DRIVE_FIXED:     // The media cannot be removed from the drive.
		hDevice = CreateFile(nameNoSlash, FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if (hDevice == INVALID_HANDLE_VALUE)
		{
			wchar_t *errormessage = NULL;
			FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), 0, (LPWSTR)&errormessage, 0, NULL);
			CString errText(errormessage);
			MessageBox(errText, _T("Volume Error"), MB_ICONEXCLAMATION | MB_ICONWARNING);
			// QMessageBox::critical(NULL, QObject::tr("Volume Error"), QObject::tr("An error occurred when attempting to get a handle on %3.\n Error %1: %2").arg(GetLastError()).arg(errText).arg(nameWithSlash));
			LocalFree(errormessage);
		}
		else
		{
			int arrSz = sizeof(STORAGE_DEVICE_DESCRIPTOR) + 512 - 1;
			pDevDesc = (PSTORAGE_DEVICE_DESCRIPTOR)new BYTE[arrSz];
			pDevDesc->Size = arrSz;

			// get the device number if the drive is
			// removable or (fixed AND on the usb bus, SD, or MMC (undefined in XP/mingw))
			if (GetDisksProperty(hDevice, pDevDesc, &deviceInfo) &&
				(((driveType == DRIVE_REMOVABLE) && (pDevDesc->BusType != BusTypeSata))
					|| ((driveType == DRIVE_FIXED) && ((pDevDesc->BusType == BusTypeUsb) || (pDevDesc->BusType == 0xC) || (pDevDesc->BusType == 0xD)))))
			{
				// ensure that the drive is actually accessible
				// multi-card hubs were reporting "removable" even when empty
				if (DeviceIoControl(hDevice, IOCTL_STORAGE_CHECK_VERIFY2, NULL, 0, NULL, 0, &cbBytesReturned, (LPOVERLAPPED)NULL))
				{
					*pid = deviceInfo.DeviceNumber;
					retVal = true;
				}
			}

			delete pDevDesc;
			CloseHandle(hDevice);
		}

		break;
	default:
		retVal = false;
	}

	// free the strings allocated by slashify
	free(nameWithSlash);
	free(nameNoSlash);

	return(retVal);
}
