//Nettester �Ĺ����ļ�
#include <iostream>
#include <conio.h>
#include "winsock.h"
#include "stdio.h"
#include "CfgFileParms.h"
#include "function.h"

typedef unsigned short 		u16;

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
int time_count = 0; // TimeOut�õļ�����
int is_timeout = 1;//��ʱ�ش���־λ
int start_timeout = 0;//��ʼ��ʱ�ش�����־λ
U8* resent_buf = NULL;//�ش�ָ���¼
int resent_len = 0;//�ش����ȼ�¼
U8 ack_array[57] = { 0,1,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,1,1,1,1,1,1,0 };//ack֡
#define max_device_num 100//����豸��
int mac_port_table[max_device_num][3];//��һ�е�һ�еڶ����Ƕ�Ӧ��macֵ�����Ƕ����macֵ����Ԫ��ż�PHY��ʵ�����ɣ���������Ϊ���뽻�����Ķ˿ںţ���PHY��ʵ��ţ�


//��ӡͳ����Ϣ
void print_statistics();
void menu();
U8* MakeFrame(U8* byte_data, int len, int* return_data_len);
U8* getFrame(U8* bit_data, int* len);
int broadcast(U8* bit_data, int len, int ifNo);
U8* getSrcDstAndTypeFromHead(U8* bit_data, int* src1, int* src2, int* dst1, int* dst2, int* frame_type, int len);

U8* makeFrameHead(U8* buf, int ctr, int addr, int len)//��������HDLC֡ͷ��ʽ����ַ�Ϳ����ֶξ�Ϊ1���ֽڣ�Ĭ�Ϸֱ�Ϊ0XFF��0X03����bufΪ�ֽ����飬lenΪ�ֽ����鳤��
{													  //ctr-----0x03��ʾΪ����֡ 0x01��ʾΪack֡ 0x02��ʾΪsyn֡ 0x00Ϊfin֡
	U8* new_buf = (U8*)malloc(sizeof(U8) * (len + 2));
	new_buf[0] = addr;
	new_buf[1] = ctr;
	if(len>0)
	for (int i = 0; i < len ; i++)
		new_buf[2 + i] = buf[i];
	//free(buf);//�ͷ�ԭ��buf
	
	return new_buf;
}
U8* removeFrameHeadAndFCS(U8* buf,int len)//����֡��ͷ���ֽ�����ȥͷ��CRC16У���룬����ȥͷ���FCS�ĳ���
{
	U8* new_buf = (U8*)malloc(sizeof(U8) * (len - 4));
	for (int i = 0; i < len - 4;i++)
	{
		new_buf[i] = buf[2 + i];
	}
	return new_buf;
}

void initialMacPortTable()
{
	for (int i = 0; i < 100; i++)
	{
		for (int j = 0; j < 3; j++)
			mac_port_table[i][j] = -1;
	}
}


int useMacPortTable(int s_mac1, int s_mac2, int in_port, int t_mac1, int t_mac2)//ѧϰ�����س��˿�,����-1����Ҫ�㲥
{
	int flag_empty = -1;//���ڼ�¼���еĿռ�
	int flag_exist = 0;//�����ж�Դmac��ַ�Ƿ񱻼�¼��0��ʾδ��¼��1��ʾ��¼
	int out_port = -1;//����Ŀ��mac��ַĿ�꽻������Ӧ�Ķ˿�
	for (int i = 0; i < max_device_num; i++)
	{
		if (mac_port_table[i][2] == -1 && flag_empty == -1)
		{
			flag_empty = i;
		}
		if (mac_port_table[i][0] == s_mac1 &&
			mac_port_table[i][1] == s_mac2 &&
			mac_port_table[i][2] != -1)
		{
			flag_exist = 1;
		}
		if (mac_port_table[i][0] == t_mac1 &&
			mac_port_table[i][1] == t_mac2 &&
			mac_port_table[i][2] != -1)
		{
			out_port = mac_port_table[i][2];
		}
	}
	if (flag_exist == 0)//δ�ҵ���Ӧ��Դmac������Ӽ�¼
	{
		mac_port_table[flag_empty][0] = s_mac1;
		mac_port_table[flag_empty][1] = s_mac2;
		mac_port_table[flag_empty][2] = in_port;
	}
	return out_port;
}

