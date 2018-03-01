#include<stdio.h>
#include"parse.h"
#include<stdlib.h>
#include<unistd.h>
#include<err.h>
#include<string.h>
#include<errno.h>
#include<sys/user.h>
#include<sys/resource.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<sys/stat.h>

#define BUFFER 1024
char name[1024];
int pathno;
extern char **environ;
char *builtin[12]={"bg","fg","cd","echo","jobs","kill","logout","nice","where","pwd","setenv","unsetenv"};
int redir,pip,redirin=0,redirout=0,redirerr=0;
int sigint=0;
int fd1[2],fd2[2];
int fdstdin,fdstdout,fdstderr;
char *errlist[2]={"Permission Denied","Command not found"};
pid_t mainID;
char*  hostname()
{
//	char name[1024];
	size_t len=1024;
	if(gethostname(name,len)<0)
	perror("");	//printf("Gethostname Error\n");
	else
	return (char *)name;
}

char** getpath(char **temp)
{	
	int status=1;
	pathno=0;
	char **path=(char **)malloc(sizeof(temp));
	char **work;
	*temp=strchr(*temp,'/');
	work=temp;
	while(status)
	{
		if(strchr(*temp,':')!=0)
		{
			path[pathno]=*work;
			*work=strchr(*temp,':');			
			**work='\0';
			*work=*work+1;
		}	
		else
		{
			path[pathno]=*work;
			status=0;
			pathno--;
		}
//		printf("here\n");
//		printf("work=%s\npath=%s\n",*work,path[pathno]);
		pathno++;
	}
	return path;	
}

int check_dir(char **arg)
{
	struct stat sb;
	stat(arg[0], &sb);
        if( S_ISDIR(sb.st_mode)!=0)
	{
		//path is a directory
		return 1;
	}
	else return 0;//path is not a directory
}
/*
int check_path(char **arg)
{
	char **rpath;
//	if(*arg[0]=='/')
//	rpath=arg;
		
	if(check_dir(arg)==0)
	{
		printf("%s\n",errlist[0]);
		return 0;
	}
	if(access(arg[0],X_OK)<0)
	{
		printf ("%s\n",errlist[0]);
		return 0;
	}
	else*//* if(access(arg[0],F_OK)<0)
	{
		printf ("from checkpath:%s\n",errlist[1]);
		return 0;
	}
	else if(check_dir==1)
	{
		printf("from checkdir: %s\n",errlist[0]);	
		return 0;
	}
	else 
	{
		rpath=getpath(arg,1);
 		int i,k=0;
		printf("%s: ",arg[0]);
		for(i=0;i<=pathno;i++)
		{
			char str[200];
			strcpy(str,rpath[i]);
			strcat(str,"/");
			strcat(str,arg[0]);
			printf("%s\n",str);
			if(access(str,X_OK)==0)
			{
				printf("%s/%s ",rpath[i],arg[1]);
				return 1;
			}
		}
		
		printf("from last checkpath: %s",errlist[0]);
		return 0;
	}

}*/
void exec_pwd()
{
	char wd[1024];
	if(strcmp(getcwd(wd,1024),"")==0)
	perror("");
	else
	printf("%s\n",wd);
//	if(redir==1)
//	exit(0);
}

void exec_echo(char **arg,int narg)
{
	int i;
	for(i=1;i<narg;i++)
	{
		if(arg[i][0]=='$')
		printf("%s ",getenv(arg[i]+1));
		else
		printf("%s ",arg[i]);
	}
	printf("\n");
}

void exec_cd(char **arg,int narg)
{
	int i;
	if(narg==1)
	{
		if(access((getenv("HOME")),R_OK)==0)
		chdir(getenv("HOME"));
	}
	else //if(check_dir(arg+1)==1)
	if (chdir(arg[1])<0)
	fprintf(stderr,"Permission denied\n");
//	if(redir==1)
//	exit(0);
//	printf("dir=%s",arg[0]);
}

