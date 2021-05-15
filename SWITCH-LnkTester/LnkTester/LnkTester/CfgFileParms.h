// readfile.cpp : Defines the entry point for the console application.
//
//#include "pch.h"
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <winsock.h>
#pragma comment(lib,"wsock32.lib")
//#include <iostream>
using namespace std;


class CCfgFileParms
{
public:
	enum AddrType { LOWER, LOCAL, UPPER, CMDER };
	vector<string*> archSection; //��Ÿ���������ݵ�ԭʼ����
	vector<string*> linksSection;
	vector<string*> parmsSection;
	struct parmEntry {
		string name;
		string strValue;
		int value;
	};
	vector <struct parmEntry> cfgParms; //�ֽ��Ĳ���

	bool isConfigExist;

	int getValueInt(int& val, char* name); //���ݲ��������֣�ȡ�ò�������ֵ��name �������֣�0��ʾ�����ҵ���-1��ʾû�ҵ�
	int getValueInt(int& val, string name);
	string getValueStr(char* name);//���ݲ������֣�ȡ�ò���������ֵ
	string getValueStr(string name);//���ݲ������֣�ȡ�ò���������ֵ

	void print();
	int read();
	void setDeviceID(string ID)
	{
		deviceID = ID;
	}
	/*	void setDeviceID(int ID)
		{
			deviceID = itoa(ID,10);
		}
	*/	void setLayer(string lay)
	{
		layer = lay;
	}

	void setEntityID(string ID)
	{
		entityID = ID;
	}
	/*	void setEntityID(int ID)
		{
			entityID = itoa(ID,10);
		}

	*/
	CCfgFileParms();
	CCfgFileParms(string devID, string layerID, string entID);

	~CCfgFileParms();
protected:
	string deviceID;
	string layer;
	string entityID;

	string localAddr; //���ļ���ֻ��Ҫ����ַ������͵�ַ����ִ�����У��ٸ�����Ҫת��
	string localPort;
	vector<string> lowerAddr;
	vector<string> lowerPort;
	string upperAddr;
	string upperPort;
	string cmdAddr;
	string cmdPort;

	void myStrcpy(char* str, string src);

	void printArray();
	int readArray();

	int findAddr(string* pStr, string* pLay, string* pEnt);//�ڽṹ�ִ����ҵ�ָ���������ʵ��ŵ��ִ�λ��

private:
	unsigned short createHexPort(int inID, int inLayer, int inEntity, int inType);
	int getLayerNo(string* pStr, string* lay); //���ݽṹ�ִ���ָ�������������λ�ã���ò�εı�ţ������ɶ˿ں�
	string getAddr(string* str);
	string getPort(string* dev, int iLayer, string* ent);//��dev����κţ�ʵ������ɶ˿ں�
	string getEnt(string* str);//�ӽ�ȡ�õ�ʵ����������ȡȥ���������ʵ���
	bool isParmsLine(string* pstr);
	bool isSplitLine(string* pstr);
	int cleanSections();
	int compareDevID(string* pstr, string* dev);
	int getAddrFromSection(string* dev, string* lay, string* ent, string& addr, string& port, int& indexOfDev, int& indexOfThis, int& iLayNo);
	int getUpperAddr(size_t indexOfDev, size_t indexOfThis, int iLay, string& addr, string& port);
	int getLowerAddr(size_t indexOfDev, size_t indexOfThis, int iLay, vector<string>& addr, vector<string>& port);
	// �������ִ�����ȡ�豸�ţ�����Կո�ʼ�������
	string getDev(string* pstr);
	// ����·�ִ����趨��Χ�ڣ����ݶ��ţ�ȡ���豸�ź�ʵ��ţ�start�����ڷ�Χ�ڣ�end��������
	int getDevEntFromLink(string* pstr, int start, int end, string& dev, string& port);
public:
	string getUDPAddrString(AddrType type, int index);
	sockaddr_in getUDPAddr(AddrType type, int index);
	int getUDPAddrNumber(AddrType type);
	unsigned long myInetAddr2n(string* pstr);
	int getLinkIndex();
	string getDeviceID();
	string getLayer();
	string getEntity();
};
