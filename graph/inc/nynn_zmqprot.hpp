#ifndef NYNN_ZMQPROT_HPP_BY_SATANSON
#define NYNN_ZMQPROT_HPP_BY_SATANSON

#include<nynn_zmq.hpp>
#include<linuxcpp.hpp>
#include<nynn_common.hpp>
#include<nynn_mm_config.hpp>
using namespace std;
using namespace nynn;
namespace nynn{namespace prot{

enum{VERSION_NO=0x02000000ul};
enum{ASK_VERSION,ASK_CMD,ASK_OPTIONS,ASK_DATA,ASK_SIZE};
enum{CMD_INVALID,CMD_SUBMIT,CMD_NOTIFY,CMD_HELLO,CMD_WRITE,CMD_READ,CMD_GET_SGSDIR,CMD_VTX_EXISTS,CMD_GET_REMOTE};
enum{ANS_STATUS,ANS_DATA,ANS_SIZE};

static uint32_t const STATUS_OK=0;
static uint32_t const STATUS_ERR=INVALID_BLOCKNO;

static string v2s(uint32_t version_no){
	return ip2string(version_no);
}
class Requester{
public:
	explicit Requester(zmq::socket_t& s):sock(s){}
	void ask(uint8_t cmd,const void*options,size_t osize,const void*data,size_t dsize){
		zmq::message_t omsg[ASK_SIZE];
		
		omsg[ASK_VERSION].rebuild(sizeof(uint32_t));
		*(uint32_t*)omsg[ASK_VERSION].data()=VERSION_NO;
		
		omsg[ASK_CMD].rebuild(sizeof(uint8_t));
		*(uint8_t*)omsg[ASK_CMD].data()=cmd;
		
		omsg[ASK_OPTIONS].rebuild(osize);
		memcpy(omsg[ASK_OPTIONS].data(),options,osize);
		
		omsg[ASK_DATA].rebuild(dsize);
		memcpy(omsg[ASK_DATA].data(),data,dsize);
		
		int i=0;
		while(i<ASK_SIZE-1)sock.send(omsg[i++],ZMQ_SNDMORE);
		sock.send(omsg[i],0);
	}

	bool parse_ans(){
		int i=0;
		do{
			sock.recv(imsg[i],0);
		}while(imsg[i++].more()&&i<ANS_SIZE);
		if (i!=ANS_SIZE){
			log_w("ans msg must has %d parts,but actually recv %d part(s)",ANS_SIZE,i);
			return false;
		}
		return true;
	}
	uint32_t get_status(){
		return *(uint32_t*)imsg[ANS_STATUS].data();
	}
	void* get_data(){
		if(likely(get_data_size()))return imsg[ANS_DATA].data();
		else return NULL;
	}
	size_t get_data_size(){
		return imsg[ANS_DATA].size();
	}

private:
	zmq::socket_t& sock;
	zmq::message_t imsg[ANS_SIZE];
	Requester(Requester const&);
	Requester& operator=(Requester const&);
};

class Replier{
public:
	explicit Replier(zmq::socket_t& s):sock(s){}
	bool parse_ask(){
		int i=0;
		do{
			sock.recv(imsg[i],0);
		}while(imsg[i++].more()&&i<ASK_SIZE);
		if (i!=ASK_SIZE){
			log_w("ask msg must has %d parts,but actually recv %d part(s)",ASK_SIZE,i);
			return false;
		}
		//check version
		uint32_t v=*(uint32_t*)imsg[ASK_VERSION].data();
		if (v!=VERSION_NO){
			log_w("APIs's version(%s) is not compatible with lib's(%s)",
					v2s(v).c_str(),v2s(VERSION_NO).c_str());
			return false;
		}
		return true;
	}

	uint8_t get_cmd(){
		return *(uint8_t*)imsg[ASK_CMD].data();
	}
	void* get_options(){
		return imsg[ASK_OPTIONS].data();
	}
	size_t get_options_size(){
		return imsg[ASK_OPTIONS].size();
	}
	void* get_data(){
		if (likely(get_data_size()))return imsg[ASK_DATA].data();
		else return NULL;
	}
	size_t get_data_size(){
		return imsg[ASK_DATA].size();
	}
	void ans(uint32_t status,void* data,size_t size){
		zmq::message_t omsg[ANS_SIZE];	
		omsg[ANS_STATUS].rebuild(sizeof(uint32_t));
		*(uint32_t*)omsg[ANS_STATUS].data()=status;
		omsg[ANS_DATA].rebuild(size);
		memcpy(omsg[ANS_DATA].data(),data,size);

		int i=0;
		while(i<ANS_SIZE-1)sock.send(omsg[i++],ZMQ_SNDMORE);
		sock.send(omsg[i],0);
	}
	//void ans_begin()
	//void ans_more()
	//void ans_end()
private:
	zmq::socket_t& sock;
	zmq::message_t imsg[ASK_SIZE];
	Replier(Replier const&);
	Replier& operator=(Replier const&);
};

}}
#endif
