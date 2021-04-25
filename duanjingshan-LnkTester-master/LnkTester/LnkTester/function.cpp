//Nettester 的功能文件
#include <iostream>
#include <conio.h>
#include "winsock.h"
#include "stdio.h"
#include "CfgFileParms.h"
#include "function.h"

typedef unsigned short 		u16;

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
int time_count = 0; // TimeOut用的计数器
int is_timeout = 1;//超时重传标志位
int start_timeout = 0;//开始超时重传检测标志位
U8* resent_buf = NULL;//重传指针记录
int resent_len = 0;//重传长度记录
U8 ack_array[7] = { 0x7E,0xff,0x01,0x00,0x04,0x0c,0x7E };//ack帧


//打印统计信息
void print_statistics();
void menu();
U8* MakeFrame(U8* byte_data, int len, int* return_data_len);
U8* getFrame(U8* bit_data, int* len);

U8* makeFrameHead(U8* buf, int ctr, int addr, int len)//采用类似HDLC帧头格式，地址和控制字段均为1个字节（默认分别为0XFF和0X03），buf为字节数组，len为字节数组长度
{													  //ctr-----0x03表示为数据帧 0x01表示为ack帧 0x02表示为syn帧 0x00为fin帧
	U8* new_buf = (U8*)malloc(sizeof(U8) * (len + 2));
	new_buf[0] = addr;
	new_buf[1] = ctr;
	if(len>0)
	for (int i = 0; i < len ; i++)
		new_buf[2 + i] = buf[i];
	//free(buf);//释放原来buf
	
	return new_buf;
}
U8* removeFrameHeadAndFCS(U8* buf,int len)//传入帧带头的字节数组去头和CRC16校验码，返回去头后和FCS的长度
{
	U8* new_buf = (U8*)malloc(sizeof(U8) * (len - 4));
	for (int i = 0; i < len - 4;i++)
	{
		new_buf[i] = buf[2 + i];
	}
	return new_buf;
}

unsigned short int GICREN_CalcCRC16(unsigned char* data, unsigned char len)
{
	unsigned char i;
	unsigned short int Init = 0xffff;  			//1111111111111111B，即CRC初值
	while (len--)
	{
		Init ^= ((unsigned short int)(*(data++))) << 8;	//源数据流后先补8个0，下方的循环语句等效于源数据流与CRC初值后一同补8个0
		for (i = 0; i < 8; i++)
		{
			if (Init & 0x8000)
				Init = (Init << 1) ^ 0x8005;		//1000000000000101B，即CRC生成多项式系数的简式
			else
				Init <<= 1;
		}
	}
	return Init;                        		//CRC终值
}

U8* creatCrc(U8* src, unsigned char len)//传字节数组和大小
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

