// readfile.cpp : Defines the entry point for the console application.
//
//#include "stdafx.h"
//#include "pch.h"
#include <string>
#include <fstream>
#include <iostream>
#include "cfgFileParms.h"
//#include <iostream>
using namespace std;
//20200228���������ļ���
//Ŀ����UDP�˿ںŶ������豸�ź�ʵ����Զ����ɣ��˿ڲ��ܹ���ʱ������һ������������ȼ�����ж˿��Ƿ��ܹ�����
//���²��ϵ��һ��ͳһ������������Ҳ���Զ����ɣ����������Զ�����
//�������ӹ�ϵ��һ����������������������������ӶԶ˵���Ϣ��ע�⹲���ŵ���Ĭ�ϵ�һ��������ʵ���ǹ���˿�
//ͳһ����Ҳ��ڵ����ݶ˿ڣ�����������Դ���ж�
//�������������ˣ����ٷ�BASIC���飬���һ���ж��������Ͳ�ӿ�ģʽ����lowerMode0=��lowerMode1=����ʾ
CCfgFileParms::CCfgFileParms()
{
	//��ʼ��

	isConfigExist = false;

}
CCfgFileParms::CCfgFileParms(string devID, string layerID, string entID)
{
	//��ʼ��
	isConfigExist = false;

	deviceID = devID;
	layer = layerID;
	entityID = entID;
	read();
}

void CCfgFileParms::myStrcpy(char* str, string src) //ֻ����ASCII�����32���ַ���32Ϊ�ո����µĶ��ǿ����ַ�
{
	size_t i, j;
	j = 0;
	for (i = 0; i < strlen(src.c_str()); i++) {
		if (src[i] > 32 || src[i] < 0) {
			str[j] = src.c_str()[i];
			j++;
		}
	}
	str[j] = 0;
}
int CCfgFileParms::getValueInt(int& val, char* name)
{
	size_t i;
	int retval;

	for (i = 0; i < cfgParms.size(); i++) {
		retval = (int)cfgParms[i].name.find(name);
		if (retval >= 0) {
			val = cfgParms[i].value;
			return 0;
		}
	}
	return -1;
}
int CCfgFileParms::getValueInt(int& val, string name)
{
	size_t i;
	int retval;

	for (i = 0; i < cfgParms.size(); i++) {
		retval = (int)cfgParms[i].name.find(name);
		if (retval >= 0) {
			val = cfgParms[i].value;
			return 0;
		}
	}
	return -1;
}
string CCfgFileParms::getValueStr(char* name)
{
	size_t i;
	int retval;

	for (i = 0; i < cfgParms.size(); i++) {
		retval = (int)cfgParms[i].name.find(name);
		if (retval >= 0) {
			return cfgParms[i].strValue;
		}
	}
	return string();
}
string CCfgFileParms::getValueStr(string name)
{
	size_t i;
	int retval;

	for (i = 0; i < cfgParms.size(); i++) {
		retval = (int)cfgParms[i].name.find(name);
		if (retval >= 0) {
			return cfgParms[i].strValue;
		}
	}
	return string();
}
//�����ӡ
void CCfgFileParms::printArray()
{
	size_t i;
	// TODO: �ڴ˴����ʵ�ִ���.
	for (i = 0; i < cfgParms.size(); i++) {
		cout << cfgParms[i].name << " = " << cfgParms[i].strValue << endl;
	}
	return;
}

