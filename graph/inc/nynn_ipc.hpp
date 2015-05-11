#ifndef NYNN_IPC_HPP_BY_SATANSON
#define NYNN_IPC_HPP_BY_SATANSON
#include<nynn_exception.hpp>
#include<nynn_util.hpp>
using namespace std;
using namespace nynn;
#if not defined __x86_64__
typedef unsigned long long int uint64_t;
typedef long long int int64_t;
typedef unsigned short int uint16_t;
typedef short int int16_t;
typedef unsigned char uint8_t;
#endif

#ifdef __x86_64__
inline void* MMAP(void*addr,size_t length,int prot,int flags,int fd,off64_t offset)
{
	return mmap64(addr,length,prot,flags,fd,offset);
}
typedef off64_t OFF_T;
#elif defined __i386__
inline void* MMAP(void*addr,size_t length,int prot,int flags,int fd,off_t offset)
{
	return mmap(addr,length,prot,flags,fd,offset);
}
typedef off_t OFF_T;
#else
#error "unknown architecture"
#endif

namespace nynn{ 
//declaration
enum{
	SEMGET_TRY_MAX=1000,
	//rwlock type
	RWLOCK_SHARED=1,
	RWLOCK_EXCLUSIVE=0,

	DUMMY
};


class nynn_exception_t;
class MmapFile;

struct ShmAllocator;
struct Inetaddr;
class Lockop;
class Shm;

class Flock;
class Frlock;
class Fwlock;

class FlockRAII;

class Monitor;
class Synchronized;



class MmapFile{
public:
	//create a shared mapping file for already existed file
	explicit MmapFile(const string& path)
		:m_path(path),m_offset(0),m_base(0)
	{
		m_fd=open(m_path.c_str(),O_RDWR);
		if (m_fd<0)
			throw_nynn_exception(errno,"");

		m_length=lseek(m_fd,0,SEEK_END);
		if (m_length==-1)
			throw_nynn_exception(errno,"");

		m_base=MMAP(NULL,m_length,PROT_WRITE|PROT_READ,MAP_SHARED,m_fd,m_offset);

		if (m_base==MAP_FAILED)
			throw_nynn_exception(errno,"");

		if (close(m_fd)!=0)		
			throw_nynn_exception(errno,"");
	}

	//create a shared mapping file for already existed file
	MmapFile(const string& path,size_t length,off64_t offset)throw(nynn_exception_t)
		:m_path(path),m_length(length),m_offset(offset),m_base(0)
	{
		m_fd=open(m_path.c_str(),O_RDWR);
		if (m_fd<0)
			throw_nynn_exception(errno,"");

		m_length=lseek(m_fd,0,SEEK_END);
		if (m_length==-1)
			throw_nynn_exception(errno,"");
		m_base=MMAP(NULL,m_length,PROT_WRITE|PROT_READ,MAP_SHARED,m_fd,m_offset);

		if (m_base==MAP_FAILED)
			throw_nynn_exception(errno,"");

		if (close(m_fd)!=0)		
			throw_nynn_exception(errno,"");

	}

	// create a private mapping file for a new file
	MmapFile(const string& path,size_t length)throw(nynn_exception_t)
		:m_path(path),m_length(length),m_offset(0),m_base(0)
	{
		m_fd=open(m_path.c_str(),O_RDWR|O_CREAT|O_EXCL,S_IRWXU);
		if (m_fd<0)
			throw_nynn_exception(errno,"");

		if (lseek(m_fd,m_length-4,SEEK_SET)==-1)
			throw_nynn_exception(errno,"");

		if (write(m_fd,"\0\0\0\0",4)!=4)
			throw_nynn_exception(errno,"");

		m_base=MMAP(NULL,m_length,PROT_WRITE|PROT_READ,MAP_SHARED,m_fd,m_offset);

		if (m_base==MAP_FAILED)
			throw_nynn_exception(errno,"");

		if (close(m_fd)!=0)		
			throw_nynn_exception(errno,"");
	}

	void lock(void* addr,size_t length)throw(nynn_exception_t)
	{

		if (!checkPagedAlignment(addr,length))
			throw_nynn_exception(ENOMEM,"");

		if (mlock(addr,m_length)!=0)
			throw_nynn_exception(errno,"");

		return;
	}

	void lock()throw(nynn_exception_t)
	{
		try{
			lock(m_base,m_length);
		}catch (nynn_exception_t& err) {
			throw err;
		}

		return; 
	}

