//Grupo 17
//André Nicolau 47880
//Alexandre Morgado 49015
//Jorge André 49496

#include <stdio.h>
#include <errno.h>
#include "inet.h"
#include "read_write_all.h"


int write_all(int sockfd,void *buf,int len){
    int bufsize = len;


    while (len > 0) {
      int res = write(sockfd,buf,len);
      if (res<0) {
        if (errno==EINTR) continue;
        return -1;
      }
      buf +=res;
      len -=res;
      
    }
    return bufsize;
}

int read_all(int sockfd,void *buf,int len){

  int bufsize = len;
  

  while (len > 0) {


    int res = read(sockfd,buf,len);
    
    if (res == 0) {
      return -1;
    }
    
    if (res < 0) {
      if (errno==EINTR) continue;
      return -1;
    }

    buf +=res;
    len -=res;

    
  }

  
  return bufsize;

}