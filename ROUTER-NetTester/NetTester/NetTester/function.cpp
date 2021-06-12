//Nettester �Ĺ����ļ�
#include <iostream>
#include <conio.h>
#include "winsock.h"
#include "stdio.h"
#include "CfgFileParms.h"
#include "function.h"
using namespace std;

//����Ϊ��Ҫ�ı���
U8* sendbuf;        //������֯�������ݵĻ��棬��СΪMAX_BUFFER_SIZE,���������������������ƣ��γ��ʺϵĽṹ��������û��ʹ�ã�ֻ������һ��
int printCount = 0; //��ӡ����
int spin = 0;  //��ӡ��̬��Ϣ����

//------�����ķָ��ߣ�һЩͳ���õ�ȫ�ֱ���------------
int iSndTotal = 0;  //������������
int iSndTotalCount = 0; //���������ܴ���
int iSndErrorCount = 0;  //���ʹ������
int iRcvForward = 0;     //ת����������
int iRcvForwardCount = 0; //ת�������ܴ���
int iRcvToUpper = 0;      //�ӵͲ�ݽ��߲���������
int iRcvToUpperCount = 0;  //�ӵͲ�ݽ��߲������ܴ���
int iRcvUnknownCount = 0;  //�յ�������Դ�����ܴ���
int router_table2[20][2] = {130,0,
							146,0,};//·�ɱ�
int router_table3[20][2] = { 130,0,
							146,0,};//·�ɱ�
int router_table4[20][2] = { 130,0,
							146,0,};//·�ɱ�


//��ӡͳ����Ϣ
void print_statistics();
void menu();
//***************��Ҫ��������******************************
//���ƣ�InitFunction
//���ܣ���ʼ�������棬��main�����ڶ��������ļ�����ʽ������������ǰ����
//���룺
//�����
void InitFunction(CCfgFileParms& cfgParms)
{
	sendbuf = (char*)malloc(MAX_BUFFER_SIZE);
	if (sendbuf == NULL ) {
		cout << "�ڴ治��" << endl;
		//����������Ҳ̫���˳���
		exit(0);
	}
	return;
}
//***************��Ҫ��������******************************
//���ƣ�EndFunction
//���ܣ����������棬��main�������յ�exit������������˳�ǰ����
//���룺
//�����
void EndFunction()
{
	if(sendbuf != NULL)
		free(sendbuf);
	return;
}

//***************��Ҫ��������******************************
//���ƣ�TimeOut
//���ܣ�������������ʱ����ζ��sBasicTimer�����õĳ�ʱʱ�䵽�ˣ�
//      �������ݿ���ȫ���滻Ϊ������Լ����뷨
//      ������ʵ���˼���ͬʱ���й��ܣ����ο�
//      1)����iWorkMode����ģʽ���ж��Ƿ񽫼�����������ݷ��ͣ�
//        ��Ϊscanf�����������¼�ʱ���ڵȴ����̵�ʱ����ȫʧЧ������ʹ��_kbhit()������������ϵ��ڼ�ʱ�Ŀ������жϼ���״̬�������Get��û��
//      2)����ˢ�´�ӡ����ͳ��ֵ��ͨ����ӡ���Ʒ��Ŀ��ƣ�����ʼ�ձ�����ͬһ�д�ӡ��Get��
//���룺ʱ�䵽�˾ʹ�����ֻ��ͨ��ȫ�ֱ�����������
//���������Ǹ�����Ŭ���ɻ����ʵ����
void TimeOut()
{

	printCount++;
	if (_kbhit()) {
		//�����ж���������˵�ģʽ
		menu();
	}

	print_statistics();
}
//------------�����ķָ��ߣ����������ݵ��շ�,--------------------------------------------

