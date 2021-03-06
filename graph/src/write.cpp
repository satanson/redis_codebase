#include<linuxcpp.hpp>
#include<nynn_common.hpp>
#include<hiredis.h>
using namespace std;
using namespace nynn;

namespace redis{
typedef void(*reply_func)(int type,redisReply*,void*,void*);
typedef map<int,reply_func> RedisReplyMap;
void reply_default(int type,redisReply* reply, void* repmap,void*arg);

void reply_string(int type,redisReply* reply,void* repmap,void*arg){
	cout<<reply->str<<":str"<<endl;
}

void reply_array(int type,redisReply* reply,void* repmap,void*arg){
	RedisReplyMap& rmap=*(RedisReplyMap*)repmap;
	int level=(intptr_t)arg;
	cout<<"array=>["<<endl;
	for(int i=0;i<reply->elements;i++){
		cout<<string(level*4,' ')<<i<<"=>";
		redisReply *subReply=reply->element[i];
		reply_func f=rmap.count(subReply->type)!=0?rmap[subReply->type]:reply_default;
		int sublevel=level+1;
		f(subReply->type,subReply,repmap,(void*)sublevel);
		freeReplyObject(subReply);
	}
	cout<<"]"<<endl;
}

void reply_integer(int type,redisReply* reply,void* repmap,void*arg){
	cout<<reply->integer<<":int"<<endl;
}
void reply_nil(int type,redisReply* reply,void* repmap,void*arg){
	cout<<"nil"<<endl;
}
void reply_status(int type,redisReply* reply,void* repmap,void*arg){
	cout<<format("redis status:%d",reply->integer)<<endl;
}
void reply_error(int type,redisReply* reply,void* repmap,void*arg){
	cout<<format("redis error:(%d)%s",reply->integer,reply->str)<<endl;
}
void reply_default(int type,redisReply* reply, void* repmap,void*arg){
	cout<<"unknown redis reply:"<<type<<endl;
}

RedisReplyMap get_reply_map(){
	RedisReplyMap replymap;
	replymap[REDIS_REPLY_STRING]=reply_string;
	replymap[REDIS_REPLY_ARRAY]=reply_array;
	replymap[REDIS_REPLY_INTEGER]=reply_integer;
	replymap[REDIS_REPLY_NIL]=reply_nil;
	replymap[REDIS_REPLY_STATUS]=reply_status;
	replymap[REDIS_REPLY_ERROR]=reply_error;
	return replymap;
}
}
using namespace redis;
int main(){
	redisContext *c;
	redisReply *reply;
	RedisReplyMap repmap=get_reply_map();
	struct timeval timeout={1,500000};
	c=redisConnectWithTimeout((char*)"127.0.0.1",30000,timeout);
	if (c->err){
		printf("Connection error:%s\n",c->errstr);
		exit(1);
	}
	string stmt;
	while(getline(cin,stmt)){
		reply = (redisReply*)redisCommand(c,stmt.c_str());
		reply_func f=repmap.count(reply->type)!=0?repmap[reply->type]:reply_default;
		int level=0;
		f(reply->type,reply,&repmap,(void*)level);
		freeReplyObject(reply);
	}
	redisFree(c);
}
