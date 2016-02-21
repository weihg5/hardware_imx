#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define GPS_RESET "/sys/class/gpio/gpio192/value" //gpio7 0
#define GPS_POWER "/sys/class/gpio/gpio84/value" //gpio3 20

static void gpio_write(const char *gpio, int val)
{
	int fd = open(gpio, O_RDWR);
	const char *v = val?"0":"1";
	if (fd < 0)
		return;

	write(fd, v, 1);

	close(fd);
}

static void gps_reset()
{
	gpio_write(GPS_RESET, 1);
	usleep(1000*200);
	gpio_write(GPS_RESET, 0);
}

static void gps_power(int on)
{
	gpio_write(GPS_POWER, on);
}



