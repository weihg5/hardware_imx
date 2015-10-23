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


#ifdef _ANDROID
#include <asm-generic/termbits.h>
#include <asm-generic/ioctls.h>
#include <private/android_filesystem_config.h>
#include <cutils/log.h>

#endif

#include <dirent.h>


#define W_SYSFS_SIZE 256

char DEV_NAME_SYSFS[W_SYSFS_SIZE];
char SQ_SYSFS[W_SYSFS_SIZE];// noiseSQ_SYSFS
char VOXDEC_SYSFS[W_SYSFS_SIZE]; //handfree detected
char PTT_SYSFS[W_SYSFS_SIZE]; // ptt 1=recv, 0=send
char PD_SYSFS[W_SYSFS_SIZE];  //0=sleep
char HL_SYSFS[W_SYSFS_SIZE]; // 0=0.5w, 1=1w

#ifdef _ANDROID
//#define LOG_TAG "wireless_ptt"
#define RFS_ERR(fmt, arg...)  ALOGE("wireless:"fmt"\n" , ##arg)

#define RFS_START_FUNC()      ALOGE("wireless: Inside %s", __FUNCTION__)
#define RFS_DBG(fmt, arg...)  ALOGE("wireless:"fmt"\n" , ## arg)
#define RFS_INFO(fmt, arg...)  ALOGE("wireless:"fmt"\n" , ## arg)
#else

#define RFS_ERR(fmt, arg...)  printf("wireless:"fmt"\n" , ##arg)

#define RFS_START_FUNC()      printf("wireless: Inside %s", __FUNCTION__)
#define RFS_DBG(fmt, arg...)  printf("wireless:"fmt"\n" , ## arg)
#define RFS_INFO(fmt, arg...)  printf("wireless:"fmt"\n" , ## arg)
#endif

static void process_mode(char *);
static void set_speek_dir(char *);
static void set_power(char *);
static void set_sqiut(char *);
static void exit_func(char *);
static void set_sleep(char *);
/* File descriptor for the UART device*/
int dev_fd = -1;
typedef void (*modefunc)(char *);

typedef enum { false = 0, true, } bool;

struct mode_func{
	char *cmd;
	modefunc func;
	char *p;
};
static struct mode_func _mode[] = {
	{"SEND", set_speek_dir, "SNED"},
	{"RECV", set_speek_dir, "RECV"},
	
	{"LOW", set_power, "LOW"},
	{"HIGH", set_power, "HIGH"},

	{"SLEEP", set_sleep, "SLEEP"},
	{"WAKE", set_sleep, "WAKE"},

	{"ON", set_sqiut, "ON"},
	{"OFF", set_sqiut, "OFF"},
	{"EXIT", exit_func, ""},
	{NULL, NULL, NULL},
};

static void process_mode(char *buf)
{
	int i;
	for (i = 0; ; i++){
		if (_mode[i].cmd == NULL)
			return;
		//RFS_DBG("mode cmd=%s\n", _mode[i].cmd);
		if (strcmp(_mode[i].cmd, buf) == 0)
		{
			RFS_DBG("found cmd:%s", buf);
			_mode[i].func(buf);
		}
	}
	
}
static void set_speek_dir(char *dir)
{
	char val;
	int sval = 0;
	int fd = open((const char*) PTT_SYSFS, O_RDWR);
	if (fd < 0){
		RFS_ERR("open file %s Error\n", PTT_SYSFS);
		return;
	}
	if (!memcmp(dir, "SEND", 4)){
		val = '0';
	}else if (!memcmp(dir, "RECV", 4)){
		val = '1';
	}else {
		RFS_ERR("dir %s error!!!", dir);
		return;
	}
	sval = (int)val;
	if (write(fd, &sval, 4) != 4)
		RFS_ERR("write file %s Error\n", PTT_SYSFS);
	close(fd);
}

static void set_power(char *power)
{
	char val;
	int sval = 0;
	int fd = open((const char*) HL_SYSFS, O_RDWR);
	if (fd < 0){
		RFS_ERR("open file %s Error\n", HL_SYSFS);
		return;
	}	
	if (!memcmp(power, "LOW", 4)){
		val = '0';
	}else if (!memcmp(power, "HIGH", 4)){
		val = '1';
	}else {
		RFS_ERR("power %s error!!!", power);
		return;
	}
	sval = (int)val;
	if (write(fd, &sval, 4) != 4)
		RFS_ERR("write file %s Error\n", HL_SYSFS);

	close(fd);
}

static void set_sleep(char *sleep)
{
	char val;
	int sval = 0;
	int fd = open((const char*) PD_SYSFS, O_RDWR);
	if (fd < 0){
		RFS_ERR("open file %s Error\n", PD_SYSFS);
		return;
	}	
	if (!memcmp(sleep, "SLEEP", 4)){
		val = '0';
	}else if (!memcmp(sleep, "WAKE", 4)){
		val = '1';
	}else {
		RFS_ERR("sleep %s error!!!", sleep);
		return;
	}
	sval = (int)val;
	if (write(fd, &sval, 4) != 4)
		RFS_ERR("write file %s Error\n", PD_SYSFS);

	close(fd);
}

