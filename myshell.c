#include <sys/utsname.h> 
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/time.h>
#include<dirent.h>
#define MAX 150;
#include<fcntl.h>
#include<pwd.h>
#include<grp.h>
#include<time.h>
#define MINOR_BITS 8
#define MAJOR(dev)   ((unsigned)dev>>MINOR_BITS)
#define MINOR(dev)   (dev & MINOR_BITS)
char xtbl[10]="rwxrwxrwx";
struct dirent *mydir;
char *mycmd[100];
int argcount=0;
DIR *p;


//the following are for echo command which takes various options as shown
/* If defined, interpret backslash escapes unless -E is given.
   V9_ECHO must also be defined.  */
/* #define V9_DEFAULT */

#if defined (V9_ECHO)
# if defined (V9_DEFAULT)
#  define VALID_ECHO_OPTIONS "neE"
# else
#  define VALID_ECHO_OPTIONS "ne"
# endif /* !V9_DEFAULT */
#else /* !V9_ECHO */
# define VALID_ECHO_OPTIONS "n"
#endif /* !V9_ECHO */


char *program_name;

//here v9 is used to know whether its for system v9 unix or linux distro bcuz v9 diesnt allow some 
//options 


void readcurdir(void)
{
	int j;
	if(argcount!=1)
	{
		printf("Max only one arguments\n");
		exit(0);
	}


	p=opendir(".");
	while((mydir=readdir(p))!=NULL)
	{
		printf("%s \t",mydir->d_name);
	}
	printf("\n");
	return ;
}

int muname()
{
	struct utsname bf;
	uname(&bf);
	puts(bf.sysname);
	puts(bf.nodename);
	puts(bf.release);
	puts(bf.machine);
	puts(bf.version);
	#ifdef _GNU_SOURCE	
	puts(bf.domainname);
	#endif
	return 0;
}
void copyit(void)
{
	int fd,fd1;
	char buf[100];
	if(argcount<3)
	{
		printf("parameters error\n");
		exit(0);
	}
	
	fd = open(mycmd[1],O_RDONLY);

	if(fd==-1)
	{
	printf("Cant continue \n");	
	exit(0);
	}

	fd1=creat(mycmd[2],O_CREAT|O_TRUNC|O_APPEND|S_IRWXU);
	
	while(read(fd,&buf,1)!=0)
	{
		write(fd1,&buf,1);
	}

}

void pwd(void)
{
	char cwd[1024];
	if(argcount!=1)
	{
		printf("pwd takes only one argument\n");
		return ;
	}

	if(getcwd(cwd,sizeof(cwd))!=NULL)
	printf("%s \n",cwd);
	
}

void rmmdir()
{
	int i;	


	if(argcount<2)
	{
		printf("Min two arguments \n");
		return ;
	}	
	if(argcount>=2)
	{
		i=1;
		while(i!=argcount)
		{	
			if((rmdir(mycmd[i]))==-1)
			{
				printf("Cant remove diractory --not empty\n");
				exit(0);
			}
			i++;
		}
	}
}


void link1(void)
{
		if((strcmp(mycmd[1],"-s"))!=0)
	{
		if((link(mycmd[1],mycmd[2]))==-1)
		{
			printf("hard link failure \n");
			exit(0);
		}
		
	}
	
	else
	{
		if((symlink(mycmd[2],mycmd[3]))==-1)
		{
			printf("soft link failure \n");
			exit(0);
		}
		
	}

}
void unlink1(void)
{
	if((unlink(mycmd[1]))==-1)
	{
		printf("not possibledue to unlink\n");
		exit(0);
	}
}

void mmkdir()
{
	if(mkdir(mycmd[1],S_IRWXU)==-1)
	{
		printf("Sorry cant create directory \n");
		exit(0);
	}
}

/* grep: search for regexp in file */
int grep(char *regexp, FILE *f, char *name)
{

//	printf("in grep");
	int n, nmatch;
	char buf[BUFSIZ];
	printf("in grep\n");
	nmatch = 0;
	while (fgets(buf, sizeof buf, f) != NULL) {
		n = strlen(buf);
		if (n > 0 && buf[n-1] == '\n')
			buf[n-1] = '\0';
		if (match(regexp, buf)) {
			nmatch++;
			if (name != NULL)
				printf("%s:", name);
			printf("%s\n", buf);
		}
	}
	return nmatch;
}
void display_access_perm(int st_mode)
{
	char amode[10];
	int i,j;
	for(i=0,j=(1<<8);i<9;i++,j>>=1)
		amode[i]=(st_mode&j)?xtbl[i]:'-';
	if(st_mode&S_ISUID)amode[2]=(amode[2]=='x')?'S':'s';
	
	if(st_mode&S_ISGID)amode[5]=(amode[5]=='x')?'G':'g';
	
	if(st_mode&S_ISVTX)amode[8]=(amode[8]=='x')?'T':'t';

	printf("%s ",amode);

}