void CCfgFileParms::print()
{
	size_t i;
	cout << "�豸��: " << deviceID << " ���: " << layer << "ʵ��: " << entityID << endl;
	cout << "�ϲ�ʵ���ַ: " << upperAddr << "  UDP�˿ں�; " << upperPort << endl;
	cout << "����ʵ���ַ: " << localAddr << "  UDP�˿ں�; " << localPort << endl;
	if (layer.compare("PHY") == 0) {
		if ((int)lowerAddr.size() <= 1) {
			cout << "�²�㵽���ŵ�" << endl;
			cout << "��·�Զ˵�ַ: ";
		}
		else {
			cout << "�²�㲥ʽ�ŵ�" << endl;
			cout << "�����ŵ�վ�㣺";
		}
	}
	else {
		cout << "�²�ʵ��";
	}
	if ((int)lowerAddr.size() == 1) {
		cout << "��ַ��" << lowerAddr[0] << "  UDP�˿ں�; " << lowerPort[0] << endl;
	}
	else {
		if (layer.compare("PHY") == 0) {
			cout << endl;
			for (i = 0; i < lowerAddr.size(); i++) {
				cout << "        ��ַ��" << lowerAddr[i] << "  UDP�˿ں�; " << lowerPort[i] << endl;
			}
		}
		else {
			cout << endl;
			for (i = 0; i < lowerAddr.size(); i++) {
				cout << "        �ӿ�: " << i << " ��ַ" << lowerAddr[i] << "  UDP�˿ں�; " << lowerPort[i] << endl;
			}
		}
	}
	cout << "ͳһ����ƽ̨��ַ: " << cmdAddr << "  UDP�˿ں�; " << cmdPort << endl;
	cout << "##################" << endl;
	printArray();
	cout << "--------------------------------------------------------------------" << endl;
	cout << endl;

}
int CCfgFileParms::readArray()
{
	string* pstrTmp;
	string strInt;
	size_t i, j;
	int pos;
	struct parmEntry sParm;
	int begin, end;
	//ÿ���ж��ٸ�����

	for (i = 0; i < parmsSection.size(); i++) {
		pstrTmp = parmsSection[i];
		pos = (int)pstrTmp->find("=");
		if (pos < 0) {
			//������ȷ�Ĳ����У�����
			continue;
		}
		sParm.name = pstrTmp->substr(0, pos); //ǰ����tab���Ϳո񲻹��ˣ��жϱ�������ʱ����find������compare
		begin = max((int)sParm.name.find_first_not_of(' ', 0), (int)sParm.name.find_first_not_of(9, 0));
		end = min((int)sParm.name.find_last_not_of(' '), (int)sParm.name.find_last_not_of(9));
		sParm.name = sParm.name.substr(begin, end - begin + 1);
		sParm.strValue = pstrTmp->substr(pos + 1);
		sParm.value = atoi(sParm.strValue.c_str());
		//���֮ǰ��û����ͬ�Ĳ������ú���ľֲ��������֮ǰ��ȫ�ֲ���
		for (j = 0; j < cfgParms.size(); j++) {
			if (sParm.name.compare(cfgParms[j].name) == 0) {
				break;
			}
		}
		if (j < cfgParms.size()) {
			//֮ǰ��ȫ�ֲ����������ֵ
			cfgParms[j].strValue = sParm.strValue;
			cfgParms[j].value = sParm.value;
		}
		else {
			//��ӽ���
			cfgParms.push_back(sParm);
		}
	}
	return 0;
}
//�ڽṹ�ִ����ҵ�ָ���������ʵ��ŵ��ִ���ʼλ�á�����ֵ-1����ʾ����û�ҵ�ʵ�����
int CCfgFileParms::findAddr(string* pStr, string* pLay, string* pEnt)
{
	string strTmp;
	int begin;

	//����Ӧ���в�������Ϊ�пո񣩣�û���豸�ţ������豸��ƥ��
	strTmp = pLay->c_str();
	strTmp += pEnt->c_str();
	begin = (int)pStr->find(strTmp, 0);
	//����ʵ����ڲ����ַ����е�λ��
	return begin;
}
//����ڽṹ�ִ��У�ָ��layer�Ĳ�κţ��ڽṹ�ִ��У�ÿ��ÿ��ֻ��һ��ʵ�塣ͬ����ʵ��Ҫ���б�ʾ
//���εı��Ҳ���ǻ�ò��λ�ã����֮���Կո�����û���豸�ţ�Ҫ�Կո�ʼ
//����ֵ1��ʾ��Ͳ㣬-1��ʾû�ҵ�
int CCfgFileParms::getLayerNo(string* pStr, string* lay)
{
	string strTmp;
	int begin;
	int end;
	int i;
	//�ж��豸�����,-2 ��ʾ�豸�Ų�ƥ�䣬û���豸����ɼ���
	begin = (int)pStr->find(' ');
	if (begin == -1) {
		//�����ַ���û�пո񣬸�ʽ����,����
		return -1;
	}
	for (i = 1; begin < (int)pStr->length() && begin >= 0; i++) {
		//��begin��ʼ�ҵ�һ����0���ֶ�,������һ�׶οո�
		begin = (int)pStr->find_first_not_of(' ', begin);
		if (begin == -1) {
			//�����ո�֮��û����
			break;
		}
		//��ȡ�Ӳ���
		strTmp = pStr->substr(begin, 3);
		if ((int)strTmp.find(lay->c_str(), 0, 3) != -1) {
			//�ҵ������Լ��Ĳ����
			break;
		}
		end = (int)pStr->find_last_of(strTmp.c_str(), begin, 3);
		begin = (int)pStr->find_first_of(' ', end);
	}
	if (begin < 0) {
		//�����Ĳ���������Ҳ���
		return -1;
	}
	return i;
}
//�ӽ�ȡ��ʵ�������ִ���@��ȡ��IP��ַ�����û��@����Ĭ��Ϊ�ػ���ַ
string CCfgFileParms::getAddr(string* str)
{
	int retval;
	string addr;
	// TODO: �ڴ˴����ʵ�ִ���.
	retval = (int)str->find('@');
	if (retval == -1) {
		//ʹ��Ĭ�ϲ���
		addr = "127.0.0.1";
	}
	else {
		//ȡ@���IP��ַ
		addr = str->substr(retval + 1);
	}
	return addr;
}
//��dev����κţ�ʵ�������UDP�˿ںţ���ʽΪʮ���ƣ�1 dev1 layer1 enty2
string CCfgFileParms::getPort(string* dev, int iLayer, string* ent)
{
	string strTmp;
	string strLayer;
	//���豸�ţ���Σ�ʵ�������ΨһUDP�˿ں�
	strTmp = "1";
	strTmp += dev->c_str();
	/*
	if (layer.compare("PHY") == 0) {
		strTmp += "1"; //��κ�;
	}
	*/
	strLayer = std::to_string(iLayer);//��κ�;

	strTmp += strLayer;

	if (ent->length() == 1) {
		//ֻ��1λ������0
		strTmp += "0";
	}
	strTmp += ent->c_str();

	return strTmp;
}
//�ӽ�ȡ�õ�ʵ�������ֶ��У���ȡʵ���
string CCfgFileParms::getEnt(string* str)
{
	string strEnt;
	int retval;
	int i;
	//Ҫ��str���ҳ�ʵ����ֶ�
	retval = (int)str->find("@");
	if (retval == -1) {
		//û��IP��ַ�����1�������ַ�������
		strEnt = str->c_str();
	}
	else {
		//ȡ@֮ǰ
		strEnt = str->substr(0, retval);
	}
	for (i = (int)strEnt.length() - 1; i >= 0; i--) {
		if (strEnt.at(i) < '0' || strEnt.at(i) > '9')
			break;
	}
	strEnt = strEnt.substr(i + 1, strEnt.length() - i - 1);

	return strEnt;
}
//�жϱ����Ƿ��ǲ����У������ǿ�ͷ��#
bool CCfgFileParms::isParmsLine(string* pstr)
{
	// TODO: �ڴ˴����ʵ�ִ���.
	if (pstr->size() == 0) {
		return false;
	}
	if (pstr->at(0) == '#' || isSplitLine(pstr)) {
		return false;
	}
	return true;
}
//�жϱ����Ƿ�Ϊ�ָ��У�������������----
bool CCfgFileParms::isSplitLine(string* pstr)
{
	int retval;
	// TODO: �ڴ˴����ʵ�ִ���.
	retval = (int)pstr->find("--------");
	if (retval < 0) {
		return false;
	}
	return true;
}