//***************��Ҫ��������******************************
//���ƣ�RecvfromUpper
//���ܣ�������������ʱ����ζ���յ�һ�ݸ߲��·�������
//      ��������ȫ�������滻��������Լ���
//      ���̹��ܽ���
//         1)ͨ���Ͳ�����ݸ�ʽ����lowerMode���ж�Ҫ��Ҫ������ת����bit�����鷢�ͣ�����ֻ�����Ͳ�ӿ�0��
//           ��Ϊû���κοɹ��ο��Ĳ��ԣ���������Ӧ�ø���Ŀ�ĵ�ַ�ڶ���ӿ���ѡ��ת���ġ�
//         2)�ж�iWorkMode�������ǲ�����Ҫ�����͵��������ݶ���ӡ������ʱ���ԣ���ʽ����ʱ�����齫����ȫ����ӡ��
//���룺U8 * buf,�߲㴫���������ݣ� int len�����ݳ��ȣ���λ�ֽ�
//�����
void RecvfromUpper(U8* buf, int len)
{
	int iSndRetval;
	U8* bufSend = NULL;
	//�Ǹ߲����ݣ�ֻ�ӽӿ�0����ȥ,�߲�ӿ�Ĭ�϶����ֽ������ݸ�ʽ
	if (lowerMode[0] == 0) {
		//�ӿ�0��ģʽΪbit���飬��ת����bit���飬�ŵ�bufSend��
		bufSend = (char*)malloc(len * 8);
		if (bufSend == NULL) {
			return;
		}
		iSndRetval = ByteArrayToBitArray(bufSend, len * 8, buf, len);
		//����
		iSndRetval = SendtoLower(bufSend, iSndRetval, 0); //��������Ϊ���ݻ��壬���ȣ��ӿں�
	}
	else {
		//�²����ֽ�����ӿڣ���ֱ�ӷ���
		iSndRetval = SendtoLower(buf, len, 0);
		iSndRetval = iSndRetval * 8;//�����λ
	}
	//����������ͣ��Э����ش�Э�飬���������Ҫ����������Ӧ����������ռ䣬��buf��bufSend�����ݱ����������Ա��ش�
	if (bufSend != NULL) {
		//����bufSend���ݣ�CODES NEED HERE

		//������û������ش�Э�飬����Ҫ�������ݣ����Խ��ռ��ͷ�
		free(bufSend);
	}
	else {
		//����buf���ݣ�CODES NEED HERE

		//������û������ش�Э�飬����Ҫ�������ݣ�buf�����������Ҳ����Ҫ�ͷſռ�

	}

	//ͳ��
	if (iSndRetval <= 0) {
		iSndErrorCount++;
	}
	else {
		iSndTotal += iSndRetval;
		iSndTotalCount++;
	}
	//printf("\n�յ��ϲ����� %d λ�����͵��ӿ�0\n", retval * 8);
	//��ӡ
	switch (iWorkMode % 10) {
	case 1:
		cout << endl << "�߲�Ҫ����ӿ� " << 0 << " �������ݣ�" << endl;
		print_data_bit(buf, len, 1);
		break;
	case 2:
		cout << endl << "�߲�Ҫ����ӿ� " << 0 << " �������ݣ�" << endl;
		print_data_byte(buf, len, 1);
		break;
	case 0:
		break;
	}

}
//***************��Ҫ��������******************************
//���ƣ�RecvfromLower
//���ܣ�������������ʱ����ζ�ŵõ�һ�ݴӵͲ�ʵ��ݽ�����������
//      ��������ȫ�������滻���������Ҫ������
//      ���̹��ܽ��ܣ�
//          1)����ʵ����һ���򵥴ֱ���������Ĳ��ԣ����дӽӿ�0�����������ݶ�ֱ��ת�����ӿ�1�����ӿ�1�������Ͻ����߲㣬������ô����
//          2)ת�����Ͻ�ǰ���ж��ս����ĸ�ʽ��Ҫ���ͳ�ȥ�ĸ�ʽ�Ƿ���ͬ��������bite��������ֽ�������֮��ʵ��ת��
//            ע����Щ�жϲ������������ݱ�����������������������ļ������������ļ��Ĳ���д���ˣ��ж�Ҳ�ͻ�ʧ��
//          3)����iWorkMode���ж��Ƿ���Ҫ���������ݴ�ӡ
//���룺U8 * buf,�Ͳ�ݽ����������ݣ� int len�����ݳ��ȣ���λ�ֽڣ�int ifNo ���Ͳ�ʵ����룬�����������ĸ��Ͳ�
//�����
void RecvfromLower(U8* buf, int len, int ifNo)
{
	//��������buf��byte
	int iSndRetval;
	int sip;
	int tip;
	U8* bittemp = (U8*)malloc(sizeof(U8) * (len * 8));
	ByteArrayToBitArray(bittemp, sizeof(U8) * (len * 8), buf, len);
	sip = buf[0];
	tip = buf[1];
	//if (tip == 84)
	//{

	//}
	for (int i = 0; i <  20; i++)
	{
		if (router_table2[i][0] == tip)
		{
			iSndRetval = SendtoLower(bittemp, sizeof(U8) * (len * 8), router_table2[i][1]);
			iSndRetval = iSndRetval * 8;//�����λ,����ͳ��
			iRcvForward += iSndRetval;
			iRcvForwardCount++;
		}
	}
		
	free(bittemp);

	//��ӡ
	switch (iWorkMode % 10) {
	case 1:
		cout <<endl<< "���սӿ� " << ifNo << " ���ݣ�" << endl;
		print_data_bit(buf, len, lowerMode[ifNo]);
		break;
	case 2:
		cout << endl << "���սӿ� " << ifNo << " ���ݣ�" << endl;
		print_data_byte(buf, len, lowerMode[ifNo]);
		break;
	case 0:
		break;
	}

}
void print_statistics()
{
	if (printCount % 10 == 0) {
		switch (spin) {
		case 1:
			printf("\r-");
			break;
		case 2:
			printf("\r\\");
			break;
		case 3:
			printf("\r|");
			break;
		case 4:
			printf("\r/");
			spin = 0;
			break;
		}
		cout << "��ת�� "<< iRcvForward<< " λ��"<< iRcvForwardCount<<" �Σ�"<<"�ݽ� "<< iRcvToUpper<<" λ��"<< iRcvToUpperCount<<" ��,"<<"���� "<< iSndTotal <<" λ��"<< iSndTotalCount<<" �Σ�"<< "���Ͳ��ɹ� "<< iSndErrorCount<<" ��,""�յ�������Դ "<< iRcvUnknownCount<<" �Ρ�";
		spin++;
	}

}
void menu()
{
	int selection;
	unsigned short port;
	int iSndRetval;
	char kbBuf[100];
	int len;
	U8* bufSend;
	//����|��ӡ��[���Ϳ��ƣ�0���ȴ��������룻1���Զ���][��ӡ���ƣ�0�������ڴ�ӡͳ����Ϣ��1����bit����ӡ���ݣ�2���ֽ�����ӡ����]
	cout << endl << endl << "�豸��:" << strDevID << ",    ���:" << strLayer << ",    ʵ���:" << strEntity ;
	cout << endl << "1-�����Զ�����(��Ч);" << endl << "2-ֹͣ�Զ����ͣ���Ч��; " << endl << "3-�Ӽ������뷢��; ";
	cout << endl << "4-����ӡͳ����Ϣ; " << endl << "5-����������ӡ��������;" << endl << "6-���ֽ�����ӡ��������;";
	cout << endl << "0-ȡ��" << endl << "����������ѡ�����";
	cin >> selection;
	switch (selection) {
	case 0:

		break;
	case 1:
		iWorkMode = 10 + iWorkMode % 10;
		break;
	case 2:
		iWorkMode = iWorkMode % 10;
		break;
	case 3:
		cout << "�����ַ���(,������100�ַ�)��";
		cin >> kbBuf;
		cout << "����Ͳ�ӿںţ�";
		cin >> port;

		len = (int)strlen(kbBuf) + 1; //�ַ�������и�������
		if (port >= lowerNumber) {
			cout << "û������ӿ�" << endl;
			return;
		}
		if (lowerMode[port] == 0) {
			//�²�ӿ��Ǳ���������,��ҪһƬ�µĻ�����ת����ʽ
			bufSend = (U8*)malloc(len * 8);

			iSndRetval = ByteArrayToBitArray(bufSend, len * 8, kbBuf, len);
			iSndRetval = SendtoLower(bufSend, iSndRetval, port);
			free(bufSend);
		}
		else {
			//�²�ӿ����ֽ����飬ֱ�ӷ���
			iSndRetval = SendtoLower(kbBuf, len, port);
			iSndRetval = iSndRetval * 8; //�����λ
		}
		//����ͳ��
		if (iSndRetval > 0) {
			iSndTotalCount++;
			iSndTotal += iSndRetval;
		}
		else {
			iSndErrorCount++;
		}
		//��Ҫ��Ҫ��ӡ����
		cout << endl << "��ӿ� " << port << " �������ݣ�" << endl;
		switch (iWorkMode % 10) {
		case 1:
			print_data_bit(kbBuf, len, 1);
			break;
		case 2:
			print_data_byte(kbBuf, len, 1);
			break;
		case 0:
			break;
		}
		break;
	case 4:
		iWorkMode = (iWorkMode / 10) * 10 + 0;
		break;
	case 5:
		iWorkMode = (iWorkMode / 10) * 10 + 1;
		break;
	case 6:
		iWorkMode = (iWorkMode / 10) * 10 + 2;
		break;
	}

}