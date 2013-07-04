#include<stdio.h>
int s[3000000];
int main(){
	int a,b,i;
	scanf("%d%d",&a,&b);
	printf("%d\n",a+b);
	for(i=0;i<3000000;i++){
		s[i]=i*2;
	}
	printf("%d\n",s[i-3]);
	return 0;
}
