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

#include <asm-generic/termbits.h>
#include <asm-generic/ioctls.h>

#include <private/android_filesystem_config.h>
#include <cutils/log.h>
#include <dirent.h>


#define W_SYSFS_SIZE 64

char DEV_NAME_SYSFS[W_SYSFS_SIZE];
char SQ_SYSFS[W_SYSFS_SIZE];// noiseSQ_SYSFS
char VOXDEC_SYSFS[W_SYSFS_SIZE]; //handfree detected
char PTT_SYSFS[W_SYSFS_SIZE]; // ptt 1=recv, 0=send
char PD_SYSFS[W_SYSFS_SIZE];  //0=sleep
char HL_SYSFS[W_SYSFS_SIZE]; // 0=0.5w, 1=1w

#define LOG_TAG "wireless_ptt"
#define RFS_ERR(fmt, arg...)  ALOGE("wireless:"fmt"\n" , ##arg)

#define RFS_START_FUNC()      ALOGE("wireless: Inside %s", __FUNCTION__)
#define RFS_DBG(fmt, arg...)  ALOGE("wireless:"fmt"\n" , ## arg)
#define RFS_INFO(fmt, arg...)  ALOGE("wireless:"fmt"\n" , ## arg)


/* Maintains the exit state of UIM*/
static int exiting;

/* File descriptor for the UART device*/
int dev_fd;

static void set_speek_dir(char *dir)
{
	char val;
	int sval = 0;
	int fd = open((const char*) PTT_SYSFS, O_RDWR);
	
	if (!memcmp(dir, "SEND", 4)){
		val = '0';
	}else if (!memcmp(dir, "RECV", 4)){
		val = '1';
	}else {
		RFS_ERR("dir %s error!!!", dir);
		return;
	}
	sval = (int)val;
	write(fd, &val, 4);
	close(fd);
}

static void set_power(char *power)
{
	char val;
	int sval = 0;
	int fd = open((const char*) HL_SYSFS, O_RDWR);
	
	if (!memcmp(power, "LOW", 4)){
		val = '0';
	}else if (!memcmp(power, "HIGH", 4)){
		val = '1';
	}else {
		RFS_ERR("power %s error!!!", power);
		return;
	}
	sval = (int)val;
	write(fd, &val, 4);
	close(fd);
}

static void set_sleep(char *sleep)
{
	char val;
	int sval = 0;
	int fd = open((const char*) PD_SYSFS, O_RDWR);
	
	if (!memcmp(sleep, "SLEEP", 4)){
		val = '0';
	}else if (!memcmp(sleep, "WAKE", 4)){
		val = '1';
	}else {
		RFS_ERR("sleep %s error!!!", sleep);
		return;
	}
	sval = (int)val;
	write(fd, &val, 4);
	close(fd);
}

static void set_sqiut(char *sq)
{
	char val;
	int sval = 0;
	int fd = open((const char*) SQ_SYSFS, O_RDWR);
	
	if (!memcmp(sq, "ON", 4)){
		val = '0';
	}else if (!memcmp(sq, "OFF", 4)){
		val = '1';
	}else {
		RFS_ERR("sq %s error!!!", sq);
		return;
	}
	sval = (int)val;
	write(fd, &val, 4);
	close(fd);
}

static int set_vox_detect()
{
	char buf[4];
	int val = 0;
	int fd = open((const char*) VOXDEC_SYSFS, O_RDONLY);
	
	if (read(fd, buf, 4) != 4)
		return 0;
	
	sscanf(buf, "%d", &val);
	
	return val;
}

static inline void cleanup(int failed)
{
	/* for future use */
	(void)failed;

	if (dev_fd == -1)
		return;

	RFS_DBG("%s", __func__);

	close(dev_fd);
	dev_fd = -1;
	/* unused failed for future reference */
}

/*****************************************************************************/
/*  Function to Read the firmware version
 *  module into the system. Currently used for
 *  debugging purpose, whenever the baud rate is changed
 */
void read_firmware_version()
{
	int index = 0;
	char resp_buffer[20] = { 0 };
	unsigned char buffer[] = { 0x01, 0x01, 0x10, 0x00 };

	RFS_START_FUNC();
	RFS_INFO(" wrote %d bytes", (int)write(dev_fd, buffer, 4));
	RFS_INFO(" reading %d bytes", (int)read(dev_fd, resp_buffer, 15));

	for (index = 0; index < 15; index++)
		RFS_INFO(" %x ", resp_buffer[index]);

	printf("\n");
}

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
		RFS_ERR(" Can't get port settings");
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
	RFS_DBG(" set_baud_rate() done");

	return 0;
}

