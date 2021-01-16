#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <thread>
using namespace std;



struct MAPS {
	long int addr;
	long int taddr;
	int type;
	struct MAPS *next;
};

struct RESULT {
	long int addr;
	struct RESULT *next;
};

struct FREEZE {
	long int addr;				// 地址
	char *value;				// 值
	int type;					// 类型
	struct FREEZE *next;		// 指向下一节点的指针
};

#define MODE_ROOT "MODE_ROOT"
#define MODE_NO_ROOT "MODE_NO_ROOT"
#define TRUE 1
#define FALSE 0

#define LEN sizeof(struct MAPS)
#define FRE sizeof(struct FREEZE)

typedef int BOOLEAN;
typedef struct MAPS *PMAPS;		// 存储maps的链表
typedef struct RESULT *PRES;	// 存储结果的链表
typedef struct FREEZE *PFREEZE;	// 存储冻结的数据的链表

typedef int TYPE;
typedef int RANGE;
typedef int COUNT;
typedef int COLOR;
typedef long int OFFSET;
typedef long int ADDRESS;
typedef char PACKAGENAME;

typedef int64_t QWORD;
typedef int32_t DWORD;
typedef int16_t WORD;
typedef int8_t BYTE;
typedef float FLOAT;
typedef double DOUBLE;

enum type {
    TYPE_DWORD,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_WORD,
    TYPE_BYTE,
    TYPE_QWORD,
};

enum Range {
    ALL,						// 所有内存
    B_BAD,						// B内存
    C_ALLOC,					// Ca内存
    C_BSS,						// Cb内存
    C_DATA,						// Cd内存
    C_HEAP,						// Ch内存
    JAVA_HEAP,					// Jh内存
    A_ANONYMOUS,				// A内存
    CODE_SYSTEM,				// Xs内存
    STACK,						// S内存
    ASHMEM						// As内存
};

enum Color {
    COLOR_SILVERY,				// 银色
    COLOR_RED,					// 红色
    COLOR_GREEN,				// 绿色
    COLOR_YELLOW,				// 黄色
    COLOR_DARK_BLUE,			// 蓝色
    COLOR_PINK,					// 粉色
    COLOR_SKY_BLUE,				// 天蓝
    COLOR_WHITE					// 白色
};

PMAPS Res = NULL;				// 全局buff(保存数据的地方)

PFREEZE Pfreeze = NULL;			// 用于存储冻结的数据
PFREEZE pEnd = NULL;
PFREEZE pNew = NULL;
int FreezeCount = 0;			// 冻结数据个数
int Freeze = 0;					// 开关
pthread_t pth;
char bm[64];					// 包名
long int delay = 30000;			// 冻结延迟,默认30000us
int handle; //mem句柄
int ResCount = 0;				// 结果数量
int gs = 0;
int MemorySearchRange = 0;		// 0为所有
int ms = 0;
int SetTextColor(int color);
int getPID(char bm[64]);		// 获取pid

void initXMemoryTools(char *argv[], char *bm, char *mode);
int SetSearchRange(int type);	// 设置搜索范围
PMAPS readmaps(int type);
PMAPS readmaps_all();	// 读取maps文件
PMAPS readmaps_bad();	// 读取maps文件
PMAPS readmaps_c_alloc();	// 读取maps文件
PMAPS readmaps_c_bss();	// 读取maps文件
PMAPS readmaps_c_data();	// 读取maps文件
PMAPS readmaps_c_heap();	// 读取maps文件
PMAPS readmaps_java_heap();	// 读取maps文件
PMAPS readmaps_a_anonymous();	// 读取maps文件
PMAPS readmaps_code_system();	// 读取maps文件
PMAPS readmaps_stack();	// 读取maps文件
PMAPS readmaps_ashmem();	// 读取maps文件

void BaseAddressSearch(char *value, int type, long int BaseAddr);	// 基址搜索
PMAPS BaseAddressSearch_DWORD(DWORD value, long int BaseAddr, PMAPS pMap);	// DWORD
PMAPS BaseAddressSearch_FLOAT(FLOAT value, long int BaseAddr, PMAPS pMap);	// FLOAT
PMAPS BaseAddressSearch_DOUBLE(DOUBLE value, long int BaseAddr, PMAPS pMap);	// DOUBLE
PMAPS BaseAddressSearch_WORD(WORD value, long int BaseAddr, PMAPS pMap);	// WORD
PMAPS BaseAddressSearch_BYTE(BYTE value, long int BaseAddr, PMAPS pMap);	// BYTE
PMAPS BaseAddressSearch_QWORD(QWORD value, long int BaseAddr, PMAPS pMap);	// QWORD


void RangeMemorySearch(char *from_value, char *to_value, int type);	// 范围搜索
PMAPS RangeMemorySearch_DWORD(DWORD from_value, DWORD to_value, PMAPS pMap);	// DWORD
PMAPS RangeMemorySearch_FLOAT(FLOAT from_value, FLOAT to_value, PMAPS pMap);	// FLOAT
PMAPS RangeMemorySearch_DOUBLE(DOUBLE from_value, DOUBLE to_value, PMAPS pMap);	// WORD
PMAPS RangeMemorySearch_WORD(WORD from_value, WORD to_value, PMAPS pMap);	// WORD
PMAPS RangeMemorySearch_BYTE(BYTE from_value, BYTE to_value, PMAPS pMap);	// BYTE
PMAPS RangeMemorySearch_QWORD(QWORD from_value, QWORD to_value, PMAPS pMap);	// QWORD

void MemorySearch(char *value, int TYPE);	// 类型搜索,这里value需要传入一个地址
PMAPS MemorySearch_DWORD(DWORD value, PMAPS pMap);	// 内存搜索DWORD
PMAPS MemorySearch_FLOAT(FLOAT value, PMAPS pMap);	// 内存搜索FLOAT
PMAPS MemorySearch_DOUBLE(DOUBLE value, PMAPS pMap);	// 内存搜索DOUBLE
PMAPS MemorySearch_WORD(WORD value, PMAPS pMap);	// 内存搜索WORD
PMAPS MemorySearch_BYTE(BYTE value, PMAPS pMap);	// 内存搜索BYTE
PMAPS MemorySearch_QWORD(QWORD value, PMAPS pMap);	// 内存搜索QWORD

void MemoryOffset(char *value, long int offset, int type);	// 搜索偏移
PMAPS MemoryOffset_DWORD(DWORD value, long int offset, PMAPS pBuff);	// 搜索偏移DWORD
PMAPS MemoryOffset_FLOAT(FLOAT value, long int offset, PMAPS pBuff);	// 搜索偏移FLOAT
PMAPS MemoryOffset_DOUBLE(DOUBLE value, long int offset, PMAPS pBuff);	// 搜索偏移DOUBLE
PMAPS MemoryOffset_WORD(WORD value, long int offset, PMAPS pBuff);	// 搜索偏移WORD
PMAPS MemoryOffset_BYTE(BYTE value, long int offset, PMAPS pBuff);	// 搜索偏移BYTE
PMAPS MemoryOffset_QWORD(QWORD value, long int offset, PMAPS pBuff);	// 搜索偏移QWORD

void RangeMemoryOffset(char *from_value, char *to_value, long int offset, int type);	// 范围偏移
PMAPS RangeMemoryOffset_DWORD(DWORD from_value, DWORD to_value, long int offset, PMAPS pBuff);	// 范围偏移DWORD
PMAPS RangeMemoryOffset_FLOAT(FLOAT from_value, FLOAT to_value, long int offset, PMAPS pBuff);	// 范围偏移FLOAT
PMAPS RangeMemoryOffset_DOUBLE(DOUBLE from_value, DOUBLE to_value, long int offset, PMAPS pBuff);	// 范围偏移DOUBLE
PMAPS RangeMemoryOffset_WORD(WORD from_value, WORD to_value, long int offset, PMAPS pBuff);	// 范围偏移WORD
PMAPS RangeMemoryOffset_BYTE(BYTE from_value, BYTE to_value, long int offset, PMAPS pBuff);	// 范围偏移BYTE
PMAPS RangeMemoryOffset_QWORD(QWORD from_value, QWORD to_value, long int offset, PMAPS pBuff);	// 范围偏移QWORD

void MemoryWrite(char *value, long int offset, int type);	// 内存写入
int MemoryWrite_DWORD(DWORD value, PMAPS pBuff, long int offset);	// 内存写入DWORD
int MemoryWrite_FLOAT(FLOAT value, PMAPS pBuff, long int offset);	// 内存写入FLOAT
int MemoryWrite_DOUBLE(DOUBLE value, PMAPS pBuff, long int offset);	// 内存写入DOUBLE
int MemoryWrite_WORD(WORD value, PMAPS pBuff, long int offset);	// 内存写入WORD
int MemoryWrite_BYTE(BYTE value, PMAPS pBuff, long int offset);	// 内存写入BYTE
int MemoryWrite_QWORD(QWORD value, PMAPS pBuff, long int offset);	// 内存写入QWORD


int WriteAddress(long int addr, char *value, int type);	// 修改地址中的值
int WriteAddress_DWORD(ADDRESS addr, DWORD value);
int WriteAddress_FLOAT(ADDRESS addr, FLOAT value);
int WriteAddress_DOUBLE(ADDRESS addr, DOUBLE value);
int WriteAddress_WORD(ADDRESS addr, WORD value);
int WriteAddress_BYTE(ADDRESS addr, BYTE value);
int WriteAddress_QWORD(ADDRESS addr, QWORD value);

void PrintResults();					// 打印Res里面的内容
void ClearResults();			// 清除链表,释放空间
void ClearMaps(PMAPS pMap);		// 清空maps
int GetResultCount();	//获取搜索结果数量

int isapkinstalled(char *bm);	// 检测应用是否安装
int isapkrunning(char *bm);		// 检测应用是否运行
int killprocess(char *bm);		// 杀掉进程
char GetProcessState(char *bm);	// 获取进程状态
int killGG();					// 杀掉gg修改器
int killXs();					// 杀xs
int uninstallapk(char *bm);		// 静默删除软件
int installapk(char *lj);		// 静默卸载软件
int rebootsystem();				// 重启系统(手机)
int PutDate();					// 输出系统日期
int GetDate(char *date);		// 获取系统时间

PMAPS GetResults();				// 获取结果,返回头指针
int AddFreezeItem_All(char *Value, int type, long int offset);	// 冻结所有结果
int AddFreezeItem(long int addr, char *value, int type, long int offset);	// 增加冻结数据
int AddFreezeItem_DWORD(long int addr, char *value);	// DWORD
int AddFreezeItem_FLOAT(long int addr, char *value);	// FLOAT
int AddFreezeItem_DOUBLE(long int addr, char *value);	// DOUBLE
int AddFreezeItem_WORD(long int addr, char *value);	// WORD
int AddFreezeItem_BYTE(long int addr, char *value);	// BYTE
int AddFreezeItem_QWORD(long int addr, char *value);	// QWORD

int RemoveFreezeItem(long int addr);	// 清除固定冻结数据
int RemoveFreezeItem_All();		// 清空所有冻结数据
int StartFreeze();		// 开始冻结
int StopFreeze();				// 停止冻结
int SetFreezeDelay(long int De);	// 设置冻结延迟
int PrintFreezeItems();			// 打印冻结表

char *GetAddressValue(ADDRESS addr,int type);
DWORD GetAddressValue_DWORD(ADDRESS addr);
FLOAT GetAddressValue_FLOAT(ADDRESS addr);
DOUBLE GetAddressValue_DOUBLE(ADDRESS addr);
WORD GetAddressValue_WORD(ADDRESS addr);
BYTE GetAddressValue_BYTE(ADDRESS addr);
QWORD GetAddressValue_QWORD(ADDRESS addr);

char *getRightText(char *dst,char *src,int n);//取右边文本
bool getMask(long int addr ,char *def);//判断蒙版
void MaskSearch(char *def, int type);//蒙版搜索

bool snowpd(long int addr);

void initXMemoryTools(char *b, char *mode) {
	strcpy(bm, b);
	if (strcmp(mode, MODE_ROOT)==0) {
		if (getuid() != 0) {
			system("echo starting");
			exit(1);
		}
	}
	system("echo 0 > /proc/sys/fs/inotify/max_user_watches");
	//system("echo 0 > /proc/sys/fs/inotify/max_queued_events");
	pid_t pid = getPID(b);
	if (pid == 0) {
		printf("\033[31;1m");
		puts("获取进程失败!");
		exit(1);
	}
	char lj[64];
	sprintf(lj, "/proc/%d/mem", pid);
	handle = open(lj, O_RDWR);
	if (handle == -1) {
		printf("\033[31;1m");
		puts("获取mem失败!");
		exit(1);
	}
	lseek(handle, 0, SEEK_SET);
}

int GetResultCount() {
	return gs;
}

bool snowpd(long int a2) {
	return a2 <= 7999 && a2 > 31;
}

DWORD GetAddressValue_DWORD(ADDRESS addr) {
	DWORD buf;
	pread64(handle, &buf, sizeof(DWORD), addr);
	return buf;
}

FLOAT GetAddressValue_FLOAT(ADDRESS addr) {
	FLOAT buf;
	pread64(handle, &buf, sizeof(FLOAT), addr);
	return buf;
}

DOUBLE GetAddressValue_DOUBLE(ADDRESS addr) {
	DOUBLE buf;
	pread64(handle, &buf, sizeof(DOUBLE), addr);
	return buf;
}

WORD GetAddressValue_WORD(ADDRESS addr) {
	WORD buf;
	pread64(handle, &buf, sizeof(WORD), addr);
	return buf;
}

BYTE GetAddressValue_BYTE(ADDRESS addr) {
	BYTE buf;
	pread64(handle, &buf, sizeof(BYTE), addr);
	return buf;
}

