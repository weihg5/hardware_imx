/*
 *  User Mode Init manager - For TI shared transport
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program;if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <poll.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include "uim.h"
#include <asm-generic/termbits.h>
#include <asm-generic/ioctls.h>
#ifdef ANDROID
#include <private/android_filesystem_config.h>
#include <cutils/log.h>
#include <dirent.h>
#endif

#define UIM_SYSFS_SIZE 64

char DEV_NAME_SYSFS[UIM_SYSFS_SIZE];

char _SYSFS[UIM_SYSFS_SIZE];
char FLOW_CTRL_SYSFS[UIM_SYSFS_SIZE];

#define LOG_TAG "wireless_ptt"
#define UIM_ERR(fmt, arg...)  ALOGE("wireless:"fmt"\n" , ##arg)

#define UIM_START_FUNC()      ALOGE("wireless: Inside %s", __FUNCTION__)
#define UIM_DBG(fmt, arg...)  ALOGE("wireless:"fmt"\n" , ## arg)
#define UIM_VER(fmt, arg...)  ALOGE("wireless:"fmt"\n" , ## arg)


/* Maintains the exit state of UIM*/
static int exiting;

/* File descriptor for the UART device*/
int dev_fd;

static inline void cleanup(int failed)
{
	/* for future use */
	(void)failed;

	if (dev_fd == -1)
		return;

	UIM_DBG("%s", __func__);

	close(dev_fd);
	dev_fd = -1;
	/* unused failed for future reference */
}

/*****************************************************************************/
#ifdef UIM_DEBUG
/*  Function to Read the firmware version
 *  module into the system. Currently used for
 *  debugging purpose, whenever the baud rate is changed
 */
void read_firmware_version()
{
	int index = 0;
	char resp_buffer[20] = { 0 };
	unsigned char buffer[] = { 0x01, 0x01, 0x10, 0x00 };

	UIM_START_FUNC();
	UIM_VER(" wrote %d bytes", (int)write(dev_fd, buffer, 4));
	UIM_VER(" reading %d bytes", (int)read(dev_fd, resp_buffer, 15));

	for (index = 0; index < 15; index++)
		UIM_VER(" %x ", resp_buffer[index]);

	printf("\n");
}
#endif /* UIM_DEBUG */


/* Function to set the default baud rate
 *
 * The default baud rate of 115200 is set to the UART from the host side
 * by making a call to this function.This function is also called before
 * making a call to set the custom baud rate
 */
static int uart_config()
{
	struct termios ti;

	fcntl(dev_fd, F_SETFL,fcntl(dev_fd, F_GETFL) | O_NONBLOCK);	
	
	tcflush(dev_fd, TCIOFLUSH);

	/* Get the attributes of UART */
	if (tcgetattr(dev_fd, &ti) < 0) {
		UIM_ERR(" Can't get port settings");
		return -1;
	}

	/* Change the UART attributes before
	 * setting the default baud rate*/
	cfmakeraw(&ti);

	ti.c_cflag |= 1;
	ti.c_cflag &= ~CRTSCTS;

	/* Set the attributes of UART after making
	 * the above changes
	 */
	tcsetattr(dev_fd, TCSANOW, &ti);

	/* Set the actual default baud rate */
	cfsetospeed(&ti, B9600);
	cfsetispeed(&ti, B9600);
	tcsetattr(dev_fd, TCSANOW, &ti);

	tcflush(dev_fd, TCIOFLUSH);
	UIM_DBG(" set_baud_rate() done");

	return 0;
}

/* Function to set the UART custom baud rate.
 *
 * The UART baud rate has already been
 * set to default value 115200 before calling this function.
 * The baud rate is then changed to custom baud rate by this function*/
