/*--------------------------------------------------------------------*/
/* XEX2WAV - GienekP                                                  */
/*--------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
/*--------------------------------------------------------------------*/
typedef unsigned char U8;
/*--------------------------------------------------------------------*/
const unsigned int SF=44100;
const unsigned int FQSPACE=3995;
const unsigned int FQSIGN=5327;
const unsigned int COUNT=73;
const unsigned int BAUDS=(SF/COUNT);
const unsigned int GAPLONG=20000;
const unsigned int GAPNORMAL=3000;
const unsigned int GAPSHORT=250;
const unsigned int GAPSILENT=1000;
/*--------------------------------------------------------------------*/
/* mono loader */
const U8 mono[128]={
  0x00, 0x01, 0x7a, 0x04, 0x71, 0xe4, 0x88, 0xf0, 0x3f, 0x38, 0x60, 0x85,
  0x04, 0x20, 0xcf, 0x04, 0x85, 0x05, 0x25, 0x04, 0xc9, 0xff, 0xf0, 0x30,
  0x20, 0xcf, 0x04, 0x85, 0x02, 0x20, 0xcf, 0x04, 0x85, 0x03, 0xa9, 0xf9,
  0x8d, 0xe2, 0x02, 0xa9, 0x04, 0x8d, 0xe3, 0x02, 0x20, 0xcf, 0x04, 0x81,
  0x04, 0xa5, 0x04, 0xc5, 0x02, 0xa5, 0x05, 0xe5, 0x03, 0xe6, 0x04, 0xd0,
  0x02, 0xe6, 0x05, 0x90, 0xeb, 0x84, 0x3d, 0x20, 0xcc, 0x04, 0xa4, 0x3d,
  0xa2, 0x00, 0x20, 0xcf, 0x04, 0xb0, 0xbc, 0x6c, 0xe0, 0x02, 0x6c, 0xe2,
  0x02, 0xcc, 0x8a, 0x02, 0x90, 0x1c, 0xc0, 0x80, 0x90, 0x1d, 0x20, 0x7a,
  0xe4, 0x10, 0x0a, 0xc0, 0x88, 0xd0, 0x15, 0xad, 0x7f, 0x04, 0x8d, 0x8a,
  0x02, 0xa9, 0x3c, 0x8d, 0x02, 0xd3, 0xa2, 0x00, 0xa0, 0x00, 0xb9, 0x00,
  0x04, 0xc8, 0x38, 0x60, 0x68, 0x68, 0x38, 0x60
  };