QWORD GetAddressValue_QWORD(ADDRESS addr) {
	QWORD buf;
	pread64(handle, &buf, sizeof(QWORD), addr);
	return buf;
}

char *GetAddressValue(ADDRESS addr,int type) {
	char *str = (char *)malloc(sizeof(char));
	DWORD i;
	FLOAT f;
	DOUBLE d;
	WORD w;
	BYTE b;
	QWORD q;
	switch (type) {
		case TYPE_DWORD:
			i = GetAddressValue_DWORD(addr);
			sprintf(str,"%d",i);
			break;
		case TYPE_FLOAT:
			f = GetAddressValue_FLOAT(addr);
			sprintf(str,"%e",f);
			break;
		case TYPE_DOUBLE:
			d = GetAddressValue_DOUBLE(addr);
			sprintf(str,"%e",d);
			break;
		case TYPE_WORD:
			w = GetAddressValue_WORD(addr);
			sprintf(str,"%d",w);
			break;
		case TYPE_BYTE:
			b = GetAddressValue_BYTE(addr);
			sprintf(str,"%d",b);
			break;
		case TYPE_QWORD:
			q = GetAddressValue_QWORD(addr);
			sprintf(str,"%ld",q);
			break;
		default:
			sprintf(str,"%s","");
			printf("\033[32;1m你选择了一个未知类型!\n");
			break;
	}
	return str;
}

char *getRightText(char *dst,char *src,int n) {
	char *p = src;
	char *q = dst;
	int len = strlen(src);
	if(n>len) n=len;
	p+=(len-n);
	while(*(q++)=*(p++));
	return dst;
}

bool getMask(long int addr ,char *def) {
	char *str = (char *)malloc(sizeof(char));
	sprintf(str,"%lX",addr);
	char *dest;
	char* s = getRightText(dest,str,strlen(def));
	if (strcmp(s, def) == 0) {
		return true; //符合
	}
	return false;
}

void MaskSearch(char *def, int type) {
	PMAPS e, n;
	e = n = (PMAPS) malloc(LEN);
	PMAPS pBuff = n;
	int iCount = 0;
	PMAPS pTemp = Res;
	for (int i = 0; i < ResCount; i++) {
		long int addr = pTemp->addr;
		bool value = getMask(addr, def);
		if (value == true) {
			iCount ++;
			n->addr = addr;
			n->type = type;
			if (iCount == 1) {
				n->next = NULL;
				e = n;
				pBuff = n;
			} else {
				n->next = NULL;
				e->next = n;
				e = n;
			}
			n = (PMAPS) malloc(LEN);
		}
		pTemp = pTemp->next;		// 指向下一个节点
	}
	ResCount = iCount;
	gs = iCount;
	Res = pBuff;
}

int SetTextColor(COLOR color) {
	switch (color) {
		case COLOR_SILVERY:
			printf("\033[30;1m");
			break;
		case COLOR_RED:
			printf("\033[31;1m");
			break;
		case COLOR_GREEN:
			printf("\033[32;1m");
			break;
		case COLOR_YELLOW:
			printf("\033[33;1m");
			break;
		case COLOR_DARK_BLUE:
			printf("\033[34;1m");
			break;
		case COLOR_PINK:
			printf("\033[35;1m");
			break;
		case COLOR_SKY_BLUE:
			printf("\033[36;1m");
			break;
		case COLOR_WHITE:
			printf("\033[37;1m");
			break;
		default:
			printf("\033[37;1m");
			break;
	}
	return 0;
}

long int GetModuleBase(char *module,char * flags,int MapSize) {
	PMAPS pHead = NULL;
	PMAPS pNew = NULL;
	PMAPS pEnd = NULL;
	pEnd = pNew = (PMAPS) malloc(MapSize);
	FILE *fp;
	int flag = 1;
	char lj[64], buff[256];
	int pid = getPID(bm);
	sprintf(lj, "/proc/%d/maps", pid);
	fp = fopen(lj, "r");
	if (fp == NULL) {
		puts("内存读取失败!");
		return 0;
	}
	while (!feof(fp)) {
		fgets(buff, sizeof(buff), fp);	// 读取一行
		if (strstr(buff, flags) != NULL && !feof(fp) && strstr(buff, module)) {
			sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
			// 这里使用lx是为了能成功读取特别长的地址
			flag = 1;
		} else {
			flag = 0;
		}
		if (flag == 1) {
			pNew->next = NULL;
			pEnd = pNew;
			pHead = pNew;
			pNew = (PMAPS) malloc(MapSize);	// 分配内存
		}
	}
	free(pNew);					// 将多余的空间释放
	fclose(fp);					// 关闭文件指针
	return pHead->addr;
}

int getPID(PACKAGENAME * PackageName) {
	DIR *dir = NULL;
	struct dirent *ptr = NULL;
	FILE *fp = NULL;
	char filepath[256];			// 大小随意，能装下cmdline文件的路径即可
	char filetext[128];			// 大小随意，能装下要识别的命令行文本即可
	dir = opendir("/proc");		// 打开路径
	if (NULL != dir) {
		while ((ptr = readdir(dir)) != NULL) {	// 循环读取路径下的每一个文件/文件夹
			// 如果读取到的是"."或者".."则跳过，读取到的不是文件夹名字也跳过
			if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0))
				continue;
			if (ptr->d_type != DT_DIR)
				continue;
			sprintf(filepath, "/proc/%s/cmdline", ptr->d_name);	// 生成要读取的文件的路径
			fp = fopen(filepath, "r");	// 打开文件
			if (NULL != fp) {
				fgets(filetext, sizeof(filetext), fp);	// 读取文件
				if (strcmp(filetext, PackageName) == 0) {
					// puts(filepath);
					// printf("packagename:%s\n",filetext);
					break;
				}
				fclose(fp);
			}
		}
	}
	if (readdir(dir) == NULL) {
		return 0;
	}
	closedir(dir);				// 关闭路径
	return atoi(ptr->d_name);
}

void PrintResults() {
	PMAPS temp = Res;
	const char *type = (char *)malloc(sizeof(char));
	for (int i = 0; i < ResCount; i++) {
		switch (temp->type) {
			case TYPE_DWORD:
				type = "DWORD";
				break;
			case TYPE_FLOAT:
				type = "FLOAT";
				break;
			case TYPE_DOUBLE:
				type = "DOUBLE";
				break;
			case TYPE_WORD:
				type = "WORD";
				break;
			case TYPE_BYTE:
				type = "BYTE";
				break;
			case TYPE_QWORD:
				type = "QWORD";
				break;
			default:
				type = "";
				break;
		}

		printf("\e[37;1mAddr:\e[32;1m0x%lX  \e[37;1mType:\e[36;1m%s\n",temp->addr,type);
		//printf("addr:\e[32;1m0x%lX type:%s\n", temp->addr,type);
		temp = temp->next;		// 指向下一个节点
	}
}

void ClearResults()	{
	PMAPS pHead = Res;
	PMAPS pTemp = pHead;
	int i;
	for (i = 0; i < ResCount; i++) {
		pTemp = pHead;
		pHead = pHead->next;
		free(pTemp);
	}
}

void BaseAddressSearch(char *value, TYPE type, ADDRESS BaseAddr) {
	PMAPS pHead = NULL;
	PMAPS pMap = NULL;
	gs = 0;
	switch (MemorySearchRange) {
		case ALL:
			pMap = readmaps(ALL);
			break;
		case B_BAD:
			pMap = readmaps(B_BAD);
			break;
		case C_ALLOC:
			pMap = readmaps(C_ALLOC);
			break;
		case C_BSS:
			pMap = readmaps(C_BSS);
			break;
		case C_DATA:
			pMap = readmaps(C_DATA);
			break;
		case C_HEAP:
			pMap = readmaps(C_HEAP);
			break;
		case JAVA_HEAP:
			pMap = readmaps(JAVA_HEAP);
			break;
		case A_ANONYMOUS:
			pMap = readmaps(A_ANONYMOUS);
			break;
		case CODE_SYSTEM:
			pMap = readmaps(CODE_SYSTEM);
			break;
		case STACK:
			pMap = readmaps(STACK);
			break;
		case ASHMEM:
			pMap = readmaps(ASHMEM);
			break;
		default:
			printf("\033[32;1mYou Select A NULL Type!\n");
			break;
	}
	if (pMap == NULL) {
		puts("map error");
		return;
	}
	switch (type) {
		case TYPE_DWORD:
			pHead = BaseAddressSearch_DWORD(atoi(value), BaseAddr, pMap);
			break;
		case TYPE_FLOAT:
			pHead = BaseAddressSearch_FLOAT(atof(value), BaseAddr, pMap);
			break;
		case TYPE_DOUBLE:
			pHead = BaseAddressSearch_DOUBLE(atof(value), BaseAddr, pMap);
			break;
		case TYPE_WORD:
			pHead = BaseAddressSearch_WORD(atoi(value), BaseAddr, pMap);
			break;
		case TYPE_BYTE:
			pHead = BaseAddressSearch_BYTE(atoi(value), BaseAddr, pMap);
			break;
		case TYPE_QWORD:
			pHead = BaseAddressSearch_QWORD(atoi(value), BaseAddr, pMap);
			break;
		default:
			printf("\033[32;1mYou Select A NULL Type!\n");
			break;
	}
	if (pHead == NULL) {
		puts("搜索内存失败!");
		return (void)0;
	}
	ResCount = gs;
	Res = pHead;				// Res指针指向链表
}

PMAPS BaseAddressSearch_DWORD(DWORD value, ADDRESS BaseAddr, PMAPS pMap) {
	//*gs = 0;
	// printf("BaseAddress:%lX\n",BaseAddr);
	PMAPS e, n;
	e = n = (PMAPS) malloc(LEN);
	PMAPS pBuff = n;
	int iCount = 0;
	long int c, ADDR;
	void *BUF[8];
	PMAPS pTemp = pMap;
	while (pTemp != NULL) {
		c = (pTemp->taddr - pTemp->addr) / 4096;
		for (int j = 0; j < c; j++) {
			ADDR = pTemp->addr + j * 4096 + BaseAddr;
			pread64(handle, BUF, 8, ADDR);
			if (*(DWORD *)&BUF[0] == value) {
				iCount++;
				gs += 1;
				n->addr = ADDR;
				n->type = TYPE_DWORD;
				// printf("addr:%lx,val:%d,buff=%d\n",n->addr,value,buff[i]);
				if (iCount == 1) {
					n->next = NULL;
					e = n;
					pBuff = n;
				} else {
					n->next = NULL;
					e->next = n;
					e = n;
				}
				n = (PMAPS) malloc(LEN);
			}
		}
		pTemp = pTemp->next;
	}
	return pBuff;
}

PMAPS BaseAddressSearch_DOUBLE(DOUBLE value, ADDRESS BaseAddr, PMAPS pMap) {
	//*gs = 0;
	// printf("BaseAddress:%lX\n",BaseAddr);
	PMAPS e, n;
	e = n = (PMAPS) malloc(LEN);
	PMAPS pBuff = n;
	int iCount = 0;
	long int c, ADDR;
	void *BUF[8];
	PMAPS pTemp = pMap;
	while (pTemp != NULL) {
		c = (pTemp->taddr - pTemp->addr) / 4096;
		for (int j = 0; j < c; j++) {
			ADDR = pTemp->addr + j * 4096 + BaseAddr;
			pread64(handle, BUF, 8, ADDR);
			if (*(DOUBLE *)&BUF[0] == value) {
				iCount++;
				gs += 1;
				n->addr = ADDR;
				n->type = TYPE_DOUBLE;
				// printf("addr:%lx,val:%d,buff=%d\n",n->addr,value,buff[i]);
				if (iCount == 1) {
					n->next = NULL;
					e = n;
					pBuff = n;
				} else {
					n->next = NULL;
					e->next = n;
					e = n;
				}
				n = (PMAPS) malloc(LEN);
			}
		}
		pTemp = pTemp->next;
	}
	return pBuff;
}

PMAPS BaseAddressSearch_FLOAT(FLOAT value, ADDRESS BaseAddr, PMAPS pMap) {
	//*gs = 0;
	PMAPS e, n;
	e = n = (PMAPS) malloc(LEN);
	PMAPS pBuff = n;
	long int c, ADDR;
	int iCount = 0;
	void *BUF[8];
	PMAPS pTemp = pMap;
	while (pTemp != NULL) {
		c = (pTemp->taddr - pTemp->addr) / 4096;
		for (int j = 0; j < c; j++) {
			ADDR = pTemp->addr + j * 4096 + BaseAddr;
			pread64(handle, BUF, 8, ADDR);
			if (*(FLOAT *)&BUF[0] == value) {
				iCount++;
				gs += 1;
				n->addr = ADDR;
				n->type = TYPE_FLOAT;
				// printf("addr:%lx,val:%d,buff=%d\n",n->addr,value,buff[i]);
				if (iCount == 1) {
					n->next = NULL;
					e = n;
					pBuff = n;
				} else {
					n->next = NULL;
					e->next = n;
					e = n;
				}
				n = (PMAPS) malloc(LEN);
			}
		}
		pTemp = pTemp->next;
	}
	return pBuff;
}

