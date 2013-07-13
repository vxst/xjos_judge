//judge byst
/*
   status Pending:0
   status Compiling:1
   status Compile OK:2
   status Compile Error:4
   status Running:8
   status Accept:16
   status Wrong Answer:32
   status Parted Score:64
   status Runtime Error:128
   status Time Limit Exceed:256
   status Memory Limit Exceed:512
   status Special Judge Error:1024
   status System Error:2048
   status Touched:4096
*/
//TODO: Transation instead of Lock
#include<cstdio>
#include<string>
#include<cstdlib>
#include<cctype>
#include<iostream>
#include<unistd.h>
#include<cstring>
#include<fstream>
#include<mysql++/mysql++.h>
#include<ctime>
#include<cassert>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h> 
#include<vector>

#define DB_ERROR 2
#define NOT_FOUND 4
#define SOME_ERROR 32768

using namespace mysqlpp;
using namespace std;

int lang,sid,pid,grade=0,spjid;
char src[256];
Connection conn,conn2,conn3,conn4;
char execname[256],inname[256],outname[256],tmpname[256],stdname[256],statusname[256],resname[256],spjname[256];
vector<string> allnames;

struct tst_result{
	int points,full_points;
	char* message;
};

bool spj_isspj;
char spj_spjtype[256],*spj_src;
char spj_path[256];


void writesocket(const char* socketserver,int port,const char* msg);
void alertnodetoupdate(int,int,double,double,int);
int isfileexist(const char*);

//------------------------------SQL And So..-----------------------------

int sqlconnect(){
	conn.connect("xjos","210.33.7.109","judge","T5SD5NCnu2zGhYwu",3306);
	if(!conn.connected())
		return DB_ERROR;
	conn2.connect("xjos","210.33.7.109","judge","T5SD5NCnu2zGhYwu",3306);
	if(!conn2.connected())
		return DB_ERROR;
	conn3.connect("xjos","210.33.7.109","judge","T5SD5NCnu2zGhYwu",3306);
	if(!conn3.connected())
		return DB_ERROR;
	conn4.connect("xjos","210.33.7.109","judge","T5SD5NCnu2zGhYwu",3306);
	if(!conn4.connected())
		return DB_ERROR;
	return 0;
}

void mkset(){
	Query q=conn4.query();
	q<<"UPDATE `xjos`.`submit` SET status="<<4096<<" WHERE sid="<<sid;
	q.use();
}

void decstatus(int a){
	Connection stconn;
//	fprintf(stderr,"DECS %d\n",a);
	stconn.connect("xjos","210.33.7.109","judge","T5SD5NCnu2zGhYwu",3306);
	Query qout=stconn.query();
	qout<<"SELECT status FROM `xjos`.`submit` WHERE sid="<<sid;
	Row r=qout.use().fetch_row();
	int status=r["status"];
	status|=a;
	status^=a;
	qout<<"UPDATE `xjos`.`submit` SET status="<<status<<" WHERE sid="<<sid;
	qout.use();
}

void addstatus(int a){
	Connection stconn;
//	fprintf(stderr,"ADDS%d\n",a);
	stconn.connect("xjos","210.33.7.109","judge","T5SD5NCnu2zGhYwu",3306);
	Query qout=stconn.query();
	qout<<"SELECT status FROM `xjos`.`submit` WHERE sid="<<sid;
	Row r=qout.use().fetch_row();
	int status=r["status"];
	status|=a;
	qout<<"UPDATE `xjos`.`submit` SET status="<<status<<" WHERE sid="<<sid;
	qout.use();
}
int closesqls(){
	conn.disconnect();
	conn2.disconnect();
	conn3.disconnect();
	conn4.disconnect();
	return 0;
}

//--------------------------FS and names--------------------------------

char randstrtable[]="qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM0123456789";
char randstr_rands[]="0123456789qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM";
char* randstr(int len){
	char* str=(char*)calloc(len+1,sizeof(char));
	for(int i=0,sl=strlen(randstr_rands);i<len;i++)
		str[i]=randstr_rands[rand()%sl];
	str[len]=0;
	return str;
}