//�� pstr�бȽ��豸�š�����ֵ0����ʾƥ�䣬-1��ʾpstr��û���豸�ţ�1��ʾpstr���豸�ŵ��ǲ�ƥ��
int CCfgFileParms::compareDevID(string* pStr, string* dev)
{
	int end;
	string strTmp;
	//�ж��豸�����,-2 ��ʾ�豸�Ų�ƥ�䣬û���豸����ɼ���
	end = (int)pStr->find(' ');
	if (end == -1) {
		//�����ַ���û�пո񣬸�ʽ����,����
		return -1;
	}
	else if (end == 0) {
		//û���豸���ֶ�
		return -1;
	}
	if (end > 1) {
		//�豸�Ź�����ʱ��֧��
		return 1;
	}
	//ȡ�µ�һ���ֶ��ж��豸��
	strTmp = pStr->substr(0, end);
	if (strTmp.compare(dev->c_str()) != 0) {
		return 1;
	}
	return 0;
}
// �������ִ�����ȡ�豸�ţ�����ִ��Կո�ʼ������ȡ������
string CCfgFileParms::getDev(string* pstr)
{
	int retval;
	// TODO: �ڴ˴����ʵ�ִ���.
	retval = (int)pstr->find_first_of(" ");
	if (retval <= 0) {
		return string();
	}
	return pstr->substr(0, retval);
}

