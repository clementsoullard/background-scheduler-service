#include <stdio.h>
#include <time.h>
#include <stdlib.h> 
#include <dirent.h>
#include <string.h>
/**
* Program pooling a directory, and doing a scheduling.
**/
int main (int argc, char** argv)
{
int nbSecond=getCoundownValue();
time_t before = time(NULL);
time_t whenItsComplete = time(NULL)+nbSecond;
int remainingSeconds;
do {
remainingSeconds=whenItsComplete-time(NULL);
int seconds=remainingSeconds%60;
int minutes=remainingSeconds/60;   
sleep(1);
printf("Clock %02d:%02d\n",minutes,seconds);
 } while ( remainingSeconds>0 );
printf("Compte à rebour expiré\n" );
}

/**
Get coundown value.
*/

int getCoundownValue(){
int nbSecond=0;
// Read the content of the directory /tmp/scheduler
DIR           *d;
struct dirent *dir;
char dirname[]="/tmp/scheduler";
d = opendir(dirname);

// Iterates on the files
if (d)
 {
  while ((dir = readdir(d)) != NULL)
   {

// if the file starts with CD then it is a countdown.

if (strstr(dir->d_name, "CD") != NULL) {
    printf("Countdown %s\n", dir->d_name);

// read the content of the file
char * buffer = 0;
long length;
char filename[255];
sprintf(filename,"%s/%s",dirname,dir->d_name);

FILE * f=fopen (filename, "rb");
printf("Testing %s\n",filename);
if (f)
{
  fseek (f, 0, SEEK_END);
  length = ftell (f);
  fseek (f, 0, SEEK_SET);
  /** The buffer stor the content of the file */
  buffer = malloc (length);
  if (buffer)
  {
    fread (buffer, 1, length, f);
	 nbSecond = atoi(buffer);
  }
  fclose (f);
}

if (buffer)
{
 printf(buffer);  // start to process your data / extract strings here...
}

}
  }
    closedir(d);
 }
 	 return nbSecond;

}