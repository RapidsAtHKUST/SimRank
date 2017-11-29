#ifndef __INBUF_H__
#define __INBUF_H__

#include <cstring>
#include <cstdio>

struct inBuf
{
	const static int bufSize = 64*1024;
	int cnt, buf[bufSize];
	FILE * fg;

	inBuf(char * fName)
	{
		fg = fopen(fName, "rb");
		cnt = 0;
		fread(buf, sizeof(int), bufSize, fg);
	}
	~inBuf()
	{
		fclose(fg);	
	}

	void nextInt(int & ans)
	{
		if (cnt == bufSize) 
		{
			fread(buf, sizeof(int), bufSize, fg);
			cnt = 0;
		}
		ans = buf[cnt++];
	}

	void nextLL(long long & ans)
	{
		if (cnt == bufSize) 
		{
			fread(buf, sizeof(int), bufSize, fg);
			cnt = 0;
		}
		ans = (((long long)(buf[cnt++])) << 32);
		if (cnt == bufSize) 
		{
			fread(buf, sizeof(int), bufSize, fg);
			cnt = 0;
		}
		ans += buf[cnt++];
	}

};


#endif