	void unlock(void* addr,size_t m_length)throw(nynn_exception_t)
	{

		if (!checkPagedAlignment(addr,m_length))
			throw_nynn_exception(ENOMEM,"");

		if (munlock(addr,m_length)!=0)
			throw_nynn_exception(errno,"");

		return;
	}

	void unlock()throw(nynn_exception_t)
	{
		try{
			unlock(m_base,m_length);
		}catch (nynn_exception_t& err) {
			throw err;
		}

		return; 
	}

	void sync(void* addr,size_t m_length,int flags)throw(nynn_exception_t)
	{

		if (!checkPagedAlignment(addr,m_length))
			throw_nynn_exception(ENOMEM,"");

		if (msync(addr,m_length,flags)!=0)
			throw_nynn_exception(errno,"");

		return;
	}

	void sync(int flags)throw(nynn_exception_t)
	{
		try{
			sync(m_base,m_length,flags);
		}catch (nynn_exception_t& err){
			throw err;
		}
		return;
	}


	~MmapFile()
	{
	}

	void *getBaseAddress()const
	{
		return m_base;
	}

	size_t getLength()const
	{
		return m_length;
	}


private:
		//forbid copying object 
	MmapFile(const MmapFile&);
	MmapFile& operator=(const MmapFile&);

	bool checkPagedAlignment(void* &addr,size_t &m_length)
	{
		size_t pagesize=static_cast<size_t>(sysconf(_SC_PAGESIZE));
		// round down addr by page boundary
		char* taddr=static_cast<char*>(addr);
		char* tbase=static_cast<char*>(m_base);

		m_length += ((uintptr_t)taddr)%pagesize;
		taddr  -= ((uintptr_t)taddr)%pagesize;
		addr=static_cast<void*>(taddr);

		// check [addr,addr+m_length] in [m_base,m_base+m_length]
		if (taddr-tbase>=0 && taddr-tbase<=m_length-m_length)
			return true;
		else 
			return false;
	}

	string 	m_path;
	int 	m_fd;
	size_t 	m_length;
	OFF_T   m_offset;
	void*   m_base;
};

struct ShmAllocator{
	static void* operator new(size_t size,void*buff){return buff;}
	static void  operator delete(void*buff,size_t size){ }
};
struct Semid0{

	Semid0(size_t slots)throw(nynn_exception_t):slot_max(slots)
	{	
		semid=semget(IPC_PRIVATE,slot_max,IPC_CREAT|IPC_EXCL|0700);
		if (semid==-1) throw_nynn_exception(errno,"");
		
		uint16_t *array=new uint16_t[slot_max];
		std::fill(array,array+slot_max,0);
		if (semctl(semid,0,SETALL,array)==-1)throw_nynn_exception(errno,"");
	}

	~Semid0(){}

	size_t get_slot_max()const{return slot_max;}
	int    get_semid()const {return semid;}
	
	int    semid;		
	size_t slot_max;
};

struct Semid1{
	Semid1(int semid):semid(semid){}
	
	~Semid1()
	{
		if (semctl(semid,0,IPC_RMID)==-1)throw_nynn_exception(errno,"");
	}

	int    semid;
};	

class Lockop {

public:

	Lockop(int semid,int slot )throw(nynn_exception_t):semid(semid),slot(slot)
	{

		struct seminfo si;
		if (semctl(0,0,IPC_INFO,&si)==-1)throw_nynn_exception(errno,"");
		int semmsl=si.semmsl;
		int semopm=si.semopm;
		log_i("semmsl=%d",semmsl);
		log_i("semopm=%d",semopm);
		

		struct semid_ds ds;
		if (semctl(semid,0,IPC_STAT,&ds)!=0)throw_nynn_exception(errno,"");

		if (slot>=ds.sem_nsems)throw_nynn_exception(EINVAL,"");

		struct sembuf sop;
		sop.sem_op=-1;
		sop.sem_num=slot;
		sop.sem_flg=SEM_UNDO;
		
		if (semop(semid,&sop,1)!=0)throw_nynn_exception(errno,"");
	}

	~Lockop()throw(nynn_exception_t)
	{

		struct sembuf sop;
		sop.sem_op=1;
		sop.sem_num=slot;
		
		if (semop(semid,&sop,1)!=0)throw_nynn_exception(errno,"");
	}

private:

