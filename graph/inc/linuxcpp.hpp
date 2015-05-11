#ifndef LINUX_CPP_HPP_BY_SATANSON 
#define LINUX_CPP_HPP_BY_SATANSON

#include<iostream>
#include<fstream>
#include<sstream>
#include<iomanip>
#include<memory>
#include<string>
#include<vector>
#include<list>
#include<set>
#include<map>
#include<unordered_set>
#include<unordered_map>
#include<exception>
#include<algorithm>

#include<cstring>
#include<cstdlib>
#include<cerrno>
#include<cstdarg>
#include<cassert>
#include<cstdio>
#include<ctime>
#include<csignal>
#include<climits>
#include<cstddef>

#include<sys/stat.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/mman.h>
#include<sys/shm.h>
#include<sys/sem.h>
#include<sys/socket.h>
#include<sys/eventfd.h>
#include<sys/time.h>
#include<sys/signalfd.h>
#include<sys/timerfd.h>
#include<sys/epoll.h>


#include<unistd.h>
#include<fcntl.h>
#include<execinfo.h>
#include<glob.h>
#include<netdb.h>

#include<arpa/inet.h>
using namespace std;
#if (defined __GNUC__)
#define likely(x) __builtin_expect((x),1)
#define unlikely(x) __builtin_expect((x),0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif
#endif