static void set_sqiut(char *sq)
{
	char val;
	int sval = 0;
	int fd = open((const char*) SQ_SYSFS, O_RDWR);
	if (fd < 0){
		RFS_ERR("open file %s Error\n", SQ_SYSFS);
		return;
	}	
	if (!memcmp(sq, "ON", 4)){
		val = '0';
	}else if (!memcmp(sq, "OFF", 4)){
		val = '1';
	}else {
		RFS_ERR("sq %s error!!!", sq);
		return;
	}
	sval = (int)val;
	if (write(fd, &sval, 4) != 4)
		RFS_ERR("write file %s Error\n", SQ_SYSFS);

	close(fd);
}

static int set_vox_detect()
{
	char buf[4];
	int val = 0;
	int fd = open((const char*) VOXDEC_SYSFS, O_RDONLY);
	if (fd < 0){
		RFS_ERR("open file %s Error\n", VOXDEC_SYSFS);
		return 0;
	}	
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


/* Function to set the default baud rate
 *
 * The default baud rate of 115200 is set to the UART from the host side
 * by making a call to this function.This function is also called before
 * making a call to set the custom baud rate
 */
static int uart_config()
{
#ifdef _ANDROID
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

	//can read
	ti.c_cflag |= CREAD;
	//not ocuppy
	//ti.c_cflag |= CLOCAL;
	//disable hardware flow ctrl
	ti.c_cflag &= ~CRTSCTS;

	//8 bit data
	ti.c_cflag &= ~CSIZE;
	ti.c_cflag |= CS8;
	//no check
	ti.c_cflag &= ~PARENB;
	ti.c_iflag &= ~INPCK;
	//1bit stop
	ti.c_cflag &= ~CSTOPB;
	//origin data output
	ti.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);
	ti.c_oflag &= ~OPOST;
#if 0
    //set wait time and min recved num
    ti.c_cc[VTIME] = 1; /* read one char wait 1*(1/10)s */

    ti.c_cc[VMIN] = 1; /* read at least one char */
#endif
	/* Set the attributes of UART after making
	 * the above changes
	 */
	tcsetattr(dev_fd, TCSANOW, &ti);

	/* Set the actual default baud rate */
	cfsetospeed(&ti, B9600);
	cfsetispeed(&ti, B9600);
	tcsetattr(dev_fd, TCSANOW, &ti);

	tcflush(dev_fd, TCIOFLUSH);
#endif	
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
#ifdef _ANDROID
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
#endif
	RFS_DBG(" set_custom_baud_rate() done");
	return 0;
}

#if 0
int uart_recv(char *rcv_buf,int data_len, int msec)
{
    int len,fs_sel;

    fd_set fs_read;

    struct timeval time;
    FD_ZERO(&fs_read);

    FD_SET(fd,&fs_read);

    time.tv_sec = msec/1000;

    time.tv_usec = (msec%1000)* 1000;

    //use select
    fs_sel = select(fd+1,&fs_read,NULL,NULL,&time);

    if(fs_sel)
	{
		len = read(fd,data,data_len);
		return len;
	}
    else
	{
		return 0;
	}
}
#endif
/*
 * Handling the Signals sent from the Kernel Init Manager.
 * After receiving the indication from rfkill subsystem, configure the
 * baud rate, flow control and Install the N_TI_WL line discipline
 */