/*--------------------------------------------------------------------*/
void addSilent(U8 *track, unsigned int *pos)
{
	if (track) {track[*pos]=0;};
	(*pos)++;
}
/*--------------------------------------------------------------------*/
void addSpace(U8 *track, unsigned int *pos)
{
	if (track) {track[*pos]=1;};
	(*pos)++;
}
/*--------------------------------------------------------------------*/
void addSign(U8 *track, unsigned int *pos)
{
	if (track) {track[*pos]=2;};
	(*pos)++;
}
/*--------------------------------------------------------------------*/
void addByte(U8 *track, unsigned int *pos, U8 byte)
{
	unsigned int i; 
	addSpace(track,pos);
	for (i=0; i<8; i++)
	{
		U8 bit=((byte>>i) & 1);
		if (bit==0) {addSpace(track,pos);} else {addSign(track,pos);};
	};
	addSign(track,pos);
}
/*--------------------------------------------------------------------*/
void addRecord(U8 *track, unsigned int *pos, const U8 *byte, U8 size)
{
	U8 i;
	addByte(track,pos,0x55);
	addByte(track,pos,0x55);
	if (size==0)
	{
		addByte(track,pos,0xFE);
		for (i=0; i<128; i++) {addByte(track,pos,0x00);};
		addByte(track,pos,0xA9);
	}
	else if (size==128)
	{
		unsigned int sum=0xA7;
		addByte(track,pos,0xFC);
		for (i=0; i<128; i++)
		{
			addByte(track,pos,byte[i]);
			sum+=byte[i];
			if (sum>0xFF)
			{
				sum&=0xFF;
				sum++;
			};
		};
		addByte(track,pos,(sum & 0xFF));
	}
	else
	{
		unsigned int sum=0xA5;
		addByte(track,pos,0xFA);
		for (i=0; i<size; i++)
		{
			addByte(track,pos,byte[i]);
			sum+=byte[i];
			if (sum>0xFF)
			{
				sum&=0xFF;
				sum++;
			};
		};
		for (i=size; i<127; i++) {addByte(track,pos,0x00);};
		addByte(track,pos,size);
		sum+=size;
		if (sum>0xFF)
		{
			sum&=0xFF;
			sum++;
		};				
		addByte(track,pos,(sum & 0xFF));
	};
}
/*--------------------------------------------------------------------*/
void addGap(U8 *track, unsigned int *pos, U8 mode)
{
	unsigned int i,ms,n;
	switch (mode)
	{
		case 1: {ms=GAPSHORT;} break;
		case 2: {ms=GAPNORMAL;} break;
		case 3: {ms=GAPLONG;} break;		
		default: {ms=0;} break;
	};
	n=(ms*BAUDS)/1000;
	for (i=0; i<n; i++) {addSign(track,pos);};
}
/*--------------------------------------------------------------------*/
void addInterspace(U8 *track, unsigned int *pos)
{
	unsigned int i,ms,n;
	ms=GAPSILENT;
	n=(ms*BAUDS)/1000;
	for (i=0; i<n; i++) {addSilent(track,pos);};
}
/*--------------------------------------------------------------------*/
U8 detectINIT(const U8 *file, unsigned int size, unsigned int m)
{
	U8 ret=1;
	unsigned int start,stop,p=0;
	if ((file[0]==0xFF) && (file[1]==0xFF))
	{
		while (p<size)
		{
			if ((file[p]==0xFF) && (file[p+1]==0xFF)) {p+=2;};
			start=file[p+1];
			start<<=8;
			start|=file[p];
			p+=2;
			stop=file[p+1];
			stop<<=8;
			stop|=file[p];	
			p+=2;		
			p+=((stop-start)+1);
			if ((start==0x02E2) && (stop==0x02E3))
			{
				if ((p>m) && (p<(m+128))) {ret=2;};
			};
		};
	};
	return ret;
}
/*--------------------------------------------------------------------*/
void addFile(U8 *track, unsigned int *pos, const U8 *file, unsigned int size, U8 xex)
{
	unsigned int m=0;
	int cnt=size;
	if (xex==0) {addGap(track,pos,3);};
	while (cnt>0)
	{
		U8 init=detectINIT(file,size,m);
		addGap(track,pos,init);
		if (cnt>128)
		{
			addRecord(track,pos,&file[m],128);
			m+=128;
			cnt-=128;
		}
		else
		{
			addRecord(track,pos,&file[m],cnt);
			cnt-=128;
		};
	};
	if (xex)
	{
		addGap(track,pos,1);
		addRecord(track,pos,file,0);
	};
}
/*--------------------------------------------------------------------*/
void addXEX(U8 *track, unsigned int *pos, const U8 *xex, unsigned int size)
{
	addFile(track,pos,mono,sizeof(mono),0);
	addFile(track,pos,xex,size,1);
	addInterspace(track,pos);
}
/*--------------------------------------------------------------------*/
unsigned int loadBIN(const char *filename, U8 *buf, unsigned int size)
{
	unsigned int ret=0;
	FILE *pf;
	pf=fopen(filename,"rb");
	if (pf)
	{
		ret=fread(buf,sizeof(U8),size,pf);
		fclose(pf);
	};
	return ret;
}
/*--------------------------------------------------------------------*/
void putInt32(U8 *data, unsigned int value)
{
	unsigned int a,b,c,d;
	a=(value & 0xFF);
	value>>=8;
	b=(value & 0xFF);
	value>>=8;
	c=(value & 0xFF);
	value>>=8;
	d=(value & 0xFF);
	data[0]=(U8)(a);
	data[1]=(U8)(b);
	data[2]=(U8)(c);
	data[3]=(U8)(d);	
}
/*--------------------------------------------------------------------*/
unsigned int calcSin(U8 bit)
{
	static double samp=SF;
	static double phase=0;
	int ret;
	if (bit)
	{
		double freq;
		if (bit==1) {freq=FQSPACE;} else {freq=FQSIGN;};
		phase+=(4.0*acos(0.0)*freq/samp);
		ret=30719.0*sin(phase);
	}
	else
	{
		phase=0;
		ret=0;
	};
	return ret;
};
/*--------------------------------------------------------------------*/
void saveWAV(const char *filename, const U8 *track, unsigned int size)
{
	U8 header[44]={0x52,0x49,0x46,0x46,
		0xFF,0xFF,0xFF,0xFF,0x57,0x41,0x56,0x45,
		0x66,0x6D,0x74,0x20,0x10,0x00,0x00,0x00,
		0x01,0x00,0x02,0x00,0x44,0xAC,0x00,0x00,
		0x10,0xB1,0x02,0x00,0x04,0x00,0x10,0x00,
		0x64,0x61,0x74,0x61,0xFF,0xFF,0xFF,0xFF};
	FILE *pf;
	unsigned int filesize,datasize,i,j;
	datasize=(4*size*COUNT);
	filesize=(sizeof(header)+datasize-8);
	putInt32(&header[4],filesize);
	putInt32(&header[40],datasize);
	pf=fopen(filename,"wb");
	if (pf)
	{
		unsigned int t=size/BAUDS;
		printf("Save \"%s\"  %i min %i seconds\n",filename,t/60,t%60);
		fwrite(header,sizeof(U8),sizeof(header),pf);
		for (j=0; j<size; j++)
		{
			U8 bit=track[j];
			for (i=0; i<COUNT; i++)
			{
				unsigned int code=calcSin(bit);
				U8 a=(code & 0xFF);
				U8 b=((code >> 8) & 0xFF);
				fputc(a/*0x00*/,pf);
				fputc(b/*0x00*/,pf);
				fputc(a,pf);
				fputc(b,pf);
			};
		};
		fclose(pf);
	};
}
/*--------------------------------------------------------------------*/
void buildList(int argc, char *argv[])
{
	U8 buf[128*1024];
	unsigned int i,pos,size;
	U8 *track;
	track=NULL;
	printf("Sampling frequency %i [Hz]\n",SF);
	printf("Speed %i [bauds]\n",BAUDS);
	printf("Space frequency %i [Hz]\n",FQSPACE);
	printf("Sign frequency %i [Hz]\n",FQSIGN);
	for (i=0; i<(argc-2); i++)
	{
		size=loadBIN(argv[1+i],buf,sizeof(buf));
		printf("Scanning \"%s\" (%i bytes)\n",argv[1+i],size);
		addXEX(track,&pos,buf,size);
	};
	printf("Need %i pulses\n",pos);
	track=(U8 *)malloc(sizeof(U8)*pos);
	for (i=0; i<pos; i++) {track[i]=0;};
	if (track)
	{
		pos=0;
		for (i=0; i<(argc-2); i++)
		{
			unsigned int lpos=pos;
			size=loadBIN(argv[1+i],buf,sizeof(buf));
			printf("Building \"%s\"\n",argv[1+i]);
			addXEX(track,&pos,buf,size);
			printf("\"%s\" need %i seconds\n",argv[1+i],(pos-lpos)/BAUDS);
		};	
		saveWAV(argv[argc-1],track,pos);
		free(track);
	};
}
/*--------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	printf("XEX2WAV (c)GienekP\n");
	if (argc>2)
	{
		buildList(argc,argv);
	}
	else
	{
		printf("use:\n");
		printf("   xex2wav file.xex file.wav\n");
		printf("   xex2wav file1.xex file2.xex file.wav\n");
	};
	return 0;
}
/*--------------------------------------------------------------------*/
