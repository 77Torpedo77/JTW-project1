//Nettester 的功能文件
#include <iostream>
#include <conio.h>
#include "winsock.h"
#include "stdio.h"
#include "CfgFileParms.h"
#include "function.h"
using namespace std;

//以下为重要的变量
U8* sendbuf;        //用来组织发送数据的缓存，大小为MAX_BUFFER_SIZE,可以在这个基础上扩充设计，形成适合的结构，例程中没有使用，只是提醒一下
int printCount = 0; //打印控制
int spin = 0;  //打印动态信息控制

//------华丽的分割线，一些统计用的全局变量------------
int iSndTotal = 0;  //发送数据总量
int iSndTotalCount = 0; //发送数据总次数
int iSndErrorCount = 0;  //发送错误次数
int iRcvForward = 0;     //转发数据总量
int iRcvForwardCount = 0; //转发数据总次数
int iRcvToUpper = 0;      //从低层递交高层数据总量
int iRcvToUpperCount = 0;  //从低层递交高层数据总次数
int iRcvUnknownCount = 0;  //收到不明来源数据总次数
int router_table2[20][2] = {130,0,
							146,0,};//路由表
int router_table3[20][2] = { 130,0,
							146,0,};//路由表
int router_table4[20][2] = { 130,0,
							146,0,};//路由表


//打印统计信息
void print_statistics();
void menu();
//***************重要函数提醒******************************
//名称：InitFunction
//功能：初始化功能面，由main函数在读完配置文件，正式进入驱动机制前调用
//输入：
//输出：
void InitFunction(CCfgFileParms& cfgParms)
{
	sendbuf = (char*)malloc(MAX_BUFFER_SIZE);
	if (sendbuf == NULL ) {
		cout << "内存不够" << endl;
		//这个，计算机也太，退出吧
		exit(0);
	}
	return;
}
//***************重要函数提醒******************************
//名称：EndFunction
//功能：结束功能面，由main函数在收到exit命令，整个程序退出前调用
//输入：
//输出：
void EndFunction()
{
	if(sendbuf != NULL)
		free(sendbuf);
	return;
}

//***************重要函数提醒******************************
//名称：TimeOut
//功能：本函数被调用时，意味着sBasicTimer中设置的超时时间到了，
//      函数内容可以全部替换为设计者自己的想法
//      例程中实现了几个同时进行功能，供参考
//      1)根据iWorkMode工作模式，判断是否将键盘输入的数据发送，
//        因为scanf会阻塞，导致计时器在等待键盘的时候完全失效，所以使用_kbhit()无阻塞、不间断地在计时的控制下判断键盘状态，这个点Get到没？
//      2)不断刷新打印各种统计值，通过打印控制符的控制，可以始终保持在同一行打印，Get？
//输入：时间到了就触发，只能通过全局变量供给输入
//输出：这就是个不断努力干活的老实孩子
void TimeOut()
{

	printCount++;
	if (_kbhit()) {
		//键盘有动作，进入菜单模式
		menu();
	}

	print_statistics();
}
//------------华丽的分割线，以下是数据的收发,--------------------------------------------