void exec_setenv(char **arg, int narg)
{	
	if(narg==1)
	{
		extern char **environ;
		char **temp;
		for (temp = environ; *temp; temp++)
			printf ("%s\n", *temp);		
	}
		/*
		int i;
		for(i=0;strcmp(environ[i],"")!=0;i++)
		{
			printf("%s\n",environ[i]);
		}*/
//		printf ("\nthis is it");
	else if(narg==2)
	{
		setenv(arg[1],"",1);
//		printf("setenv successful");
	}
	else if(narg==3)
	{	
		setenv(arg[1],arg[2],1);
//		printf("setenv successful");
	}
	else
	printf("Invalid request\n");
//	if(redir==1)
//	exit(0);
}

void exec_unsetenv(char **arg)
{
	unsetenv(arg[1]);
//	if(redir==1)
//	exit(0);
//	printf("Unset successful");
//	else 
//	printf("Error in unset");
}

void exec_where(char **arg,int narg)
{
	char **temp=malloc(1024);
	char **temp1;
	char **paths;
//	char **path=malloc(sizeof(temp));
//	char **paths =(char **)malloc(1024*sizeof(char*));
//	char **work;// =(char **)malloc(1024*sizeof(char*));
	if(narg>1)
	{
		for(temp1=environ; *temp1!=NULL ;temp1++)
		{
//			printf("%s\n", *temp1);
//			printf("here\n");
	//		strcpy(*temp,*temp1);
			if(strncmp(*temp1,"PATH=",5)==0)
			{
				*temp=strdup(*temp1);
				*temp=strchr(*temp,'/');
//				printf("%s",*temp);
				paths=getpath(temp);
				break;
			}
		}
		int i,j,k=0;
		printf("%s: ",arg[1]);
		for(i=0;i<=pathno;i++)
		{
			char str[200];
			strcpy(str,paths[i]);
			strcat(str,"/");
			strcat(str,arg[1]);
//			printf("%s\n",str);
			if(access(str,R_OK)==0)
			printf("%s/%s ",paths[i],arg[1]);
		}
		for(j=0;j<=12;j++)
		{
			if(strcmp(arg[1],builtin[j])==0)
			k=1;//printf("built-in\n");
		}
		if(k==1)
		printf("It is a built-in command\n");
		else
		printf("\n");
//		free(path);
	}
	else
	{	
		printf("Invalid request\n");
	}
//	if(redir==1)
//	exit(0);	
}

void exec_nice(char **arg, int narg)
{
	int which=PRIO_PROCESS;
	pid_t who=getpid();
	if(narg==1)
	{	
		if(setpriority(which,who,4)<0)
		perror("");
//		if (redir==1)
//		exit(0);
	}
	else if(narg==2)
	{
		if(atoi(arg[1])==0 || atoi(arg[1])<(-19) || atoi(arg[1])>20)
		printf("Invalid syntax or priority value\n");
		else if(setpriority(which,who,atoi(arg[1]))<0)
		perror("");
//		printf("nice failure\n");
//		if(redir==1)
//		exit(0);
	}
	else if(narg>=3)
	{	
		pid_t ID;
		ID=fork();
		if(ID==-1)
		{
			err(EXIT_FAILURE,"error on fork()");
		}
		else if(ID==0)
		{
			if(atoi(arg[1])==0 || atoi(arg[1])<(-19) || atoi(arg[1])>20)
			printf("Invalid syntax or priority value\n");
			else
			{ 
				if(setpriority(which,getpid(),atoi(arg[1]))<0)
				perror("");
				else if(execvp(arg[2],arg+2)<0)
				perror("");
			}
		}
		else
		{
			wait(0);
//			exit(0);
		}
	}
}
	


void exec_default(char **arg,int narg)
{
//	if(redir==0)
//	{	
//		printf("exec vp with fork redir=%d\n",redir);
	//	if(check_path(arg)==1)
		
		pid_t ID;
		ID=fork();	
		if(ID==-1)
		{
			err(EXIT_FAILURE,"error on fork()");
		}
		else if(ID==0)
		{
			if(execvp(arg[0],arg)<0)
			fprintf(stderr,"permission denied\n");
			exit(0);
//			}
		}
		else
		{
			wait(0);
//			printf("parent redir=%d\n",redir);
//			exit(0);
		}
}