PMAPS BaseAddressSearch_WORD(WORD value, ADDRESS BaseAddr, PMAPS pMap) {
	//*gs = 0;
	PMAPS e, n;
	e = n = (PMAPS) malloc(LEN);
	PMAPS pBuff = n;
	long int c, ADDR;
	int iCount = 0;
	void *BUF[8];
	PMAPS pTemp = pMap;
	while (pTemp != NULL) {
		c = (pTemp->taddr - pTemp->addr) / 4096;
		for (int j = 0; j < c; j++) {
			ADDR = pTemp->addr + j * 4096 + BaseAddr;
			pread64(handle, BUF, 8, ADDR);
			if (*(WORD *)&BUF[0] == value) {
				iCount++;
				gs += 1;
				n->addr = ADDR;
				n->type = TYPE_WORD;
				// printf("addr:%lx,val:%d,buff=%d\n",n->addr,value,buff[i]);
				if (iCount == 1) {
					n->next = NULL;
					e = n;
					pBuff = n;
				} else {
					n->next = NULL;
					e->next = n;
					e = n;
				}
				n = (PMAPS) malloc(LEN);
			}
		}
		pTemp = pTemp->next;
	}
	return pBuff;
}

PMAPS BaseAddressSearch_QWORD(QWORD value, ADDRESS BaseAddr, PMAPS pMap) {
	//*gs = 0;
	PMAPS e, n;
	e = n = (PMAPS) malloc(LEN);
	PMAPS pBuff = n;
	long int c, ADDR;
	int iCount = 0;
	void *BUF[8];
	PMAPS pTemp = pMap;
	while (pTemp != NULL) {
		c = (pTemp->taddr - pTemp->addr) / 4096;
		for (int j = 0; j < c; j++) {
			ADDR = pTemp->addr + j * 4096 + BaseAddr;
			pread64(handle, BUF, 8, ADDR);
			if (*(QWORD *)&BUF[0] == value) {
				iCount++;
				gs += 1;
				n->addr = ADDR;
				n->type = TYPE_QWORD;
				// printf("addr:%lx,val:%d,buff=%d\n",n->addr,value,buff[i]);
				if (iCount == 1) {
					n->next = NULL;
					e = n;
					pBuff = n;
				} else {
					n->next = NULL;
					e->next = n;
					e = n;
				}
				n = (PMAPS) malloc(LEN);
			}
		}
		pTemp = pTemp->next;
	}
	return pBuff;
}

PMAPS BaseAddressSearch_BYTE(BYTE value, ADDRESS BaseAddr, PMAPS pMap) {
	//*gs = 0;
	
	PMAPS e, n;
	e = n = (PMAPS) malloc(LEN);
	PMAPS pBuff = n;
	long int c, ADDR;
	int iCount = 0;
	void *BUF[8];
	PMAPS pTemp = pMap;
	while (pTemp != NULL) {
		c = (pTemp->taddr - pTemp->addr) / 4096;
		for (int j = 0; j < c; j++) {
			ADDR = pTemp->addr + j * 4096 + BaseAddr;
			pread64(handle, BUF, 8, ADDR);
			if (*(BYTE *)&BUF[0] == value) {
				iCount++;
				gs += 1;
				n->addr = ADDR;
				n->type = TYPE_BYTE;
				// printf("addr:%lx,val:%d,buff=%d\n",n->addr,value,buff[i]);
				if (iCount == 1) {
					n->next = NULL;
					e = n;
					pBuff = n;
				} else {
					n->next = NULL;
					e->next = n;
					e = n;
				}
				n = (PMAPS) malloc(LEN);
			}
		}
		pTemp = pTemp->next;
	}
	return pBuff;
}

void RangeMemorySearch(char *from_value, char *to_value, TYPE type) {
	PMAPS pHead = NULL;
	PMAPS pMap = NULL;
	gs = 0;
	switch (MemorySearchRange) {
		case ALL:
			pMap = readmaps(ALL);
			break;
		case B_BAD:
			pMap = readmaps(B_BAD);
			break;
		case C_ALLOC:
			pMap = readmaps(C_ALLOC);
			break;
		case C_BSS:
			pMap = readmaps(C_BSS);
			break;
		case C_DATA:
			pMap = readmaps(C_DATA);
			break;
		case C_HEAP:
			pMap = readmaps(C_HEAP);
			break;
		case JAVA_HEAP:
			pMap = readmaps(JAVA_HEAP);
			break;
		case A_ANONYMOUS:
			pMap = readmaps(A_ANONYMOUS);
			break;
		case CODE_SYSTEM:
			pMap = readmaps(CODE_SYSTEM);
			break;
		case STACK:
			pMap = readmaps(STACK);
			break;
		case ASHMEM:
			pMap = readmaps(ASHMEM);
			break;
		default:
			printf("\033[32;1mYou Select A NULL Type!\n");
			break;
	}
	if (pMap == NULL) {
		puts("map error");
		return;
	}
	switch (type) {
		case TYPE_DWORD:
			if (atoi(from_value) > atoi(to_value))
				pHead = RangeMemorySearch_DWORD(atoi(to_value), atoi(from_value), pMap);
			else
				pHead = RangeMemorySearch_DWORD(atoi(from_value), atoi(to_value), pMap);
			break;
		case TYPE_FLOAT:
			if (atof(from_value) > atof(to_value))
				pHead = RangeMemorySearch_FLOAT(atof(to_value), atof(from_value), pMap);
			else
				pHead = RangeMemorySearch_FLOAT(atof(from_value), atof(to_value), pMap);
			break;
		case TYPE_DOUBLE:
			if (atof(from_value) > atof(to_value))
				pHead = RangeMemorySearch_DOUBLE(atof(to_value), atof(from_value), pMap);
			else
				pHead = RangeMemorySearch_DOUBLE(atof(from_value), atof(to_value), pMap);
			break;
		case TYPE_WORD:
			if (atoi(from_value) > atoi(to_value))
				pHead = RangeMemorySearch_WORD(atoi(to_value), atoi(from_value), pMap);
			else
				pHead = RangeMemorySearch_WORD(atoi(from_value), atoi(to_value), pMap);
			break;
		case TYPE_BYTE:
			if (atoi(from_value) > atoi(to_value))
				pHead = RangeMemorySearch_BYTE(atoi(to_value), atoi(from_value), pMap);
			else
				pHead = RangeMemorySearch_BYTE(atoi(from_value), atoi(to_value), pMap);
			break;
		case TYPE_QWORD:
			if (atoi(from_value) > atoi(to_value))
				pHead = RangeMemorySearch_QWORD(atoi(to_value), atoi(from_value), pMap);
			else
				pHead = RangeMemorySearch_QWORD(atoi(from_value), atoi(to_value), pMap);
			break;
		default:
			printf("\033[32;1mYou Select A NULL Type!\n");
			break;
	}
	if (pHead == NULL) {
		puts("Range搜索内存失败!");
		return;
	}
	ResCount = gs;
	Res = pHead;				// Res指针指向链表
}

PMAPS RangeMemorySearch_DWORD(DWORD from_value, DWORD to_value, PMAPS pMap) {	// DWORD
	pid_t pid = getPID(bm);
	if (pid == 0) {
		puts("获取进程失败!");
		return NULL;
	}
	//*gs = 0;
	PMAPS pTemp = NULL;
	pTemp = pMap;
	PMAPS n, e;
	e = n = (PMAPS) malloc(LEN);
	PMAPS pBuff;
	pBuff = n;
	int iCount = 0;				// 链表长度
	int c;
	DWORD buff[1024] = { 0 };		// 缓冲区
	memset(buff, 0, 4);
	while (pTemp != NULL) {	// 读取maps里面的地址
		c = (pTemp->taddr - pTemp->addr) / 4096;
		for (int j = 0; j < c; j++) {
			pread64(handle, buff, 0x1000, pTemp->addr + j * 4096);
			for (int i = 0; i < 1024; i++) {
				if (buff[i] >= from_value && buff[i] <= to_value) {	// 判断值是否在这两者之间
					iCount++;
					gs += 1;
					n->addr = (pTemp->addr) + (j * 4096) + (i * 4);
					n->type = TYPE_DWORD;
					if (iCount == 1) {
						n->next = NULL;
						e = n;
						pBuff = n;
					} else {
						n->next = NULL;
						e->next = n;
						e = n;
					}
					n = (PMAPS) malloc(LEN);
				}
			}
		}
		pTemp = pTemp->next;
	}
	free(n);
	return pBuff;
}

PMAPS RangeMemorySearch_FLOAT(FLOAT from_value, FLOAT to_value, PMAPS pMap) {	// FLOAT
	pid_t pid = getPID(bm);
	if (pid == 0) {
		puts("获取进程失败!");
		return NULL;
	}
	//*gs = 0;
	PMAPS pTemp = NULL;
	pTemp = pMap;
	PMAPS n, e;
	e = n = (PMAPS) malloc(LEN);
	PMAPS pBuff;
	pBuff = n;
	int iCount = 0;				// 链表长度
	int c;
	FLOAT buff[1024] = { 0 };	// 缓冲区
	while (pTemp->next != NULL) {
		c = (pTemp->taddr - pTemp->addr) / 4096;
		for (int j = 0; j < c; j += 1) {
			pread64(handle, buff, 0x1000, pTemp->addr + (j * 4096));
			for (int i = 0; i < 1024; i += 1) {
				if (buff[i] >= from_value && buff[i] <= to_value) {	// 判断。。。
					iCount++;
					gs += 1;
					n->addr = (pTemp->addr) + (j * 4096) + (i * 4);
					n->type = TYPE_FLOAT;
					if (iCount == 1) {
						n->next = NULL;
						e = n;
						pBuff = n;
					} else {
						n->next = NULL;
						e->next = n;
						e = n;
					}
					n = (PMAPS) malloc(LEN);
				}
				// printf("buff[%d]=%f\n",l,buff[l]);
				// usleep(1);
			}
			// memset(buff,0,4);
		}
		pTemp = pTemp->next;
	}
	free(n);
	return pBuff;
}

PMAPS RangeMemorySearch_DOUBLE(DOUBLE from_value, DOUBLE to_value, PMAPS pMap) {	// FLOAT
	pid_t pid = getPID(bm);
	if (pid == 0) {
		puts("获取进程失败!");
		return NULL;
	}
	//*gs = 0;
	PMAPS pTemp = NULL;
	pTemp = pMap;
	PMAPS n, e;
	e = n = (PMAPS) malloc(LEN);
	PMAPS pBuff;
	pBuff = n;
	int iCount = 0;				// 链表长度
	int c;
	DOUBLE buff[1024] = { 0 };	// 缓冲区
	while (pTemp->next != NULL) {
		c = (pTemp->taddr - pTemp->addr) / 4096;
		for (int j = 0; j < c; j += 1) {
			pread64(handle, buff, 0x1000, pTemp->addr + (j * 4096));
			for (int i = 0; i < 1024; i += 1) {
				if (buff[i] >= from_value && buff[i] <= to_value) {	// 判断。。。
					iCount++;
					gs += 1;
					n->addr = (pTemp->addr) + (j * 4096) + (i * 4);
					n->type = TYPE_DOUBLE;
					if (iCount == 1) {
						n->next = NULL;
						e = n;
						pBuff = n;
					} else {
						n->next = NULL;
						e->next = n;
						e = n;
					}
					n = (PMAPS) malloc(LEN);
				}
				// printf("buff[%d]=%f\n",l,buff[l]);
				// usleep(1);
			}
			// memset(buff,0,4);
		}
		pTemp = pTemp->next;
	}
	free(n);
	return pBuff;
}

PMAPS RangeMemorySearch_WORD(WORD from_value, WORD to_value, PMAPS pMap) {	// FLOAT
	pid_t pid = getPID(bm);
	if (pid == 0) {
		puts("获取进程失败!");
		return NULL;
	}
	//*gs = 0;
	PMAPS pTemp = NULL;
	pTemp = pMap;
	PMAPS n, e;
	e = n = (PMAPS) malloc(LEN);
	PMAPS pBuff;
	pBuff = n;
	int iCount = 0;				// 链表长度
	int c;
	WORD buff[1024] = { 0 };	// 缓冲区
	while (pTemp->next != NULL) {
		c = (pTemp->taddr - pTemp->addr) / 4096;
		for (int j = 0; j < c; j += 1) {
			pread64(handle, buff, 0x1000, pTemp->addr + (j * 4096));
			for (int i = 0; i < 1024; i += 1) {
				if (buff[i] >= from_value && buff[i] <= to_value) {	// 判断。。。
					iCount++;
					gs += 1;
					n->addr = (pTemp->addr) + (j * 4096) + (i * 4);
					n->type = TYPE_WORD;
					if (iCount == 1) {
						n->next = NULL;
						e = n;
						pBuff = n;
					} else {
						n->next = NULL;
						e->next = n;
						e = n;
					}
					n = (PMAPS) malloc(LEN);
				}
				// printf("buff[%d]=%f\n",l,buff[l]);
				// usleep(1);
			}
			// memset(buff,0,4);
		}
		pTemp = pTemp->next;
	}
	free(n);
	return pBuff;
}

PMAPS RangeMemorySearch_BYTE(BYTE from_value, BYTE to_value, PMAPS pMap) {	// FLOAT
	pid_t pid = getPID(bm);
	if (pid == 0) {
		puts("获取进程失败!");
		return NULL;
	}
	//*gs = 0;
	PMAPS pTemp = NULL;
	pTemp = pMap;
	PMAPS n, e;
	e = n = (PMAPS) malloc(LEN);
	PMAPS pBuff;
	pBuff = n;
	int iCount = 0;				// 链表长度
	int c;
	BYTE buff[1024] = { 0 };	// 缓冲区
	while (pTemp->next != NULL) {
		c = (pTemp->taddr - pTemp->addr) / 4096;
		for (int j = 0; j < c; j += 1) {
			pread64(handle, buff, 0x1000, pTemp->addr + (j * 4096));
			for (int i = 0; i < 1024; i += 1) {
				if (buff[i] >= from_value && buff[i] <= to_value) {	// 判断。。。
					iCount++;
					gs += 1;
					n->addr = (pTemp->addr) + (j * 4096) + (i * 4);
					n->type = TYPE_BYTE;
					if (iCount == 1) {
						n->next = NULL;
						e = n;
						pBuff = n;
					} else {
						n->next = NULL;
						e->next = n;
						e = n;
					}
					n = (PMAPS) malloc(LEN);
				}
				// printf("buff[%d]=%f\n",l,buff[l]);
				// usleep(1);
			}
			// memset(buff,0,4);
		}
		pTemp = pTemp->next;
	}
	free(n);
	return pBuff;
}

