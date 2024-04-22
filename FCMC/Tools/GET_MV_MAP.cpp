#include<bits/stdc++.h>
using namespace std;
int main()
{
	freopen("MV_MAP.txt","r",stdin);
	while(1)
	{
		int a,b;
		cin>>a>>b;
		if(a==0) return 0;
		while(1)
		{
			int c,d;
			cin>>c>>d;
			if(c==0) break;
			printf("NA[make_pair(%d,%d)].push_back(make_pair(%d,%d)),",a,b,c,d);
		}
		printf(";\n");
	}
	fclose(stdout);
}
