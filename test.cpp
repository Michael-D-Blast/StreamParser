#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "Protocol.h"
#include "time.h"
#include <stdio.h>
#include <openssl/md5.h>

using namespace std;
const char CHARGER_NUMBER[] = "18000020";
const char CHARGER_PRIVATE_KEY[] = "XuUYgM37\n";
const uint16 PROTOCOL_MAGIC_NUM = 0xF566;

static void getCurrentBCDTime(uint8 *outputTime)
{
	time_t calendaTime = time(NULL);	
	struct tm *brokendownTime = localtime(&calendaTime);

	outputTime[0]=(uint8)(((((brokendownTime->tm_year + 1900)/100)/10)<<4)|(((brokendownTime->tm_year + 1900)/100)%10));
	outputTime[1]=(uint8)(((((brokendownTime->tm_year + 1900)%100)/10)<<4)|(((brokendownTime->tm_year + 1900)%100)%10));
	outputTime[2]=(uint8)((((brokendownTime->tm_mon + 1)/10)<<4)|((brokendownTime->tm_mon + 1)%10));
	outputTime[3]=(uint8)((((brokendownTime->tm_mday)/10)<<4)|((brokendownTime->tm_mday)%10));
	outputTime[4]=(uint8)((((brokendownTime->tm_hour)/10)<<4)|((brokendownTime->tm_hour)%10));
	outputTime[5]=(uint8)((((brokendownTime->tm_min)/10)<<4)|((brokendownTime->tm_min)%10));
	outputTime[6]=(uint8)((((brokendownTime->tm_sec)/10)<<4)|((brokendownTime->tm_sec)%10));

//	for (int i = 0; i < 7; i++)
//	{
//		printf("%02x", outputTime[i]);
//	}
}

static void generateMD5WithTimeAndPrivateKey (uint8 *time, uint8 timeLen, uint8 *privateKey, uint8 privateKeyLen, char *md5InASCII)
{
	uint8 md5InHex[16];
	memset(md5InHex, 0, 16);

#if 0
	MD5_CTX ctx;

	MD5_Init(&ctx);
	MD5_Update(&ctx, time, timeLen);
	MD5_Update(&ctx, privateKey, privateKeyLen);
	MD5_Final(md5InHex, &ctx);
#endif

	uint8 *tmp = (uint8 *)malloc(timeLen + privateKeyLen);
	memset(tmp, 0, timeLen + privateKeyLen);
	memcpy(tmp, time, timeLen);
	memcpy(tmp + timeLen, privateKey, privateKeyLen);
	MD5(tmp, timeLen + privateKeyLen, (uint8 *)&md5InHex);

	for (int i = 0; i < 16; i++)
	{
		md5InASCII[i*2] = (md5InHex[i] >> 4) + '0';
		md5InASCII[i*2 + 1] = (md5InHex[i] & 0xF) + '0';
		printf("%.2x", md5InHex[i]);
	}
}

// Return the LSB 8bits of checksum
static uint8 generateChecksumWithCmdTypeAndCmdBody (uint8 *cmdType, uint8 cmdTypeLen, uint8 *cmdBody, uint16 cmdBodyLen)
{
	uint32 checksum = 0;

	for (int i = 0; i < cmdTypeLen; i++)
	{
		checksum += cmdType[i];
	}

	for (int i = 0; i < cmdBodyLen; i++)
	{
		checksum += cmdBody[i];
	}

	return (uint8)(checksum & 0xFF);
}


void test()
{
	uint16 cmdType = 0x66;
	uint8 cmdBody[3] = {0xfb, 0xff, 0x0a};
	uint8 checksum = generateChecksumWithCmdTypeAndCmdBody((uint8 *)&cmdType, 2, (uint8 *)&cmdBody, 3);	
	printf("checksum is %.2x\n", checksum);

	
	//uint8 time[8] = {0x20, 0x18, 0x08, 0x12, 0x00, 0x00, 0x00, 0xff};
	uint8 time[3] = {'a', 'b', 'c'};
	char md5[32] = {0};
	generateMD5WithTimeAndPrivateKey((uint8 *)&time, 3, (uint8 *)CHARGER_PRIVATE_KEY, 9, (char *)md5);

	printf("\nmd5 ascii:");
	for(int i = 0; i < 32; i++)
		printf("%c", md5[i]);

	printf("\n");
}

int main (int argc, char *argv[])
{

	test();
	return 0;

	int sfd;
	if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		cout << "Failed to create socket (errno : " << errno << ")" << endl;	

		return -1;
	}

	struct sockaddr_in serverAddress;
	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr("106.15.58.35");
	serverAddress.sin_port = htons(18888);


//	if (connect(sfd, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr_in)) < 0)
//	{
//		cout << "Failed to connect socket (errno : " << errno << ")" << endl;	
//
//		close(sfd);
//
//		return -2;
//	}
//	
//	cout << "Connect successfully !" << endl;

	int headLen  = sizeof(struct FrameHead);
	int cmdLen = sizeof(struct CmdSignIn);
	int tailLen = sizeof(struct FrameTail);
	int frameLen = headLen + cmdLen + tailLen;

	void *frame = malloc(frameLen); 
	memset(frame, 0, frameLen);

	struct FrameHead *head = (struct FrameHead *)frame;
	head->magicNumber = PROTOCOL_MAGIC_NUM;
	memcpy((void *)head->chargerNumber, (void *)CHARGER_NUMBER, sizeof(CHARGER_NUMBER));
	head->frameLength = frameLen;
	head->cmdType = CMD_SIGN_IN;
	head->cmdSerialNumber = 0;

	struct CmdSignIn *cmd = (struct CmdSignIn *)head->cmdBody;
	cmd->startTimes = 10;
	cmd->gunQuantity = 1;
	cmd->heartBeatInterval = 10;
	cmd->heartBeatTimeoutTimes = 3;
	getCurrentBCDTime(cmd->currentServerTimeInBCD);
	cmd->currentServerTimeInBCD[7] = 0xff;

	struct FrameTail *tail = (struct FrameTail *)cmd->frameTail;
	tail->status = 0x80;
	memcpy(tail->timeInBCD, cmd->currentServerTimeInBCD, 8);
	generateMD5WithTimeAndPrivateKey(tail->timeInBCD, 8, (uint8 *)CHARGER_PRIVATE_KEY, strlen(CHARGER_PRIVATE_KEY), tail->signatureInMD5);
	tail->checksum = generateChecksumWithCmdTypeAndCmdBody((uint8 *)(&head->cmdType), sizeof(head->cmdType), head->cmdBody, sizeof(struct CmdSignIn));

	if (send(sfd, frame, frameLen, 0) != frameLen)
	{
		printf("Failed to send frame (errno : %d)\n", errno);

		close(sfd);
		
		return -3;
	}


	uint8 buf[512];	
	memset((void *)buf, 0, sizeof(buf));

	if(recv(sfd, buf, sizeof(struct FrameHead), 0) != sizeof(struct FrameHead))
	{
		printf("Failed to read frame head (errno : %d)\n", errno);

		close(sfd);
		
		return -4;
	}

	printf("We got a reply!\n");

	head = (struct FrameHead *)buf;
	printf("head magix number is %x", head->magicNumber);

	sleep(10);

	close(sfd);

	return 0;
}