PMAPS RangeMemorySearch_QWORD(QWORD from_value, QWORD to_value, PMAPS pMap) {	// FLOAT
	pid_t pid = getPID(bm);
	if (pid == 0) {
		puts("获取进程失败!");
		return NULL;
	}
	//*gs = 0;
	PMAPS pTemp = NULL;
	pTemp = pMap;
	PMAPS n, e;
	e = n = (PMAPS) malloc(LEN);
	PMAPS pBuff;
	pBuff = n;
	int iCount = 0;				// 链表长度
	int c;
	QWORD buff[1024] = { 0 };	// 缓冲区
	while (pTemp->next != NULL) {
		c = (pTemp->taddr - pTemp->addr) / 4096;
		for (int j = 0; j < c; j += 1) {
			pread64(handle, buff, 0x1000, pTemp->addr + (j * 4096));
			for (int i = 0; i < 1024; i += 1) {
				if (buff[i] >= from_value && buff[i] <= to_value) {	// 判断。。。
					iCount++;
					gs += 1;
					n->addr = (pTemp->addr) + (j * 4096) + (i * 4);
					n->type = TYPE_QWORD;
					if (iCount == 1) {
						n->next = NULL;
						e = n;
						pBuff = n;
					} else {
						n->next = NULL;
						e->next = n;
						e = n;
					}
					n = (PMAPS) malloc(LEN);
				}
				// printf("buff[%d]=%f\n",l,buff[l]);
				// usleep(1);
			}
			// memset(buff,0,4);
		}
		pTemp = pTemp->next;
	}
	free(n);
	return pBuff;
}

void MemorySearch(char *value, TYPE type) {
	PMAPS pHead = NULL;
	PMAPS pMap = NULL;
	gs = 0;
	switch (MemorySearchRange) {
		case ALL:
			pMap = readmaps(ALL);
			break;
		case B_BAD:
			pMap = readmaps(B_BAD);
			break;
		case C_ALLOC:
			pMap = readmaps(C_ALLOC);
			break;
		case C_BSS:
			pMap = readmaps(C_BSS);
			break;
		case C_DATA:
			pMap = readmaps(C_DATA);
			break;
		case C_HEAP:
			pMap = readmaps(C_HEAP);
			break;
		case JAVA_HEAP:
			pMap = readmaps(JAVA_HEAP);
			break;
		case A_ANONYMOUS:
			pMap = readmaps(A_ANONYMOUS);
			break;
		case CODE_SYSTEM:
			pMap = readmaps(CODE_SYSTEM);
			break;
		case STACK:
			pMap = readmaps(STACK);
			break;
		case ASHMEM:
			pMap = readmaps(ASHMEM);
			break;
		default:
			printf("\033[32;1mYou Select A NULL Type!\n");
			break;
	}
	if (pMap == NULL) {
		puts("map error");
		return (void)0;
	}
	switch (type) {
		case TYPE_DWORD:
			pHead = MemorySearch_DWORD(atoi(value), pMap);
			break;
		case TYPE_FLOAT:
			pHead = MemorySearch_FLOAT(atof(value), pMap);
			break;
		case TYPE_DOUBLE:
			pHead = MemorySearch_DOUBLE(atof(value), pMap);
			break;
		case TYPE_WORD:
			pHead = MemorySearch_WORD(atoi(value), pMap);
			break;
		case TYPE_BYTE:
			pHead = MemorySearch_BYTE(atoi(value), pMap);
			break;
		case TYPE_QWORD:
			pHead = MemorySearch_QWORD(atoi(value), pMap);
			break;
		default:
			printf("\033[32;1mYou Select A NULL Type!\n");
			break;
	}
	if (pHead == NULL) {
		puts("搜索内存失败!");
		return;
	}
	ResCount = gs;
	Res = pHead;				// Res指针指向链表
}

PMAPS MemorySearch_DWORD(DWORD value, PMAPS pMap) {
	pid_t pid = getPID(bm);
	if (pid == 0) {
		puts("获取进程失败!");
		return NULL;
	}
	//*gs = 0;
	PMAPS pTemp = NULL;
	pTemp = pMap;
	PMAPS n, e;
	e = n = (PMAPS) malloc(LEN);
	PMAPS pBuff;
	pBuff = n;
	int iCount = 0;				// 链表长度
	int c;
	DWORD buff[1024] = { 0 };		// 缓冲区
	//memset(buff, 0, 4);
	while (pTemp != NULL) {	// 读取maps里面的地址
		c = (pTemp->taddr - pTemp->addr) / 4096;
		for (int j = 0; j < c; j++) {
			pread64(handle, buff, 0x1000, pTemp->addr + j * 4096);
			for (int i = 0; i < 1024; i++) {
				if (buff[i] == value) {
					iCount++;
					gs += 1;
					n->addr = (pTemp->addr) + (j * 4096) + (i * 4);
					n->type = TYPE_DWORD;
					// printf("addr:%lx,val:%d,buff=%d\n",n->addr,value,buff[i]);
					if (iCount == 1) {
						n->next = NULL;
						e = n;
						pBuff = n;
					} else {
						n->next = NULL;
						e->next = n;
						e = n;
					}
					n = (PMAPS) malloc(LEN);
				}
			}
		}
		pTemp = pTemp->next;
	}
	free(n);
	return pBuff;
}

PMAPS MemorySearch_FLOAT(FLOAT value, PMAPS pMap) {
	pid_t pid = getPID(bm);
	if (pid == 0) {
		puts("获取进程失败!");
		return NULL;
	}
	//*gs = 0;
	PMAPS pTemp = NULL;
	pTemp = pMap;
	PMAPS n, e;
	e = n = (PMAPS) malloc(LEN);
	PMAPS pBuff;
	pBuff = n;
	int iCount = 0;				// 链表长度
	int c;
	FLOAT buff[1024] = { 0 };	// 缓冲区
	while (pTemp->next != NULL) {
		c = (pTemp->taddr - pTemp->addr) / 4096;
		for (int j = 0; j < c; j += 1) {
			pread64(handle, buff, 0x1000, pTemp->addr + (j * 4096));
			for (int i = 0; i < 1024; i += 1) {
				if (buff[i] == value) {
					iCount++;
					gs += 1;
					n->addr = (pTemp->addr) + (j * 4096) + (i * 4);
					n->type = TYPE_FLOAT;
					if (iCount == 1) {
						n->next = NULL;
						e = n;
						pBuff = n;
					} else {
						n->next = NULL;
						e->next = n;
						e = n;
					}
					n = (PMAPS) malloc(LEN);
				}
				// printf("buff[%d]=%f\n",l,buff[l]);
				// usleep(1);
			}
			// memset(buff,0,4);
		}
		pTemp = pTemp->next;
	}
	free(n);
	return pBuff;
}

PMAPS MemorySearch_DOUBLE(DOUBLE value, PMAPS pMap) {
	pid_t pid = getPID(bm);
	if (pid == 0) {
		puts("获取进程失败!");
		return NULL;
	}
	//*gs = 0;
	PMAPS pTemp = NULL;
	pTemp = pMap;
	PMAPS n, e;
	e = n = (PMAPS) malloc(LEN);
	PMAPS pBuff;
	pBuff = n;
	int iCount = 0;				// 链表长度
	int c;
	DOUBLE buff[1024] = { 0 };	// 缓冲区
	while (pTemp->next != NULL) {
		c = (pTemp->taddr - pTemp->addr) / 4096;
		for (int j = 0; j < c; j += 1) {
			pread64(handle, buff, 0x1000, pTemp->addr + (j * 4096));
			for (int i = 0; i < 1024; i += 1) {
				if (buff[i] == value) {
					iCount++;
					gs += 1;
					n->addr = (pTemp->addr) + (j * 4096) + (i * 4);
					n->type = TYPE_DOUBLE;
					if (iCount == 1) {
						n->next = NULL;
						e = n;
						pBuff = n;
					} else {
						n->next = NULL;
						e->next = n;
						e = n;
					}
					n = (PMAPS) malloc(LEN);
				}
				// printf("buff[%d]=%f\n",l,buff[l]);
				// usleep(1);
			}
			// memset(buff,0,4);
		}
		pTemp = pTemp->next;
	}
	free(n);
	return pBuff;
}

PMAPS MemorySearch_WORD(WORD value, PMAPS pMap) {
	pid_t pid = getPID(bm);
	if (pid == 0) {
		puts("获取进程失败!");
		return NULL;
	}
	//*gs = 0;
	PMAPS pTemp = NULL;
	pTemp = pMap;
	PMAPS n, e;
	e = n = (PMAPS) malloc(LEN);
	PMAPS pBuff;
	pBuff = n;
	int iCount = 0;				// 链表长度
	int c;
	WORD buff[1024] = { 0 };	// 缓冲区
	while (pTemp->next != NULL) {
		c = (pTemp->taddr - pTemp->addr) / 4096;
		for (int j = 0; j < c; j += 1) {
			pread64(handle, buff, 0x1000, pTemp->addr + (j * 4096));
			n->type = TYPE_WORD;
			for (int i = 0; i < 1024; i += 1) {
				if (buff[i] == value) {
					iCount++;
					gs += 1;
					n->addr = (pTemp->addr) + (j * 4096) + (i * 4);
					if (iCount == 1) {
						n->next = NULL;
						e = n;
						pBuff = n;
					} else {
						n->next = NULL;
						e->next = n;
						e = n;
					}
					n = (PMAPS) malloc(LEN);
				}
				// printf("buff[%d]=%f\n",l,buff[l]);
				// usleep(1);
			}
			// memset(buff,0,4);
		}
		pTemp = pTemp->next;
	}
	free(n);
	return pBuff;
}

PMAPS MemorySearch_BYTE(BYTE value, PMAPS pMap) {
	pid_t pid = getPID(bm);
	if (pid == 0) {
		puts("获取进程失败!");
		return NULL;
	}
	//*gs = 0;
	PMAPS pTemp = NULL;
	pTemp = pMap;
	PMAPS n, e;
	e = n = (PMAPS) malloc(LEN);
	PMAPS pBuff;
	pBuff = n;
	int iCount = 0;				// 链表长度
	int c;
	BYTE buff[1024] = { 0 };	// 缓冲区
	while (pTemp->next != NULL) {
		c = (pTemp->taddr - pTemp->addr) / 4096;
		for (int j = 0; j < c; j += 1) {
			pread64(handle, buff, 0x1000, pTemp->addr + (j * 4096));
			for (int i = 0; i < 1024; i += 1) {
				if (buff[i] == value) {
					iCount++;
					gs += 1;
					n->addr = (pTemp->addr) + (j * 4096) + (i * 4);
					n->type = TYPE_BYTE;
					if (iCount == 1) {
						n->next = NULL;
						e = n;
						pBuff = n;
					} else {
						n->next = NULL;
						e->next = n;
						e = n;
					}
					n = (PMAPS) malloc(LEN);
				}
				// printf("buff[%d]=%f\n",l,buff[l]);
				// usleep(1);
			}
			// memset(buff,0,4);
		}
		pTemp = pTemp->next;
	}
	free(n);
	return pBuff;
}

PMAPS MemorySearch_QWORD(QWORD value, PMAPS pMap) {
	pid_t pid = getPID(bm);
	if (pid == 0) {
		puts("获取进程失败!");
		return NULL;
	}
	//*gs = 0;
	PMAPS pTemp = NULL;
	pTemp = pMap;
	PMAPS n, e;
	e = n = (PMAPS) malloc(LEN);
	PMAPS pBuff;
	pBuff = n;
	int iCount = 0;				// 链表长度
	int c;
	QWORD buff[1024] = { 0 };	// 缓冲区
	while (pTemp->next != NULL) {
		c = (pTemp->taddr - pTemp->addr) / 4096;
		for (int j = 0; j < c; j += 1) {
			pread64(handle, buff, 0x1000, pTemp->addr + (j * 4096));
			for (int i = 0; i < 1024; i += 1) {
				if (buff[i] == value) {
					iCount++;
					gs += 1;
					n->addr = (pTemp->addr) + (j * 4096) + (i * 4);
					n->type = TYPE_QWORD;
					if (iCount == 1) {
						n->next = NULL;
						e = n;
						pBuff = n;
					} else {
						n->next = NULL;
						e->next = n;
						e = n;
					}
					n = (PMAPS) malloc(LEN);
				}
				// printf("buff[%d]=%f\n",l,buff[l]);
				// usleep(1);
			}
			// memset(buff,0,4);
		}
		pTemp = pTemp->next;
	}
	free(n);
	return pBuff;
}

void MemoryOffset(char *value, OFFSET offset, TYPE type) {
	PMAPS pHead = NULL;
	gs = 0;
	switch (type) {
		case TYPE_DWORD:
			pHead = MemoryOffset_DWORD(atoi(value), offset, Res);
			break;
		case TYPE_FLOAT:
			pHead = MemoryOffset_FLOAT(atof(value), offset, Res);
			break;
		case TYPE_DOUBLE:
			pHead = MemoryOffset_DOUBLE(atof(value), offset, Res);
			break;
		case TYPE_WORD:
			pHead = MemoryOffset_WORD(atoi(value), offset, Res);
			break;
		case TYPE_BYTE:
			pHead = MemoryOffset_BYTE(atoi(value), offset, Res);
			break;
		case TYPE_QWORD:
			pHead = MemoryOffset_QWORD(atoi(value), offset, Res);
			break;
		default:
			printf("\033[32;1mYou Select A NULL Type!\n");
			break;
	}
	if (pHead == NULL) {
		puts("偏移失败!");
		return (void)0;
	}
	ResCount = gs;				// 全局个数
	ClearResults();				// 清空存储的数据(释放空间)
	Res = pHead;				// 指向新搜索到的空间
}