void mkrandstr(char* str,int len){
	assert(len>0);
	str[len]=0;
	while(len--)
		str[len]=randstrtable[rand()%strlen(randstrtable)];
}

void mkrandnameendwith(char* name,const char* end){
	mkrandstr(tmpname,8);
	name[0]=0;
	strcpy(name,"/home/judge/");
	strcat(name,tmpname);
	strcat(name,end);
	allnames.push_back(string(name));
}
void mkrandname(char* name){
	mkrandstr(tmpname,8);
	strcpy(name,"/home/judge/");
	strcat(name,tmpname);
	allnames.push_back(string(name));
}
void setrandname(int isexec){
	char order[1024];
	if(!isexec){
		mkrandname(inname);
		mkrandname(outname);
		mkrandname(stdname);
		mkrandname(resname);
		sprintf(order,"mv /home/judge/input.file %s",inname);
		system(order);
		sprintf(order,"mv /home/judge/output.file.std %s",stdname);
		system(order);
	}else{
		mkrandname(execname);
		usleep(100000);
		sprintf(order,"mv /home/judge/test_exec %s",execname);
		system(order);
//		usleep(100000);
		sprintf(order,"chmod +x %s",execname);
		system(order);
//		usleep(100000);
	}
}
int deletefile(const char* name){
	char* str=new char[256];
	sprintf(str,"rm %s",name);
	system(str);
	delete[] str;
	return 0;
}
int deletefiles(){
/*	deletefile(inname);
	deletefile(outname);
	deletefile(stdname);
	deletefile(statusname);
	deletefile(execname);*/
	for(int i=0;i<allnames.size();i++){
		deletefile(allnames[i].c_str());
	}
	return 0;
}
//--------------------------------Compile-----------------------------
char* getmessage(int isok){
	FILE * pFile;
	long lSize;
	char * buffer;
	size_t result;

	if(isok)return(char*)1;

	pFile = fopen ( "ci" , "r" );//ci stand for compile info
	if (pFile==NULL) {fputs ("File error",stderr); return 0;}

	fseek (pFile , 0 , SEEK_END);
	lSize = ftell (pFile);
//	if(lSize==0)return (char*)1;
	rewind (pFile);

	buffer = (char*) malloc (sizeof(char)*lSize);
	if (buffer == NULL) {fputs ("Memory error",stderr); return 0;}

	result = fread (buffer,1,lSize,pFile);
	if (result != lSize) {fputs ("Reading error",stderr); return 0;}

	fclose (pFile);
	return buffer;
}
//ret:1 unknown; 2 cok 4 ce 8 noneed 16 SE
int compile(){
	char order[4096];
	int retv=1;
	addstatus(1);
	sprintf(order,"SELECT * FROM `language` WHERE `id` =%d",lang);
	Row r=conn.query(order).use().fetch_row();
	if(!r)
		return 16;
	sprintf(src,"test.%s",r["ext"].c_str());
	sprintf(order,"mv test %s",src);

	system(order);
	
	if(strcmp(r["compiler"].c_str(),"_noneed")!=0){
		freopen("ci","w",stderr);
		sprintf(order,r["compiler"].c_str(),src,"test_exec","ci");
		system(order);
		usleep(750000);//Should use proc check to produce a better solution
		fclose(stderr);
		int isok=isfileexist("test_exec");

		Query qout=conn.query();
		char* t=getmessage(isok);
		if(t==(char*)1){
			t=(char*)calloc(256,sizeof(char));
			retv=2;
			strcpy(t,"Compile OK!");
			addstatus(2);
			qout<<"UPDATE `xjos`.`submit` SET `status` = '2' WHERE `submit`.`sid` ="<<sid;
			qout.use();
			qout<<"UPDATE `xjos`.`submit` SET `result` ="<<quote<<"<res>Compile OK.</res>"<< "WHERE `submit`.`sid` ="<<sid;
			qout.use();
			alertnodetoupdate(-1,2,0,0,0);
				//retv=0;
		}else{
			retv=4;
			addstatus(4);
			qout<<"UPDATE `xjos`.`submit` SET `result` ="<<quote<<"<res>Compile Error!</res>"<< "WHERE `submit`.`sid` ="<<sid;
			qout.use();

			if(t==0){
				t=(char*)calloc(256,sizeof(char));
				strcpy(t,"Unknown Compile Error\nContact admin for futher information.\nJE:30\n");
				addstatus(4);
				qout<<"UPDATE `xjos`.`submit` SET `result` ="<<quote<<"<res>Unknown Compile Error!</res>"<< "WHERE `submit`.`sid` ="<<sid;
				qout.use();
				retv=16;
			}
			qout<<"UPDATE `xjos`.`submit` SET `infoboard` ="<<quote<<string(t)<< " WHERE `submit`.`sid` ="<<sid;
			qout.use();
			alertnodetoupdate(-1,4,0,0,0);
		}
		if(retv==2){
			system("mkdir /home/judge");
			system("chown judge:judge test_exec");
			system("mv test_exec /home/judge");
		}
		free(t);
	}else{
		retv=8;
		sprintf(order,"mv %s /home/judge/test_exec",src);
		system(order);
		system("chown judge:judge /home/judge/test_exec");
		system("chmod 777 /home/judge/test_exec");
	}
	decstatus(1);
	setrandname(1);
	return retv;
}
int compilespj(int spjid,const char* tmppath,const char* savepath,Connection& conn){//tmppath must be .cpp
	char order[4096];
	Query sqlc=conn.query();
	sqlc<<"SELECT spjsrc,spjbin,spjtype,spjbin_iscompiled FROM xjos.spj_table WHERE spjid="<<spjid;
	//Currently spjbin has no use at all
	Row r=sqlc.use().fetch_row();
	spj_src=(char*)calloc(strlen(r["spjsrc"].c_str())+32,sizeof(char));
	strcpy(spj_src,r["spjsrc"].c_str());
	FILE* spjsrcf=fopen(tmppath,"w");
	fputs(spj_src,spjsrcf);
	fflush(spjsrcf);
	sprintf(order,"g++ %s -o %s -O2",tmppath,savepath);
	system(order);
	usleep(500000);
	if(!isfileexist(savepath)){
		return 0;
	}else{
		return 1;
	}
}