//��ָ���豸����Σ�ʵ��ŵ���������section���ҵ�ʵ�������ӣ�������ַ��UDP�˿ں�
//����ֵ��С��0��ʾ���ִ���0��ʾ�ɹ�
//�����addr����ַ��port��UDP�˿ںţ�indexOfDev�����豸����������λ�ã�indexOfThis����ʵ�������У�iLayNo�����α��
int CCfgFileParms::getAddrFromSection(string* dev, string* lay, string* ent, string& addr, string& port, int& indexOfDev, int& indexOfThis, int& iLayNo)
{
	int deviceFind;
	size_t index;
	string* pstrTmp;
	int retval;
	int begin;
	int end;
	string strTmp;
	// ��archSection����ָ���豸����κ�ʵ��ĵ�ַ��UDP�˿ں���ȡ����
	deviceFind = 0;
	for (index = 0; index < archSection.size(); index++) {
		pstrTmp = archSection[index];
		//����豸��
		retval = compareDevID(pstrTmp, dev);
		if (!deviceFind) {
			if (retval != 0) {
				continue;
			}
			else {
				deviceFind = 1;
				indexOfDev = (int)index;
			}
		}
		else {
			if (retval == 1) {
				break;
			}
		}
		//�豸��ƥ�䣬����λ���豸��ƥ��ķ�Χ�ڣ��ж�ʵ������Ƿ����
		begin = findAddr(pstrTmp, lay, ent);
		if (begin == -1) {
			//����ָ��ʵ�����������һ��
			continue;
		}
		//�ҵ�ʵ�壬��ȡ����ֵ
		indexOfThis = (int)index;
		end = (int)pstrTmp->find(' ', begin);
		if (end == -1) {
			strTmp = pstrTmp->substr(begin);
		}
		else {
			end = end - begin;
			strTmp = pstrTmp->substr(begin, end);
		}
		//�Ӳ����л�ȡIP��ַ��������Ĭ�ϵı��ػػ���ַ
		addr = getAddr(&strTmp);
		//ͨ��������������κ�,1�ǵ�һ��
		iLayNo = getLayerNo(pstrTmp, lay);
		if (iLayNo < 1) {
			//ָ����layer���������ļ����Ҳ���
			return -3;
		}
		//����UDP�˿ں�
		port = getPort(dev, iLayNo, ent);
		break;
	}
	if (index == archSection.size()) {
		return -1;
	}
	return 0;
}
//���ܣ���ָ�����������У�ȡ��ĳ��ʵ����ϲ��ַ��Ϣ
//      �����Ǵӱ������ϲ㣬û������������Ѱ�ң�ֱ����ȫ���豸��
//���룺����index��indexOfDevָ�����豸������section����ʼλ�ã�indexOfThis��ʾ�����ڵ������У�iLay����Ĳ�α�ţ����ڶ�λ���²�
//����ֵ��-1��ʾû�и��߲㣬0��ʾ�ҵ�
//�����addr�����IP��ַ��port���UDP�˿ں�
int CCfgFileParms::getUpperAddr(size_t indexOfDev, size_t indexOfThis, int iLay, string& addr, string& port)
{
	string* pstrTmp;
	size_t index;
	string strTmp;
	string strEnt;
	string strDev;
	int begin;
	int end;
	int count;
	//�õ��豸��
	strDev = getDev(archSection[indexOfDev]);
	for (index = indexOfThis; (int)index >= (int)indexOfDev; index--) {
		pstrTmp = archSection[index];
		//������һ���ո�����
		begin = (int)pstrTmp->find_first_of(" ", 0);
		begin = (int)pstrTmp->find_first_not_of(" ", begin);

		end = begin;
		count = 0;
		while (end > 0) {
			count++;
			if (count == iLay + 1)
				break;
			end = (int)pstrTmp->find_first_of(" ", begin);
			if (end == -1)
				break;
			begin = (int)pstrTmp->find_first_not_of(" ", end);
			end = begin;
		}
		if (count < iLay + 1) {
			//û���ҵ��߲�
			continue;
		}
		else {
			//�и߲㣬��ȡ�߲��ֶ�
			end = (int)pstrTmp->find_first_of(" ", begin);
			if (end > 0) {
				//�и߲��ֶΣ���ȡ
				strTmp = pstrTmp->substr(begin, end - begin);
			}
			else {
				strTmp = pstrTmp->substr(begin);
			}
			//��ȡ�߲����ֵ
			addr = getAddr(&strTmp);
			strEnt = getEnt(&strTmp);
			port = getPort(&strDev, iLay + 1, &strEnt);
			//port = getPort(&strTmp, deviceID, iLayer + 1);
			break;
		}
	}
	if ((int)index < (int)indexOfDev) {
		//û�ҵ�
		return -1;
	}
	return 0;
}
// ����·�ִ����趨��Χ�ڣ����ݶ��ţ�ȡ���豸�ź�ʵ��ţ�start�����ڷ�Χ�ڣ�end�������ڷ�Χ�ڣ�
int CCfgFileParms::getDevEntFromLink(string* pstr, int start, int end, string& dev, string& port)
{
	int com;
	// TODO: �ڴ˴����ʵ�ִ���.
	com = (int)pstr->find_first_of(",", start);
	if (com == -1) {
		return -6;//��·��ʽ����ȱ�� --
	}
	dev = pstr->substr(start, com - start);
	port = pstr->substr(com + 1, end - com - 1);
	return 0;
}

