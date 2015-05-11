#ifndef NYNN_UTIL_HPP_BY_SATANSON
#define NYNN_UTIL_HPP_BY_SATANSON
#include<linuxcpp.hpp>
#include<nynn_log.hpp>
#include<nynn_exception.hpp>
namespace nynn{

uint32_t parse_int(const char* s,uint32_t value);

template<typename T>
string to_string(T const& a){
	return ((stringstream&)(stringstream()<<a)).str();
}
string format(const string& fmt,...){
	va_list ap;
	va_start(ap,fmt);
	string s(256,0);
	if(unlikely(vsnprintf((char*)&*s.begin(),s.size(),fmt.c_str(),ap)<0)){
		throw_nynn_exception(0,"failed to format s string");
	}
	va_end(ap);

	s.resize(s.size()-1);
	return s;
}

void set_signal_handler(int signum,void(*handler)(int));

void nanosleep_for(uint32_t ns);
void rand_nanosleep();

uint32_t rand_range(uint32_t min,uint32_t max);

bool file_exist(const string& path);

string get_host();
uint32_t get_ip(string &hostname);
string ip2string(uint32_t ip);
uint32_t string2ip(string const& s);
string ip2host(uint32_t);
uint32_t host2ip(string const&);

time_t string2time(string const& s);
string time2string(time_t t);

char* ltrim(const char *chars,const char *src, char *dest);
char* rtrim(const char *chars,const char *src, char *dest);
char* chop(const char ch,const char *src,char *dest);

vector<string> get_a_line_of_words(istream & inputstream); 
vector<vector<string> > get__multi_lines_of_words(istream & inputstream); 

inline void add_signal_handler(int signum,void(*handler)(int)){
	struct sigaction sigact;
	sigact.sa_handler=handler;
	sigaction(signum,&sigact,NULL);
}
string time2string(time_t t)
{
	struct tm tt;
	char buff[32];
	strftime(buff,sizeof(buff),"%Y-%m-%d %T",gmtime_r(&t,&tt));
	return string(buff);
}

time_t string2time(string const& str)
{
	struct tm tt;
	time_t t;
	strptime(str.c_str(),"%Y-%m-%d %T",&tt);
	return mktime(&tt);
}

inline uint32_t parse_int(const char* s,uint32_t value){
	if (s==NULL||s[0]=='\0'){
		log_w("s shouldn't be NULL or empty string");
		return value;
	}
	char *endptr;
	uint32_t value2=strtoul(s,&endptr,0);
	if (endptr[0]!='\0'){
		log_w("fail to convert %s to uint32_t",s);
		return value;
	}
	return value2;
}

uint32_t rand_range(uint32_t min,uint32_t max){
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC,&ts);
	uint32_t val=(uint32_t)rand_r((uint32_t*)&ts.tv_nsec);
	return min+val%(max-min);
}
inline void nanosleep_for(uint32_t ns){
	struct timespec ts;
	ts.tv_sec=0;
	ts.tv_nsec=ns;
	nanosleep(&ts,NULL);
}

inline void rand_nanosleep(){
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME,&ts);
	ts.tv_sec=0;
	ts.tv_nsec=rand_r((uint32_t*)&ts.tv_nsec);
	//cout<<"sleep "<<ts.tv_nsec<<"ns"<<endl;
	nanosleep(&ts,NULL);
}


inline string get_host()
{
	char host[128];
	int rc=gethostname(host,sizeof(host));
	if (unlikely(rc!=0))
		throw_nynn_exception(errno,"failed to get hostname");
	return string(host);
}

