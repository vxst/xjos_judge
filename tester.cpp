#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<cctype>
#include<cassert>
//1:Out 2:Std 3:Res
char* getnexttoken(FILE* file){
	char* ret=new char[16000000];//FIXIT:Need to be unlimited if possible
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
	FILE* fdbg=fopen("/home/judge/ss","a");
	int point;
	char* out,*std;
	sscanf(filename[4],"%d",&point);
	for(out=getnexttoken(fout),std=getnexttoken(fstd);out!=0&&std!=0;out=getnexttoken(fout),std=getnexttoken(fstd)){
		fprintf(fdbg,"Compare:Std:%s:Out:%s\n",std,out);
		if(strcmp(out,std)!=0){
			fprintf(fdbg,"%s != %s\n",out,std);
			fprintf(fres,"W\n");
			fprintf(fres,"0\n");
			fclose(fres);
			return 0;
		}
		delete[] out;
		delete[] std;
	}
	if((out==0&&std!=0)||(out!=0&&std==0)){
		fprintf(fdbg,"Num Err\n");
		if(out!=0){
			fprintf(fdbg,"Out:%s\n",out);
		}else{
			fprintf(fdbg,"Std:%s\n",std);
		}
		fprintf(fres,"W\n0\n");
	}else{
		fprintf(fres,"A\n%d\n",point);
	}
	fclose(fres);
	return 1;
}