int isEnter(Pipe p)
{
 	if(p==NULL)
	return 1;
	else
	return 0;

}

int isLogout(Pipe p)
{
	if(strcmp(p->head->args[0],"logout")==0)
	return 1;
	else
	return 0;
}
void shell_execute(Pipe p)
{	
//	printf("command:%s\n", p->head->args[0]);	
//	printf("no of commands:%d\n", p->head->nargs);
//	printf("size of commands:%d\n", p->head->maxargs);

/*	 if(strcmp(p->head->args[0],"logout")==0)
	{
		return 1;
	}
	else*/ if(strcmp(p->head->args[0],"pwd")==0)
	{
		exec_pwd();
//		return 0;
	}
	else if(strcmp(p->head->args[0],"echo")==0)
	{
		exec_echo(p->head->args,p->head->nargs);
//		return 0;
	}
	else if(strcmp(p->head->args[0],"cd")==0)
	{
		exec_cd(p->head->args,p->head->nargs);
//		return 0;
	}
	else if(strcmp(p->head->args[0],"setenv")==0)
	{
		exec_setenv(p->head->args,p->head->nargs);
//		return 0;
	}
	else if(strcmp(p->head->args[0],"unsetenv")==0)
	{
		exec_unsetenv(p->head->args);
//		return 0;
	}
	else if(strcmp(p->head->args[0],"where")==0)
	{
		exec_where(p->head->args,p->head->nargs);
//		return 0;
	}
	else if(strcmp(p->head->args[0],"nice")==0)
	{
		exec_nice(p->head->args,p->head->nargs);
//		return 0;
	}
	else
	{
		exec_default(p->head->args,p->head->nargs);
//		return 0;
	}
}
redirect_out(Pipe p)
{
	redirout=1;
	int a=open(p->head->outfile,O_RDWR | O_CREAT | O_TRUNC,0664);
	dup2(a,1);
	close(a);
}
redirect_app(Pipe p)
{
	redirout=1;
	int a=open(p->head->outfile,O_RDWR| O_APPEND | O_CREAT,0664);
	dup2(a,1);
	close(a);
}

redirect_outerr(Pipe p)
{
	redirout=1;
	redirerr=1;
	int a=open(p->head->outfile,O_WRONLY| O_CREAT| O_TRUNC,0664);
	dup2(a,1);
	dup2(a,2);
	close(a);
}

redirect_apperr(Pipe p)
{
	redirout=1;
	redirerr=1;
	int a=open(p->head->outfile,O_RDWR| O_CREAT | O_APPEND,0664 );
	dup2(a,1);
	dup2(a,2);
	close(a);
}

redirect_outpipe(Pipe p)
{
	redirout=1;
//	printf("redir out pipe. redir=%d redirin=%d redirout=%d\n",redir,redirin,redirout);
	if(pip==0)
	{
		pipe(fd1);
//		printf("pipe fd1 piped\n");
		dup2(fd1[1],1);
		close(fd1[1]);
//		close(fd1[0]);
		pip++;
	}
	else if(pip==1)
	{
		pipe(fd2);
//		printf("pipe fd1 piped\n");
		dup2(fd2[1],1);
		close(fd2[1]);
//		close(fd2[0]);
		pip--;
	}
}

redirect_outpipeerr(Pipe p)
{
	redirout=1;
	redirerr=1;
	if(pip==0)
	{
		pipe(fd1);
		dup2(fd1[1],1);
		dup2(fd1[1],2);
		close(fd1[1]);
//		close(fd1[0]);
		pip++;
	}
	else if(pip==1)
	{
		pipe(fd2);
		dup2(fd2[1],1);
		dup2(fd2[1],2);
		close(fd2[1]);
//		close(fd2[0]);
		pip--;
	}
}

