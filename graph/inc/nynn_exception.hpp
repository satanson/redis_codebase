#ifndef NYNN_EXCEPTION_HPP_BY_SATANSON
#define NYNN_EXCEPTION_HPP_BY_SATANSON
#include<linuxcpp.hpp>
#include<nynn_log.hpp>
using namespace std;
using namespace nynn;
namespace nynn{

#define throw_nynn_exception(errnum,msg) \
   	do{\
		throw nynn_exception_t(__FILE__,__LINE__,__FUNCTION__,errnum,(msg));\
	}while(0);

class nynn_exception_t:public exception{
public:
	enum {
		BACKTRACE_FRAME_SIZE=20
	};	
	nynn_exception_t(
			const char* file,
			const int line,
			const char *func,
			const int errnum,
			const string& msg)
	{
		stringstream pack;
		log(pack,LOG_EXCEPTION,file,line,func,errnum,msg.c_str());
		m_msg=pack.str();
		m_framesize=backtrace(m_frames,BACKTRACE_FRAME_SIZE);
	}
	~nynn_exception_t()throw(){}

	const char* what()const throw()
	{
		return m_msg.c_str();
	}

	void printBacktrace(){
		char **symbols=backtrace_symbols(m_frames,m_framesize);
		cerr<<"backtrace:"<<endl;
		if (symbols!=NULL) {
			for (size_t i=0;i<m_framesize;i++)
				cerr<<symbols[i]<<endl;
		}
		free(symbols);
	}

private:
	string m_msg;
	void*  m_frames[BACKTRACE_FRAME_SIZE];
	size_t m_framesize;
};

}
#endif
