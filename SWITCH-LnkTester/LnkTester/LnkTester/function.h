#pragma once
//yeah once, sure
//һ����¶��ǴӶ��忪ʼ
#define MAX_BUFFER_SIZE 65000 //Ĭ�ϻ��������С��Ҳ��UDP���ĵ�����
#define DEFAULT_TIMER_INTERVAL 50 //Ĭ��������ʱ��ʱ��������λ�Ǻ���
typedef char U8; //���嵥�ֽ��޷��������������Ͳ�����char��������ˣ������ǵ��ֽ�����

//============�ص�����================================================================================================
//------------��Ҫ�Ŀ��Ʋ���------------------------------------------------------------
//���´������ļ�ne.txt�ж�ȡ����Ҫ����
extern int lowerMode[10]; //����Ͳ��������ģ����������������õ���ÿ���ӿڶ�Ӧ�����ݸ�ʽ����0Ϊ�������飬1Ϊ�ֽ�����
extern int lowerNumber;   //�Ͳ�ʵ�����������������֮�¿����ж����·��
extern int iWorkMode;     //ʵ��Ĺ���ģʽ��ʮλ����1 �Զ����ͣ�0 �ֶ����ͣ���λ�� 1 ��ӡ�������ݣ�0 ��ӡͳ������
extern string strDevID;    //�豸�ţ��ַ�����ʽ����1��ʼ
extern string strLayer;    //�����
extern string strEntity;   //ʵ��ã��ַ�����ʽ����0��ʼ������ͨ��atoi������������ڳ�����ʹ��

//start------�����ķָ���---------------------------------------------------------
//------------��Ҫ����Ҫ����ʵ�������ݵĺ���------------------------------------------------------------
void TimeOut();
void RecvfromLower(U8* buf, int len, int ifNo);
void RecvfromUpper(U8* buf, int len);
void InitFunction(CCfgFileParms& cfgParms);
void EndFunction();

//------------��Ҫ�Ļ�ʹ�õ��ĺ���------------------------------------------------------------
//************�������ݵĺ���*************
int SendtoUpper(U8* buf, int len);
int SendtoLower(U8* buf, int len, int ifNo);
int SendtoCommander(U8* buf, int len);
//************һЩ���ߺ���***************
void print_data_bit(U8* A, int length, int iMode);
void print_data_byte(U8* A, int length, int iMode);
void code(unsigned long x, U8 A[], int length);
unsigned long decode(U8 A[], int length);
int ByteArrayToBitArray(U8* bitA, int iBitLen, U8* byteA, int iByteLen);
int BitArrayToByteArray(U8* bitA, int iBitLen, U8* byteA, int iByteLen);
void StartTimerOnce(ULONG ulInterval);
void StartTimerPeriodically(ULONG ulInterval);