inline uint32_t get_ip(){
	try{
		return host2ip(get_host());
	}catch(nynn_exception_t& ex){
		cerr<<ex.what()<<endl;
		throw_nynn_exception(0,"failed to get ip");
	}
}
string ip2host(uint32_t ip){
	struct sockaddr sa;
	struct sockaddr_in& sai=*(struct sockaddr_in*)&sa;
	sai.sin_family=AF_INET;
	sai.sin_addr=*(in_addr*)&ip;
	socklen_t salen=sizeof(struct sockaddr_in);
	char host[128];
	int flags=NI_NAMEREQD|NI_NOFQDN;
	int rc=getnameinfo(&sa,salen,host,sizeof(host),NULL,0,flags);
	if (rc!=0){
		throw_nynn_exception(0,gai_strerror(rc));
	}
	return string(host);
}
uint32_t host2ip(string const& host){
	struct addrinfo  hint, *res, *p;
	memset(&hint,0,sizeof(hint));
	hint.ai_family = AF_INET;
	hint.ai_flags = AI_ADDRCONFIG;

	int rc=getaddrinfo(host.c_str(),NULL,&hint,&res);
	if (rc!=0){
		throw_nynn_exception(0,gai_strerror(rc));
	}

	uint32_t addr=0;
	if (res!=NULL){
		p=res;
		do{
			addr=*(uint32_t*)&((sockaddr_in*)p->ai_addr)->sin_addr;
			if ( addr!=0 && (addr&0xff)!=0x7f){
				freeaddrinfo(res);
				return addr;
			}
			p=p->ai_next;
		}while(p!=NULL);
	}
	freeaddrinfo(res);
	throw_nynn_exception(0,"failed to convert hostname to ip");

}

inline string ip2string(uint32_t ip){
	char buff[16];
	return string(inet_ntop(AF_INET,&ip,buff,16));
}
inline uint32_t string2ip(string const& s){
	int ip;
	inet_pton(AF_INET,s.c_str(),&ip);
	return ip;
}
inline char* ltrim(const char *chars,const char *src, char *dest)
{
	string s(src);
	if (strlen(src)==0){
		dest[0]='\0';
	}else if (s.find_first_not_of(chars)==string::npos){
		dest[0]='\0';
	}else{
		string s1=s.substr(s.find_first_not_of(chars));
		strcpy(dest,s1.c_str());
	}
	return dest;
}

inline char* rtrim(const char *chars,const char *src, char *dest)
{
	string s(src);
	if (strlen(src)==0){
		dest[0]='\0';
	}else if(s.find_last_not_of(chars)==string::npos){
		dest[0]='\0';
	}else{		
		string s1=s.substr(0,s.find_last_not_of(chars)+1);
		strcpy(dest,s1.c_str());
	}
	return dest;
}

inline char* chop(const char ch,const char *src,char *dest)
{	
	if (strlen(src)==0){
		dest[0]='\0';
	}else{
		string s(src);
		string s1=s.substr(0,s.find_first_of(ch));
		strcpy(dest,s1.c_str());
	}
	return dest;
}
inline string rchop(const char ch,const string& src)
{
	if (src.size()==0){ return string(); }
	else { return src.substr(0,src.find_first_of(ch));}
}
inline string lchop(const char ch,const string& src){
	string reverse_src(src.rbegin(),src.rend());
	string reverse_chopped=rchop(ch,reverse_src);
	return string(reverse_chopped.rbegin(),reverse_chopped.rend());
}

inline vector<string> get_a_line_of_words(istream &inputstream)
{
	vector<string> t;
	string word;
	t.resize(0);
	while(inputstream>>word){
		t.push_back(word);
	}
	return t;
}

inline vector<vector<string> > get_multi_line_of_words(istream &inputstream)
{
	vector<vector<string> > array;
	vector<string> t;
	stringstream ss;
	char buff[64];

	while (!inputstream.eof()){
		do{
			inputstream.setstate(inputstream.rdstate()&~ios::failbit);
			inputstream.getline(buff,64,'\n');
			ss<<buff;
		}while(inputstream.fail()&&!inputstream.eof());

		if(ss.str().size()!=0)array.push_back(get_a_line_of_words(ss));
		ss.str(string(""));
		ss.clear();
	}
	return array;
}	

inline bool file_exist(const string& path)
{
	struct stat st;
	if (stat(path.c_str(),&st)==0)return true;
	return false;
}

}
#endif