void display_file_type(int st_mode)
{
	switch(st_mode & S_IFMT){
	case  S_IFDIR : printf("d"); return;		
	case  S_IFCHR : printf("c"); return;		
	case  S_IFBLK : printf("b"); return;		
	case  S_IFREG : printf("-"); return;		
	case  S_IFLNK : printf("l"); return;		
	case  S_IFIFO : printf("p"); return;		
	}
}
/* matchhere: search for regexp at beginning of text */
int matchhere(char *regexp, char *text)
{
	if (regexp[0] == '\0')
		return 1;
	if (regexp[1] == '*')
		return matchstar(regexp[0], regexp+2, text);
	if (regexp[0] == '$' && regexp[1] == '\0')
		return *text == '\0';
	if (*text!='\0' && (regexp[0]=='.' || regexp[0]==*text))
		return matchhere(regexp+1, text+1);
	return 0;
}

/* match: search for regexp anywhere in text */
int match(char *regexp, char *text)
{
	if (regexp[0] == '^')
		return matchhere(regexp+1, text);
	do {	/* must look even if string is empty */
		if (matchhere(regexp, text))
			return 1;
	} while (*text++ != '\0');
	return 0;
}

/* matchstar: search for c*regexp at beginning of text */
int matchstar(int c, char *regexp, char *text)
{
	do {	/* a * matches zero or more instances */
		if (matchhere(regexp, text))
			return 1;
	} while (*text != '\0' && (*text++ == c || c == '.'));
	return 0;
}



void move1(void)
{
	if(argcount!=3 || !strcmp(mycmd[1],mycmd[2]))
	{
		printf("arguments error\n");
		return ;
	}
	
	else if(link(mycmd[1],mycmd[2]))
	{
		unlink(mycmd[1]);
	}
	return ;
}




void readi(char *input)
{

	char **p;
	char *token = strtok(input," ");
	p=&mycmd[0];
	//argcount++;
	while(token!=NULL)
	{
		*p=token;
		p++;
		token=strtok(NULL," ");
		argcount++;		
		
	}
}


void timeit(void)
{
	int k;
	if(!access(mycmd[1],F_OK))	
	{
		utime(mycmd[1],NULL);
	}
	else
	if((k=creat(mycmd[1],0664))==-1)
	{
		printf("cant create!!\n");
		_exit(0);
	}	
		

}	

void whoami(void)
{
	uid_t uid1;
	struct passwd *pass1;
	if(argcount!=1)
	{		
		printf("Error in i/p\n");
		_exit(0);
	}

	
	uid1=geteuid();
	pass1=getpwuid(uid1);
	



	
	printf("%s\n",pass1->pw_name);
//	printf("%s\n",pass1->pw_passwd);
}

int grepit(void)
{
	int i, nmatch;
	FILE *f;
	printf("%d",argcount);
	if (argcount < 2)
		printf("usage: grep regexp [file ...]");
	nmatch = 0;
	if (argcount == 2) {
		if (grep(mycmd[1], stdin, NULL))
			nmatch++;
	} 
	else {
		for (i = 2; i < argcount; i++) {
			f = fopen(mycmd[i], "r");
			if (f == NULL) {
				wprintf("can't open %s:", mycmd[i]);
				continue;
			}
			if (grep(mycmd[1], f, argcount>3 ? mycmd[i] : NULL) > 0)
				nmatch++;
			fclose(f);
		}
	}
	return nmatch == 0;
}


void rename1(void)
{
	if(argcount<2)
	{
		printf("Min 2 args\n");
		exit(0);
	}

	if((rename(mycmd[1],mycmd[2]))==-1)
	{
		printf("rename failure!!!");	
		exit(0);
	}
}

void change(void)
{
	if((chdir(mycmd[1])==-1))
	{
		perror("error\n");
		return ;
	}
}		
void clear_screen()
{
	printf("\033[H\033[J");

}

void printit(void)
{
	 int display_return = 1, do_v9 = 0;
  int allow_options = 1;
	int kk=0;
  program_name = mycmd[kk];
 

 #if defined (V9_ECHO) && defined (V9_DEFAULT)
  do_v9 = allow_options;
#endif

  --argcount;
  ++kk;
{
		int ll=1;
	  while (argcount > 0)
	    {
		fputs (mycmd[ll], stdout);
	      argcount--;
	      ll++;
	      if (argcount > 0)
		putchar (' ');
	    }
	
    }
  if (display_return)
    putchar ('\n');
  exit (EXIT_SUCCESS);
}