//-----------------------------------------Fetch----------------------------------------

int fetch(){
	char order[512];
	Query qout=conn.query();
	
	qout<<"LOCK TABLES `xjos`.`submit` WRITE,`xjos`.`problem` WRITE";
	if(!qout.exec())
		fprintf(stderr,"!!!SQLERROR!!!");

	qout<<"SELECT problem.pid,submit.sid,submit.language,problem.spjid FROM `submit` JOIN `problem` ON problem.pid=submit.pid WHERE `status`=0 LIMIT 0,1";
	Row r=qout.use().fetch_row();
	if(!r){
		qout<<"UNLOCK TABLES";
		if(!qout.exec())
			fprintf(stderr,"!!!SQLERROR!!!");
		return NOT_FOUND;
	}
	//cerr<<"Fetched!!"<<r["source"]<<endl;
	lang=r["language"];
	sid=r["sid"];
	pid=r["pid"];
	spjid=r["spjid"];

	fprintf(stderr,"Starting SID:%d\n",sid);
	
	qout<<"UPDATE `submit` SET `status`="<<4096<<" WHERE sid="<<sid;
	if(!qout.exec())
		fprintf(stderr,"!!!SQLERROR!!!");

	qout<<"UNLOCK TABLES";
	if(!qout.exec())
		fprintf(stderr,"!!!SQLERROR!!!");
	fprintf(stderr,"Started SID:%d\n",sid);
	ofstream fout("test");

	qout<<"SELECT source FROM `submit` WHERE sid="<<sid;
	r=qout.use().fetch_row();
	fout<<r["source"]<<endl;

	return 0;
}