void reset_streams(void)
{
//	printf("reset\n");
	if(redirin==1)
	{
		dup2(fdstdin,0);	
		redirin=0;
	}
	if(redirout==1)
	{
//		printf("fdout=%d\n",fdout);
		dup2(fdstdout,1);	
		redirout=0;
	}	
	if(redirerr==1)
	{	
//		printf("fderre=%d\n",fderr);
		dup2(fdstderr,2);	
		redirerr=0;
	}
//	printf("reset\n");
}
/*
void fork_exec(Pipe p)
{
//	printf("redir=%d",redir);
	pid_t ID;
	ID=fork();	
	if(ID==-1)
	{
		err(EXIT_FAILURE,"error on fork()");
	}
	else if(ID==0)
	{
		shell_execute(p);
		exit(0);
	}
	else
	{
		wait(NULL);
		reset_streams();
//		exit(0);
	}
}*/
redirect_in(Pipe p)
{
	int a=open(p->head->infile,O_RDONLY,0664);
	dup2(a,0);
	close(a);
	redirin=1;	

	if(p->head->out==Tout)
	{
		redir=1;
		redirect_out(p);
	}
     	else if(p->head->out==Tapp)
	{
		redir=1;			
		redirect_app(p);
	}
	else if(p->head->out==ToutErr)
	{
		redir=1;
		redirect_outerr(p);
	}
    	else if(p->head->out==TappErr)
	{	
		redir=1;
		redirect_apperr(p);
	}
	else if(p->head->out==Tpipe)
	{
		redir=1;
		redirect_outpipe(p);
	}
	else if(p->head->out==TpipeErr)
	{
		redir=1;
		redirect_outpipeerr(p);
	}
//	fork_exec(p);
//	reset_streams();
}

redirect_inpipe(Pipe p)
{
	if(pip==1)
	{
//		pipe(fd1);
		dup2(fd1[0],0);
//		close(fd1[1]);
//		close(fd1[0]);
//		pip++;
	}
	else if(pip==0)
	{
//		pipe(fd2);
		dup2(fd2[0],0);
//		close(fd2[1]);
//		close(fd2[0]);
//		pip--;
	}
	redirin=1;
	if(p->head->out==Tout)
	{
		redir=1;
		redirect_out(p);
	}
	else if(p->head->out==Tapp)
	{
		redir=1;			
		redirect_app(p);
	}
	else if(p->head->out==ToutErr)
	{
		redir=1;
		redirect_outerr(p);
	}
	else if(p->head->out==TappErr)
	{	
		redir=1;
		redirect_apperr(p);
	}
	else if(p->head->out==Tpipe)
	{
		redir=1;
		redirect_outpipe(p);
	}
	else if(p->head->out==TpipeErr)
	{
		redir=1;
		redirect_outpipeerr(p);
	}
//	fork_exec(p);
}
redirect_inpipeerr(Pipe p)
{
	if(pip==1)
	{
//		pipe(fd1[2]);
		dup2(fd1[0],0);
		dup2(fd1[0],2);
//		close(fd1[0]);
//		pip++;
	}
	else if(pip==0)
	{
//		pipe(fd2[2]);
		dup2(fd2[0],0);
		dup2(fd2[0],2);
//		close(fd2[0]);
//		pip--;
	}
	redirin=1;
	if(p->head->out==Tout)
	{
		redir=1;
		redirect_out(p);
	}
	else if(p->head->out==Tapp)
	{
		redir=1;			
		redirect_app(p);
	}
	else if(p->head->out==ToutErr)
	{
		redir=1;
		redirect_outerr(p);
	}
	else if(p->head->out==TappErr)
	{	
		redir=1;
		redirect_apperr(p);
	}
	else if(p->head->out==Tpipe)
	{
		redir=1;
		redirect_outpipe(p);
	}
	else if(p->head->out==TpipeErr)
	{
		redir=1;
		redirect_outpipeerr(p);
	}
//	fork_exec(p);
}
redirect_innil(Pipe p)
{
	redirin=0;
//	printf("redir in=%d",redirin);
	if(p->head->out==Tout)
	{
		redir=1;
		redirect_out(p);
	}
     	else if(p->head->out==Tapp)
	{
		redir=1;			
		redirect_app(p);
	}
	else if(p->head->out==ToutErr)
	{	
		redir=1;
		redirect_outerr(p);
	}
	else if(p->head->out==TappErr)
	{	
		redir=1;
		redirect_apperr(p);
	}
	else if(p->head->out==Tpipe)
	{
		redir=1;
		redirect_outpipe(p);
	}
	else if(p->head->out==TpipeErr)
	{
		redir=1;
		redirect_outpipeerr(p);
	}
	else
	{	
		redir=0;
//		shell_execute(p);
	}
//	if(redir==1)
//	fork_exec(p);
	
}

