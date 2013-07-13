#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<iostream>
#include<algorithm>
#include<string>
#include<cmath>
#include<set>
#include<queue>
using namespace std;
#define mp make_pair
#define pb push_back
#define inf 1000000000
#define INF 10000000000000000LL

struct side{int e,l,fr;}    G[10000001];
int n,m,g,q[40011],h[40011],source,sink,last[40011],last1[40011],c;
 
void add(int s,int e,int l)
{
    G[c].e=e;
    G[c].l=l;
    G[c].fr=last1[s];
    last1[s]=c++;
	G[c].e=s;
	G[c].l=0;
	G[c].fr=last1[e];
	last1[e]=c++;
}

bool bfs()
{
    memcpy(last,last1,(sizeof (int))*(sink+2));
    memset(h,0,(sizeof (int))*(sink+2));
    int open=0,clos=1;
    q[clos]=sink;
    h[sink]=1;
    while (open<clos)
    {
        open++;
        for (int j=last[q[open]];j;j=G[j].fr)
            if (!h[G[j].e] && G[j^1].l)
            {
                q[++clos]=G[j].e;
                h[q[clos]]=h[q[open]]+1;
                if (q[clos]==source)    return 1;
            }
    }
    return 0;
}
 
long long dfs(int i,long long limit)
{
    if (i==sink)    return limit;
    long long ans=0,t;
    for (int &j=last[i];j;j=G[j].fr)    
        if (G[j].l && h[G[j].e]+1==h[i])
        {
            t=dfs(G[j].e,min(limit-ans,((long long)G[j].l)));
            ans+=t;
            G[j].l-=t;
            G[j^1].l+=t;
            if (limit==ans) break;
        }
    return ans;
}

long long dinic()
{
	long long ans=0;
	while (bfs()) ans+=dfs(source,INF);
	return ans;
}

int bl[60010];
bool bj[60010];

void getans(int flag)
{
	memset(bj,0,sizeof bj);
	queue<int> Q;
	int now;
	Q.push(source);
	bj[source]=1;
	while (!Q.empty())
	{
		now=Q.front();
		bl[now]=flag;
		Q.pop();
		for (int j=last1[now];j;j=G[j].fr)
			if (G[j].l>0 && !bj[G[j].e])
			{
				Q.push(G[j].e);
				bj[G[j].e]=1;
			}
	}
}

int sx[30010],v[30010],w[3010],k[3010],ex[3010],bb[3010][15];

int main()
{
	int x,ans=0;
	c=2;
	scanf("%d%d",&n,&m);
	source=0,sink=n+m+1;
	for (int i=0;i<n;i++)	scanf("%d",v+i);
	for (int i=0;i<n;i++)	scanf("%d",sx+i);
	for (int i=0;i<n;i++)	
	{
		if (v[i]<0) sx[i]^=1,v[i]=-v[i],ans+=v[i];
		int vv=v[i]*100000;
		if (!vv) vv=1;
		if (sx[i]==1) add(i+1,sink,vv);else add(source,i+1,vv);
	}
	for (int i=0;i<m;i++)
	{
		scanf("%d%d%d",ex+i,w+i,k+i);
		for (int j=0;j<k[i];j++)	scanf("%d",&bb[i][j]);
		ans+=w[i];
		if (ex[i]==0)	add(source,i+n+1,w[i]*100000);
		else add(i+n+1,sink,w[i]*100000);
		
		for (int j=0;j<k[i];j++)
			if (ex[i]!=sx[bb[i][j]-1])
			{
				if (ex[i]==0) add(i+n+1,bb[i][j],inf);
				else add(bb[i][j],i+n+1,inf);
			}
	}
	for (int i=0;i<m;i++)
	if (ex[i]==0)
		for (int j=0;j<m;j++)
		if (ex[j]==1)
		{
			bool yes=0;
			for (int k0=0;k0<k[i];k0++)
			for (int k1=0;k1<k[j];k1++)
				if (bb[i][k0]==bb[j][k1])	yes=1;
			if (yes) add(i+n+1,j+n+1,inf);
		}
	
	getans(1);
	long long kao=dinic(),gao;
	gao=round(kao/100000.0);
	printf("%d\n",ans-gao);
	getans(2);
	for (int i=0;i<n;i++)	
	{
		if (!bl[i+1]) printf("%d ",sx[i]);
		else printf("%d ",2-bl[i+1]);
	}
	return 0;
}