//���ܣ���ָ�����������У�ȡ��ĳ��ʵ����²��ַ��Ϣ
//      �����Ǿ��ڱ����ҵͲ㣬λ��ǰ��һ��,ͬʱ��Ҫע�⣬��Ҫ�ѱ����йصĵͲ㶼��������
//���룺����index��indexOfDevָ�����豸������section����ʼλ�ã���ȡ�豸�ţ�indexOfThis��ʾ�����ڵ������У�iLay����Ĳ�α�ţ����ڶ�λ���²�
//����ֵ��-1��ʾû�еͲ㣬0��ʾ�ҵ�
//�����addr�����IP��ַ��port���UDP�˿ں�
int CCfgFileParms::getLowerAddr(size_t indexOfDev, size_t indexOfThis, int iLay, vector<string>& addr, vector<string>& port)
{
	string* pstrTmp;
	size_t index;
	string strTmp;
	string strEnt;
	string strDev;
	int begin;
	int end;
	int count;
	int retval;

	strDev = getDev(archSection[indexOfDev]);
	//�ҵ��Ͳ�ʵ�������
	if (iLay > 1) {
		//������Ͳ㣬ǰ��Ż���ʵ�壬ֻȡǰһ��ʵ���ַ�⣬��Ҫ����������ȫ����������еͲ�ʵ��
		strDev = getDev(archSection[indexOfDev]);
		for (index = indexOfThis; index < archSection.size(); index++) {
			pstrTmp = archSection[index];
			//�豸�Ų�ƥ���ֱ������
			retval = compareDevID(pstrTmp, &strDev);
			if (retval == 1) {
				return 0;
			}
			//������һ���ո�����
			begin = (int)pstrTmp->find_first_of(" ", 0);
			begin = (int)pstrTmp->find_first_not_of(" ", begin);

			end = begin;
			count = 0;
			while (end > 0) {
				count++;
				if (count == iLay - 1)
					break;
				end = (int)pstrTmp->find_first_of(" ", begin);
				if (end == -1)
					break;
				begin = (int)pstrTmp->find_first_not_of(" ", end);
				end = begin;
			}
			if (count < iLay - 1) {
				//�Ͳ㲻������������
				continue;
			}
			else {
				//�еͲ㣬���ǻ�Ҫ��ͬ���Ƿ��У����ͬ���ң���һ���ģ������Ѱ��
				if (index != indexOfThis) {
					//������
					end = (int)pstrTmp->find_first_of(" ", begin);
					if (end > 0) {
						end = (int)pstrTmp->find_first_not_of(" ", end);
						if (end > 0) {
							//��ͬ�㣬����
							return 0;
						}
					}
				}
			}
			//�к��ʵͲ㣬��ȡ�ֶ�
			end = (int)pstrTmp->find_first_of(" ", begin);
			if (end > 0) {
				//�еͲ��ֶΣ���ȡ
				strTmp = pstrTmp->substr(begin, end - begin);
			}
			else {
				strTmp = pstrTmp->substr(begin);
			}
			//��ȡ�Ͳ����ֵ
			addr.push_back(getAddr(&strTmp));
			strEnt = getEnt(&strTmp);
			port.push_back(getPort(&strDev, iLay - 1, &strEnt));
		}

	}
	else {
		int mybegin;
		string strAddrTmp;
		string strPortTmp;
		string strPeerDev;
		string strPeerEnt;//layerһ���������
		int x, y, z;
		//�����������ʵ�壬����·�ҵ��Զ˵�ַ
		strTmp = deviceID;//getDev(archSection[indexOfDev]);
		strTmp += ",";
		strTmp += entityID; //ֱ�����˹�������Ӧ�ô�Layer������Ȼ��ȡ�ã�̫�鷳


		for (index = 0; index < linksSection.size(); index++) {
			pstrTmp = linksSection[index];
			mybegin = (int)pstrTmp->find(strTmp);
			if (mybegin == -1) {
				continue;
			}
			//�ҵ�������ʵ�����·���������ҽ���һ��
			//һ��һ���г������ҵ��Զ˵�ַ�����
			begin = 0;
			end = 0;
			end = (int)pstrTmp->find("--", begin);
			while (begin < (int)pstrTmp->length()) {
				if (begin != mybegin) {
					//ȡ�öԵ�ʵ���豸�ź�ʵ���
					retval = getDevEntFromLink(pstrTmp, begin, end, strPeerDev, strPeerEnt);
					if (retval < 0) {
						return retval;
					}
					//��archSection�а��豸�ţ��������ʵ��ţ�ȡ����ַ
					retval = getAddrFromSection(&strPeerDev, &layer, &strPeerEnt, strAddrTmp, strPortTmp, x, y, z);
					if (retval < 0) {
						return -7;
					}
					lowerAddr.push_back(strAddrTmp);
					lowerPort.push_back(strPortTmp);
				}
				//ȷ����һ���ֶε�λ��
				begin = end + 2;
				end = (int)pstrTmp->find("--", begin);
				if (end == -1)
					end = (int)pstrTmp->length();
			}
			break;
		}
		if (index >= linksSection.size()) {
			//û�ҵ�����Ԫ�޷���������
			return -5;
		}
	}
	return 0;
}

