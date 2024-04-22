#include<bits/stdc++.h>
using namespace std;
int main()
{
	freopen("_2road.txt","r",stdin);
	int cnt=1,a;
	while(1)
	{
		cin>>a;
		if(a==888) return 0;
		if(a==999)
		{
			cnt++;
			printf("\b;\n");
			continue;
		}
		printf("_2ROAD[%d].push_back(%d),",cnt,a);
	}
	fclose(stdout);
}