//------------------------------------------Run------------------------------------
//------------------------------------------Run Part--------------------------------
int runspj(const char* inputfn,const char* outputfn,const char* stdfn,const char* spjfn,tst_result* result){
	char* line=(char*)calloc(4096,sizeof(char)),*statusfn=(char*)calloc(256,sizeof(char)),order[4096];
	result->message=(char*)calloc(65536,sizeof(char));
	mkrandname(statusfn);
	sprintf(order,"%s %s %s %s %s",spjfn,inputfn,stdfn,outputfn,statusfn);
	int r=system(order);
	if(r!=0)return SOME_ERROR;

	usleep(500000);
	FILE* statusfp=fopen(statusfn,"r");

	int isinformation=0;
	for(;!feof(statusfp);){
		fgets(line,65536,statusfp);
		if(!isinformation){
			char type;
			int i,maxscore;
			for(i=0;line[i]&&line[i]!=':';i++);
			if(!line[i]){
				return SOME_ERROR;
			}else{
				type=tolower(line[0]);
				char* strtmp=(char*)calloc(4096,sizeof(char));
				strcpy(strtmp,line+i+1);
				strcpy(line,strtmp);
				free((void*)strtmp);
			}
			if(type=='i'){
				isinformation=1;
				strcat(result->message,line);
				strcat(result->message,"\n");
			}else if(type=='s'){
				sscanf(line,"%d",&result->points);
			}else if(type=='m'){
				sscanf(line,"%d",&result->full_points);
			}else{
				return SOME_ERROR;
			}
		}
	}
	return 0;
}
int computetime(double a){
	int ret=((int)a)+1;
	if(ret-a<0.75)
		ret++;
	return ret;
}
struct run_result{
	double time,memory;
	int re;
};
run_result runonce(const char* execcommand,char* execname,char* inname,char* outname,double timelimit,double memlimit){
	char order[256];
	run_result ret;
	ret.re=0;
	mkrandname(statusname);
	char exec_order[256];
	sprintf(exec_order,execcommand,execname);
	sprintf(order,"su judge -c \"/usr/bin/time --quiet -o %s -f '%%U %%M' timeout --kill-after=1s %ds %s <%s >%s\"",statusname,computetime(timelimit),exec_order,inname,outname);
	system(order);
	//usleep(1000000);
	FILE* stat=fopen(statusname,"r");
	fscanf(stat,"%lf%lf",&ret.time,&ret.memory);
	fclose(stat);
	ret.memory/=4096;//To MB; Bug in GNU time make 4K Div
	return ret;
}

//------------------------------------------Test Part-----------------------------------

