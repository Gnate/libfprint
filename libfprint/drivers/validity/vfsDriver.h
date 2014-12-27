/*******************************************************************************
* libfprint Interface Functions
*
* This file contains the libfprint interface functions and definitions
*
* Copyright 2006 Validity Sensors, Inc. 

* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#ifndef __vfsDriver_h__
#define __vfsDriver_h__

#define VCS_MAX_FINGER              10
#define VALIDITY_DRIVER_ID          10
#define VALIDITY_VENDOR_ID          0x138A
#define VALIDITY_PRODUCT_ID_301     0x0005  
#define VALIDITY_PRODUCT_ID_451     0x0007
#define VALIDITY_PRODUCT_ID_5111    0x0010
#define VALIDITY_PRODUCT_ID_5011    0x0011
#define VALIDITY_PRODUCT_ID_471	    0x003c
#define VALIDITY_PRODUCT_ID_5131    0x0018
#define VALIDITY_PRODUCT_ID_491	    0x003d
#define VALIDITY_DRIVER_FULLNAME    "Validity Sensors"
#define VALIDITY_FP_COMPONENT       "Validity"
#define MAX_ENROLLMENT_PRINTS	    3
#define MAX_TEMPLATES		        10
#define VFS_NT_MATCHER	    	    1
#define VFS_COGENT_MATCHER	    2
#define VFS_FPRINT_MATCHER     3

#define VFS_RESULT_WRAPPER_OK 0

enum fp_capture_result
{
    FP_CAPTURE_ERROR = -1,
    FP_CAPTURE_COMPLETE = 1,
    FP_CAPTURE_FAIL
};

typedef struct validity_dev_s 
{
    void *pValidityCtx;
    void *hImage;
    void *pEnrollData;
}validity_dev;

#endif //__vfsDriver_h__