static int set_custom_baud_rate(int cust_baud_rate, unsigned char flow_ctrl)
{
	UIM_START_FUNC();

	struct termios ti;
	struct termios2 ti2;

	/* Get the attributes of UART */
	if (tcgetattr(dev_fd, &ti) < 0) {
		UIM_ERR(" Can't get port settings");
		return -1;
	}

	UIM_VER(" Changing baud rate to %u, flow control to %u",
			cust_baud_rate, flow_ctrl);

	/* Flush non-transmitted output data,
	 * non-read input data or both*/
	tcflush(dev_fd, TCIOFLUSH);

	/*Set the UART flow control */
	if (flow_ctrl)
		ti.c_cflag |= CRTSCTS;
	else
		ti.c_cflag &= ~CRTSCTS;

	/*
	 * Set the parameters associated with the UART
	 * The change will occur immediately by using TCSANOW
	 */
	if (tcsetattr(dev_fd, TCSANOW, &ti) < 0) {
		UIM_ERR(" Can't set port settings");
		return -1;
	}

	tcflush(dev_fd, TCIOFLUSH);

	/*Set the actual baud rate */
	ioctl(dev_fd, TCGETS2, &ti2);
	ti2.c_cflag &= ~CBAUD;
	ti2.c_cflag |= BOTHER;
	ti2.c_ospeed = cust_baud_rate;
	ioctl(dev_fd, TCSETS2, &ti2);

	UIM_DBG(" set_custom_baud_rate() done");
	return 0;
}

/*
 * Handling the Signals sent from the Kernel Init Manager.
 * After receiving the indication from rfkill subsystem, configure the
 * baud rate, flow control and Install the N_TI_WL line discipline
 */

int uart_open()
{
	unsigned char uart_dev_name[32];
	unsigned char buf[32];

	memset(buf, 0, 32);
	fd = open(DEV_NAME_SYSFS, O_RDONLY);
	if (fd < 0) {
		UIM_ERR("Can't open %s", DEV_NAME_SYSFS);
		return -1;
	}
	len = read(fd, buf, 32);
	if (len < 0) {
		UIM_ERR("read err (%s)", strerror(errno));
		close(fd);
		return len;
	}
	sscanf((const char*)buf, "%s", uart_dev_name);
	close(fd);


	if (dev_fd != -1) {
		UIM_ERR("opening %s, while already open", uart_dev_name);
		cleanup(-1);
	}
	dev_fd = open((const char*) uart_dev_name, O_RDWR);
	if (dev_fd < 0) {
		UIM_ERR(" Can't open %s", uart_dev_name);
		return -1;
	}
	/*
	 * Set only the default baud rate.
	 * This will set the baud rate to default 115200
	 */
	if (uart_config() < 0) {
		UIM_ERR(" set_baudrate() failed");
		cleanup(-1);
		return -1;
	}
	return 0;

}
// ignore the "." and ".." entries
static int dir_filter(const struct dirent *name)
{
    if (0 == strncmp("sr_frs.", name->d_name, 7))
            return 1;

    return 0;
}