bool checkCrc(U8* src_fcs, unsigned char len)//传字节数组和大小
{
	
	return !GICREN_CalcCRC16((unsigned char*)src_fcs, len);
}


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
	if (start_timeout == 1)
	{
		time_count++;
		if (time_count == 100)//5000ms
		{
			SendtoLower(resent_buf, resent_len, 0); //参数依次为数据缓冲，长度，接口号>>>>>>>>发>>>>>>>>>>>>

		}
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
	U8* fcs = NULL;
	U8* new_bit_buf = NULL;
	U8* new_buf = NULL;
	U8* new_buf_head = NULL;
	int return_data_len = 0;
	//是高层数据，只从接口0发出去,高层接口默认都是字节流数据格式
	if (lowerMode[0] == 0) {
		new_buf_head = makeFrameHead(buf, 0x03, 0xff, len);
		len = len + 2;//多了头部2字节
		U8* bit_buf = (U8*)malloc(sizeof(U8) * (len * 8));

		ByteArrayToBitArray(bit_buf, len * 8, new_buf_head, len);
		fcs = creatCrc(new_buf_head, len);
		free(new_buf_head);
		//newbuf=(U8*)malloc(sizeof(U8)*len*8+15)
		new_bit_buf = (U8*)malloc(sizeof(U8) * (len * 8 + 16));
		for (int i = 0; i < len * 8; i++)
			new_bit_buf[i] = bit_buf[i];
		//free(bit_buf);
		for (int i = 0; i < 16; i++)//因为采用的是CRC16
			new_bit_buf[len * 8 + i] = fcs[i];
		new_buf = (U8*)malloc(sizeof(U8) * (len + 2));

		BitArrayToByteArray(new_bit_buf, len * 8 + 16, new_buf, len + 2);
		bufSend = MakeFrame(new_buf, len + 2, &return_data_len);
		iSndRetval = SendtoLower(bufSend, return_data_len, 0); //参数依次为数据缓冲，长度，接口号>>>>>>>>发>>>>>>>>>>>>
		Sleep(50);
		start_timeout = 1;//开始计时
		free(new_buf);
		free(fcs);
		free(new_bit_buf);

	}
	else {
		//下层是字节数组接口，可直接发送
		iSndRetval = SendtoLower(buf, len, 0);
		iSndRetval = iSndRetval * 8;//换算成位
	}
	//如果考虑设计停等协议等重传协议，这份数据需要缓冲起来，应该另外申请空间，把buf或bufSend的内容保存起来，以备重传
	if (bufSend != NULL) {
		//保存bufSend内容，CODES NEED HERE
		//resent_buf = (U8*)malloc(sizeof(U8) * return_data_len);
		resent_buf = bufSend;
		resent_len = return_data_len;
		//本例程没有设计重传协议，不需要保存数据，所以将空间释放
		//free(bufSend);
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
	int iSndRetval = 0;
	U8* bufSend = NULL;
	if (ifNo == 0 && lowerNumber > 1) {
		//从接口0收到的数据，直接转发到接口1 —— 仅仅用于测试
		if (lowerMode[0] == lowerMode[1]) {
			//接口0和1的数据格式相同，直接转发
			iSndRetval = SendtoLower(buf, len, 1);
			if (lowerMode[0] == 1) {
				iSndRetval = iSndRetval * 8;//如果接口格式为bit数组，统一换算成位，完成统计
			}
		}
		else {
			//接口0与接口1的数据格式不同，需要转换后，再发送
			if (lowerMode[0] == 1) {
				//从接口0到接口1，接口0是字节数组，接口1是比特数组，需要扩大8倍转换
				bufSend = (U8*)malloc(len * 8);
				if (bufSend == NULL) {
					cout << "内存空间不够，导致数据没有被处理" << endl;
					return;
				}
				//byte to bit
				iSndRetval = ByteArrayToBitArray(bufSend, len * 8, buf, len);
				iSndRetval = SendtoLower(bufSend, iSndRetval, 1);
			}
			else {
				//从接口0到接口1，接口0是比特数组，接口1是字节数组，需要缩小八分之一转换
				bufSend = (U8*)malloc(len / 8 + 1);
				if (bufSend == NULL) {
					cout << "内存空间不够，导致数据没有被处理" << endl;
					return;
				}
				//bit to byte
				iSndRetval = BitArrayToByteArray(buf, len, bufSend, len / 8 + 1);
				iSndRetval = SendtoLower(bufSend, iSndRetval, 1);

				iSndRetval = iSndRetval * 8;//换算成位，做统计

			}
		}
		//统计
		if (iSndRetval <= 0) {
			iSndErrorCount++;
		}
		else {
			iRcvForward += iSndRetval;
			iRcvForwardCount++;
		}
	}
	else {
		//非接口0的数据，或者低层只有1个接口的数据，都向上递交
		if (lowerMode[ifNo] == 0) {
			//如果接口0是比特数组格式，高层默认是字节数组，先转换成字节数组，再向上递交
			bufSend = (U8*)malloc(len / 8 + 1);
			if (bufSend == NULL) {
				cout << "内存空间不够，导致数据没有被处理" << endl;
				return;
			}
			U8* true_data = (U8*)malloc(sizeof(U8) * (len / 8 + 1));//true_data:去掉定位符的数据,bit数组
			int true_data_len = 0;
			true_data = getFrame(buf, &true_data_len);
			U8* true_data_byte = (U8*)malloc(sizeof(U8) * (true_data_len / 8));
			BitArrayToByteArray(true_data, true_data_len, true_data_byte, true_data_len / 8);
			if (checkCrc(true_data_byte, true_data_len/8))
			{
				switch (true_data_byte[1]) {
					case 0x03:
						true_data_byte = removeFrameHeadAndFCS(true_data_byte, true_data_len / 8);
						iSndRetval = true_data_len / 8 - 4;
						iSndRetval = SendtoUpper(true_data_byte, iSndRetval);
						iSndRetval = iSndRetval * 8;//换算成位,进行统计
						//发送ACK确认
						SendtoLower(ack_array, 7 , 0);
						break;
					case 0x01:
						//如果是确认帧，取消超时重传
						start_timeout = 0;
						break;
				}
			}
			else //校验出错误
			{
				//不发ACK，什么都不干
			}
			
			

		}
		else {
			//低层是字节数组接口，可直接递交
			iSndRetval = SendtoUpper(buf, len);
			iSndRetval = iSndRetval * 8;//换算成位，进行统计
		}
		//统计
		if (iSndRetval <= 0) {
			iSndErrorCount++;
		}
		else {
			iRcvToUpper += iSndRetval;
			iRcvToUpperCount++;
		}
	}
	//如果需要重传等机制，可能需要将buf或bufSend中的数据另外申请空间缓存起来
	if (bufSend != NULL) {
		//缓存bufSend数据，如果有必要的话

		//本例程中没有停等协议，bufSend的空间在用完以后需要释放
		free(bufSend);
	}
	else {
		//缓存buf里的数据，如果有必要的话

		//buf空间不需要释放
	}

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
	cout << endl << endl << "设备号:" << strDevID << ",    层次:" << strLayer << ",    实体号:" << strEntity;
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

U8* MakeFrame(U8* byte_data,int len, int* return_data_len)
{
	U8* bitArray = (U8*)malloc(len * 8 + 16);  //申请一块原数据长度8倍的缓冲区，每个位扩大为1个字节，再加上两头八位的定界符：01111110
	ByteArrayToBitArray(bitArray + 8, len * 8+16, byte_data, len); //变换成比特数组格式，前面空出8位。
	U8* sendBitArray;
	sendBitArray = (U8*)malloc(len * 8 * 2 + 16);//再申请一个更大的缓冲空间
	int count = 0;
	int i, j;
	for (i = 8,j = 8; i<=8+len*8; i++, j++) {
		sendBitArray[j] = bitArray[i];
		//逐个判断是不是出现了连续5个1，具体方法自己想哦。
		if (sendBitArray[j] == 1)
		{
			count += 1;
		}
		if (sendBitArray[j] == 0)
		{
			count = 0;
		}
		if (count == 5) {
			//包括bitArray[i]有5个连续的1
			j++;
			sendBitArray[j] = 0;  //插入1个零
			count = 0;
		}
	}
	j -= 1;//for循环先自增再判断，此时j多了1
	*return_data_len = j+8;
	sendBitArray[0] = 0, sendBitArray[1] = 1, sendBitArray[2] = 1, sendBitArray[3] = 1, sendBitArray[4] = 1, sendBitArray[5] = 1, sendBitArray[6] = 1, sendBitArray[7] = 0; //首部加定界符。
	sendBitArray[j] = 0, sendBitArray[j + 1] = 1, sendBitArray[j + 2] = 1, sendBitArray[j + 3] = 1, sendBitArray[j + 4] = 1, sendBitArray[j + 5] = 1, sendBitArray[j + 6] = 1, sendBitArray[j + 7] = 0;//尾部加定界符。
	//SendtoLower(sendBitArray, j + 8, 0); //发送到低层的0号接口。
	//free(bitArray);      //缓冲区用完，释放空间
	//free(sendBitArray);    //如果不需要保留，以备重传，则释放其空间。
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