PMAPS MemoryOffset_DWORD(DWORD value, OFFSET offset, PMAPS pBuff) {
	
	//*gs = 0;					// 初始个数为0
	PMAPS pEnd = NULL;
	PMAPS pNew = NULL;
	PMAPS pTemp = pBuff;
	PMAPS BUFF = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	BUFF = pNew;
	int iCount = 0;		// 个数与句柄
	long int all;				// 总和
	DWORD *buf = (DWORD *)malloc(sizeof(DWORD));	// 缓冲区
	int jg;
	while (pTemp != NULL) {
		all = pTemp->addr + offset;
		pread64(handle, buf, 4, all);
		jg = *buf;
		if (jg == value) {
			iCount++;
			gs += 1;
			pNew->addr = pTemp->addr;
			pNew->type = TYPE_DWORD;
			if (iCount == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				BUFF = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);
			if (ResCount == 1) {
				free(pNew);
				close(handle);
				return BUFF;
			}
		}
		/* else { printf("jg:%d,value:%d\n",jg,value); } */
		pTemp = pTemp->next;	// 指向下一个节点读取数据
	}
	free(pNew);
	return BUFF;
}

PMAPS MemoryOffset_FLOAT(FLOAT value, OFFSET offset, PMAPS pBuff) {
	
	//*gs = 0;					// 初始个数为0
	PMAPS pEnd = NULL;
	PMAPS pNew = NULL;
	PMAPS pTemp = pBuff;
	PMAPS BUFF = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	BUFF = pNew;
	int iCount = 0;		// 个数与句柄
	long int all;				// 总和
	FLOAT *buf = (FLOAT *)malloc(sizeof(FLOAT));	// 缓冲区
	// int buf[16]; //出现异常使用
	FLOAT jg;
	while (pTemp != NULL) {
		all = pTemp->addr + offset;	// 偏移后的地址
		//pread64(handle, buf, sizeof(FLOAT), all);
		pread64(handle, buf, 4, all);
		jg = *buf;
		if (jg == value) {
			iCount++;
			gs += 1;
			//printf("偏移成功,addr:%lx\n",all);
			pNew->addr = pTemp->addr;
			pNew->type = TYPE_FLOAT;
			if (iCount == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				BUFF = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);
			if (ResCount == 1) {
				free(pNew);
				close(handle);
				return BUFF;
			}
		}
		/* else { printf("jg:%e,value:%e\n",jg,value); } */
		pTemp = pTemp->next;	// 指向下一个节点读取数据
	}
	free(pNew);
	return BUFF;
}

PMAPS MemoryOffset_DOUBLE(DOUBLE value, OFFSET offset, PMAPS pBuff) {
	
	//*gs = 0;					// 初始个数为0
	PMAPS pEnd = NULL;
	PMAPS pNew = NULL;
	PMAPS pTemp = pBuff;
	PMAPS BUFF = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	BUFF = pNew;
	int iCount = 0;		// 个数与句柄
	long int all;				// 总和
	DOUBLE *buf = (DOUBLE *)malloc(sizeof(DOUBLE));	// 缓冲区
	// int buf[16]; //出现异常使用
	DOUBLE jg;
	while (pTemp != NULL) {
		all = pTemp->addr + offset;	// 偏移后的地址
		pread64(handle, buf, 4, all);
		jg = *buf;
		if (jg == value) {
			iCount++;
			gs += 1;
			// printf("偏移成功,addr:%lx\n",all);
			pNew->addr = pTemp->addr;
			pNew->type = TYPE_DOUBLE;
			if (iCount == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				BUFF = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);
			if (ResCount == 1) {
				free(pNew);
				close(handle);
				return BUFF;
			}
		}
		/* else { printf("jg:%e,value:%e\n",jg,value); } */
		pTemp = pTemp->next;	// 指向下一个节点读取数据
	}
	free(pNew);
	return BUFF;
}

PMAPS MemoryOffset_WORD(WORD value, OFFSET offset, PMAPS pBuff) {
	
	//*gs = 0;					// 初始个数为0
	PMAPS pEnd = NULL;
	PMAPS pNew = NULL;
	PMAPS pTemp = pBuff;
	PMAPS BUFF = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	BUFF = pNew;
	int iCount = 0;		// 个数与句柄
	long int all;				// 总和
	WORD *buf = (WORD *)malloc(sizeof(WORD));	// 缓冲区
	// int buf[16]; //出现异常使用
	WORD jg;
	while (pTemp != NULL) {
		all = pTemp->addr + offset;	// 偏移后的地址
		pread64(handle, buf, 4, all);
		jg = *buf;
		if (jg == value) {
			iCount++;
			gs += 1;
			// printf("偏移成功,addr:%lx\n",all);
			pNew->addr = pTemp->addr;
			pNew->type = TYPE_WORD;
			if (iCount == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				BUFF = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);
			if (ResCount == 1) {
				free(pNew);
				close(handle);
				return BUFF;
			}
		}
		/* else { printf("jg:%e,value:%e\n",jg,value); } */
		pTemp = pTemp->next;	// 指向下一个节点读取数据
	}
	free(pNew);
	return BUFF;
}

PMAPS MemoryOffset_BYTE(BYTE value, OFFSET offset, PMAPS pBuff) {
	
	//*gs = 0;					// 初始个数为0
	PMAPS pEnd = NULL;
	PMAPS pNew = NULL;
	PMAPS pTemp = pBuff;
	PMAPS BUFF = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	BUFF = pNew;
	int iCount = 0;		// 个数与句柄
	long int all;				// 总和
	BYTE *buf = (BYTE *)malloc(sizeof(BYTE));	// 缓冲区
	// int buf[16]; //出现异常使用
	BYTE jg;
	while (pTemp != NULL) {
		all = pTemp->addr + offset;	// 偏移后的地址
		pread64(handle, buf, 4, all);
		jg = *buf;
		if (jg == value) {
			iCount++;
			gs += 1;
			// printf("偏移成功,addr:%lx\n",all);
			pNew->addr = pTemp->addr;
			pNew->type = TYPE_BYTE;
			if (iCount == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				BUFF = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);
			if (ResCount == 1) {
				free(pNew);
				close(handle);
				return BUFF;
			}
		}
		/* else { printf("jg:%e,value:%e\n",jg,value); } */
		pTemp = pTemp->next;	// 指向下一个节点读取数据
	}
	free(pNew);
	return BUFF;
}

PMAPS MemoryOffset_QWORD(QWORD value, OFFSET offset, PMAPS pBuff) {
	
	//*gs = 0;					// 初始个数为0
	PMAPS pEnd = NULL;
	PMAPS pNew = NULL;
	PMAPS pTemp = pBuff;
	PMAPS BUFF = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	BUFF = pNew;
	int iCount = 0;		// 个数与句柄
	long int all;				// 总和
	QWORD *buf = (QWORD *)malloc(sizeof(QWORD));	// 缓冲区
	// int buf[16]; //出现异常使用
	QWORD jg;
	while (pTemp != NULL) {
		all = pTemp->addr + offset;	// 偏移后的地址
		pread64(handle, buf, 4, all);
		jg = *buf;
		if (jg == value) {
			iCount++;
			gs += 1;
			// printf("偏移成功,addr:%lx\n",all);
			pNew->addr = pTemp->addr;
			pNew->type = TYPE_QWORD;
			if (iCount == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				BUFF = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);
			if (ResCount == 1) {
				free(pNew);
				close(handle);
				return BUFF;
			}
		}
		/* else { printf("jg:%e,value:%e\n",jg,value); } */
		pTemp = pTemp->next;	// 指向下一个节点读取数据
	}
	free(pNew);
	return BUFF;
}


void RangeMemoryOffset(char *from_value, char *to_value, OFFSET offset, TYPE type) {
	PMAPS pHead = NULL;
	gs = 0;
	switch (type) {
		case TYPE_DWORD:
			if (atoi(from_value) > atoi(to_value))
				pHead = RangeMemoryOffset_DWORD(atoi(to_value), atoi(from_value), offset, Res);
			else
				pHead = RangeMemoryOffset_DWORD(atoi(from_value), atoi(to_value), offset, Res);
			break;
		case TYPE_FLOAT:
			if (atof(from_value) > atof(to_value))
				pHead = RangeMemoryOffset_FLOAT(atof(to_value), atof(from_value), offset, Res);
			else
				pHead = RangeMemoryOffset_FLOAT(atof(from_value), atof(to_value), offset, Res);
			break;
		case TYPE_DOUBLE:
			if (atof(from_value) > atof(to_value))
				pHead = RangeMemoryOffset_DOUBLE(atof(to_value), atof(from_value), offset, Res);
			else
				pHead = RangeMemoryOffset_DOUBLE(atof(from_value), atof(to_value), offset, Res);
			break;
		case TYPE_WORD:
			if (atoi(from_value) > atoi(to_value))
				pHead = RangeMemoryOffset_WORD(atoi(to_value), atoi(from_value), offset, Res);
			else
				pHead = RangeMemoryOffset_WORD(atoi(from_value), atoi(to_value), offset, Res);
			break;
		case TYPE_BYTE:
			if (atoi(from_value) > atoi(to_value))
				pHead = RangeMemoryOffset_BYTE(atoi(to_value), atoi(from_value), offset, Res);
			else
				pHead = RangeMemoryOffset_BYTE(atoi(from_value), atoi(to_value), offset, Res);
			break;
		case TYPE_QWORD:
			if (atoi(from_value) > atoi(to_value))
				pHead = RangeMemoryOffset_QWORD(atoi(to_value), atoi(from_value), offset, Res);
			else
				pHead = RangeMemoryOffset_QWORD(atoi(from_value), atoi(to_value), offset, Res);
			break;
		default:
			printf("\033[32;1mYou Select A NULL Type!\n");
			break;
	}
	if (pHead == NULL) {
		puts("范围偏移失败!");
		return (void)0;
	}
	ResCount = gs;				// 全局个数
	ClearResults();				// 清空存储的数据(释放空间)
	Res = pHead;				// 指向新搜索到的空间
}

PMAPS RangeMemoryOffset_DWORD(DWORD from_value, DWORD to_value, OFFSET offset, PMAPS pBuff) {
	
	//*gs = 0;					// 初始个数为0
	PMAPS pEnd = NULL;
	PMAPS pNew = NULL;
	PMAPS pTemp = pBuff;
	PMAPS BUFF = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	BUFF = pNew;
	int iCount = 0;		// 个数与句柄
	long int all;				// 总和
	DWORD *buf = (DWORD *)malloc(sizeof(DWORD));	// 缓冲区
	DWORD jg;
	while (pTemp != NULL) {
		all = pTemp->addr + offset;
		pread64(handle, buf, 4, all);
		jg = *buf;
		if (jg >= from_value && jg <= to_value) {
			iCount++;
			gs += 1;
			pNew->addr = pTemp->addr;
			pNew->type = TYPE_DWORD;
			if (iCount == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				BUFF = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);
			if (ResCount == 1) {
				free(pNew);
				close(handle);
				return BUFF;
			}
		}
		/* else { printf("jg:%d,value:%d\n",jg,value); } */
		pTemp = pTemp->next;	// 指向下一个节点读取数据
	}
	free(pNew);
	return BUFF;
}

PMAPS RangeMemoryOffset_FLOAT(FLOAT from_value, FLOAT to_value, OFFSET offset, PMAPS pBuff) {
	
	//*gs = 0;					// 初始个数为0
	PMAPS pEnd = NULL;
	PMAPS pNew = NULL;
	PMAPS pTemp = pBuff;
	PMAPS BUFF = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	BUFF = pNew;
	int iCount = 0;		// 个数与句柄
	long int all;				// 总和
	FLOAT *buf = (FLOAT *)malloc(sizeof(FLOAT));	// 缓冲区
	// int buf[16]; //出现异常使用
	FLOAT jg;
	while (pTemp != NULL) {
		all = pTemp->addr + offset;	// 偏移后的地址
		pread64(handle, buf, 4, all);
		jg = *buf;
		if (jg >= from_value && jg <= to_value) {
			iCount++;
			gs += 1;
			// printf("偏移成功,addr:%lx\n",all);
			pNew->addr = pTemp->addr;
			pNew->type = TYPE_FLOAT;
			if (iCount == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				BUFF = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);
			if (ResCount == 1) {
				free(pNew);
				close(handle);
				return BUFF;
			}
		}
		/* else { printf("jg:%e,value:%e\n",jg,value); } */
		pTemp = pTemp->next;	// 指向下一个节点读取数据
	}
	free(pNew);
	return BUFF;
}

