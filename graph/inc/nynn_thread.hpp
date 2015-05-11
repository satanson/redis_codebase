#ifndef NYNN_THREAD_HPP_BY_SATANSON
#define NYNN_THREAD_HPP_BY_SATANSON
#include<linuxcpp.hpp>
#include<nynn_log.hpp>
#include<nynn_exception.hpp>
using namespace std;
using namespace nynn;
namespace nynn{

class ring_t{
private:
	int efd;
public:
	ring_t(){
		efd=eventfd(0,0);
		if(efd==-1)throw_nynn_exception(errno,"failed to initialize ring_t");
	}
	~ring_t(){close(efd);}
	void wait(){
		uint64_t val;
		if(read(efd,&val,sizeof(val))!=sizeof(val))
			throw_nynn_exception(errno,"failed to read from eventfd");
	}
	void notify(){
		uint64_t val=1ull;
		if(write(efd,&val,sizeof(val))!=sizeof(val))
			throw_nynn_exception(errno,"failed to write to eventfd");
	}
};


class thread_t{
public:

	enum {SIGNAL_STOP_THREAD=SIGUSR1};
	typedef void*(*func)(void*);
	typedef struct {
		func f;
		void* args;
		ring_t ring0;
		ring_t ring1;
	}fargs_t;
	
	static void stop_handler(int){
		log_i("teminate thread");
		pthread_exit(NULL);
	}

	static void add_stop_listener(){
		struct sigaction sigact;
		sigact.sa_handler=&stop_handler;
		sigfillset(&sigact.sa_mask);
		sigaction(SIGNAL_STOP_THREAD,&sigact,NULL);
	}
	static void* decorate(void*decorated){
		try{ 
			fargs_t* fargs=(fargs_t*)decorated;	

			sigset_t  sigs;
			pthread_sigmask(SIG_SETMASK,NULL,&sigs);
			sigaddset(&sigs,SIGINT);
			sigaddset(&sigs,SIGQUIT);
			sigdelset(&sigs,SIGTERM);
			sigdelset(&sigs,SIGNAL_STOP_THREAD);
			pthread_sigmask(SIG_SETMASK,&sigs,NULL);
			
			log_i("create a thread");
			add_stop_listener();
			fargs->ring0.notify();
			fargs->ring1.wait();
			(*fargs->f)(fargs->args);
		}catch(nynn_exception_t& ex){
			cout<<ex.what();
		}
	}

	thread_t(func f,void*args){
		decorated.f=f;
		decorated.args=args;
		int errnum=pthread_create(&id,NULL,decorate,&decorated);
		if (errnum!=0)throw_nynn_exception(errnum,"failed to create thread via pthread_create");
		decorated.ring0.wait();
	}
	void start(){
		decorated.ring1.notify();
	}
	void stop(){
		int errnum=pthread_kill(id,SIGNAL_STOP_THREAD);
		if (errnum!=0)throw_nynn_exception(errnum,"failed to stop thread");
	}
	void kill(int signum){
		int errnum=pthread_kill(id,signum);
		if (errnum!=0)throw_nynn_exception(errnum,"failed to kill thread");
	}
	bool is_alive(){
		int errnum=pthread_kill(id,0);
		if (errnum==0)return true;
		else return false;
	}
	void join(){
		int errnum=pthread_join(id,NULL);
		if (errnum!=0)throw_nynn_exception(errnum,"failed to join thread via pthread_create");
	}
	pthread_t thread_id()const{return id;}
	~thread_t(){}
private:
	thread_t(const thread_t& thr);
	thread_t& operator=(const thread_t& thr);
	pthread_t id;
	fargs_t decorated;
};

}
#endif