//-1�������ļ������ڣ�-2�������ļ���û����Ч������;-3 �ڸ��豸�Ľṹ��������û�ҵ�ָ��ʵ��
//-4: �ṹ�����У���Ӧ������Ͳ��ǰ��û�еͲ�ʵ�������-5����·�����Ҳ�����ʵ��
//-6����·�����и�ʽ�д�û�ҵ�--,-7����·��Զ�û���ڽṹ����������
//-8: ���������������ڵ�
int CCfgFileParms::read()
{
	string strTmp;
	string strTmp2;
	string* pstrTmp; //ÿһ���ִ�ָ��
	//string strInt;
	bool isStart = false;
	int deviceFind = 0;
	int layerFind = 0;
	int entityFind = 0;
	int retval;
	size_t index = 0;
	int iLayer;
	int indexDev;
	int indexThis;
	struct parmEntry sParmEntry;

	ifstream cfgFile("ne.txt");

	if (!cfgFile.is_open())
	{
		isConfigExist = false;
		return -1;
	}

	//���������������
	cleanSections();
	cfgParms.clear();
	//���ļ����ݷ���ȫ������
	//��������һ��������֮ǰ��
	while (!cfgFile.eof()) {
		getline(cfgFile, strTmp);
		if (isSplitLine(&strTmp)) {
			break;
		}
	}
	if (cfgFile.eof()) {
		//û�ж�����Ч����
		isConfigExist = 0;
		cfgFile.close();
		return -2;
	}
	//����ܹ��͵�ַ��������
	while (!cfgFile.eof()) {
		getline(cfgFile, strTmp);
		if (isSplitLine(&strTmp)) {
			break;
		}
		if (!isParmsLine(&strTmp)) {
			continue;
		}
		pstrTmp = new string(strTmp.c_str());
		archSection.push_back(pstrTmp);
	}
	//������·��������
	while (!cfgFile.eof()) {
		getline(cfgFile, strTmp);
		if (isSplitLine(&strTmp)) {
			break;
		}
		if (!isParmsLine(&strTmp)) {
			continue;
		}
		pstrTmp = new string(strTmp.c_str());
		linksSection.push_back(pstrTmp);
	}
	//����ͳһƽ̨��������
	while (!cfgFile.eof()) {
		getline(cfgFile, strTmp);
		if (isSplitLine(&strTmp)) {
			break;
		}
		if (!isParmsLine(&strTmp)) {
			continue;
		}
		retval = (int)strTmp.find("cmdIpAddr");
		if (retval >= 0) {
			cmdAddr = strTmp.substr(strTmp.find("=") + 1);
		}
		else {
			retval = (int)strTmp.find("cmdPort");
			if (retval >= 0) {
				cmdPort = strTmp.substr((int)strTmp.find("=") + 1);
			}
			else {
				//ȫ�ֲ��������������
				pstrTmp = new string(strTmp.c_str());
				parmsSection.push_back(pstrTmp);
			}
		}
	}
	//�������������������ݣ�ֻ����ʵ���йز���
	deviceFind = 0;
	layerFind = 0;
	entityFind = 0;
	while (!cfgFile.eof()) {
		getline(cfgFile, strTmp);
		if (!isParmsLine(&strTmp)) {
			continue;
		}
		if (isSplitLine(&strTmp)) {
			//�ָ��ߣ���״̬
			if (entityFind) {
				//�ý�����
				break;
			}
		}
		//��deviceID��ʼ
		if (!deviceFind) {
			retval = (int)strTmp.find("deviceID");
			if (retval == -1) {
				continue;
			}
			retval = (int)strTmp.find(deviceID);
			if (retval == -1) {
				continue;
			}
			//�ҵ��豸
			deviceFind = 1;
			continue;
		}
		else {
			retval = (int)strTmp.find("deviceID");
			if (retval >= 0) {
				//���Ѿ����ֹ�deviceID������£��ֶ���deviceID�У������µ�deviceID�ˣ��˳�
				break;
			}
		}
		if (!layerFind) {
			retval = (int)strTmp.find("layer");
			if (retval == -1) {
				continue;
			}
			retval = (int)strTmp.find(layer);
			if (retval == -1) {
				continue;
			}
			//�ҵ����
			layerFind = 1;
			continue;
		}
		else {
			retval = (int)strTmp.find("layer");
			if (retval >= 0) {
				//���Ѿ����ֹ�layer������£��ֶ���layer�У������µ�layer�ˣ��˳�
				break;
			}

		}
		if (!entityFind) {
			retval = (int)strTmp.find("entityID");
			if (retval == -1) {
				continue;
			}
			retval = (int)strTmp.find(entityID);
			if (retval == -1) {
				continue;
			}
			//�ҵ�ʵ��
			entityFind = 1;
			continue;
		}
		else {
			retval = (int)strTmp.find("entityID");
			if (retval >= 0 || isSplitLine(&strTmp)) {
				//���Ѿ����ֹ�entieyID������£��ֶ���entieyID�л��߷ָ��У������µ�entieyID�ˣ��˳�
				break;
			}

		}

		//���������
		pstrTmp = new string(strTmp.c_str());
		parmsSection.push_back(pstrTmp);
	}

	cfgFile.close();

	//�ڼܹ��������ҵ���ʵ���IP��ַ��UDP�˿ں��ִ�
	retval = getAddrFromSection(&deviceID, &layer, &entityID, localAddr, localPort, indexDev, indexThis, iLayer);
	if (retval < 0) {
		isConfigExist = false;
		return retval;
	}
	//��õͲ��ַ
	retval = getLowerAddr(indexDev, indexThis, iLayer, lowerAddr, lowerPort);
	if (retval < 0) {
		//����û�еͲ㣬PHY�ĵͲ������·�Զ�
		return retval;
	}
	//��ø߲��ַ
	retval = getUpperAddr(indexDev, indexThis, iLayer, upperAddr, upperPort);

	//�ж��ж��ٸ���Ԫ���Ի�ò���layout,�����ò�������ʽ���뵽��������
	strTmp = "z"; //�Ƚ��豸��ʱ��ƥ��

	sParmEntry.value = 0;
	for (index = 0; index < archSection.size(); index++) {
		pstrTmp = archSection[index];
		retval = compareDevID(pstrTmp, &strTmp);
		if (retval == 1) {
			sParmEntry.value++;
		}
	}
	if (sParmEntry.value >= 2) {
		sParmEntry.name = "layOut";
		sParmEntry.strValue = std::to_string(sParmEntry.value);
	}
	else {
		return -8;
	}

	cfgParms.push_back(sParmEntry);

	//���������������ϣ���ֳɱ������ͱ���ֵ�ṹ
	readArray();

	isConfigExist = true;

	return 0;
}

