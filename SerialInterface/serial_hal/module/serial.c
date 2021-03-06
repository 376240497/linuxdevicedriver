#include <hardware/hardware.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <cutils/log.h>
#include <cutils/atomic.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include "../include/serial.h"

int fd;

static int serial_device_close(struct hw_device_t* device)
{
	LOGD("%s E", __func__);
	struct serial_control_device_t* ctx = (struct serial_control_device_t*)device;
	if (ctx) {
		free(ctx);
	}
	close(fd);
	LOGD("%s X", __func__);
	return 0; 
}

static int serial_read_drv(struct serial_control_device_t *dev, char *buf, int count)
{	
	LOGD("%s E", __func__);
	int len = 0;
	len = read(fd, buf, count);
	if(len < 0)
	{
		perror("read");
	}
	LOGD("%s X", __func__);
	return len;
}

static int serial_write_drv(struct serial_control_device_t *dev, char *buf, int size)
{	
	LOGD("%s E", __func__);
	int len = write(fd, buf, size);
	if(len < 0)
	{
		perror("write");
	}
	LOGD("%s X", __func__);
	return len;
}

static int serial_device_open(const struct hw_module_t* module, const char* name,
		struct hw_device_t** device)
{
	LOGD("%s E", __func__);
	struct serial_control_device_t *dev;
	struct termios opt; 

	dev = (struct serial_control_device_t *)malloc(sizeof(*dev));
	memset(dev, 0, sizeof(*dev)); 

	//HAL must init property
	dev->common.tag= HARDWARE_DEVICE_TAG; //必须写这个
	dev->common.version = 0;
	dev->common.module= module;

	dev->serial_read_hal = serial_read_drv;

	dev->serial_write_hal = serial_write_drv;
	*device= &dev->common;

	// MichaelTang add for open ttyUSBx 
	char devname[PATH_MAX];
	DIR *dir;
	struct dirent *de;
	dir = opendir("/dev");
	if(dir == NULL)
		return -1;
	strcpy(devname, "/dev");
	char * filename = devname + strlen(devname);
	*filename++ = '/';
	while((de = readdir(dir))) {
		if(de->d_name[0] == '.' || strncmp(de->d_name, "ttyUSB", 6))        // start with . will ignor
			continue;
		strcpy(filename, de->d_name);
		if((fd = open(devname, O_RDWR | O_NOCTTY | O_NDELAY)) < 0)
		{       
			LOGE("open error");
			return -1;
		}else {
			LOGD("open ok\n");
			break;
		}
	}


	//初始化串口
	tcgetattr(fd, &opt);
	//tcflush(fd, TCIOFLUSH);
	cfsetispeed(&opt, B9600);
	cfsetospeed(&opt, B9600);

	//tcflush(fd, TCIOFLUSH);

	opt.c_cflag |= (CLOCAL | CREAD);

	opt.c_cflag &= ~CSIZE;
	opt.c_cflag &= ~CRTSCTS;
	opt.c_cflag |= CS8;

	/* 
	   opt.c_cflag |= PARENB;  // enable; 允许输入奇偶校验
	   opt.c_cflag |= PARODD;  // J check     对输入使用奇校验          
	   opt.c_iflag |= (INPCK | ISTRIP);  // 
	   */ 
	opt.c_iflag |= IGNPAR;

	opt.c_cflag &= ~CSTOPB;

	opt.c_oflag = 0;
	opt.c_lflag = 0;

	tcsetattr(fd, TCSANOW, &opt);

	LOGD("%s X", __func__);

	return 0;
}

//定一个hw_module_methods_t结构体，关联入口函数
static struct hw_module_methods_t serial_module_methods = {
open: serial_device_open  
};

//定义Stub入口
//注意必须使用：
//1。hw_module_t继承类
//2。必须使用HAL_MODULE_INFO_SYM这个名字

const struct serial_module_t HAL_MODULE_INFO_SYM = {
common: {
tag: HARDWARE_MODULE_TAG,
     version_major: 1,
     version_minor: 0,
     id: serial_HARDWARE_MODULE_ID, 
     //模块ID，上层的Service通过这个ID应用当前Stub
     name: "serial HAL module",
     author: "farsight",
     methods: &serial_module_methods, //入口函数管理结构体
	}, 
	/* supporting APIs go here */
};