tst_result testonce(char* tester,char* outname,char* stdname,char* resname,int points){
	char order[256],*msg=new char[1024];
	tst_result ret;
	ret.message=msg;
	ret.full_points=points;
	sprintf(order,"./tester %s %s %s %d",outname,stdname,resname,points);
	system(order);
	FILE* fres=fopen(resname,"r");
	fscanf(fres,"%s%d",msg,&ret.points);
	return ret;
}
int laststatusid;
char* formatoutput(int point,run_result run,tst_result tst,double maxtime,double maxmem){
	int status_id=0;
	char overallstatus[256]="AC",*ret=new char[256];
	laststatusid=16;

	if(run.re){
		strcpy(overallstatus,"RE");
		status_id|=128;
		laststatusid=128;
	}
	if(run.time>maxtime){
		strcpy(overallstatus,"TLE");
		status_id|=256;
		laststatusid=256;
	}
	if(run.memory>maxmem){
		strcpy(overallstatus,"MLE");
		status_id|=512;
		laststatusid=512;
	}
	if(tst.points==tst.full_points)
		status_id|=16;
	if(tst.points!=0&&tst.points!=tst.full_points)
		status_id|=64;
	if(tst.points==0)
		status_id|=32;
	if(strcmp(overallstatus,"AC")!=0){
		if(tst.points==tst.full_points){
			strcat(overallstatus,", but the answer is right.");
		}else if(tst.points!=0){
			strcat(overallstatus,", but the answer is partly right.");
		}
	}
	if(tst.points==0&&strcmp(overallstatus,"AC")==0){
		strcpy(overallstatus,"WA");
		laststatusid=32;
	}
	if(tst.message==NULL){
		tst.message=(char*)calloc(10,sizeof(char));
		strcpy(tst.message,"");
	}
	sprintf(ret,"<point><id>%d</id><mem>%.2lf</mem><time>%.2lf</time><status>%s</status><status_id>%d</status_id><score>%d</score><message>%s</message></point>",point,run.memory,run.time,overallstatus,status_id,tst.points,tst.message);

	return ret;
}
//----------------------------------------Main Run---------------------------
int run(){
	char runallstatus[65536];
	*runallstatus=0;
	addstatus(8);
	Query qout=conn.query();
	Row r;
	qout<<"SELECT * FROM  xjos.problem_data WHERE pid="<<pid<<" ORDER BY  `problem_data`.`problem_data_rank` ASC";
	UseQueryResult qr=qout.use();
	for(;(r=qr.fetch_row());){
		Query qupd=conn2.query();
		qupd<<"UPDATE `xjos`.`submit` SET `running`="<<r["problem_data_rank"]<<" WHERE `submit`.`sid`="<<sid;
		qupd.use();

		Query qz=conn3.query();
		qz<<"SELECT * FROM `xjos`.`language` WHERE id="<<lang;
		UseQueryResult slng=qz.use();
		Row lang=slng.fetch_row();

		FILE* fin=fopen("/home/judge/input.file","w");
		FILE* fout=fopen("/home/judge/output.file.std","w");
		fputs(r["problem_data_input"],fin);
		fputs(r["problem_data_output"],fout);
		fclose(fin);
		fclose(fout);
		setrandname(0);

		double ktime=r["problem_data_time"];
		double mxmem=r["problem_data_memory"];
		run_result rn=runonce(lang["runner"],execname,inname,outname,ktime,mxmem);
		tst_result ts;
		if(spjid==0){
			ts=testonce((char*)"tester",outname,stdname,resname,r["problem_data_score"]);
		}else{
			runspj(inname,outname,stdname,spjname,&ts);
		}
		if(rn.time>ktime)
			addstatus(256);
		if(rn.memory>mxmem)
			addstatus(512);
		if(ts.points==0)
			addstatus(32);
		grade+=ts.points;
		int pdscore=r["problem_data_score"];
		if(ts.points!=0&&pdscore!=ts.points)
			addstatus(64);
		if(pdscore==ts.points)
			addstatus(16);
		char *status=formatoutput(r["problem_data_rank"],rn,ts,ktime,mxmem);
		strcat(runallstatus,status);

		qupd<<"UPDATE `xjos`.`submit` SET `result`="<<quote<<string("<res>")+string(runallstatus)+string("</res>")<<" WHERE `submit`.`sid`="<<sid;
		qupd.use();
		qupd<<"UPDATE `xjos`.`submit` SET `grade`="<<quote<<grade<<" WHERE `submit`.`sid`="<<sid;
		qupd.use();
		alertnodetoupdate(r["problem_data_id"],laststatusid,rn.memory,rn.time,ts.points);
	}
	decstatus(8);
	return 0;
}

//--------------------------------Nodejs Communication--------------------------
//Functions Of Network To STNODE
void writesocket(const char* socketserver,int port,const char* msg)
{
	int sockfd, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	char buffer[256];
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		perror("ERROR opening socket");
	server = gethostbyname(socketserver);
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, 
		 (char *)&serv_addr.sin_addr.s_addr,
		 server->h_length);
	serv_addr.sin_port = htons(port);
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
		perror("ERROR connecting");
	write(sockfd,msg,strlen(msg));
}
int isfileexist(const char* fn){
	FILE* f=fopen(fn,"r");
	if(f==NULL)return 0;
	fclose(f);
	return 1;
}

void alertnodetoupdate(int dtid,int status,double mem,double time,int grade){
	char* str=new char[256];
	sprintf(str,"P%d:%d:%d:%.3lf:%.3lf:%d",sid,dtid,status,time,mem,grade);
	writesocket("210.33.7.108",8127,str);
}