	//disallow copy.
	Lockop(const Lockop&);
	Lockop& operator=(const Lockop&);
	//disallow created on free store(heap);
	static void*operator new(size_t);
	static void*operator new(size_t,void*);
	int semid;
	int slot;
};

struct Shmid{
	int m_shmid;
	size_t m_length;
	explicit Shmid(size_t m_length)throw(nynn_exception_t):
		m_shmid(0),m_length(m_length)
	{
		m_shmid=shmget(IPC_PRIVATE,m_length,IPC_CREAT|IPC_EXCL|0700);
		if (m_shmid==-1)
			throw_nynn_exception(errno,"");
	}
	~Shmid(){}
	int get_shmid()const{return m_shmid;}
	size_t getLength()const{return m_length;}

private:
	Shmid(const Shmid&);
	Shmid& operator=(const Shmid&);
};

class Shm{
public:

	explicit Shm(int m_shmid,size_t m_length=0)throw(nynn_exception_t):
		m_shmid(m_shmid),m_length(m_length),m_base(0)
	{ 
		m_base=shmat(m_shmid,NULL,0);
		if(m_base==(void*)-1)throw_nynn_exception(errno,"");
	}
	
	explicit Shm(const Shmid& id)throw(nynn_exception_t):
		m_shmid(id.m_shmid),m_length(id.m_length)
	{
		m_base=shmat(m_shmid,NULL,0);
		if(m_base==(void*)-1)throw_nynn_exception(errno,"");
	}

	~Shm()
	{
		if(shmdt(m_base)==-1)
			throw_nynn_exception(errno,"");

		struct shmid_ds ds;
		if (shmctl(m_shmid,IPC_STAT,&ds)==-1)
			throw_nynn_exception(errno,"");

		if (ds.shm_nattch>0)return;
		if (shmctl(m_shmid,IPC_RMID,NULL)==-1)
			throw_nynn_exception(errno,"");
	}

	void* getBaseAddress()const{return m_base;}
	size_t getLength()const{return m_length;}
	int   get_shmid()const{return m_shmid;}
private:
	Shm(const Shm&);
	Shm& operator=(const Shm&);

	int    m_shmid;
	size_t m_length;
	void*  m_base;
};

class Flock{
protected:
	string m_path;
	int m_fd;

	Flock(const string& m_path)throw(nynn_exception_t):m_path(m_path){
		if (!file_exist(m_path.c_str()))
			throw_nynn_exception(ENOENT,"");

		m_fd=open(m_path.c_str(),O_RDWR);
		if (m_fd==-1)
			throw_nynn_exception(ENOENT,"");
	}

public:
	virtual void lock(off_t start,off_t m_length)=0;
	virtual void unlock(off_t start,off_t m_length)=0;
	virtual bool trylock(off_t start,off_t m_length)=0;
	virtual ~Flock() { close(m_fd);}
};

class Frlock:public Flock{
public:
	Frlock(const string& m_path):Flock(m_path){}
	~Frlock(){}
	virtual void lock(off_t start,off_t m_length)
	{
		struct flock op;
		op.l_type=F_RDLCK;
		op.l_start=start;
		op.l_len=m_length;
		op.l_whence=SEEK_SET;
		if (fcntl(m_fd,F_SETLKW,&op)!=0)
			throw_nynn_exception(errno,"");
	}
	
	virtual void unlock(off_t start,off_t m_length)
	{
		struct flock op;
		op.l_type=F_UNLCK;
		op.l_start=start;
		op.l_len=m_length;
		op.l_whence=SEEK_SET;
		if (fcntl(m_fd,F_SETLKW,&op)!=0)
			throw_nynn_exception(errno,"");

	}
	virtual bool trylock(off_t start,off_t m_length)
	{
		struct flock op;
		op.l_type=F_RDLCK;
		op.l_start=start;
		op.l_len=m_length;
		op.l_whence=SEEK_SET;
		if (fcntl(m_fd,F_SETLK,&op)==0) return true;
		if (errno==EAGAIN) return false;
		throw_nynn_exception(errno,"");
	}
};

class Fwlock:public Flock{
public:	
	Fwlock(const string&path):Flock(path){}
	~Fwlock(){} 
	virtual void lock(off_t start,off_t m_length)
	{
		struct flock op;
		op.l_type=F_WRLCK;
		op.l_start=start;
		op.l_len=m_length;
		op.l_whence=SEEK_SET;
		if (fcntl(m_fd,F_SETLKW,&op)!=0)
			throw_nynn_exception(errno,"");
	}