int CCfgFileParms::cleanSections()
{
	string* pstrTmp;
	// TODO: �ڴ˴����ʵ�ִ���.
	while (!archSection.empty()) {
		pstrTmp = archSection[archSection.size() - 1];
		archSection.pop_back();  //��β�͵���
		delete pstrTmp;
	}
	while (!linksSection.empty()) {
		pstrTmp = linksSection[linksSection.size() - 1];
		linksSection.pop_back();
		delete pstrTmp;
	}
	while (!parmsSection.empty()) {
		pstrTmp = parmsSection[parmsSection.size() - 1];
		parmsSection.pop_back();
		delete pstrTmp;
	}

	return 0;
}

CCfgFileParms::~CCfgFileParms()
{
	//�ͷ��ļ�����
	cleanSections();

}
//�����豸�ţ���κţ�ʵ��ţ���16���Ƹ�ʽ����UDP�˿ں�,��Ϊ����ֵ��
//����:���λΪ0���θ�λΪ1������Ϊbit0��b0~b3�������ͣ�b4~b7�����豸�ţ�b8~b11������κţ�b12~b15����ʵ���
unsigned short CCfgFileParms::createHexPort(int inID, int inLayer, int inEntity, int inType)
{
	// TODO: �ڴ˴����ʵ�ִ���.
	unsigned short port;
	port = 0;
	//��װʵ���
	port = (inEntity & 0xf) | port;
	//��װ��κ�
	port = ((inLayer & 0xf) << 4) | port;
	//��װ�豸��
	port = ((inID & 0xf) << 8) | port;
	//��װ���ͺţ�typeֻ�����֣���ͨΪ0������Ϊ1
	port = ((inType & 0xf) << 12) | port;
	//���λ��λ0���θ�λ��Ϊ1
	port = (port & 0x3fff) | 0x4000;

	return port;
}
//���ָ��type���׽��ֵ�ַ
//���룺type�� -1��ʾ�Ͳ㣬0��ʾ���㣬1��ʾ�߲�
sockaddr_in CCfgFileParms::getUDPAddr(AddrType type, int index)
{
	sockaddr_in addr;
	addr.sin_family = 0;
	addr.sin_addr.S_un.S_addr = 0;
	addr.sin_port = 0;
	// TODO: �ڴ˴����ʵ�ִ���.
	switch (type) {
	case LOWER:
		if (lowerAddr.size() == 0 || index >= (int)lowerAddr.size()) {
			break;
		}
		addr.sin_family = AF_INET;
		addr.sin_addr.S_un.S_addr = myInetAddr2n(&(lowerAddr[index]));
		addr.sin_port = htons(atoi(lowerPort[index].c_str()));
		break;
	case LOCAL:
		addr.sin_family = AF_INET;
		addr.sin_addr.S_un.S_addr = myInetAddr2n(&localAddr);
		addr.sin_port = htons(atoi(localPort.c_str()));

		break;
	case UPPER:
		addr.sin_family = AF_INET;
		addr.sin_addr.S_un.S_addr = myInetAddr2n(&upperAddr);
		addr.sin_port = htons(atoi(upperPort.c_str()));
		break;
	case CMDER:
		addr.sin_family = AF_INET;
		addr.sin_addr.S_un.S_addr = myInetAddr2n(&cmdAddr);
		addr.sin_port = htons(atoi(cmdPort.c_str()));
		break;
	}
	return addr;
}
string CCfgFileParms::getUDPAddrString(AddrType type, int index)
{
	string strTmp;
	// TODO: �ڴ˴����ʵ�ִ���.
	switch (type) {
	case LOWER:
		if (lowerAddr.size() == 0 || index >= (int)lowerAddr.size()) {
			break;
		}
		strTmp = lowerAddr[index].c_str();
		strTmp += ":";
		strTmp += lowerPort[index].c_str();
		break;
	case LOCAL:
		strTmp = localAddr.c_str();
		strTmp += ":";
		strTmp += localAddr.c_str();
		break;
	case UPPER:
		strTmp = upperAddr.c_str();
		strTmp += ":";
		strTmp += upperPort.c_str();
		break;
	}
	return strTmp;
}
//���һ���ַ����������Ҫ�ǵͲ�ӿڵ�������
//���룺type�� -1��ʾ�Ͳ㣬0��ʾ���㣬1��ʾ�߲�
int CCfgFileParms::getUDPAddrNumber(AddrType type)
{
	// TODO: �ڴ˴����ʵ�ִ���.
	switch (type) {
	case LOWER:
		return (int)lowerAddr.size();
	}
	return 1;
}
//�Ա��IP��ַ�ַ���ת��Ϊ�����ֽ���������ʽ
unsigned long CCfgFileParms::myInetAddr2n(string* pstr)
{
	int begin;
	int end;
	unsigned long retval;
	unsigned long temp;
	string strTmp;
	// TODO: �ڴ˴����ʵ�ִ���.
	retval = 0;
	begin = (int)pstr->find_first_of(".");
	if (begin < 0)
		return 0;
	strTmp = pstr->substr(0, begin);
	temp = atoi(strTmp.c_str());
	retval = (temp << 24) & 0xff000000;

	begin = begin + 1;
	end = (int)pstr->find_first_of(".", begin);
	if (end < 0) {
		return 0;
	}
	strTmp = pstr->substr(begin, end - begin);
	temp = atoi(strTmp.c_str());
	retval += (temp << 16) & 0x00ff0000;

	begin = end + 1;
	end = (int)pstr->find_first_of(".", begin);
	if (end < 0) {
		return 0;
	}
	strTmp = pstr->substr(begin, end - begin);
	temp = atoi(strTmp.c_str());
	retval += (temp << 8) & 0x0000ff00;

	begin = end + 1;
	strTmp = pstr->substr(begin);
	temp = atoi(strTmp.c_str());
	retval += temp & 0x000000ff;

	return htonl(retval);
}

