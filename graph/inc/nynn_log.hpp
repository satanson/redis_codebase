#ifndef NYNN_LOG_HPP_BY_SATANSON
#define NYNN_LOG_HPP_BY_SATANSON
#include<linuxcpp.hpp>
using namespace std;
namespace nynn{
enum{
	//log level
	LOG_INFO=0,
	LOG_WARN=1,
	LOG_ERROR=2,
	LOG_ASSERT=3,
	LOG_DEBUG=4,
	LOG_EXCEPTION=5,
	ERR_BUFF_SIZE=256,
	VSNPRINTF_BUFF_SIZE=256
};

string err(int errnum);
string now();
void vlog(
		ostream &out,
		const int level,
		const char* file,
		const int line,
		const char* func,
		int errnum,
		const char* fmt,
		va_list ap
		);
void log_debug(
		ostream&out,
		const int level,
		const char* file,
		const int line,
		const char* func,
		int errnum,
		const char* fmt,
		...
		);
void log(
		ostream& out,
		const int level,
		const char* file,
		const int line,
		const char* func,
		int errnum,
		const char* fmt,
		...
		);

#define log_i(msg,...)\
	do{\
		log(cerr,LOG_INFO,__FILE__,__LINE__,__FUNCTION__,0,(msg),##__VA_ARGS__);\
	}while(0);

#define log_w(msg,...)\
	do{\
   		log(cerr,LOG_WARN,__FILE__,__LINE__,__FUNCTION__,0,(msg),##__VA_ARGS__);\
	}while(0);

#define log_e(errnum,msg,...)\
	do{\
		log(cerr,LOG_ERROR,__FILE__,__LINE__,__FUNCTION__,errnum,(msg),##__VA_ARGS__);\
		exit(0);\
	}while(0);

#define log_a(boolexpr,msg,...)\
	do{\
		log(cerr,LOG_ASSERT,__FILE__,__LINE__,__FUNCTION__,0,(msg),##__VA_ARGS__);\
		assert(boolexpr);\
	}while(0);

#define log_d(msg,...)\
	do{\
   		log(cerr,LOG_DEBUG,__FILE__,__LINE__,__FUNCTION__,0,(msg),##__VA_ARGS__);\
	}while(0);


inline string err(int errnum)
{
	char buff[ERR_BUFF_SIZE];
	string s;
	s.reserve(ERR_BUFF_SIZE);
	memset(buff,0,ERR_BUFF_SIZE);
#if (POSIX_C_SOURCE >= 200112L || XOPEN_SOURCE >= 600) && ! GNU_SOURCE
	//XSI-compliant
	if (strerror_r(errnum,buff,ERR_BUFF_SIZE)==0)
		s=buff;
	else
		s="'UNKNOWN_ERROR'";
#else
	//GNU-specific
	char *msg=strerror_r(errnum,buff,ERR_BUFF_SIZE);
	if (msg!=NULL)
		s=msg;
	else
		s="'UNKNOWN_ERROR'";
#endif
	return s;
}
string now()
{
	time_t t;
	struct tm tm0;
	char buff[64];
	time(&t);
	size_t n=strftime(buff,64,"%Y-%m-%d %T",localtime_r(&t,&tm0));
	buff[n]=0;
	return string(buff);
}
inline void log_debug(
		ostream& out,
		const int level,
		const char* file,
		const int line,
		const char* func,
		int errnum,
		const char* fmt,
		...
		)
{
#ifdef DEBUG
	va_list ap;
	va_start(ap,fmt);
	vlog(out,level,file,line,func,errnum,fmt,ap);
	va_end(ap);
#endif
}

inline void log(
		ostream& out,
		const int level,
		const char* file,
		const int line,
		const char* func,
		int errnum,
		const char* fmt,
		...
		)
{
	va_list ap;
	va_start(ap,fmt);
	vlog(out,level,file,line,func,errnum,fmt,ap);
	va_end(ap);

}

inline void vlog(
		ostream& out,
		const int level,
		const char* file,
		const int line,
		const char* func,
		int errnum,
		const char* fmt,
		va_list ap)
{
	const char* logs[6]={"INFO","WARN","ERROR","ASSERT","DEBUG","EXCEPTION"};
	stringstream pack;
	string s;
	//char abspath[VSNPRINTF_BUFF_SIZE];
	//realpath(file,abspath);
	char what[VSNPRINTF_BUFF_SIZE];
	vsnprintf(what,VSNPRINTF_BUFF_SIZE,fmt,ap);
	what[VSNPRINTF_BUFF_SIZE-1]='\0'; 

	pack<<"{ "<<"\""<<logs[level]<<"\":{ "
		<<"\"time\":"<<"\""<<now()<<"\", "
		<<"\"file\":"<<"\""<<file<<"\", "
		<<"\"line\":"<<"\""<<line<<"\", "
		<<"\"func\":"<<"\""<<func<<"\", "
		<<"\"errno\":"<<"\""<<errnum<<"\", "
		<<"\"err\":"<<"\""<<err(errnum)<<"\", "
		<<"\"what\":"<<"\""<<what<<"\" }}";
	out<<pack.str()<<endl<<endl;
}
}
#endif