void yes1(void)
{
	int i,l;
	printf("yes in 1");
	if(argcount==1)
	{	while(10)
		printf("y\n");
	}
	while(1)
	{
		for(i=1,l=1;i<argcount;i++,l++)
		{
			printf("%s",mycmd[l]);
		}		
		printf("\n");
		i=1;l=1;
	}	
	    
	
}
void compare(void)
{
	
	char buf1[10240],buf2[10240];
	struct stat f1,f2;
	int i=0;
	int fd1,fd2;
	char ch1,ch2;
	fd1=open(mycmd[1],O_RDONLY);
	fd2=open(mycmd[2],O_RDONLY);
	
	stat(mycmd[1],&f1);
	stat(mycmd[2],&f2);


	read(fd1,&buf1,f1.st_size);
	read(fd2,&buf2,f2.st_size);

	
	while(i!=f1.st_size)
	{
		if(buf1[i]==buf2[i])
		{
			i++;
		}
		else
		{
			printf("Files differ at position %d\n",i+1);
			printf("cya bye\n");
			return;
		}
	}
	

	printf("Files ar eequal\n");
}
void cathi1()
{
		char buf[1026];
		FILE *fp1;
		FILE *fp2;
		fp1=fopen(mycmd[1],"r");

		if(access(mycmd[3],F_OK))	
		{	
		fp2=fopen(mycmd[3],"w");
		}
		else
		{
		printf("%snot exists\n",mycmd[3]);
		creat(mycmd[2],0644);
		fp2=fopen(mycmd[3],"w");
		}
		

		while(!(feof(fp1)))
		{
			fgets(buf,60,fp1);
			fputs(buf,fp2);
		}			



return ;
}



void catit(void)
{
	char buf[1024];
		FILE *fp1;
			

			
	if(argcount==2)	
	{
		fp1=fopen(mycmd[1],"r");
	

		while(!feof(fp1))			
		{
			fgets(buf,60,fp1);		
			fputs(buf,stdout);
		}
	}
}

void listall(void)
{
	char xtbl[10]="rwxrwxrwx";
		struct stat statv;
		struct group *gr_p;
		struct passwd *pw_p;
		
		if(lstat(mycmd[1],&statv))
		{
			printf("Invalid name\n");
			return ;
		}


	
		display_file_type(statv.st_mode);
		display_access_perm(statv.st_mode);

		printf("%d",statv.st_nlink);
		
		
		gr_p = getgrgid(statv.st_gid);
		pw_p = getpwuid(statv.st_uid);
		

		if(pw_p->pw_name)
		printf("%s  ",pw_p->pw_name);
		else
		printf("%d  ",statv.st_uid);


		if(gr_p->gr_name)
		printf("%s  ",gr_p->gr_name);
		else
		printf("%d  ",statv.st_gid);


		
		if((statv.st_mode & S_IFMT) == S_IFCHR ||(statv.st_mode & S_IFMT) == S_IFBLK )
		{
			printf("%i,%i",MAJOR(statv.st_rdev),MINOR(statv.st_rdev));
		}
		else
		{
			printf("%ld",statv.st_size);
		}

		printf("%s\b",ctime(&statv.st_mtime));
		printf("%s\n",mycmd[1]);

}

void mydate()
{
	time_t now=time(0);
	char *dt=ctime(&now);
	printf("%s\n",dt);
}

int main(int argc,char **argv)
{
	char input[100];
	int j;
	char cwd1[1024];
	
	while(1)
	{	
		argcount=0;
		if(getcwd(cwd1,sizeof(cwd1))!=NULL)
		printf("mash@manikandan:~/%s$",cwd1);			
		gets(input);
		readi(input);
	
//		printf("%s",mycmd[2]);
		
		if(!(strcmp(mycmd[0],"pwd")))
		pwd();
	
		else if(!(strcmp(mycmd[0],"mkdir")))	

		mmkdir();

		
		else if(!(strcmp(mycmd[0],"rmdir")))
		rmmdir();

		else if(!(strcmp(mycmd[0],"link")))
		link1();

		else if(!(strcmp(mycmd[0],"unlink")))
		unlink1();
		
		else if(!(strcmp(mycmd[0],"touch")))
		timeit();

		else if(!(strcmp(mycmd[0],"copy")))
		copyit();
	
		else if(!(strcmp(mycmd[0],"whoami")))
		whoami();

		else if(!(strcmp(mycmd[0],"mv")))
		move1();	
			
		
		else if(!(strcmp(mycmd[0],"ll")))
		listall();	

		else if(!(strcmp(mycmd[0],"rename")))
		rename1();

		else if(!(strcmp(mycmd[0],"ls")))
		readcurdir();		

		else if(!(strcmp(mycmd[0],"clear")))
		clear_screen();
		
		
		else if(!(strcmp(mycmd[0],"cmp")))
		compare();

		else if(!(strcmp(mycmd[0],"grep")))		
		grepit();
		
		else if(!(strcmp(mycmd[0],"yes")))
		yes1();		
		
		else if(!(strcmp(mycmd[0],"cd")))
		change();		

		else if(!(strcmp(mycmd[0],"date")))
		mydate();	
		
		else if(!(strcmp(mycmd[0],"echo")))
		printit();

		else if(!(strcmp(mycmd[0],"uname")))
		muname();

		else if(!(strcmp(mycmd[0],"cat")))
		{
				catit();		
		}

		else if(!(strcmp(mycmd[0],"exit")))
		exit(0);

		
		


	}
	return 0;

}



	
		