	virtual void unlock(off_t start,off_t m_length)
	{
		struct flock op;
		op.l_type=F_UNLCK;
		op.l_start=start;
		op.l_len=m_length;
		op.l_whence=SEEK_SET;
		if (fcntl(m_fd,F_SETLKW,&op)!=0)
			throw_nynn_exception(errno,"");
	}

	virtual bool trylock(off_t start,off_t m_length)
	{
		struct flock op;
		op.l_type=F_WRLCK;
		op.l_start=start;
		op.l_len=m_length;
		op.l_whence=SEEK_SET;
		if (fcntl(m_fd,F_SETLK,&op)==0) return true;
		if (errno==EAGAIN) return false;
		throw_nynn_exception(errno,"");
	}
};

class FlockRAII{
public:
	explicit FlockRAII(Flock* lck):lock(lck){lock->lock(0,0);}
			~FlockRAII(){lock->unlock(0,0);}
private:
	Flock *lock;
};
inline int rand_int()
{
	struct timespec ts;
	ts.tv_sec=0;
	ts.tv_nsec=1;
	clock_nanosleep(CLOCK_MONOTONIC,0,&ts,NULL);

	clock_gettime(CLOCK_MONOTONIC,&ts);
	unsigned seed=static_cast<unsigned>(0x0ffffffffl & ts.tv_nsec);
	return rand_r(&seed);
}

class Monitor{
	pthread_spinlock_t m_mutex;
public:
	Monitor(){pthread_spin_init(&this->m_mutex,PTHREAD_PROCESS_PRIVATE);}
	~Monitor(){pthread_spin_destroy(&this->m_mutex);}
	pthread_spinlock_t* get(){return &this->m_mutex;}
};
class RWLock{
	pthread_rwlock_t m_rwlock;
public:
	RWLock() { pthread_rwlock_init(&this->m_rwlock,NULL);}
	~RWLock() { pthread_rwlock_destroy(&this->m_rwlock);}
	pthread_rwlock_t* get() {return &this->m_rwlock; }
	
};

class Synchronization{
	Monitor *m_monitor;
public:
	Synchronization(Monitor* m):m_monitor(m){pthread_spin_lock(m_monitor->get());}
	~Synchronization(){pthread_spin_unlock(m_monitor->get());}
};	
class SharedSynchronization{
	RWLock *m_lock;
public:
	SharedSynchronization(RWLock *lock):m_lock(lock)
	{ 
		pthread_rwlock_rdlock(m_lock->get());
	}
	~SharedSynchronization(){pthread_rwlock_unlock(m_lock->get());}
};

class ExclusiveSynchronization{
	RWLock *m_lock;
public:
	ExclusiveSynchronization(RWLock *lock):m_lock(lock)
	{ 
		pthread_rwlock_wrlock(m_lock->get());
	}
	~ExclusiveSynchronization(){pthread_rwlock_unlock(m_lock->get());}
};
template <typename Result,typename Class,typename MemFunc,typename...Args> 
Result mfspinsync(Monitor& m,Class& obj, MemFunc mf,Args&&... args){
	Synchronization get(&m);
	return (obj.*mf)(forward<Args>(args)...);
}

template <typename Result,typename Class,typename MemFunc,typename...Args> 
Result mfsyncw(RWLock& lock,Class& obj, MemFunc mf,Args&&... args){
	ExclusiveSynchronization get(&lock);
	return (obj.*mf)(forward<Args>(args)...);
}

template <typename Result,typename Class,typename MemFunc,typename...Args> 
Result mfsyncr(RWLock& lock,Class& obj, MemFunc mf,Args&&... args){
	SharedSynchronization get(&lock);
	return (obj.*mf)(forward<Args>(args)...);
}

template <typename Result,typename Func,typename...Args> 
Result spinsync(Monitor& m,Func f,Args&&... args){
	Synchronization get(&m);
	return f(forward<Args>(args)...);
}

template <typename Result,typename Func,typename...Args> 
Result syncw(RWLock& lock,Func f,Args&&... args){
	ExclusiveSynchronization get(&lock);
	return f(forward<Args>(args)...);
}

template <typename Result,typename Func,typename...Args> 
Result syncr(RWLock& lock,Func f,Args&&... args){
	SharedSynchronization get(&lock);
	return f(forward<Args>(args)...);
}
}
#endif
