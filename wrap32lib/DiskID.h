#pragma once

#include <intrin.h>
#include <winioctl.h>
#include "wrap32lib.h"

#define  MAX_IDE_DRIVES  16

/*
	DiskID id;
	char buff[1024];
	id.GetFirstDriveSerialNumber(buff);
*/

class DiskID {
	char* flipAndCodeBytes(const char* str, int pos, int flip, char* buf)
	{
		int i;
		int j = 0;
		int k = 0;

		buf[0] = '\0';
		if (pos <= 0)
			return buf;

		if (!j)
		{
			char p = 0;


			// First try to gather all characters representing hex digits only.
			j = 1;
			k = 0;
			buf[k] = 0;
			for (i = pos; j && str[i] != '\0'; ++i)
			{
				char c = (char)tolower(str[i]);

				if (isspace(c))
					c = '0';

				++p;
				buf[k] <<= 4;

				if (c >= '0' && c <= '9')
					buf[k] |= (unsigned char)(c - '0');
				else if (c >= 'a' && c <= 'f')
					buf[k] |= (unsigned char)(c - 'a' + 10);
				else
				{
					j = 0;
					break;
				}

				if (p == 2)
				{
					if (buf[k] != '\0' && (k < 0x20 || k > 0x7f))
					{
						j = 0;
						break;
					}
					++k;
					p = 0;
					buf[k] = 0;
				}

			}
		}

		if (!j)
		{
			// There are non-digit characters, gather them as is.
			j = 1;
			k = 0;
			for (i = pos; j && str[i] != '\0'; ++i)
			{
				char c = str[i];

				if (!isprint(c))
				{
					j = 0;
					break;
				}

				buf[k++] = c;
			}
		}

		if (!j)
		{
			// The characters are not there or are not printable.
			k = 0;
		}

		buf[k] = '\0';

		if (flip)
			// Flip adjacent characters
			for (j = 0; j < k; j += 2)
			{
				char t = buf[j];
				buf[j] = buf[j + 1];
				buf[j + 1] = t;
			}

		// Trim any beginning and end space
		i = j = -1;
		for (k = 0; buf[k] != '\0'; ++k)
		{
			if (!isspace(buf[k]))
			{
				if (i < 0)
					i = k;
				j = k;
			}
		}

		if ((i >= 0) && (j >= 0))
		{
			for (k = i; (k <= j) && (buf[k] != '\0'); ++k)
				buf[k - i] = buf[k];
			buf[k - i] = '\0';
		}

		return buf;
	}

public:
	bool GetFirstDriveSerialNumber(char* serialnumber)
	{
		char local_buffer[10000];
		for (int drive = 0; drive < MAX_IDE_DRIVES; drive++)
		{
			//  Try to get a handle to PhysicalDrive IOCTL, report failure
			//  and exit if can't.
			wchar_t driveName[256];
			wsprintf(driveName, L"\\\\.\\PhysicalDrive%d", drive);

			//  Windows NT, Windows 2000, Windows XP - admin rights not required
			HANDLE hPhysicalDriveIOCTL = CreateFile(driveName, 0,
				FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
				OPEN_EXISTING, 0, NULL);

			if (hPhysicalDriveIOCTL == INVALID_HANDLE_VALUE)
				continue;

			STORAGE_PROPERTY_QUERY query;
			DWORD cbBytesReturned = 0;

			memset((void*)&query, 0, sizeof(query));
			query.PropertyId = StorageDeviceProperty;
			query.QueryType = PropertyStandardQuery;

			memset(local_buffer, 0, sizeof(local_buffer));

			if (DeviceIoControl(hPhysicalDriveIOCTL, IOCTL_STORAGE_QUERY_PROPERTY,
				&query,
				sizeof(query),
				&local_buffer[0],
				sizeof(local_buffer),
				&cbBytesReturned, NULL))
			{
				STORAGE_DEVICE_DESCRIPTOR* descrip = (STORAGE_DEVICE_DESCRIPTOR*)&local_buffer;

/*				flipAndCodeBytes(local_buffer,
					descrip->VendorIdOffset,
					0, serialnumber);
				flipAndCodeBytes(local_buffer,
					descrip->ProductIdOffset,
					0, serialnumber);
				flipAndCodeBytes(local_buffer,
					descrip->ProductRevisionOffset,
					0, serialnumber);*/
				flipAndCodeBytes(local_buffer,
					descrip->SerialNumberOffset,
					1, serialnumber);

				if (isalnum(serialnumber[0])) {
					CloseHandle(hPhysicalDriveIOCTL);
					return true;
					// This means that a legitimate hard disk serial number has been obtained.
					// The serial number of the hard disk is in the serialNumber array
					// The hard disk model is in the modelNumber array
					// Hard disk manufacturer number in vendorId
					// Hard disk firmware version number in product review
				}

			}
			CloseHandle(hPhysicalDriveIOCTL);
		}
		return false;
	}
};