void isRedir(Pipe p)
{
	if(p->head->in==Tin)// && p->head-out==Tnil)
	{
//		redir=1;
		redirect_in(p);
//		return 1;
	}
	else if(p->head->in == Tpipe)
	{	
//		redir=1;
		redirect_inpipe(p);
		//read from previous pipe output i.e dup to previuos pipe's output
	}
	else if(p->head->in==TpipeErr)
	{
//		redir=1;
		redirect_inpipeerr(p);
	}
	else if(p->head->in==Tnil)
	{
//		redir=1;
		redirect_innil(p);	
	}
}

void shell_prompt(int isUshrc)
{
	Pipe p;
	while(1)
	{
		if(!isUshrc && isatty(0))
		printf("%s%% ",hostname()); //Shell prompt
		fflush(stdout);
		redir=0;
		fdstdin=dup(0);
		fdstdout=dup(1);
		fdstderr=dup(2);
		pip=0;
		p=parse();
		
		if(isUshrc==1)
		{
			if(!strcmp(p->head->args[0],"end"))
			break;
		}
		
		if(!isEnter(p))
		{
			
			if(!isLogout(p))
			{	
				if (strcmp(p->head->args[0],"end")!=0) 
				{
					while(p!=NULL) 
					{
						while(1)
						{
//							mainID=getpid();
							isRedir(p);
							shell_execute(p);
							reset_streams();
//							printf("redir done\n");
							p->head=p->head->next;
							if(p->head==NULL || !strcmp(p->head->args[0],"end"))
							break;
						}
						p = p->next;
					}
				}
				else
				exit(0);
			}
			else
			exit(0);
		}
	}
}
void read_ushrc(void)
{
	char ush_add1[200],*ush_add2,*ush_add;
	int ush=0,fdushin;
	
	strcpy(ush_add1,strcat(getenv("HOME"),"/.ushrc"));
//	ush_add2="./.ushrc";
	//printf("%s\n%s",ush_add1,ush_add2);
	if(!access(ush_add1,R_OK))
	{
		ush=1;
		ush_add=ush_add1;
	}
//	else if(!access(ush_add2,R_OK))
//	{
//		ush=1;
//		ush_add=ush_add2;
//	}
//	else
//	{
//		printf("./ushrc not found!\n");
//	}
	if(ush)
	{
		int a=open(ush_add,O_RDONLY);
		fdushin=dup(0);
		dup2(a,0);
		close(a);
//		while(read()!=0)
		shell_prompt(ush);
		dup2(fdushin,0);
	}
}

void handle_sigterm(int signo)
{
	pid_t ID=getpid();
	if(signo==SIGTERM)
	{
		if(ID!=mainID)
		exit(0);
	}
}
void handle_sigint(int signo)
{
	if(signo==SIGINT)
	printf("\n");	
//	printf("%s%% ",hostname()); //Shell prompt

}
	
void main(int argNum, char **args)
{
//	Pipe p;
	mainID=getpid();
	signal(SIGINT,handle_sigint);
	signal(SIGTERM,handle_sigterm);
	read_ushrc();
//	while(1)
	shell_prompt(0);

}