PMAPS RangeMemoryOffset_DOUBLE(DOUBLE from_value, DOUBLE to_value, OFFSET offset, PMAPS pBuff) {
	
	//*gs = 0;					// 初始个数为0
	PMAPS pEnd = NULL;
	PMAPS pNew = NULL;
	PMAPS pTemp = pBuff;
	PMAPS BUFF = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	BUFF = pNew;
	int iCount = 0;		// 个数与句柄
	long int all;				// 总和
	DOUBLE *buf = (DOUBLE *)malloc(sizeof(DOUBLE));	// 缓冲区
	// int buf[16]; //出现异常使用
	DOUBLE jg;
	while (pTemp != NULL) {
		all = pTemp->addr + offset;	// 偏移后的地址
		pread64(handle, buf, 4, all);
		jg = *buf;
		if (jg >= from_value && jg <= to_value) {
			iCount++;
			gs += 1;
			// printf("偏移成功,addr:%lx\n",all);
			pNew->addr = pTemp->addr;
			pNew->type = TYPE_DOUBLE;
			if (iCount == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				BUFF = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);
			if (ResCount == 1) {
				free(pNew);
				close(handle);
				return BUFF;
			}
		}
		/* else { printf("jg:%e,value:%e\n",jg,value); } */
		pTemp = pTemp->next;	// 指向下一个节点读取数据
	}
	free(pNew);
	return BUFF;
}

PMAPS RangeMemoryOffset_WORD(WORD from_value, WORD to_value, OFFSET offset, PMAPS pBuff) {
	
	//*gs = 0;					// 初始个数为0
	PMAPS pEnd = NULL;
	PMAPS pNew = NULL;
	PMAPS pTemp = pBuff;
	PMAPS BUFF = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	BUFF = pNew;
	int iCount = 0;		// 个数与句柄
	long int all;				// 总和
	WORD *buf = (WORD *)malloc(sizeof(WORD));	// 缓冲区
	// int buf[16]; //出现异常使用
	WORD jg;
	while (pTemp != NULL) {
		all = pTemp->addr + offset;	// 偏移后的地址
		pread64(handle, buf, 4, all);
		jg = *buf;
		if (jg >= from_value && jg <= to_value) {
			iCount++;
			gs += 1;
			// printf("偏移成功,addr:%lx\n",all);
			pNew->addr = pTemp->addr;
			pNew->type = TYPE_WORD;
			if (iCount == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				BUFF = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);
			if (ResCount == 1) {
				free(pNew);
				close(handle);
				return BUFF;
			}
		}
		/* else { printf("jg:%e,value:%e\n",jg,value); } */
		pTemp = pTemp->next;	// 指向下一个节点读取数据
	}
	free(pNew);
	return BUFF;
}

PMAPS RangeMemoryOffset_BYTE(BYTE from_value, BYTE to_value, OFFSET offset, PMAPS pBuff) {
	
	//*gs = 0;					// 初始个数为0
	PMAPS pEnd = NULL;
	PMAPS pNew = NULL;
	PMAPS pTemp = pBuff;
	PMAPS BUFF = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	BUFF = pNew;
	int iCount = 0;		// 个数与句柄
	long int all;				// 总和
	BYTE *buf = (BYTE *)malloc(sizeof(BYTE));	// 缓冲区
	// int buf[16]; //出现异常使用
	BYTE jg;
	while (pTemp != NULL) {
		all = pTemp->addr + offset;	// 偏移后的地址
		pread64(handle, buf, 4, all);
		jg = *buf;
		if (jg >= from_value && jg <= to_value) {
			iCount++;
			gs += 1;
			// printf("偏移成功,addr:%lx\n",all);
			pNew->addr = pTemp->addr;
			pNew->type = TYPE_BYTE;
			if (iCount == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				BUFF = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);
			if (ResCount == 1) {
				free(pNew);
				close(handle);
				return BUFF;
			}
		}
		/* else { printf("jg:%e,value:%e\n",jg,value); } */
		pTemp = pTemp->next;	// 指向下一个节点读取数据
	}
	free(pNew);
	return BUFF;
}

PMAPS RangeMemoryOffset_QWORD(QWORD from_value, QWORD to_value, OFFSET offset, PMAPS pBuff) {
	
	//*gs = 0;					// 初始个数为0
	PMAPS pEnd = NULL;
	PMAPS pNew = NULL;
	PMAPS pTemp = pBuff;
	PMAPS BUFF = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	BUFF = pNew;
	int iCount = 0;		// 个数与句柄
	long int all;				// 总和
	QWORD *buf = (QWORD *)malloc(sizeof(QWORD));	// 缓冲区
	// int buf[16]; //出现异常使用
	QWORD jg;
	while (pTemp != NULL) {
		all = pTemp->addr + offset;	// 偏移后的地址
		pread64(handle, buf, 4, all);
		jg = *buf;
		if (jg >= from_value && jg <= to_value) {
			iCount++;
			gs += 1;
			// printf("偏移成功,addr:%lx\n",all);
			pNew->addr = pTemp->addr;
			pNew->type = TYPE_QWORD;
			if (iCount == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				BUFF = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);
			if (ResCount == 1) {
				free(pNew);
				close(handle);
				return BUFF;
			}
		}
		/* else { printf("jg:%e,value:%e\n",jg,value); } */
		pTemp = pTemp->next;	// 指向下一个节点读取数据
	}
	free(pNew);
	return BUFF;
}

void MemoryWrite(char *value, OFFSET offset, TYPE type) {
	switch (type) {
		case TYPE_DWORD:
			MemoryWrite_DWORD(atoi(value), Res, offset);
			break;
		case TYPE_FLOAT:
			MemoryWrite_FLOAT(atof(value), Res, offset);
			break;
		case TYPE_DOUBLE:
			MemoryWrite_DOUBLE(atof(value), Res, offset);
			break;
		case TYPE_WORD:
			MemoryWrite_WORD(atoi(value), Res, offset);
			break;
		case TYPE_BYTE:
			MemoryWrite_BYTE(atoi(value), Res, offset);
			break;
		case TYPE_QWORD:
			MemoryWrite_QWORD(atoi(value), Res, offset);
			break;
		default:
			printf("\033[32;1mYou Select A NULL Type!\n");
			break;
	}
	// ClearResults();//清空list
}

int MemoryWrite_DWORD(DWORD value, PMAPS pBuff, OFFSET offset) {
	
	PMAPS pTemp = NULL;
	pTemp = pBuff;
	int i;
	for (i = 0; i < ResCount; i++) {
		pwrite64(handle, &value, 4, pTemp->addr + offset);
		if (pTemp->next != NULL)
			pTemp = pTemp->next;
	}
	return 0;
}

int MemoryWrite_FLOAT(FLOAT value, PMAPS pBuff, OFFSET offset) {
	
	PMAPS pTemp = NULL;
	pTemp = pBuff;
	int i;
	for (i = 0; i < ResCount; i++) {
		pwrite64(handle, &value, 4, pTemp->addr + offset);
		if (pTemp->next != NULL)
			pTemp = pTemp->next;
	}
	return 0;
}

int MemoryWrite_DOUBLE(DOUBLE value, PMAPS pBuff, OFFSET offset) {
	
	PMAPS pTemp = NULL;
	int handle;
	pTemp = pBuff;
	int i;
	for (i = 0; i < ResCount; i++) {
		pwrite64(handle, &value, 4, pTemp->addr + offset);
		if (pTemp->next != NULL)
			pTemp = pTemp->next;
	}
	return 0;
}

int MemoryWrite_WORD(WORD value, PMAPS pBuff, OFFSET offset) {
	
	PMAPS pTemp = NULL;
	pTemp = pBuff;
	int i;
	for (i = 0; i < ResCount; i++) {
		pwrite64(handle, &value, 4, pTemp->addr + offset);
		if (pTemp->next != NULL)
			pTemp = pTemp->next;
	}
	return 0;
}

int MemoryWrite_BYTE(BYTE value, PMAPS pBuff, OFFSET offset) {
	
	PMAPS pTemp = NULL;
	pTemp = pBuff;
	int i;
	for (i = 0; i < ResCount; i++) {
		pwrite64(handle, &value, 4, pTemp->addr + offset);
		if (pTemp->next != NULL)
			pTemp = pTemp->next;
	}
	return 0;
}

int MemoryWrite_QWORD(QWORD value, PMAPS pBuff, OFFSET offset) {
	
	PMAPS pTemp = NULL;
	pTemp = pBuff;
	int i;
	for (i = 0; i < ResCount; i++) {
		pwrite64(handle, &value, 4, pTemp->addr + offset);
		if (pTemp->next != NULL)
			pTemp = pTemp->next;
	}
	return 0;
}

int WriteAddress(ADDRESS addr, char *value, TYPE type) {
	switch (type) {
		case TYPE_DWORD:
			WriteAddress_DWORD(addr, atoi(value));
			break;
		case TYPE_FLOAT:
			WriteAddress_FLOAT(addr, atof(value));
			break;
		case TYPE_DOUBLE:
			WriteAddress_DOUBLE(addr, atof(value));
			break;
		case TYPE_WORD:
			WriteAddress_WORD(addr, atoi(value));
			break;
		case TYPE_BYTE:
			WriteAddress_BYTE(addr, atoi(value));
			break;
		case TYPE_QWORD:
			WriteAddress_QWORD(addr, atoi(value));
			break;
		default:
			printf("\033[32;1mYou Select A NULL Type!\n");
			break;
	}
	return 0;
}

int WriteAddress_DWORD(ADDRESS addr, DWORD value) {
	
	pwrite64(handle, &value, 4, addr);
	return 0;
}

int WriteAddress_FLOAT(ADDRESS addr, FLOAT value) {
	
	pwrite64(handle, &value, 4, addr);
	return 0;
}

int WriteAddress_DOUBLE(ADDRESS addr, DOUBLE value) {
	
	pwrite64(handle, &value, 4, addr);
	return 0;
}

int WriteAddress_WORD(ADDRESS addr, WORD value) {
	
	pwrite64(handle, &value, 4, addr);
	return 0;
}

int WriteAddress_BYTE(ADDRESS addr, BYTE value) {
	
	pwrite64(handle, &value, 4, addr);
	return 0;
}

int WriteAddress_QWORD(ADDRESS addr, QWORD value) {
	pwrite64(handle, &value, 4, addr);
	return 0;
}

int isapkinstalled(PACKAGENAME * bm) {
	char LJ[128];
	sprintf(LJ, "/data/data/%s/", bm);
	DIR *dir;
	dir = opendir(LJ);
	if (dir == NULL) {
		return 0;
	} else {
		return 1;
	}
}

int isapkrunning(PACKAGENAME * bm) {
	DIR *dir = NULL;
	struct dirent *ptr = NULL;
	FILE *fp = NULL;
	char filepath[50];			// 大小随意，能装下cmdline文件的路径即可
	char filetext[128];			// 大小随意，能装下要识别的命令行文本即可
	dir = opendir("/proc/");	// 打开路径
	if (dir != NULL) {
		while ((ptr = readdir(dir)) != NULL) {
			// 如果读取到的是"."或者".."则跳过，读取到的不是文件夹名字也跳过
			if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0))
				continue;
			if (ptr->d_type != DT_DIR)
				continue;
			sprintf(filepath, "/proc/%s/cmdline", ptr->d_name);	// 生成要读取的文件的路径
			fp = fopen(filepath, "r");	// 打开文件
			if (NULL != fp) {
				fgets(filetext, sizeof(filetext), fp);	// 读取文件
				if (strcmp(filetext, bm) == 0) {
					closedir(dir);
					return 1;
				}
				fclose(fp);
			}
		}
	}
	closedir(dir);				// 关闭路径
	return 0;
}

int uninstallapk(PACKAGENAME * bm) {
	char ml[128];
	sprintf(ml, "pm uninstall %s", bm);
	system(ml);
	system("clear");
	return 0;
}

int installapk(char *lj) {
	char ml[128];
	sprintf(ml, "pm install %s", lj);
	system(ml);
	system("clear");
	return 0;
}

int killprocess(PACKAGENAME * bm) {
	int pid = getPID(bm);
	if (pid == 0) {
		return -1;
	}
	char ml[32];
	sprintf(ml, "kill %d", pid);
	system(ml);					// 杀掉进程
	return 0;
}

char GetProcessState(PACKAGENAME * bm) {
	/*
	   D 无法中断的休眠状态（通常 IO 的进程）； R
	   正在运行，在可中断队列中； S
	   处于休眠状态，静止状态； T
	   停止或被追踪，暂停执行； W
	   进入内存交换（从内核2.6开始无效）； X
	   死掉的进程； Z 僵尸进程不存在但暂时无法消除； W:
	   没有足够的记忆体分页可分配WCHAN
	   正在等待的进程资源； <: 高优先级进程 N:
	   低优先序进程 L: 有记忆体分页分配并锁在记忆体内
	   (即时系统或捱A I/O)，即,有些页被锁进内存 s
	   进程的领导者（在它之下有子进程）； l
	   多进程的（使用 CLONE_THREAD, 类似 NPTL pthreads）； +
	   位于后台的进程组； */
	int pid = getPID(bm);		// 获取pid
	if (pid == 0) {
		return 0;				// 无进程
	}
	FILE *fp;
	char lj[64];
	char buff[64];
	char zt;
	char zt1[16];
	sprintf(lj, "/proc/%d/status", pid);
	fp = fopen(lj, "r");		// 打开status文件
	if (fp == NULL) {
		return 0;				// 无进程
	}
	// puts("loop");
	while (!feof(fp)) {
		fgets(buff, sizeof(buff), fp);	// 读取
		if (strstr(buff, "State")) {
			sscanf(buff, "State: %c", &zt);	// 选取
			// printf("state:%c\n",zt);
			// sleep(1);
			// puts("emmmm");
			break;				// 跳出循环
		}
	}
	// putchar(zt);
	// puts(zt2);
	fclose(fp);
	// puts("loopopp");
	return zt;					// 返回状态
}

int rebootsystem() {
	return system("su -c 'reboot'");
}

int PutDate() {
	return system("date +%F-%T");
}

int GetDate(char *date) {
	FILE *fp;					// 文件指针
	system("date +%F-%T > log.txt");	// 执行
	if ((fp = fopen("log.txt", "r")) == NULL) {
		return 0;
	}
	fscanf(fp, "%s", date);		// 读取
	remove("log.txt");			// 删除
	return 1;
}