/*****************************************************************************/
int main(int argc, char *argv[])
{
	int st_fd,err;
	struct stat file_stat;

	struct pollfd   p;
	unsigned char install;

	int i, n;
	struct dirent **namelist;

	UIM_START_FUNC();
	err = 0;

	n = scandir("/sys/devices", &namelist, dir_filter, alphasort);

	if (n == -1) {
		ALOGE("Found zero sr_frs devices:%s", strerror(errno));
		return -1;
	}

	if (n != 1) {
		ALOGE("unexpected - found %d sr_frs devices", n);
		for (i = 0; i < n; i++)
			free(namelist[i]);
		free(namelist);
		return -1;
	}

	UIM_DBG("sr_frs sysfs name: %s", namelist[0]->d_name);
	snprintf(INSTALL_SYSFS_ENTRY, UIM_SYSFS_SIZE, "/sys/devices/%s/install", namelist[0]->d_name);
	snprintf(DEV_NAME_SYSFS, UIM_SYSFS_SIZE, "/sys/devices/%s/dev_name", namelist[0]->d_name);
	snprintf(BAUD_RATE_SYSFS, UIM_SYSFS_SIZE, "/sys/devices/%s/baud_rate", namelist[0]->d_name);
	snprintf(FLOW_CTRL_SYSFS, UIM_SYSFS_SIZE, "/sys/devices/%s/flow_cntrl", namelist[0]->d_name);

	free(namelist[0]);
	free(namelist);

#ifndef ANDROID
	if (uname (&name) == -1) {
		UIM_ERR("cannot get kernel release name");
		return -1;
	}
#else  /* if ANDROID */

	if (0 == lstat("/st_drv.ko", &file_stat)) {
		if (insmod("/st_drv.ko", "") < 0) {
			UIM_ERR(" Error inserting st_drv module");
			return -1;
		} else {
			UIM_DBG(" Inserted st_drv module");
		}
	} else {
		if (0 == lstat(INSTALL_SYSFS_ENTRY, &file_stat)) {
			UIM_DBG("ST built into the kernel ?");
		} else {
			UIM_ERR("BT/FM/GPS would be unavailable on system");
			return -1;
		}
	}

	if (0 == lstat("/btwilink.ko", &file_stat)) {
		if (insmod("/btwilink.ko", "") < 0) {
			UIM_ERR(" Error inserting btwilink module, NO BT? ");
		} else {
			UIM_DBG(" Inserted btwilink module");
		}
	} else {
		UIM_DBG("BT driver module un-available... ");
		UIM_DBG("BT driver built into the kernel ?");
	}

	if (0 == lstat("/fm_drv.ko", &file_stat)) {
		if (insmod("/fm_drv.ko", "") < 0) {
			UIM_ERR(" Error inserting fm_drv module, NO FM? ");
		} else {
			UIM_DBG(" Inserted fm_drv module");
		}
	} else {
		UIM_DBG("FM driver module un-available... ");
		UIM_DBG("FM driver built into the kernel ?");
	}

	if (0 == lstat("/gps_drv.ko", &file_stat)) {
		if (insmod("/gps_drv.ko", "") < 0) {
			UIM_ERR(" Error inserting gps_drv module, NO GPS? ");
		} else {
			UIM_DBG(" Inserted gps_drv module");
		}
	} else {
		UIM_DBG("GPS driver module un-available... ");
		UIM_DBG("GPS driver built into the kernel ?");
	}

	if (0 == lstat("/fm_v4l2_drv.ko", &file_stat)) {
		if (insmod("/fm_v4l2_drv.ko", "") < 0) {
			UIM_ERR(" Error inserting fm_v4l2_drv module, NO FM? ");
		} else {
			UIM_DBG(" Inserted fm_v4l2_drv module");
		}
	} else {
		UIM_DBG("FM V4L2 driver module un-available... ");
		UIM_DBG("FM V4L2 driver built into the kernel ?");
	}
	/* Change the permissions for v4l2 Fm device node */
	if ((0 == lstat("/dev/radio0", &file_stat)) && chmod("/dev/radio0", 0666) < 0) {
		UIM_ERR("unable to chmod /dev/radio0, might not exist");
	}
	if ((0 == lstat("/dev/tifm", &file_stat)) && chmod("/dev/tifm", 0666) < 0) {
		UIM_ERR("unable to chmod /dev/tifm, might not exist");
	}
	/* change rfkill perms after insertion of BT driver which asks
	 * the Bluetooth sub-system to create the rfkill device of type
	 * "bluetooth"
	 */
	if (change_rfkill_perms() < 0) {
		/* possible error condition */
		UIM_ERR("rfkill not enabled in st_drv - BT on from UI might fail\n");
	}

#endif /* ANDROID */
	/* rfkill device's open/poll/read */
	st_fd = open(INSTALL_SYSFS_ENTRY, O_RDONLY);
	if (st_fd < 0) {
		UIM_DBG("unable to open %s (%s)", INSTALL_SYSFS_ENTRY,
				strerror(errno));
		remove_modules();
		return -1;
	}

RE_POLL:
	err = read(st_fd, &install, 1);
	if ((err > 0) && (install == '1')) {
		UIM_DBG("install already set");
		st_uart_config(install);
	}

	memset(&p, 0, sizeof(p));
	p.fd = st_fd;
	/* sysfs entries can only break poll for following events */
	p.events = POLLERR | POLLHUP;

	while (!exiting) {
		p.revents = 0;
		err = poll(&p, 1, -1);
		if (err < 0 && errno == EINTR)
			continue;
		if (err)
			break;
	}

	close(st_fd);
	st_fd = open(INSTALL_SYSFS_ENTRY, O_RDONLY);
	if (st_fd < 0) {
		UIM_ERR("re-opening %s failed: %s", INSTALL_SYSFS_ENTRY,
		strerror(errno));
		return -1;
	}

	if (!exiting)
	{
		err = read(st_fd, &install, 1);
		if (err <= 0) {
			UIM_ERR("reading %s failed: %s", INSTALL_SYSFS_ENTRY,
					strerror(errno));
			goto RE_POLL;
		}
		st_uart_config(install);
		goto RE_POLL;
	}

	if(remove_modules() < 0) {
		UIM_ERR(" Error removing modules ");
		close(st_fd);
		return -1;
	}

	close(st_fd);
	return 0;
}
