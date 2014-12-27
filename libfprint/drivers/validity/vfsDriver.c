

/*! @file vfsDriver.c
*******************************************************************************
* libfprint Interface Functions
*
* This file contains the libfprint interface functions for validity fingerprint sensor device.
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

#include <errno.h>
#include <string.h>
#include <glib.h>
#include <usb.h>
#include <fp_internal.h>
#include <stdio.h>
#include <dlfcn.h>
#include "vfsDriver.h"
#include "vfsWrapper.h"
#include <syslog.h>

/* Maximum image height */
#define VFS_IMG_MAX_HEIGHT	1023

/* Minimum image height */
#define VFS_IMG_MIN_HEIGHT	200

/* Number of enroll stages */
#define VFS_NR_ENROLL		1

#define VAL_MIN_ACCEPTABLE_MINUTIAE (2*MIN_ACCEPTABLE_MINUTIAE)
#define VAL_DEFAULT_THRESHOLD 60

static const struct usb_id id_table[] = {
    { .vendor = VALIDITY_VENDOR_ID, .product = VALIDITY_PRODUCT_ID_301,  },
    { .vendor = VALIDITY_VENDOR_ID, .product = VALIDITY_PRODUCT_ID_451,  },
    { .vendor = VALIDITY_VENDOR_ID, .product = VALIDITY_PRODUCT_ID_5111, },
    { .vendor = VALIDITY_VENDOR_ID, .product = VALIDITY_PRODUCT_ID_5011, },
    { .vendor = VALIDITY_VENDOR_ID, .product = VALIDITY_PRODUCT_ID_471,  },
    { .vendor = VALIDITY_VENDOR_ID, .product = VALIDITY_PRODUCT_ID_5131, },
    { .vendor = VALIDITY_VENDOR_ID, .product = VALIDITY_PRODUCT_ID_491,  },
    { 0, 0, 0, }, /* terminating entry */
};

#define VFS_ASSERT(_state, _message, _err) ({ \
	if (__builtin_expect(!(_state), 0)) \
	{ \
		fp_err((_message)); \
		fpi_imgdev_session_error(dev, (_err)); \
		if (img) \
		{ \
			fp_img_free(img); \
		} \
		result = (_err); \
		goto cleanup; \
	} \
})

static int vfs_extract_image(struct fp_img_dev *dev,
                             void *const handle,
                             struct fp_img *img,
                             const size_t data_len)
{
	validity_dev *vdev = dev->priv;
	unsigned char *data;

	vfs_get_img_width_t		vfs_get_img_width;
	vfs_get_img_height_t	vfs_get_img_height;
	vfs_get_img_data_t		vfs_get_img_data;
	vfs_free_img_data_t		vfs_free_img_data;

	vfs_get_img_width = dlsym(handle, "vfs_get_img_width");
	if (__builtin_expect(!vfs_get_img_width, 0))
	{
		fp_err(dlerror());
		return -ENODEV;
	}

	img->width =(* vfs_get_img_width)(vdev);

	vfs_get_img_height = dlsym(handle, "vfs_get_img_height");
	if (__builtin_expect(!vfs_get_img_height, 0))
	{
		fp_err(dlerror());
		return -ENODEV;
	}

	img->height =(*vfs_get_img_height)(vdev);

	fp_dbg("%d x %d image returned\n", img->width, img->height );

	vfs_get_img_data = dlsym(handle, "vfs_get_img_data");
	if (__builtin_expect(!vfs_get_img_data, 0))
	{
		fp_err(dlerror());
		return -ENODEV;
	}

	data =(*vfs_get_img_data)(vdev);

	if (data)
	{
		g_memmove(img->data, data, data_len);

		img->flags = FP_IMG_COLORS_INVERTED | FP_IMG_V_FLIPPED;

		vfs_free_img_data = dlsym(handle, "vfs_free_img_data");
		if (__builtin_expect(!vfs_free_img_data, 0))
		{
			fp_err(dlerror());
			return -ENODEV;
		}

		(*vfs_free_img_data)(data);
	}
	else
	{
		fp_err("Failed to get finger print image data");
		return -ENODATA;
	}

	return 0;
}

