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
	// �ڱ��ؿռ�õ�Java����Ԫ��
	buf = (char *)env->GetByteArrayElements(data,NULL);
	if (sSerialDevice) {
		ret = sSerialDevice->serial_read_hal(sSerialDevice, buf, len);//����hal���ע��ķ��� 
	}else{
		LOGE("sSerialDevice is null");
	}
	// Java����ʹ������ͷŵ�
	env->ReleaseByteArrayElements(data, (jbyte*)buf, 0);
	LOGD("%s X", __func__);
	return ret;
}


/*************************************************/
static int serial_write(JNIEnv* env, jobject thiz, jbyteArray data) {
	LOGD("%s E", __func__);

	int ret;
	char *buf;
	// �ڱ��ؿռ�õ�Java����Ԫ��
	buf = (char *)env->GetByteArrayElements(data,NULL);
	int size = env->GetArrayLength(data);
	if (sSerialDevice) {
		ret = sSerialDevice->serial_write_hal(sSerialDevice, buf, size);//����hal���ע��ķ��� 
	}else{
		LOGI("sSerialDevice is null");
		return -1;
	}
	// Java����ʹ������ͷŵ�
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
	//����Android HAL��׼����hw_get_module��ͨ��serial_HARDWARE_MODULE_ID��ȡ����Stub�ľ����
	//���������module������
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
   JNINativeMethod�� jni��ע��ķ�����Framework�����ʹ����Щ����
 *_init ��serial_read��serial_write ����Framework�е��õķ������ƣ����������ͼ�����ֵ���£�
 *()Z�޲��� ����ֵΪbool��
 * ()Ljava/lang/String; �޲��� ����ֵΪString��
 * (II)I  ����Ϊ2��int�� ����ֵΪint��
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
	clazz = env->FindClass(kClassName); //���ұ�ע�����
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
	} //������ע�᱾SO��Native�Ľӿڣ��ӿڶ�����gMethods������
	/* fill out the rest of the ID cache */
	return 0;
} 

/*
 *
 *   * This is called by the VM when the shared library is first loaded.
 */ 
/* ����һ�μ��ش˿�ʱ�����ô˺���ע�᱾�ط��� */
jint JNI_OnLoad(JavaVM* vm, void* reserved) {
	JNIEnv* env = NULL;
	jint result = -1;
	LOGI("zzs_JNI_OnLoad");

	if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
		LOGE("ERROR: GetEnv failed\n");
		goto fail;
	} //��ȡ��ǰ��VM�Ļ�����������env������

	assert(env != NULL);
	if (registerMethods(env) != 0) {
		LOGE("ERROR: PlatformLibrary native registration failed\n");
		goto fail;
	} //�Լ�д�ĺ�������ǰJAVA������ע��ӿ�
	/* success -- return valid version number */	
	result = JNI_VERSION_1_4;

fail:
	return result;
} 

