#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<cctype>
#include<cassert>
//1:Out 2:Std 3:Res
char* getnexttoken(FILE* file){
	char* ret=new char[16777216];//FIXIT:Need to be unlimited if possible
	int k=fscanf(file,"%s",ret);
	if(k!=1){
		delete[] ret;
		ret=0;
	}
	return ret;
}
int main(int count, char** filename){
	assert(count==5);
	FILE* fout=fopen(filename[1],"r");
	FILE* fstd=fopen(filename[2],"r");
	FILE* fres=fopen(filename[3],"w");
	int point;
	char* out,*std;
	sscanf(filename[4],"%d",&point);
	for(out=getnexttoken(fout),std=getnexttoken(fstd);out!=0&&std!=0;out=getnexttoken(fout),std=getnexttoken(fstd)){
		if(strcmp(out,std)!=0){
			fprintf(fres,"W\n");
			fprintf(fres,"0\n");
			fclose(fres);
			return 0;
		}
		delete[] out;
		delete[] std;
	}
	if((out==0&&std!=0)||(out!=0&&std==0)){
		fprintf(fres,"W\n0\n");
	}else{
		fprintf(fres,"A\n%d\n",point);
	}
	fclose(fres);
	return 1;
}