int killGG() {
	// 在/data/data/[GG修改器包名]/files/里面有一个文件夹名字是GG-****
	// 如果有这个文件夹，就获取上面所说的包名，杀掉GG修改器
	DIR *dir = NULL;
	DIR *dirGG = NULL;
	struct dirent *ptr = NULL;
	struct dirent *ptrGG = NULL;
	char filepath[256];			// 大小随意，能装下cmdline文件的路径即可
	char filetext[128];			// 大小随意，能装下要识别的命令行文本即可
	dir = opendir("/data/data");	// 打开路径
	// puts("正在杀GG");
	int flag = 1;
	if (dir != NULL) {
		while (flag && (ptr = readdir(dir)) != NULL) {
			// 如果读取到的是"."或者".."则跳过，读取到的不是文件夹名字也跳过
			if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0))
				continue;
			if (ptr->d_type != DT_DIR)
				continue;
			sprintf(filepath, "/data/data/%s/files", ptr->d_name);	// 生成要读取的文件的路径
			dirGG = opendir(filepath);	// 打开文件
			if (dirGG != NULL) {
				while ((ptrGG = readdir(dirGG)) != NULL) {
					if ((strcmp(ptrGG->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0))
						continue;
					if (ptrGG->d_type != DT_DIR)
						continue;
					if (strstr(ptrGG->d_name, "GG")) {
						int pid;	// pid
						pid = getPID(ptr->d_name);	// 获取GG包名
						// ptr->d_name存储文件名字(也就是软件包名)
						if (pid == 0)	// 如果pid是0，代表GG没有运行
							continue;
						else	// 如果成功获取pid
							killprocess(ptr->d_name);
					}
				}
			}
			/* else { puts(filepath);//调试 } */
		}
	}
	closedir(dir);				// 关闭
	closedir(dirGG);
	return 0;
}

int killXs() {
	DIR *dir = NULL;
	struct dirent *ptr = NULL;
	char filepath[256];
	char filetext[128];			// 大小随意，能装下要识别的命令行文本即可
	dir = opendir("/data/data");	// 打开路径
	FILE *fp = NULL;
	if (NULL != dir) {
		while ((ptr = readdir(dir)) != NULL) {
			// 如果读取到的是"."或者".."则跳过，读取到的不是文件夹名字也跳过
			if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0))
				continue;
			if (ptr->d_type != DT_DIR)
				continue;
			// /data/data/%s/lib/libxscript.so
			sprintf(filepath, "/data/data/%s/lib/libxscript.so", ptr->d_name);	// 生成要读取的文件的路径
			fp = fopen(filepath, "r");
			if (fp == NULL)
				continue;
			else {
				killprocess(ptr->d_name);
				// 杀进程
			}
			// killprocess(ptr->d_name);
		}
	}
	closedir(dir);				// 关闭
	return 0;
}

void FreezeThread() {
	int pid;
	pid = getPID(bm);
	if (pid == 0) {
		puts("Error -1");
		return;
	}
	DWORD buf_i;
	FLOAT buf_f;
	DOUBLE buf_d;
	WORD buf_w;
	BYTE buf_b;
	QWORD buf_q;
	if (handle == -1) {
		puts("Error -2");
		return;
	}
	//puts("冻结数据中...");
	PFREEZE pTemp = Pfreeze;
	while (Freeze == 1) {
		pid = getPID(bm);
		if (pid == 0) {
			puts("Game is Not Run");
			exit(1);
			break;
		}
		for (int i = 0; i < FreezeCount; i++) {
			switch (pTemp->type) {
				case TYPE_DWORD:
					buf_i = atoi(pTemp->value);
					pwrite64(handle, &buf_i, 4, pTemp->addr);
					break;
				case TYPE_FLOAT:
					buf_f = atof(pTemp->value);
					pwrite64(handle, &buf_f, 4, pTemp->addr);
					break;
				case TYPE_DOUBLE:
					buf_d = atof(pTemp->value);
					pwrite64(handle, &buf_d, 4, pTemp->addr);
					break;
				case TYPE_WORD:
					buf_w = atoi(pTemp->value);
					pwrite64(handle, &buf_w, 4, pTemp->addr);
					break;
				case TYPE_BYTE:
					buf_b = atoi(pTemp->value);
					pwrite64(handle, &buf_b, 4, pTemp->addr);
					break;
				case TYPE_QWORD:
					buf_q = atoi(pTemp->value);
					pwrite64(handle, &buf_q, 4, pTemp->addr);
					break;
				default:
					break;
			}
			pTemp = pTemp->next;
			usleep(delay);
		}
		pTemp = Pfreeze;		// 重新指向头指针
	}
	//puts("冻结结束");
	return;
}

PMAPS GetResults() {
	if (Res == NULL) {
		return NULL;
	} else {
		return Res;				// 返回头指针
	}
}

int AddFreezeItem_All(char *Value, TYPE type, OFFSET offset) {
	if (ResCount == 0) {
		return -1;
	}
	PMAPS pTemp = Res;
	for (int i = 0; i < ResCount; i++) {
		switch (type) {
			case TYPE_DWORD:
				AddFreezeItem(pTemp->addr, Value, TYPE_DWORD, offset);
				break;
			case TYPE_FLOAT:
				AddFreezeItem(pTemp->addr, Value, TYPE_FLOAT, offset);
				break;
			case TYPE_DOUBLE:
				AddFreezeItem(pTemp->addr, Value, TYPE_DOUBLE, offset);
				break;
			case TYPE_WORD:
				AddFreezeItem(pTemp->addr, Value, TYPE_WORD, offset);
				break;
			case TYPE_BYTE:
				AddFreezeItem(pTemp->addr, Value, TYPE_BYTE, offset);
				break;
			case TYPE_QWORD:
				AddFreezeItem(pTemp->addr, Value, TYPE_QWORD, offset);
				break;
			default:
				SetTextColor(COLOR_SKY_BLUE);
				puts("You Choose a NULL type");
				break;
		}
		pTemp = pTemp->next;
	}
	return 0;
}

int AddFreezeItem(ADDRESS addr, char *value, TYPE type, OFFSET offset) {
	switch (type) {
		case TYPE_DWORD:
			AddFreezeItem_DWORD(addr + offset, value);
			break;
		case TYPE_FLOAT:
			AddFreezeItem_FLOAT(addr + offset, value);
			break;
		case TYPE_DOUBLE:
			AddFreezeItem_DOUBLE(addr + offset, value);
			break;
		case TYPE_WORD:
			AddFreezeItem_WORD(addr + offset, value);
			break;
		case TYPE_BYTE:
			AddFreezeItem_BYTE(addr + offset, value);
			break;
		case TYPE_QWORD:
			AddFreezeItem_QWORD(addr + offset, value);
			break;
		default:
			SetTextColor(COLOR_SKY_BLUE);
			puts("You Choose a NULL type");
			break;
	}
	return 0;
}

int AddFreezeItem_DWORD(ADDRESS addr, char *value) {
	if (FreezeCount == 0) {
		Pfreeze = pEnd = pNew = (PFREEZE) malloc(FRE);	// 分配新空间
		pNew->next = NULL;
		pEnd = pNew;
		Pfreeze = pNew;
		pNew->addr = addr;		// 储存地址
		pNew->type = TYPE_DWORD;
		pNew->value = value;
		FreezeCount += 1;
	} else {
		pNew = (PFREEZE) malloc(FRE);	// 分配空间
		pNew->next = NULL;
		pEnd->next = pNew;
		pEnd = pNew;
		pNew->addr = addr;
		pNew->type = TYPE_DWORD;
		pNew->value = value;
		FreezeCount += 1;
	}
	return 0;
}

int AddFreezeItem_FLOAT(ADDRESS addr, char *value) {
	if (FreezeCount == 0) {
		Pfreeze = pEnd = pNew = (PFREEZE) malloc(FRE);	// 分配新空间
		pNew->next = NULL;
		pEnd = pNew;
		Pfreeze = pNew;
		pNew->addr = addr;		// 储存地址
		pNew->type = TYPE_FLOAT;
		pNew->value = value;
		FreezeCount += 1;
	} else {
		pNew = (PFREEZE) malloc(FRE);	// 分配空间
		pNew->next = NULL;
		pEnd->next = pNew;
		pEnd = pNew;
		pNew->addr = addr;
		pNew->type = TYPE_FLOAT;
		pNew->value = value;
		FreezeCount += 1;
	}
	return 0;
}

int AddFreezeItem_DOUBLE(ADDRESS addr, char *value) {
	if (FreezeCount == 0) {
		Pfreeze = pEnd = pNew = (PFREEZE) malloc(FRE);	// 分配新空间
		pNew->next = NULL;
		pEnd = pNew;
		Pfreeze = pNew;
		pNew->addr = addr;		// 储存地址
		pNew->type = TYPE_DOUBLE;
		pNew->value = value;
		FreezeCount += 1;
	} else {
		pNew = (PFREEZE) malloc(FRE);	// 分配空间
		pNew->next = NULL;
		pEnd->next = pNew;
		pEnd = pNew;
		pNew->addr = addr;
		pNew->type = TYPE_DOUBLE;
		pNew->value = value;
		FreezeCount += 1;
	}
	return 0;
}

int AddFreezeItem_WORD(ADDRESS addr, char *value) {
	if (FreezeCount == 0) {
		Pfreeze = pEnd = pNew = (PFREEZE) malloc(FRE);	// 分配新空间
		pNew->next = NULL;
		pEnd = pNew;
		Pfreeze = pNew;
		pNew->addr = addr;		// 储存地址
		pNew->type = TYPE_WORD;
		pNew->value = value;
		FreezeCount += 1;
	} else {
		pNew = (PFREEZE) malloc(FRE);	// 分配空间
		pNew->next = NULL;
		pEnd->next = pNew;
		pEnd = pNew;
		pNew->addr = addr;
		pNew->type = TYPE_WORD;
		pNew->value = value;
		FreezeCount += 1;
	}
	return 0;
}

int AddFreezeItem_BYTE(ADDRESS addr, char *value) {
	if (FreezeCount == 0) {
		Pfreeze = pEnd = pNew = (PFREEZE) malloc(FRE);	// 分配新空间
		pNew->next = NULL;
		pEnd = pNew;
		Pfreeze = pNew;
		pNew->addr = addr;		// 储存地址
		pNew->type = TYPE_BYTE;
		pNew->value = value;
		FreezeCount += 1;
	} else {
		pNew = (PFREEZE) malloc(FRE);	// 分配空间
		pNew->next = NULL;
		pEnd->next = pNew;
		pEnd = pNew;
		pNew->addr = addr;
		pNew->type = TYPE_BYTE;
		pNew->value = value;
		FreezeCount += 1;
	}
	return 0;
}

int AddFreezeItem_QWORD(ADDRESS addr, char *value) {
	if (FreezeCount == 0) {
		Pfreeze = pEnd = pNew = (PFREEZE) malloc(FRE);	// 分配新空间
		pNew->next = NULL;
		pEnd = pNew;
		Pfreeze = pNew;
		pNew->addr = addr;		// 储存地址
		pNew->type = TYPE_QWORD;
		pNew->value = value;
		FreezeCount += 1;
	} else {
		pNew = (PFREEZE) malloc(FRE);	// 分配空间
		pNew->next = NULL;
		pEnd->next = pNew;
		pEnd = pNew;
		pNew->addr = addr;
		pNew->type = TYPE_QWORD;
		pNew->value = value;
		FreezeCount += 1;
	}
	return 0;
}

int RemoveFreezeItem(ADDRESS addr) {
	PFREEZE pTemp = Pfreeze;
	PFREEZE p1 = NULL;
	PFREEZE p2 = NULL;
	for (int i = 0; i < FreezeCount; i++) {
		p1 = pTemp;
		p2 = pTemp->next;
		if (pTemp->addr == addr) {
			p1->next = p2;
			free(pTemp);
			FreezeCount -= 1;
			// printf("冻结个数:%d\n",FreezeCount);
			// break;//防止地址重复冻结，所以不加，当然也可以加上
		}
		pTemp = p2;
	}
	return 0;
}

int RemoveFreezeItem_All() {
	PFREEZE pHead = Pfreeze;
	PFREEZE pTemp = pHead;
	int i;
	for (i = 0; i < FreezeCount; i++) {
		pTemp = pHead;
		pHead = pHead->next;
		free(pTemp);
		FreezeCount -= 1;
	}
	free(Pfreeze);
	FreezeCount -= 1;
	return 0;
}

int StartFreeze() {
	if (Freeze == 1) {
		return -1;
	}
	//int a;
	//strcpy(Fbm, bm);			// ******
	Freeze = 1;
	//pthread_create(&pth, NULL, FreezeThread, NULL);
	// 创建线程(开始冻结线程)
	thread th1(FreezeThread);
	//th1.join();
	return 0;
}

int StopFreeze() {
	Freeze = 0;
	return 0;
}

int SetFreezeDelay(long int De) {
	delay = De;
	return 0;
}

int PrintFreezeItems() {
	PFREEZE pTemp = Pfreeze;
	for (int i = 0; i < FreezeCount; i++) {
		const char *type = (char *)malloc(sizeof(char));
		switch (pTemp->type) {
			case TYPE_DWORD:
				type = "DWORD";
				break;
			case TYPE_FLOAT:
				type = "FLOAT";
				break;
			case TYPE_DOUBLE:
				type = "DOUBLE";
				break;
			case TYPE_WORD:
				type = "WORD";
				break;
			case TYPE_BYTE:
				type = "BYTE";
				break;
			case TYPE_QWORD:
				type = "QWORD";
				break;
			default:
				type = "";
				break;
		}
		printf("FreezeAddr:0x%lX  Type:%s  Value:%s\n",pTemp->addr,type,pTemp->value);
		pTemp = pTemp->next;
	}
	return 0;
}


