package cn.com.farsight.SerialService;
import java.io.BufferedReader;
import java.io.UnsupportedEncodingException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

import android.util.Log;

public class SerialService {
	private static final String TAG = "SerialService";
	// �ײ��ʼ��״̬
	private boolean isInitOk = false;

	// ���ر��ؿ⣬read()��write()��ʵ��ͨ��JNI�ڱ��ش���ʵ��
	static {
		Log.d(TAG, "System.loadLibrary()");
		System.loadLibrary("serial_runtime");
	}   
	
	// �����������ڳ�ʼ������HAL��runtime
	public SerialService(){	  
		Log.d(TAG, "SerialService()");
		// ��ʼ������HAL
		isInitOk =  _init();
		Log.d(TAG, "_init()");
	}

	// ����������
	public String read() {
		Log.d(TAG, "read()");
		if(!isInitOk)
			return "���ڳ�ʼ��ʧ�ܣ���ȷ���Ѿ����Ӵ����豸��";
		// ���� Java String��˫�ֽ��ַ���������������ԭʼ�ֽ���������Ҫ����ת��
		byte[] data = new byte[128];
		// ��������ȡ�ֽ���
		_read(data, 128);
		 
		String ret;
		try{
			// ת��ΪJava String�ַ�
			ret = new String(data, "GBK");
		}catch(UnsupportedEncodingException e1) {
			return null;
		}
		return ret;
	}

	// д���ַ�������
	public int  write(String s) {
		Log.d(TAG, "write()");
		int len;
		try{
			// ��Java String�ַ���ת����ֽ�����д�봮��
			len = _write(s.getBytes("GBK"));
		}catch(UnsupportedEncodingException e1) {
			return -1;
		}
		return len;
	}

	private static native boolean _init();
	private static native int _read(byte[] data, int len);
	private static native int _write(byte[] data);

}
