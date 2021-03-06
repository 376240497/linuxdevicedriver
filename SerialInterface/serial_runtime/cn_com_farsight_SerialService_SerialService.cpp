#define LOG_TAG "SerialService Runtime"
#include "utils/Log.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <jni.h>
#include "../serial_hal/include/serial.h"

static serial_control_device_t *sSerialDevice = 0;
static serial_module_t* sSerialModule=0;

static int serial_read(JNIEnv* env, jobject thiz, jbyteArray data, jint len) {
	LOGD("%s E", __func__);
	int ret;
	
	char *buf;
	// 在本地空间得到Java对象元素
	buf = (char *)env->GetByteArrayElements(data,NULL);
	if (sSerialDevice) {
		ret = sSerialDevice->serial_read_hal(sSerialDevice, buf, len);//调用hal层的注册的方法 
	}else{
		LOGE("sSerialDevice is null");
	}
	// Java对象使用完后，释放掉
	env->ReleaseByteArrayElements(data, (jbyte*)buf, 0);
	LOGD("%s X", __func__);
	return ret;
}


/*************************************************/
static int serial_write(JNIEnv* env, jobject thiz, jbyteArray data) {
	LOGD("%s E", __func__);

	int ret;
	char *buf;
	// 在本地空间得到Java对象元素
	buf = (char *)env->GetByteArrayElements(data,NULL);
	int size = env->GetArrayLength(data);
	if (sSerialDevice) {
		ret = sSerialDevice->serial_write_hal(sSerialDevice, buf, size);//调用hal层的注册的方法 
	}else{
		LOGI("sSerialDevice is null");
		return -1;
	}
	// Java对象使用完后，释放掉
	env->ReleaseByteArrayElements(data, (jbyte*)buf, 0);
	LOGI("%s X", __func__);

	return ret ;
}



/** helper APIs */

static inline int serial_control_open(const struct hw_module_t* module,
		struct serial_control_device_t** device) {
	LOGD("%s E", __func__);
	return module->methods->open(module,
			serial_HARDWARE_MODULE_ID, (struct hw_device_t**)device);
}

static jboolean serial_init(JNIEnv *env, jclass clazz)
{
	LOGD("%s E", __func__);
	serial_module_t const * module;
	//调用Android HAL标准函数hw_get_module，通过serial_HARDWARE_MODULE_ID获取串口Stub的句柄，
	//句柄保存在module变量中
	if (hw_get_module(serial_HARDWARE_MODULE_ID, (const hw_module_t**)&module) == 0) {
		LOGD(" get module OK");
		sSerialModule = (serial_module_t *) module;
		if (serial_control_open(&module->common, &sSerialDevice) != 0) 
		{
			LOGE("serial_control_open error");
			return false;
		}
	}
	LOGD("%s X", __func__);
	return true;

}

/*
   JNINativeMethod是 jni层注册的方法，Framework层可以使用这些方法
 *_init 、serial_read、serial_write 是在Framework中调用的方法名称，函数的类型及返回值如下：
 *()Z无参数 返回值为bool型
 * ()Ljava/lang/String; 无参数 返回值为String型
 * (II)I  参数为2个int型 返回值为int型
 */
static const JNINativeMethod gMethods[] = {
	{"_init",     "()Z", (void*)serial_init},

	{ "_read",   "([BI)I", (void*)serial_read },

	{ "_write",   "([B)I", (void*)serial_write },
};

static int registerMethods(JNIEnv* env) {
	static const char* const kClassName = "cn/com/farsight/SerialService/SerialService"; 
	jclass clazz; 
	/* look up the class */
	clazz = env->FindClass(kClassName); //查找被注册的类
	if (clazz == NULL) {
		LOGE("Can't find class %s\n", kClassName);
		return -1;
	} 

	/* register all the methods */
	if (env->RegisterNatives(clazz, gMethods,
				sizeof(gMethods) / sizeof(gMethods[0])) != JNI_OK)
	{
		LOGE("Failed registering methods for %s\n", kClassName);
		return -1;
	} //向类中注册本SO中Native的接口，接口定义在gMethods数组中
	/* fill out the rest of the ID cache */
	return 0;
} 

/*
 *
 *   * This is called by the VM when the shared library is first loaded.
 */ 
/* 当第一次加载此库时，调用此函数注册本地方法 */
jint JNI_OnLoad(JavaVM* vm, void* reserved) {
	JNIEnv* env = NULL;
	jint result = -1;
	LOGI("zzs_JNI_OnLoad");

	if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
		LOGE("ERROR: GetEnv failed\n");
		goto fail;
	} //获取当前的VM的环境，保存在env变量中

	assert(env != NULL);
	if (registerMethods(env) != 0) {
		LOGE("ERROR: PlatformLibrary native registration failed\n");
		goto fail;
	} //自己写的函数，向当前JAVA环境中注册接口
	/* success -- return valid version number */	
	result = JNI_VERSION_1_4;

fail:
	return result;
} 