int SetSearchRange(TYPE type) {
	switch (type) {
		case ALL:
			MemorySearchRange = 0;
			break;
		case B_BAD:
			MemorySearchRange = 1;
			break;
		case C_ALLOC:
			MemorySearchRange = 2;
			break;
		case C_BSS:
			MemorySearchRange = 3;
			break;
		case C_DATA:
			MemorySearchRange = 4;
			break;
		case C_HEAP:
			MemorySearchRange = 5;
			break;
		case JAVA_HEAP:
			MemorySearchRange = 6;
			break;
		case A_ANONYMOUS:
			MemorySearchRange = 7;
			break;
		case CODE_SYSTEM:
			MemorySearchRange = 8;
			break;
		case STACK:
			MemorySearchRange = 9;
			break;
		case ASHMEM:
			MemorySearchRange = 10;
			break;
		default:
			printf("\033[32;1mYou Select A NULL Type!\n");
			break;
	}
	return 0;
}

PMAPS readmaps(TYPE type) {
	PMAPS pMap = NULL;
	switch (type) {
		case ALL:
			pMap = readmaps_all();
			break;
		case B_BAD:
			pMap = readmaps_bad();
			break;
		case C_ALLOC:
			pMap = readmaps_c_alloc();
			break;
		case C_BSS:
			pMap = readmaps_c_bss();
			break;
		case C_DATA:
			pMap = readmaps_c_data();
			break;
		case C_HEAP:
			pMap = readmaps_c_heap();
			break;
		case JAVA_HEAP:
			pMap = readmaps_java_heap();
			break;
		case A_ANONYMOUS:
			pMap = readmaps_a_anonymous();
			break;
		case CODE_SYSTEM:
			pMap = readmaps_code_system();
			break;
		case STACK:
			pMap = readmaps_stack();
			break;
		case ASHMEM:
			pMap = readmaps_ashmem();
			break;
		default:
			printf("\033[32;1mYou Select A NULL Type!\n");
			break;
	}
	if (pMap == NULL) {
		return 0;
	}
	return pMap;
}

PMAPS readmaps_all() {
	PMAPS pHead = NULL;
	PMAPS pNew;
	PMAPS pEnd;
	pEnd = pNew = (PMAPS) malloc(LEN);
	FILE *fp;
	int i = 0, flag = 1;
	char lj[64], buff[256];
	int pid = getPID(bm);
	sprintf(lj, "/proc/%d/maps", pid);
	fp = fopen(lj, "r");
	if (fp == NULL) {
		puts("内存读取失败!");
		return NULL;
	}
	while (!feof(fp)) {
		fgets(buff, sizeof(buff), fp);	// 读取一行
		if (strstr(buff, "rw") != NULL && !feof(fp)) {
			sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
			// 这里使用lx是为了能成功读取特别长的地址
			flag = 1;
		} else {
			flag = 0;
		}
		if (flag == 1) {
			i++;
			if (i == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				pHead = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);	// 分配内存
		}
	}
	free(pNew);					// 将多余的空间释放
	fclose(fp);					// 关闭文件指针
	return pHead;
}

PMAPS readmaps_bad() {
	PMAPS pHead = NULL;
	PMAPS pNew = NULL;
	PMAPS pEnd = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	FILE *fp;
	int i = 0, flag = 1;
	char lj[64], buff[256];
	int pid = getPID(bm);
	sprintf(lj, "/proc/%d/maps", pid);
	fp = fopen(lj, "r");
	if (fp == NULL) {
		puts("内存读取失败!");
		return NULL;
	}
	while (!feof(fp)) {
		fgets(buff, sizeof(buff), fp);	// 读取一行
		if (strstr(buff, "rw") != NULL && !feof(fp) && strstr(buff, "kgsl-3d0")) {
			sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
			// 这里使用lx是为了能成功读取特别长的地址
			flag = 1;
		} else {
			flag = 0;
		}
		if (flag == 1) {
			i++;
			if (i == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				pHead = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);	// 分配内存
		}
	}
	free(pNew);					// 将多余的空间释放
	fclose(fp);					// 关闭文件指针
	return pHead;
}

PMAPS readmaps_c_alloc() {
	PMAPS pHead = NULL;
	PMAPS pNew = NULL;
	PMAPS pEnd = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	FILE *fp;
	int i = 0, flag = 1;
	char lj[64], buff[256];
	int pid = getPID(bm);
	sprintf(lj, "/proc/%d/maps", pid);
	fp = fopen(lj, "r");
	if (fp == NULL) {
		puts("内存读取失败!");
		return NULL;
	}
	while (!feof(fp)) {
		fgets(buff, sizeof(buff), fp);	// 读取一行
		if (strstr(buff, "rw") != NULL && !feof(fp) && strstr(buff, "[anon:libc_malloc]")) {
			sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
			// 这里使用lx是为了能成功读取特别长的地址
			flag = 1;
		} else {
			flag = 0;
		}
		if (flag == 1) {
			i++;
			if (i == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				pHead = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);	// 分配内存
		}
	}
	free(pNew);					// 将多余的空间释放
	fclose(fp);					// 关闭文件指针
	return pHead;
}

PMAPS readmaps_c_bss() {
	PMAPS pHead = NULL;
	PMAPS pNew = NULL;
	PMAPS pEnd = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	FILE *fp;
	int i = 0, flag = 1;
	char lj[64], buff[256];
	int pid = getPID(bm);
	sprintf(lj, "/proc/%d/maps", pid);
	fp = fopen(lj, "r");
	if (fp == NULL) {
		puts("内存读取失败!");
		return NULL;
	}
	while (!feof(fp)) {
		fgets(buff, sizeof(buff), fp);	// 读取一行
		if (strstr(buff, "rw") != NULL && !feof(fp) && strstr(buff, "[anon:.bss]")) {
			sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
			// 这里使用lx是为了能成功读取特别长的地址
			flag = 1;
		} else {
			flag = 0;
		}
		if (flag == 1) {
			i++;
			if (i == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				pHead = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);	// 分配内存
		}
	}
	free(pNew);					// 将多余的空间释放
	fclose(fp);					// 关闭文件指针
	return pHead;
}

PMAPS readmaps_c_data() {
	PMAPS pHead = NULL;
	PMAPS pNew = NULL;
	PMAPS pEnd = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	FILE *fp;
	int i = 0, flag = 1;
	char lj[64], buff[256];
	int pid = getPID(bm);
	sprintf(lj, "/proc/%d/maps", pid);
	fp = fopen(lj, "r");
	if (fp == NULL) {
		puts("内存读取失败!");
		return NULL;
	}
	while (!feof(fp)) {
		fgets(buff, sizeof(buff), fp);	// 读取一行
		if (strstr(buff, "rw") != NULL && !feof(fp) && strstr(buff, "/data/app/")) {
			sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
			// 这里使用lx是为了能成功读取特别长的地址
			flag = 1;
		} else {
			flag = 0;
		}
		if (flag == 1) {
			i++;
			if (i == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				pHead = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);	// 分配内存
		}
	}
	free(pNew);					// 将多余的空间释放
	fclose(fp);					// 关闭文件指针
	return pHead;
}

PMAPS readmaps_c_heap() {
	PMAPS pHead = NULL;
	PMAPS pNew = NULL;
	PMAPS pEnd = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	FILE *fp;
	int i = 0, flag = 1;
	char lj[64], buff[256];
	int pid = getPID(bm);
	sprintf(lj, "/proc/%d/maps", pid);
	fp = fopen(lj, "r");
	if (fp == NULL) {
		puts("内存读取失败!");
		return NULL;
	}
	while (!feof(fp)) {
		fgets(buff, sizeof(buff), fp);	// 读取一行
		if (strstr(buff, "rw") != NULL && !feof(fp) && strstr(buff, "[heap]")) {
			sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
			// 这里使用lx是为了能成功读取特别长的地址
			flag = 1;
		} else {
			flag = 0;
		}
		if (flag == 1) {
			i++;
			if (i == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				pHead = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);	// 分配内存
		}
	}
	free(pNew);					// 将多余的空间释放
	fclose(fp);					// 关闭文件指针
	return pHead;
}

PMAPS readmaps_java_heap() {
	PMAPS pHead = NULL;
	PMAPS pNew = NULL;
	PMAPS pEnd = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	FILE *fp;
	int i = 0, flag = 1;
	char lj[64], buff[256];
	int pid = getPID(bm);
	sprintf(lj, "/proc/%d/maps", pid);
	fp = fopen(lj, "r");
	if (fp == NULL) {
		puts("内存读取失败!");
		return NULL;
	}
	while (!feof(fp)) {
		fgets(buff, sizeof(buff), fp);	// 读取一行
		if (strstr(buff, "rw") != NULL && !feof(fp) && strstr(buff, "/dev/ashmem/")) {
			sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
			// 这里使用lx是为了能成功读取特别长的地址
			flag = 1;
		} else {
			flag = 0;
		}
		if (flag == 1) {
			i++;
			if (i == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				pHead = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);	// 分配内存
		}
	}
	free(pNew);					// 将多余的空间释放
	fclose(fp);					// 关闭文件指针
	return pHead;
}

PMAPS readmaps_a_anonymous() {
	PMAPS pHead = NULL;
	PMAPS pNew = NULL;
	PMAPS pEnd = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	FILE *fp;
	int i = 0, flag = 1;
	char lj[64], buff[256];
	int pid = getPID(bm);
	sprintf(lj, "/proc/%d/maps", pid);
	fp = fopen(lj, "r");
	if (fp == NULL) {
		puts("内存读取失败!");
		return NULL;
	}
	while (!feof(fp)) {
		fgets(buff, sizeof(buff), fp);	// 读取一行
		if (strstr(buff, "rw") != NULL && !feof(fp) && (strlen(buff) < 42)) {
			sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
			// 这里使用lx是为了能成功读取特别长的地址
			flag = 1;
		} else {
			flag = 0;
		}
		if (flag == 1) {
			i++;
			if (i == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				pHead = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);	// 分配内存
		}
	}
	free(pNew);					// 将多余的空间释放
	fclose(fp);					// 关闭文件指针
	return pHead;
}

PMAPS readmaps_code_system() {
	PMAPS pHead = NULL;
	PMAPS pNew = NULL;
	PMAPS pEnd = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	FILE *fp;
	int i = 0, flag = 1;
	char lj[64], buff[256];
	int pid = getPID(bm);
	sprintf(lj, "/proc/%d/maps", pid);
	fp = fopen(lj, "r");
	if (fp == NULL) {
		puts("内存读取失败!");
		return NULL;
	}
	while (!feof(fp)) {
		fgets(buff, sizeof(buff), fp);	// 读取一行
		if (strstr(buff, "rw") != NULL && !feof(fp) && strstr(buff, "/system")) {
			sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
			// 这里使用lx是为了能成功读取特别长的地址
			flag = 1;
		} else {
			flag = 0;
		}
		if (flag == 1) {
			i++;
			if (i == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				pHead = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);	// 分配内存
		}
	}
	free(pNew);					// 将多余的空间释放
	fclose(fp);					// 关闭文件指针
	return pHead;
}

PMAPS readmaps_stack() {
	PMAPS pHead = NULL;
	PMAPS pNew = NULL;
	PMAPS pEnd = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	FILE *fp;
	int i = 0, flag = 1;
	char lj[64], buff[256];
	int pid = getPID(bm);
	sprintf(lj, "/proc/%d/maps", pid);
	fp = fopen(lj, "r");
	if (fp == NULL) {
		puts("内存读取失败!");
		return NULL;
	}
	while (!feof(fp)) {
		fgets(buff, sizeof(buff), fp);	// 读取一行
		if (strstr(buff, "rw") != NULL && !feof(fp) && strstr(buff, "[stack]")) {
			sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
			// 这里使用lx是为了能成功读取特别长的地址
			flag = 1;
		} else {
			flag = 0;
		}
		if (flag == 1) {
			i++;
			if (i == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				pHead = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);	// 分配内存
		}
	}
	free(pNew);					// 将多余的空间释放
	fclose(fp);					// 关闭文件指针
	return pHead;
}

PMAPS readmaps_ashmem() {
	PMAPS pHead = NULL;
	PMAPS pNew = NULL;
	PMAPS pEnd = NULL;
	pEnd = pNew = (PMAPS) malloc(LEN);
	FILE *fp;
	int i = 0, flag = 1;
	char lj[64], buff[256];
	int pid = getPID(bm);
	sprintf(lj, "/proc/%d/maps", pid);
	fp = fopen(lj, "r");
	if (fp == NULL) {
		puts("内存读取失败!");
		return NULL;
	}
	while (!feof(fp)) {
		fgets(buff, sizeof(buff), fp);	// 读取一行
		if (strstr(buff, "rw") != NULL && !feof(fp) && strstr(buff, "/dev/ashmem/")
		        && !strstr(buff, "dalvik")) {
			sscanf(buff, "%lx-%lx", &pNew->addr, &pNew->taddr);
			// 这里使用lx是为了能成功读取特别长的地址
			flag = 1;
		} else {
			flag = 0;
		}
		if (flag == 1) {
			i++;
			if (i == 1) {
				pNew->next = NULL;
				pEnd = pNew;
				pHead = pNew;
			} else {
				pNew->next = NULL;
				pEnd->next = pNew;
				pEnd = pNew;
			}
			pNew = (PMAPS) malloc(LEN);	// 分配内存
		}
	}
	free(pNew);					// 将多余的空间释放
	fclose(fp);					// 关闭文件指针
	return pHead;
}