//----------------------------Something about TJDA Problem--------------------------
int tjda_stid;
int fetchtjdaprob(){
	char order[512];
	Query qout=conn.query();
	
	qout<<"LOCK TABLES `xjos`.`submit_tjda` WRITE,`xjos`.`problem_data` WRITE,`xjos`.`problem` WRITE";
	if(!qout.exec())
		fprintf(stderr,"!!!SQLERROR!!!");

	qout<<"SELECT problem.spjid,submit_tjda.stid,submit_tjda.output,submit_tjda.rank,problem_data.problem_data_input,problem_data.problem_data_output,problem_data.problem_data_score FROM `submit_tjda` JOIN `problem_data` ON problem_data.pid=submit_tjda.pid AND problem_data.problem_data_rank=submit_tjda.rank JOIN problem ON problem.pid=submit_tjda.pid WHERE `isjudged`=0 LIMIT 0,1";

	Row r=qout.use().fetch_row();
	if(!r){
		qout<<"UNLOCK TABLES";
		if(!qout.exec())
			fprintf(stderr,"!!!SQLERROR!!!");
		return NOT_FOUND;
	}
	spjid=r["spjid"];
	grade=r["problem_data_score"];
	tjda_stid=r["stid"];

	fprintf(stderr,"Starting STID:%d\n",tjda_stid);
	
	qout<<"UPDATE `submit_tjda` SET `isjudged`="<<1<<" WHERE stid="<<r["stid"];
	if(!qout.exec())
		fprintf(stderr,"!!!SQLERROR!!!");

	qout<<"UNLOCK TABLES";
	if(!qout.exec())
		fprintf(stderr,"!!!SQLERROR!!!");
	fprintf(stderr,"Started STID:%d\n",tjda_stid);
	ofstream ftin("/home/judge/test.in");
	ofstream ftout("/home/judge/test.out");
	ofstream ftstd("/home/judge/test.std");

	ftin<<r["problem_data_input"]<<endl;
	ftstd<<r["problem_data_output"]<<endl;
	ftout<<r["output"]<<endl;

	return 0;
}

tst_result tjdajudgeonce(const char* tester,const char* outname,const char* stdname,const char* resname,int points){
	char order[256],*msg=new char[1024];
	tst_result ret;
	ret.message=msg;
	ret.full_points=points;
	sprintf(order,"./tester %s %s %s %d",outname,stdname,resname,points);
	system(order);
	FILE* fres=fopen(resname,"r");
	fscanf(fres,"%s%d",msg,&ret.points);
	return ret;
}
void tjdaupdate(int stid,tst_result t){
	Query qout=conn.query();
	qout<<"UPDATE submit_tjda SET grade="<<t.points<<" WHERE stid="<<stid;
	qout.exec();
}



//----------------------------And the main function---------------------------------

int print(){
	fprintf(stderr,"Finished sid:%d",sid);
	return 0;
}

int main(){
	char* anothertmpname=(char*)calloc(256,sizeof(char));;
	srand(time(0));
	sqlconnect();
	if(fetch()){
		if(!fetchtjdaprob()){
			tst_result t;
			if(spjid!=0){
				char* spjsrc=(char*)calloc(256,sizeof(char));
				mkrandnameendwith(spjsrc,".cpp");
				char* spjrun=(char*)calloc(256,sizeof(char));
				mkrandname(spjrun);
				compilespj(spjid,spjsrc,spjrun,conn2);
				runspj("/home/judge/test.in","/home/judge/test.out","/home/judge/test.std",spjrun,&t);
			}else{
				t=tjdajudgeonce("tester","test.out","test.std","test.res",grade);
			}
			tjdaupdate(tjda_stid,t);
		}
		return 0;
	}
	if(spjid!=0){
		mkrandname(spjname);
		mkrandnameendwith(anothertmpname,".cpp");
		compilespj(spjid,anothertmpname,spjname,conn3);
	}
	//addstatus(4096);//Touched
//	mkset();
	int cvv=compile();
	if(cvv!=2&&cvv!=8)return 0;
	run();
	print();
	closesqls();
	deletefiles();
}

