/*******************************************************************************
* Helper functions for Validity driver interface functions
*
* This file contains the Helper functions for Validity driver interface functions
* and their definitions
*
* Copyright 2006 Validity Sensors, Inc. 
*
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

#ifndef __vfsWrapper_h__
#define __vfsWrapper_h__

typedef int (*vfs_dev_init_t)(void *);
typedef int (*vfs_wait_for_service_t)(void);
typedef int (*vfs_set_matcher_type_t)(int);
typedef int (*vfs_capture_t)(void *, int);
typedef int (*vfs_get_img_datasize_t) (void *);
typedef int (*vfs_get_img_width_t) (void *);
typedef int (*vfs_get_img_height_t) (void *);
typedef unsigned char* (*vfs_get_img_data_t)(void *);
typedef void (*vfs_free_img_data_t)(unsigned char *);
typedef void (*vfs_clean_handles_t)(void *);
typedef void (*vfs_dev_exit_t)(void *);

#endif /*vfsWrapper */
