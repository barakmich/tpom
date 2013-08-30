#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

const int kDefaultLengthSecs = 25 * 60;

char* MakeSocketName() {
  char* username = getenv("USER");
  char* socket_name =  (char*) calloc(strlen(username) + strlen(P_tmpdir) + 40, sizeof(char));
  strcat(socket_name, P_tmpdir);
  strcat(socket_name, "/.tpm-");
  strcat(socket_name, username);
  return socket_name;
}

int DaemonMain(int countdown_time) {
        /* Our process ID and Session ID */
        int ok;
        pid_t pid, sid;
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 1000 /*micro*/ * 1000 /* milli */ * 50;

        char * socket_name = MakeSocketName();
    
        int sock_fd;
        struct sockaddr_un local, remote;

        if ((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
          perror("socket");
          exit(1);
        }
        local.sun_family = AF_UNIX;
        strcpy(local.sun_path, socket_name);
        unlink(local.sun_path);
        free(socket_name);
        int len = strlen(local.sun_path) + sizeof(local.sun_family);
        if (bind(sock_fd, (struct sockaddr *)&local, len) == -1) {
          perror("bind");
          exit(1);
        }
        int flags = fcntl(sock_fd,F_GETFL,0);
        fcntl(sock_fd, F_SETFL, flags | O_NONBLOCK);

        /* Fork off the parent process */
        pid = fork();
        if (pid < 0) {
                exit(EXIT_FAILURE);
        }
        /* If we got a good PID, then
           we can exit the parent process. */
        if (pid > 0) {
                exit(EXIT_SUCCESS);
        }

        /* Change the file mode mask */
        umask(0);
                
        /* Open any logs here */        
                
        /* Create a new SID for the child process */
        sid = setsid();
        if (sid < 0) {
                /* Log the failure */
                exit(EXIT_FAILURE);
        }
        
        
        /* Change the current working directory */
        if ((chdir("/")) < 0) {
                /* Log the failure */
                exit(EXIT_FAILURE);
        }
        
        /* Close out the standard file descriptors */
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        
        if (listen(sock_fd, 5) == -1) {
          exit(1);
        }

        
        /* The Big Loop */
        int wakeup = 0;
        struct timeval start_time;
        ok = gettimeofday(&start_time, NULL);
        int remote_fd = -1;

        while (!wakeup) {
           struct timeval current_time;
           struct timeval diff_time;
           ok = gettimeofday(&current_time, NULL);
           timersub(&current_time, &start_time, &diff_time);
           if (diff_time.tv_sec >= countdown_time) {
             wakeup = 1;
             continue;
           }
           long remaining_time = (long)countdown_time - diff_time.tv_sec;
           socklen_t t = sizeof(remote);
           while ((remote_fd = accept(sock_fd, (struct sockaddr *)&remote, &t)) != -1) {
             char message[100];
             int message_len = snprintf(message, 100, "%ld:%02ld", remaining_time / 60, remaining_time % 60);
             if (send(remote_fd, message, message_len, 0) < 0) {
              exit(1);
             }
             close(remote_fd);
           }
           // Make sure it was just EWOULDBLOCK
           if (errno != EAGAIN && errno != EWOULDBLOCK) {
             exit(1);
           }
           ok = nanosleep(&ts, NULL);
        }
   unlink(local.sun_path);
   exit(EXIT_SUCCESS);
}

int main(int argc, char** argv) {
    int c;
    int countdown_time = kDefaultLengthSecs;
    while ( (c = getopt(argc, argv, "bsm")) != -1) {
        switch (c) {
        case 's':
            countdown_time = atoi(optarg);
            break;
        case 'm':
            countdown_time = atoi(optarg) * 60;
            break;
        }
    }
    if (optind < argc) {
        char* command = argv[optind];
        if (strcmp(command, "start") == 0) {
          DaemonMain(countdown_time);
        }
    } else {
      return DaemonMain(25);
    }
    exit (0);   
}