//�����·�������ļ��еı�ţ�����ͳһ������ɫ����ͬ��·�ϵĽ��涼��һ����ɫ
//�������ʹ��
int CCfgFileParms::getLinkIndex()
{
	string strTmp;
	string* pstrTmp;
	size_t index;
	int retval;
	// TODO: �ڴ˴����ʵ�ִ���.
	if (layer.compare("PHY") != 0) {
		return -1;
	}
	//�����������ʵ�壬����·�ҵ��Զ˵�ַ
	strTmp = deviceID;//getDev(archSection[indexOfDev]);
	strTmp += ",";
	strTmp += entityID; //ֱ�����˹�������Ӧ�ô�Layer������Ȼ��ȡ�ã�̫�鷳


	for (index = 0; index < linksSection.size(); index++) {
		pstrTmp = linksSection[index];
		retval = (int)pstrTmp->find(strTmp);
		if (retval >= 0) {
			break;
		}
	}
	if (index >= linksSection.size()) {
		return -1;
	}
	else
		return (int)index;
}

string CCfgFileParms::getDeviceID()
{
	// TODO: �ڴ˴����ʵ�ִ���.
	return deviceID;
}


string CCfgFileParms::getLayer()
{
	// TODO: �ڴ˴����ʵ�ִ���.
	return layer;
}


string CCfgFileParms::getEntity()
{
	// TODO: �ڴ˴����ʵ�ִ���.
	return entityID;
}