//***************重要函数提醒******************************
//名称：RecvfromUpper
//功能：本函数被调用时，意味着收到一份高层下发的数据
//      函数内容全部可以替换成设计者自己的
//      例程功能介绍
//         1)通过低层的数据格式参数lowerMode，判断要不要将数据转换成bit流数组发送，发送只发给低层接口0，
//           因为没有任何可供参考的策略，讲道理是应该根据目的地址在多个接口中选择转发的。
//         2)判断iWorkMode，看看是不是需要将发送的数据内容都打印，调试时可以，正式运行时不建议将内容全部打印。
//输入：U8 * buf,高层传进来的数据， int len，数据长度，单位字节
//输出：
void RecvfromUpper(U8* buf, int len)
{
	int iSndRetval;
	U8* bufSend = NULL;
	//是高层数据，只从接口0发出去,高层接口默认都是字节流数据格式
	if (lowerMode[0] == 0) {
		//接口0的模式为bit数组，先转换成bit数组，放到bufSend里
		bufSend = (char*)malloc(len * 8);
		if (bufSend == NULL) {
			return;
		}
		iSndRetval = ByteArrayToBitArray(bufSend, len * 8, buf, len);
		//发送
		iSndRetval = SendtoLower(bufSend, iSndRetval, 0); //参数依次为数据缓冲，长度，接口号
	}
	else {
		//下层是字节数组接口，可直接发送
		iSndRetval = SendtoLower(buf, len, 0);
		iSndRetval = iSndRetval * 8;//换算成位
	}
	//如果考虑设计停等协议等重传协议，这份数据需要缓冲起来，应该另外申请空间，把buf或bufSend的内容保存起来，以备重传
	if (bufSend != NULL) {
		//保存bufSend内容，CODES NEED HERE

		//本例程没有设计重传协议，不需要保存数据，所以将空间释放
		free(bufSend);
	}
	else {
		//保存buf内容，CODES NEED HERE

		//本例程没有设计重传协议，不需要保存数据，buf是输入参数，也不需要释放空间

	}

	//统计
	if (iSndRetval <= 0) {
		iSndErrorCount++;
	}
	else {
		iSndTotal += iSndRetval;
		iSndTotalCount++;
	}
	//printf("\n收到上层数据 %d 位，发送到接口0\n", retval * 8);
	//打印
	switch (iWorkMode % 10) {
	case 1:
		cout << endl << "高层要求向接口 " << 0 << " 发送数据：" << endl;
		print_data_bit(buf, len, 1);
		break;
	case 2:
		cout << endl << "高层要求向接口 " << 0 << " 发送数据：" << endl;
		print_data_byte(buf, len, 1);
		break;
	case 0:
		break;
	}

}
//***************重要函数提醒******************************
//名称：RecvfromLower
//功能：本函数被调用时，意味着得到一份从低层实体递交上来的数据
//      函数内容全部可以替换成设计者想要的样子
//      例程功能介绍：
//          1)例程实现了一个简单粗暴不讲道理的策略，所有从接口0送上来的数据都直接转发到接口1，而接口1的数据上交给高层，就是这么任性
//          2)转发和上交前，判断收进来的格式和要发送出去的格式是否相同，否则，在bite流数组和字节流数组之间实现转换
//            注意这些判断并不是来自数据本身的特征，而是来自配置文件，所以配置文件的参数写错了，判断也就会失误
//          3)根据iWorkMode，判断是否需要把数据内容打印
//输入：U8 * buf,低层递交上来的数据， int len，数据长度，单位字节，int ifNo ，低层实体号码，用来区分是哪个低层
//输出：
void RecvfromLower(U8* buf, int len, int ifNo)
{
	//收上来的buf是byte
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
			iSndRetval = iSndRetval * 8;//换算成位,进行统计
			iRcvForward += iSndRetval;
			iRcvForwardCount++;
		}
	}
		
	free(bittemp);

	//打印
	switch (iWorkMode % 10) {
	case 1:
		cout <<endl<< "接收接口 " << ifNo << " 数据：" << endl;
		print_data_bit(buf, len, lowerMode[ifNo]);
		break;
	case 2:
		cout << endl << "接收接口 " << ifNo << " 数据：" << endl;
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
		cout << "共转发 "<< iRcvForward<< " 位，"<< iRcvForwardCount<<" 次，"<<"递交 "<< iRcvToUpper<<" 位，"<< iRcvToUpperCount<<" 次,"<<"发送 "<< iSndTotal <<" 位，"<< iSndTotalCount<<" 次，"<< "发送不成功 "<< iSndErrorCount<<" 次,""收到不明来源 "<< iRcvUnknownCount<<" 次。";
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
	//发送|打印：[发送控制（0，等待键盘输入；1，自动）][打印控制（0，仅定期打印统计信息；1，按bit流打印数据，2按字节流打印数据]
	cout << endl << endl << "设备号:" << strDevID << ",    层次:" << strLayer << ",    实体号:" << strEntity ;
	cout << endl << "1-启动自动发送(无效);" << endl << "2-停止自动发送（无效）; " << endl << "3-从键盘输入发送; ";
	cout << endl << "4-仅打印统计信息; " << endl << "5-按比特流打印数据内容;" << endl << "6-按字节流打印数据内容;";
	cout << endl << "0-取消" << endl << "请输入数字选择命令：";
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
		cout << "输入字符串(,不超过100字符)：";
		cin >> kbBuf;
		cout << "输入低层接口号：";
		cin >> port;

		len = (int)strlen(kbBuf) + 1; //字符串最后有个结束符
		if (port >= lowerNumber) {
			cout << "没有这个接口" << endl;
			return;
		}
		if (lowerMode[port] == 0) {
			//下层接口是比特流数组,需要一片新的缓冲来转换格式
			bufSend = (U8*)malloc(len * 8);

			iSndRetval = ByteArrayToBitArray(bufSend, len * 8, kbBuf, len);
			iSndRetval = SendtoLower(bufSend, iSndRetval, port);
			free(bufSend);
		}
		else {
			//下层接口是字节数组，直接发送
			iSndRetval = SendtoLower(kbBuf, len, port);
			iSndRetval = iSndRetval * 8; //换算成位
		}
		//发送统计
		if (iSndRetval > 0) {
			iSndTotalCount++;
			iSndTotal += iSndRetval;
		}
		else {
			iSndErrorCount++;
		}
		//看要不要打印数据
		cout << endl << "向接口 " << port << " 发送数据：" << endl;
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