unsigned short int GICREN_CalcCRC16(unsigned char* data, unsigned char len)
{
	unsigned char i;
	unsigned short int Init = 0xffff;  			//1111111111111111B����CRC��ֵ
	while (len--)
	{
		Init ^= ((unsigned short int)(*(data++))) << 8;	//Դ���������Ȳ�8��0���·���ѭ������Ч��Դ��������CRC��ֵ��һͬ��8��0
		for (i = 0; i < 8; i++)
		{
			if (Init & 0x8000)
				Init = (Init << 1) ^ 0x8005;		//1000000000000101B����CRC���ɶ���ʽϵ���ļ�ʽ
			else
				Init <<= 1;
		}
	}
	return Init;                        		//CRC��ֵ
}

U8* creatCrc(U8* src, unsigned char len)//���ֽ�����ʹ�С
{
	unsigned short int fcs_short = 0;
	unsigned short int flag = 0x01;
	fcs_short = GICREN_CalcCRC16((unsigned char*)src, len);
	U8* fcs = (U8*)malloc(sizeof(U8) * 16);
	for (int i = 15; i >=0; i--)
	{
		fcs[i] = fcs_short & flag;
		fcs_short >>= 1;
	}
	return fcs;
}

bool checkCrc(U8* src_fcs, unsigned char len)//���ֽ�����ʹ�С
{
	
	return !GICREN_CalcCRC16((unsigned char*)src_fcs, len);
}


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
	if (start_timeout == 1)
	{
		time_count++;
		if (time_count >= 100)//5000ms
		{
			SendtoLower(resent_buf, resent_len, 0); //��������Ϊ���ݻ��壬���ȣ��ӿں�>>>>>>>>��>>>>>>>>>>>>
			time_count = 0;
		}
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
	U8* fcs = NULL;
	U8* new_bit_buf = NULL;
	U8* new_buf = NULL;
	U8* new_buf_head = NULL;
	int return_data_len = 0;
	//�Ǹ߲����ݣ�ֻ�ӽӿ�0����ȥ,�߲�ӿ�Ĭ�϶����ֽ������ݸ�ʽ
	if (lowerMode[0] == 0) {
		new_buf_head = makeFrameHead(buf, 0x03, 0xff, len);
		len = len + 2;//����ͷ��2�ֽ�
		U8* bit_buf = (U8*)malloc(sizeof(U8) * (len * 8));

		ByteArrayToBitArray(bit_buf, len * 8, new_buf_head, len);
		fcs = creatCrc(new_buf_head, len);
		free(new_buf_head);
		//newbuf=(U8*)malloc(sizeof(U8)*len*8+15)
		new_bit_buf = (U8*)malloc(sizeof(U8) * (len * 8 + 16));
		for (int i = 0; i < len * 8; i++)
			new_bit_buf[i] = bit_buf[i];
		//free(bit_buf);
		for (int i = 0; i < 16; i++)//��Ϊ���õ���CRC16
			new_bit_buf[len * 8 + i] = fcs[i];
		new_buf = (U8*)malloc(sizeof(U8) * (len + 2));

		BitArrayToByteArray(new_bit_buf, len * 8 + 16, new_buf, len + 2);
		bufSend = MakeFrame(new_buf, len + 2, &return_data_len);
		iSndRetval = SendtoLower(bufSend, return_data_len, 0); //��������Ϊ���ݻ��壬���ȣ��ӿں�>>>>>>>>��>>>>>>>>>>>>
		Sleep(50);
		start_timeout = 1;//��ʼ��ʱ
		free(new_buf);
		free(fcs);
		free(new_bit_buf);

	}
	else {
		//�²����ֽ�����ӿڣ���ֱ�ӷ���
		iSndRetval = SendtoLower(buf, len, 0);
		iSndRetval = iSndRetval * 8;//�����λ
	}
	//����������ͣ��Э����ش�Э�飬���������Ҫ����������Ӧ����������ռ䣬��buf��bufSend�����ݱ����������Ա��ش�
	if (bufSend != NULL) {
		//����bufSend���ݣ�CODES NEED HERE
		//resent_buf = (U8*)malloc(sizeof(U8) * return_data_len);
		resent_buf = bufSend;
		resent_len = return_data_len;
		//������û������ش�Э�飬����Ҫ�������ݣ����Խ��ռ��ͷ�
		//free(bufSend);
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
	int iSndRetval = 0;
	//�ǽӿ�0�����ݣ����ߵͲ�ֻ��1���ӿڵ����ݣ������ϵݽ�
	if (lowerMode[ifNo] == 0) {
		//����ӿ�0�Ǳ��������ʽ���߲�Ĭ�����ֽ����飬��ת�����ֽ����飬�����ϵݽ�
		U8* true_data = (U8*)malloc(sizeof(U8) * (len / 8 + 1));//true_data:ȥ����λ��������,bit����
		int true_data_len = 0;
		true_data = getFrame(buf, &true_data_len);
		//copyһ�飬У����ȷֱ�ӷ����
		//U8* true_data_copy = (U8*)malloc(sizeof(U8) * (len / 8 + 1));
		//for (int i = 0; i < true_data_len; i++)
		//{
		//	true_data_copy[i] = true_data[i];
		//}
		//U8* true_data_copy_byte = (U8*)malloc(sizeof(U8) * (true_data_len / 8));
		//BitArrayToByteArray(true_data, true_data_len, true_data_copy_byte, true_data_len / 8);
		int send_bit_data_copy_len = -1;
		U8* send_bit_data_copy = NULL;
		//send_bit_data_copy = MakeFrame(true_data_copy_byte, true_data_len / 8, &send_bit_data_copy_len);
		//free(true_data_copy_byte);
		U8* true_data_byte = (U8*)malloc(sizeof(U8) * (true_data_len / 8));
		BitArrayToByteArray(true_data, true_data_len, true_data_byte, true_data_len / 8);
		send_bit_data_copy = MakeFrame(true_data_byte, true_data_len / 8, &send_bit_data_copy_len);

		//�������߼���ʼ������case���ʼ������
		U8* geted_data_byte = NULL;
		U8* geted_data_bit = (U8*)malloc((true_data_len / 8 - 4 )* 8);//true_data_len / 8 - 4 ���������iSndRetval����Ҫ��ʼ�����԰�������
		int src1 = -1, src2 = -1, dst1 = -1, dst2 = -1, frame_type = -1;
		int tran_port = -1;

		if (checkCrc(true_data_byte, true_data_len/8))
		{
			switch (true_data_byte[1]) {
				case 0x03:
					SendtoLower(ack_array, 57, ifNo);
					true_data_byte = removeFrameHeadAndFCS(true_data_byte, true_data_len / 8);
					iSndRetval = true_data_len / 8 - 4;
					//���������֡�����Ͻ�
					//iSndRetval = SendtoUpper(true_data_byte, iSndRetval);
					//iSndRetval = iSndRetval * 8;//�����λ,����ͳ��
					//����ACKȷ��

					//�����ǽ������߼�
					geted_data_byte = getSrcDstAndTypeFromHead(true_data_byte, &src1, &src2, &dst1, &dst2, &frame_type, iSndRetval);
					tran_port = useMacPortTable(src1, src2, ifNo, dst1, dst2);//ѧϰ

					ByteArrayToBitArray(geted_data_bit, iSndRetval * 8, geted_data_byte, iSndRetval);
					if (tran_port == -1)
					{
						broadcast(send_bit_data_copy, send_bit_data_copy_len, ifNo);
					}
					else
					{
						SendtoLower(send_bit_data_copy, send_bit_data_copy_len, ifNo);
					}
					iSndRetval = send_bit_data_copy_len;//�����λ,����ͳ��
					//SendtoLower(ack_array, 57 , ifNo);
					break;
				case 0x01:
					//�����ȷ��֡��ȡ����ʱ�ش�
					start_timeout = 0;
					break;
			}
		}
		else //У�������
		{
				//����ACK��ʲô������
		}
	}
		//ͳ��
	if (iSndRetval <= 0) {
		iSndErrorCount++;
	}
	else {
		iRcvToUpper += iSndRetval;
		iRcvToUpperCount++;
	}
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
	cout << endl << endl << "�豸��:" << strDevID << ",    ���:" << strLayer << ",    ʵ���:" << strEntity;
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

U8* MakeFrame(U8* byte_data,int len, int* return_data_len)
{
	U8* bitArray = (U8*)malloc(len * 8 + 16);  //����һ��ԭ���ݳ���8���Ļ�������ÿ��λ����Ϊ1���ֽڣ��ټ�����ͷ��λ�Ķ������01111110
	ByteArrayToBitArray(bitArray + 8, len * 8+16, byte_data, len); //�任�ɱ��������ʽ��ǰ��ճ�8λ��
	U8* sendBitArray;
	sendBitArray = (U8*)malloc(len * 8 * 2 + 16);//������һ������Ļ���ռ�
	int count = 0;
	int i, j;
	for (i = 8,j = 8; i<=8+len*8; i++, j++) {
		sendBitArray[j] = bitArray[i];
		//����ж��ǲ��ǳ���������5��1�����巽���Լ���Ŷ��
		if (sendBitArray[j] == 1)
		{
			count += 1;
		}
		if (sendBitArray[j] == 0)
		{
			count = 0;
		}
		if (count == 5) {
			//����bitArray[i]��5��������1
			j++;
			sendBitArray[j] = 0;  //����1����
			count = 0;
		}
	}
	j -= 1;//forѭ�����������жϣ���ʱj����1
	*return_data_len = j+8;
	sendBitArray[0] = 0, sendBitArray[1] = 1, sendBitArray[2] = 1, sendBitArray[3] = 1, sendBitArray[4] = 1, sendBitArray[5] = 1, sendBitArray[6] = 1, sendBitArray[7] = 0; //�ײ��Ӷ������
	sendBitArray[j] = 0, sendBitArray[j + 1] = 1, sendBitArray[j + 2] = 1, sendBitArray[j + 3] = 1, sendBitArray[j + 4] = 1, sendBitArray[j + 5] = 1, sendBitArray[j + 6] = 1, sendBitArray[j + 7] = 0;//β���Ӷ������
	//SendtoLower(sendBitArray, j + 8, 0); //���͵��Ͳ��0�Žӿڡ�
	//free(bitArray);      //���������꣬�ͷſռ�
	//free(sendBitArray);    //�������Ҫ�������Ա��ش������ͷ���ռ䡣
	//U8 testArray[90] = {0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0};
	//U8* reArray = (U8*)malloc(200*sizeof(char));
	//int a = BitArrayToByteArray(testArray, 26, reArray, 200);
	free(bitArray);
	return sendBitArray;
}

U8* getFrame(U8* bit_data , int* len)
{
	int get_head = 0, get_tail = 0, head = -1, tail = -1;
	for (int i = 0;i<=10000 && get_tail == 0; i++)
	{
		if (bit_data[i] == 0 && 
			bit_data[i + 1] == 1 && 
			bit_data[i + 2] == 1 &&
			bit_data[i + 3] == 1 &&
			bit_data[i + 4] == 1 &&
			bit_data[i + 5] == 1 &&
			bit_data[i + 6] == 1 &&
			bit_data[i + 7] == 0 && get_head==0 && get_tail==0)
		{
			get_head = 1;
			head = i + 8;
			i += 7;
		}

		if (bit_data[i] == 0 &&
			bit_data[i + 1] == 1 &&
			bit_data[i + 2] == 1 &&
			bit_data[i + 3] == 1 &&
			bit_data[i + 4] == 1 &&
			bit_data[i + 5] == 1 &&
			bit_data[i + 6] == 1 &&
			bit_data[i + 7] == 0 && get_head == 1 && get_tail == 0)
		{
			get_tail = 1;
			tail = i - 1;
		}
	}

	U8* return_bit_data = (U8*)malloc(sizeof(U8) * (tail - head + 1));
	int count = 0;
	int j = 0;
	for (int i = head ; i <= tail; i++,j++)
	{
		return_bit_data[j] = bit_data[i];
		if (return_bit_data[j] == 1)
		{
			count += 1;
		}
		if (count == 5) {
			i++;
			count = 0;
		}
		if (return_bit_data[j] == 0)
		{
			count = 0;
		}
	}
	*len = j;
	return return_bit_data;
}

int broadcast(U8* bit_data, int len, int ifNo)
{
	for (int i = 0; i < max_device_num; i++)
	{
		if (i==ifNo)
		{
			continue;
		}
		SendtoLower(bit_data, len, i);
	}
	return 1;
}

U8* getSrcDstAndTypeFromHead(U8* byte_data, int* src1, int* src2, int* dst1, int* dst2, int* frame_type, int len)
{
	//U8* byte_data = (U8*)malloc((len / 8) + 1);
	int src, dst;
	//BitArrayToByteArray(bit_data, len, byte_data, (len / 8) + 1);
	src = (int)byte_data[0];
	*src1 = src / 2 / 2 / 2 / 2;
	*src2 = src - *src1 * 2 * 2 * 2 * 2;
	dst = (int)byte_data[1];
	*dst1 = dst / 2 / 2 / 2 / 2;
	*dst2 = dst - *dst1 * 2 * 2 * 2 * 2;
	*frame_type = (int)byte_data[2];
	//free(byte_data);

	U8* return_byte_data = (U8*)malloc(len-4);
	for (int i = 0; i < len- 4; i++)
	{
		return_byte_data[i] = byte_data[i + 4];
	}
	return return_byte_data;
}

U8* encapsulation(U8* byte_data ,int len , int* return_data_len)
{
	U8* fcs = NULL;
	U8* new_bit_buf = NULL;
	U8* new_buf = NULL;
	U8* bufSend = NULL;
	U8* new_buf_head = (U8*)malloc(len+2);
	new_buf_head = makeFrameHead(byte_data, 0x03, 0xff, len);
	len = len + 2;//����ͷ��2�ֽ�

	U8* bit_buf = (U8*)malloc(sizeof(U8) * (len * 8));
	ByteArrayToBitArray(bit_buf, len * 8, new_buf_head, len);
	fcs = creatCrc(new_buf_head, len);
	free(new_buf_head);
	//newbuf=(U8*)malloc(sizeof(U8)*len*8+15)
	new_bit_buf = (U8*)malloc(sizeof(U8) * (len * 8 + 16));
	for (int i = 0; i < len * 8; i++)
		new_bit_buf[i] = bit_buf[i];
	//free(bit_buf);
	for (int i = 0; i < 16; i++)//��Ϊ���õ���CRC16
		new_bit_buf[len * 8 + i] = fcs[i];
	new_buf = (U8*)malloc(sizeof(U8) * (len + 2));

	BitArrayToByteArray(new_bit_buf, len * 8 + 16, new_buf, len + 2);
	bufSend = MakeFrame(new_buf, len + 2, return_data_len);
	free(new_buf);
	free(fcs);
	free(new_bit_buf);
	return bufSend;
}