/* Function to set the UART custom baud rate.
 *
 * The UART baud rate has already been
 * set to default value 115200 before calling this function.
 * The baud rate is then changed to custom baud rate by this function*/
static int set_custom_baud_rate(int cust_baud_rate, unsigned char flow_ctrl)
{
	RFS_START_FUNC();

	struct termios ti;
	struct termios2 ti2;

	/* Get the attributes of UART */
	if (tcgetattr(dev_fd, &ti) < 0) {
		RFS_ERR(" Can't get port settings");
		return -1;
	}

	RFS_INFO(" Changing baud rate to %u, flow control to %u",
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
		RFS_ERR(" Can't set port settings");
		return -1;
	}

	tcflush(dev_fd, TCIOFLUSH);

	/*Set the actual baud rate */
	ioctl(dev_fd, TCGETS2, &ti2);
	ti2.c_cflag &= ~CBAUD;
	ti2.c_cflag |= BOTHER;
	ti2.c_ospeed = cust_baud_rate;
	ioctl(dev_fd, TCSETS2, &ti2);

	RFS_DBG(" set_custom_baud_rate() done");
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
		RFS_ERR("Can't open %s", DEV_NAME_SYSFS);
		return -1;
	}
	len = read(fd, buf, 32);
	if (len < 0) {
		RFS_ERR("read err (%s)", strerror(errno));
		close(fd);
		return len;
	}
	sscanf((const char*)buf, "%s", uart_dev_name);
	close(fd);


	if (dev_fd != -1) {
		RFS_ERR("opening %s, while already open", uart_dev_name);
		cleanup(-1);
	}
	dev_fd = open((const char*) uart_dev_name, O_RDWR);
	if (dev_fd < 0) {
		RFS_ERR(" Can't open %s", uart_dev_name);
		return -1;
	}
	/*
	 * Set only the default baud rate.
	 * This will set the baud rate to default 115200
	 */
	if (uart_config() < 0) {
		RFS_ERR(" set_baudrate() failed");
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

	RFS_START_FUNC();
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

	RFS_DBG("sr_frs sysfs name: %s", namelist[0]->d_name);
	snprintf(DEV_NAME_SYSFS, W_SYSFS_SIZE, "/sys/devices/%s/dev_name", namelist[0]->d_name);
	snprintf(SQ_SYSFS, W_SYSFS_SIZE, "/sys/devices/%s/sq_gpios", namelist[0]->d_name);
	snprintf(PD_SYSFS, W_SYSFS_SIZE, "/sys/devices/%s/pd_gpios", namelist[0]->d_name);
	snprintf(PTT_SYSFS, W_SYSFS_SIZE, "/sys/devices/%s/ptt_gpios", namelist[0]->d_name);
	snprintf(HL_SYSFS, W_SYSFS_SIZE, "/sys/devices/%s/hl_gpios", namelist[0]->d_name);
	snprintf(VOXDEC_SYSFS, W_SYSFS_SIZE, "/sys/devices/%s/voxdec_gpios", namelist[0]->d_name);

	free(namelist[0]);
	free(namelist);

	if (0 == lstat(DEV_NAME_SYSFS, &file_stat)) {
		RFS_DBG("File %s is ok\n", DEV_NAME_SYSFS);
	} else {
		RFS_DBG("File %s is not found\n", DEV_NAME_SYSFS);
		return -1;
	}
	if (0 == lstat(SQ_SYSFS, &file_stat)) {
		RFS_DBG("File %s is ok\n", SQ_SYSFS);
	} else {
		RFS_DBG("File %s is not found\n", SQ_SYSFS);
		return -1;
	}
	if (0 == lstat(PD_SYSFS, &file_stat)) {
		RFS_DBG("File %s is ok\n", PD_SYSFS);
	} else {
		RFS_DBG("File %s is not found\n", PD_SYSFS);
		return -1;
	}
	if (0 == lstat(PTT_SYSFS, &file_stat)) {
		RFS_DBG("File %s is ok\n", PTT_SYSFS);
	} else {
		RFS_DBG("File %s is not found\n", PTT_SYSFS);
		return -1;
	}
	if (0 == lstat(HL_SYSFS, &file_stat)) {
		RFS_DBG("File %s is ok\n", HL_SYSFS);
	} else {
		RFS_DBG("File %s is not found\n", HL_SYSFS);
		return -1;
	}
	if (0 == lstat(VOXDEC_SYSFS, &file_stat)) {
		RFS_DBG("File %s is ok\n", VOXDEC_SYSFS);
	} else {
		RFS_DBG("File %s is not found\n", VOXDEC_SYSFS);
		return -1;
	}	


	return 0;
}