int uart_open()
{
	unsigned char uart_dev_name[32];
	unsigned char buf[32];
	int fd,len;
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

static void msleep(int msec)
{
	struct timespec req, rem;
	int ret = 0;
	req.tv_sec = msec/1000;
	msec = msec - req.tv_sec*1000;
	req.tv_nsec = msec*1000*1000;
	do {
		ret = nanosleep(&req, &rem);
		if (ret == -1 && errno == EINTR){
			req.tv_sec = rem.tv_sec;
			req.tv_nsec = rem.tv_nsec;
		}else{
			if (ret)
				RFS_ERR("nanosleep error, ret=%d, error=%d\n", ret, errno);
			break;
		}
	}while(!ret);
}
static int g_exit = 0;
void exit_func(char *p)
{
	(void)p;
	g_exit = 1;
}

void send_cmd_wait_ack(char *buf)
{
	int ret ;
	char rbuf[128];
	int recv_num = 0;
	memset(rbuf, 0, 128);

	ret = write(dev_fd, buf, strlen(buf));
	if (ret < (int)strlen(buf)){
		RFS_ERR("tty send failt, ret=%d\n", ret);
		return;
	}	
	msleep(1000);
	do {
		ret = read(dev_fd, rbuf+recv_num, 1);
		if (ret==1){
			recv_num++;
		}else{
			break;
		}
	}while(1);
	if (recv_num)
		RFS_DBG("RECV :-->%s\n", rbuf);
	else
		RFS_ERR("RECV NULL\n");
}

void process_cmd(char *buf)
{
	RFS_DBG("CMD: %s\n", buf);
	if (strlen(buf) < 4 || memcmp("AT+", buf, 3)){
		RFS_DBG("cmd buf error %s\n", buf);
		return;
	}

	if (!memcmp(buf, "AT+DMOMES", 9)){
		if (strlen(buf) < 12){
			RFS_ERR("SMS len is too short=%d\n", strlen(buf));
			return;
		}
		char len = buf[10];
		buf[10]=len-'0';
		if (buf[10] == 0)
			return;
		RFS_DBG("SMS len=%d\n", buf[10]);
		send_cmd_wait_ack(buf);
	}else{
		send_cmd_wait_ack(buf);
	}
}

void process_commands(void)
{
	char buf[256];
	char cmd[16];
	char args[128];
	int argv = 0;
	char *p;
	memset(buf, 0, 245);
	
	RFS_INFO("Pls enter the MODE or CMD :");
	
	p = fgets(buf, 100, stdin);
	if (p==NULL)
		return;
	
	RFS_DBG("get buf: --> %s\n", p);
	
	argv = sscanf(p, "%s %s", cmd, args);
	if (!memcmp(cmd, "MODE", 4))
	{
		if (argv != 2) {
			RFS_DBG("process mode error");
			return;
		}
		RFS_DBG("process mode %s\n", args);
		process_mode(args);
	}
	if (!memcmp(cmd, "ATCMD", 3))
	{
		if (argv != 2) {
			RFS_DBG("process ATCMD error");
			return;
		}
		RFS_DBG("process ATCMD %s\n", args);
		process_cmd(args);
	}	
}

static bool system_format(const char *command, ...)
{
	va_list ap;
	char buff[1024];

	va_start(ap, command);
	vsnprintf(buff, sizeof(buff), command, ap);
	va_end(ap);

	command = buff;

	RFS_INFO("run command: %s", command);

	if (system(command)) {
		RFS_ERR("Failed to run command: %s", command);
		return false;
	}

	return true;
}

static bool tinymix_command(const char *control, const char *value)
{
	return system_format("/system/bin/tinymix '%s' '%s'", control, value);
}

static void set_audio_route(bool enable)
{
	tinymix_command("MIXINR PGA Switch", "0");

	if (enable) {
		tinymix_command("HPOUTR PGA", "Mixer");
		tinymix_command("HPMIXR MIXINR Switch", "1");
		tinymix_command("MIXINR IN3R Switch", "1");
		tinymix_command("Headphone Mixer Switch", "1");
		tinymix_command("Headphone Volume", "121");
		tinymix_command("Headphone Switch", "1");
	} else {
		tinymix_command("HPOUTR PGA", "DAC");
		tinymix_command("HPMIXR MIXINR Switch", "0");
		tinymix_command("MIXINR IN3R Switch", "0");
		tinymix_command("Headphone Mixer Switch", "0");
		// tinymix_command("Headphone Volume", "0");
		// tinymix_command("Headphone Switch", "0");
	}
}

/*****************************************************************************/
#define SYS_FS_DEVICES_DIR "/sys/bus/platform/devices"

int main(int argc, char *argv[])
{
#ifdef _ANDROID
	int err;
	struct stat file_stat;

	int i, n;
	struct dirent **namelist;

	RFS_START_FUNC();
	err = 0;

	n = scandir(SYS_FS_DEVICES_DIR, &namelist, dir_filter, alphasort);

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
	snprintf(DEV_NAME_SYSFS, W_SYSFS_SIZE, SYS_FS_DEVICES_DIR"/%s/dev_name", namelist[0]->d_name);
	snprintf(SQ_SYSFS, W_SYSFS_SIZE, SYS_FS_DEVICES_DIR"/%s/sq_gpios", namelist[0]->d_name);
	snprintf(PD_SYSFS, W_SYSFS_SIZE, SYS_FS_DEVICES_DIR"/%s/pd_gpios", namelist[0]->d_name);
	snprintf(PTT_SYSFS, W_SYSFS_SIZE, SYS_FS_DEVICES_DIR"/%s/ptt_gpios", namelist[0]->d_name);
	snprintf(HL_SYSFS, W_SYSFS_SIZE, SYS_FS_DEVICES_DIR"/%s/hl_gpios", namelist[0]->d_name);
	snprintf(VOXDEC_SYSFS, W_SYSFS_SIZE, SYS_FS_DEVICES_DIR"/%s/voxdec_gpios", namelist[0]->d_name);

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
	if (uart_open() < 0){
		RFS_ERR("open  uart  failt\n");
		return -1;
	}
#endif	

	set_audio_route(true);

	while(!g_exit){
		process_commands();
	}

	set_audio_route(false);

	cleanup(1);

	return 0;
}