/* Activate device */
static int dev_activate(struct fp_img_dev *dev, enum fp_imgdev_state state)
{
	validity_dev *vdev = dev->priv;
	void *handle;
	size_t data_len;
	struct fp_img *img = NULL;
	int result = 0;

	vfs_wait_for_service_t	vfs_wait_for_service;
	vfs_set_matcher_type_t	vfs_set_matcher_type;
	vfs_dev_init_t			vfs_dev_init;
	vfs_capture_t			vfs_capture;
	vfs_get_img_datasize_t	vfs_get_img_datasize;
	vfs_clean_handles_t		vfs_clean_handles;
	vfs_dev_exit_t			vfs_dev_exit;

	/* Notify activate complete */
	fpi_imgdev_activate_complete(dev, 0);

	handle = dlopen("libvfsFprintWrapper.so",
					RTLD_LAZY | RTLD_GLOBAL | RTLD_NODELETE);
	VFS_ASSERT(handle, dlerror(), -ENODEV);

	/* wait for validity device to come up and be ready to take a finger swipe
	 * Wait will happen for a stipulated time(10s - 40s), then errors
	 */
	vfs_wait_for_service = dlsym(handle, "vfs_wait_for_service");
	VFS_ASSERT(vfs_wait_for_service, dlerror(), -ENODEV);

	result = (*vfs_wait_for_service)();
	VFS_ASSERT(result == VFS_RESULT_WRAPPER_OK,
			"VFS module failed to wait for service", -EPERM);

	/* Set the matcher type */
	vfs_set_matcher_type = dlsym(handle, "vfs_set_matcher_type");
	VFS_ASSERT(vfs_set_matcher_type, dlerror(), -ENODEV);

	(*vfs_set_matcher_type)(VFS_FPRINT_MATCHER);

	vfs_dev_init = dlsym(handle, "vfs_dev_init");
	VFS_ASSERT(vfs_dev_init, dlerror(), -ENODEV);

	result = (*vfs_dev_init)(vdev);
	VFS_ASSERT(result == VFS_RESULT_WRAPPER_OK,
			"VFS module failed to initialize", -EPERM);

	vfs_capture = dlsym(handle, "vfs_capture");
	VFS_ASSERT(vfs_capture, dlerror(), -ENODEV);

	result = (*vfs_capture)(vdev, 1);
	VFS_ASSERT(result == FP_CAPTURE_COMPLETE,
			"Could not capture fingerprint", -EIO);

	vfs_get_img_datasize = dlsym(handle, "vfs_get_img_datasize");
	VFS_ASSERT(vfs_get_img_datasize, dlerror(), -ENODEV);

	data_len = (*vfs_get_img_datasize)(vdev);
	VFS_ASSERT(data_len, "Zero image size", -ENOMEM);

	img = fpi_img_new(data_len);
	VFS_ASSERT(img, "Could not get new fpi img", -ENOMEM);

	/* Fingerprint is present, load image from reader */
	fpi_imgdev_report_finger_status(dev, TRUE);

	result = vfs_extract_image(dev, handle, img, data_len);
	VFS_ASSERT(!result, "", result);

	fpi_imgdev_image_captured(dev, img);

	/* NOTE: finger off is expected only after submitting image... */
	fpi_imgdev_report_finger_status(dev, FALSE);

	result = 0;

cleanup:
	if (result != -ENODEV)
	{
		vfs_clean_handles = dlsym(handle, "vfs_clean_handles");
		if (vfs_clean_handles)
		{
			(*vfs_clean_handles)(vdev);
		}

		vfs_dev_exit = dlsym(handle, "vfs_dev_exit");
		if (vfs_dev_exit)
		{
			(*vfs_dev_exit)(vdev);
		}
	}

	dlclose(handle);

	return result;
}

/* Deactivate device */
static void dev_deactivate(struct fp_img_dev *dev)
{
	fpi_imgdev_deactivate_complete(dev);
}

static int dev_open(struct fp_img_dev *dev, unsigned long driver_data)
{
    validity_dev *vdev = NULL;
    int result;

    /* Set enroll stage number */
	dev->dev->nr_enroll_stages = VFS_NR_ENROLL;

    /* Initialize private structure */
	vdev = g_malloc0(sizeof(validity_dev));
	dev->priv = vdev;

	/* Notify open complete */
	fpi_imgdev_open_complete(dev, 0);

    return 0;
}

static void dev_close(struct fp_img_dev *dev)
{
    /* Release private structure */
	g_free(dev->priv);

	/* Notify close complete */
	fpi_imgdev_close_complete(dev);
}

struct fp_img_driver  validity_driver = {
    .driver = {
        .id = VALIDITY_DRIVER_ID,
        .name = VALIDITY_FP_COMPONENT,
        .full_name = VALIDITY_DRIVER_FULLNAME,
        .id_table = id_table,
		.scan_type = FP_SCAN_TYPE_SWIPE,
    },

	/* Image specification */
	.flags = 0,
	.img_width = -1,
	.img_height = -1,

    .open = dev_open,
    .close = dev_close,
	.activate = dev_activate,
	.deactivate = dev_deactivate
};

