#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>

template<class T> struct Array {
    T *p=nullptr; int n=0, cap=0;
    ~Array(){delete[] p;}
    void push(const T &v){if(n==cap){int c=cap?cap*2:16; T *q=new T[c]; for(int i=0;i<n;++i)q[i]=p[i]; delete[] p;p=q;cap=c;}p[n++]=v;}
    T &operator[](int i){return p[i];}
    void clear(){n=0;}
};
struct Key {char s[41];int value,next;};
struct Map {
    static const int B=131071; int *head; Array<Key> a;
    Map(){head=new int[B];clear();} ~Map(){delete[] head;}
    void clear(){memset(head,-1,B*sizeof(int));a.clear();}
    unsigned hash(const char *s){unsigned h=2166136261u;while(*s)h=(h^(unsigned char)*s++)*16777619u;return h%B;}
    int find(const char *s){for(int i=head[hash(s)];i!=-1;i=a[i].next)if(!strcmp(a[i].s,s))return a[i].value;return -1;}
    void set(const char *s,int v){unsigned h=hash(s);for(int i=head[h];i!=-1;i=a[i].next)if(!strcmp(a[i].s,s)){a[i].value=v;return;}Key k{};strcpy(k.s,s);k.value=v;k.next=head[h];head[h]=a.n;a.push(k);}
};
template<class T> struct File {
    FILE *f=nullptr;const char *name;int n=0;
    File(const char *s):name(s){open();} ~File(){fclose(f);}
    void open(){f=fopen(name,"r+b");if(!f)f=fopen(name,"w+b");fseek(f,0,SEEK_END);n=ftell(f)/sizeof(T);}
    T get(int i){T v{};fseek(f,(long)i*sizeof(T),SEEK_SET);fread(&v,sizeof(T),1,f);return v;}
    void put(int i,const T &v){fseek(f,(long)i*sizeof(T),SEEK_SET);fwrite(&v,sizeof(T),1,f);}
    int add(const T &v){int i=n++;put(i,v);return i;}
    void clear(){fclose(f);f=fopen(name,"w+b");n=0;}
    void flush(){fflush(f);}
};
struct User {char id[21],pw[31],name[41],mail[31];int privilege,head,count;};
struct Train {char id[21],type;bool released,deleted;int n,capacity,first,last,start,base;int station[100],price[100],arr[100],dep[100];};
struct Station {char name[41];};
struct Run {int seat[100],head,tail;};
struct Order {int train,day,from,to,num,status,prev,next;};
struct Post {int train,pos,next;};
File<User> users("users.dat");File<Train> trains("trains.dat");File<Station> stations("stations.dat");File<Run> runs("runs.dat");File<Order> orders("orders.dat");
Map um,tm,sm;Array<unsigned char> online;Array<int> heads;Array<Post> posts;
struct Cache {int id=-1;Train t;};Cache cache[256];
Train train(int id){Cache &c=cache[id%256];if(c.id!=id){c.t=trains.get(id);c.id=id;}return c.t;}
void saveTrain(int id,const Train&t){trains.put(id,t);cache[id%256].id=id;cache[id%256].t=t;}
void indexTrain(int id,const Train&t){for(int j=0;j<t.n;++j){int s=t.station[j];Post p{id,j,heads[s]};heads[s]=posts.n;posts.push(p);}}
int station(const std::string&s){int id=sm.find(s.c_str());if(id>=0)return id;Station t{};strcpy(t.name,s.c_str());id=stations.add(t);sm.set(s.c_str(),id);heads.push(-1);return id;}
const char *stationName(int id){return sm.a[id].s;}
int monthDays[]={0,31,28,31,30,31,30,31,31,30,31,30,31};
int date(const std::string&s){int m=atoi(s.c_str()),d=atoi(s.c_str()+3)-1;for(int i=1;i<m;++i)d+=monthDays[i];return d;}
std::string timeText(int t){int d=t/1440,m=1;while(m<12&&d>=monthDays[m])d-=monthDays[m++];char b[32];snprintf(b,sizeof(b),"%02d-%02d %02d:%02d",m,d+1,t%1440/60,t%60);return b;}
void split(const std::string&s,Array<std::string>&a){size_t i=0;while(true){size_t j=s.find('|',i);a.push(s.substr(i,j==std::string::npos?j:j-i));if(j==std::string::npos)break;i=j+1;}}
int minimum(const Run&r,int a,int b){int v=1000000000;for(int i=a;i<b;++i)if(r.seat[i]<v)v=r.seat[i];return v;}
int runId(const Train&t,int day){return t.base+day-t.first;}
struct Ticket {int id,a,b,day,cost,duration;};
Ticket ticket(int id,const Train&t,int a,int b,int d){return {id,a,b,d,t.price[b]-t.price[a],t.arr[b]-t.dep[a]};}
void printTicket(const Ticket&v,int number=-1){Train t=train(v.id);if(number<0)number=minimum(runs.get(runId(t,v.day)),v.a,v.b);printf("%s %s %s -> %s %s %d %d\n",t.id,stationName(t.station[v.a]),timeText(v.day*1440+t.dep[v.a]).c_str(),stationName(t.station[v.b]),timeText(v.day*1440+t.arr[v.b]).c_str(),v.cost,number);}
bool costSort=false;
int compareTicket(const void *a,const void*b){const Ticket &x=*(const Ticket*)a,&y=*(const Ticket*)b;int d=costSort?x.cost-y.cost:x.duration-y.duration;if(d)return d;Train tx=train(x.id),ty=train(y.id);return strcmp(tx.id,ty.id);}
void profile(const User&u){printf("%s %s %s %d\n",u.id,u.name,u.mail,u.privilege);}
void initialize(){for(int i=0;i<users.n;++i){User u=users.get(i);um.set(u.id,i);online.push(0);}for(int i=0;i<stations.n;++i){Station s=stations.get(i);sm.set(s.name,i);heads.push(-1);}for(int i=0;i<trains.n;++i){Train t=trains.get(i);if(!t.deleted)tm.set(t.id,i);if(t.released)indexTrain(i,t);}}
void flush(){users.flush();trains.flush();stations.flush();runs.flush();orders.flush();fflush(stdout);}
int main(){std::ios::sync_with_stdio(false);std::cin.tie(nullptr);initialize();std::string line;
while(std::getline(std::cin,line)){
    std::string args[26],cmd,prefix;size_t at=0;
    auto word=[&](){while(at<line.size()&&line[at]==' ')++at;size_t b=at;while(at<line.size()&&line[at]!=' ')++at;return line.substr(b,at-b);};
    cmd=word();if(cmd.empty())continue;if(cmd[0]=='['){prefix=cmd;cmd=word();printf("%s ",prefix.c_str());}
    while(at<line.size()){std::string k=word();if(k.size()==2&&k[0]=='-')args[k[1]-'a']=word();}
    auto arg=[&](char c)->std::string&{return args[c-'a'];};
    auto uid=[&](char c){return um.find(arg(c).c_str());};
    auto logged=[&](int i){return i>=0&&online[i];};
    int u=uid('u'),c=uid('c');
    if(cmd=="exit"){puts("bye");flush();break;}
    else if(cmd=="clean"){users.clear();trains.clear();stations.clear();runs.clear();orders.clear();um.clear();tm.clear();sm.clear();online.clear();heads.clear();posts.clear();for(auto &x:cache)x.id=-1;puts("0");}
    else if(cmd=="add_user"){
        int g=atoi(arg('g').c_str());if(u>=0||(users.n&&(!logged(c)||users.get(c).privilege<=g)))puts("-1");else{User v{};strcpy(v.id,arg('u').c_str());strcpy(v.pw,arg('p').c_str());strcpy(v.name,arg('n').c_str());strcpy(v.mail,arg('m').c_str());v.privilege=users.n?g:10;v.head=-1;int id=users.add(v);um.set(v.id,id);online.push(0);puts("0");}}
    else if(cmd=="login"){if(u<0||online[u]||arg('p')!=users.get(u).pw)puts("-1");else{online[u]=1;puts("0");}}
    else if(cmd=="logout"){if(!logged(u))puts("-1");else{online[u]=0;puts("0");}}
    else if(cmd=="query_profile"||cmd=="modify_profile"){
        User v{},cur{};if(u>=0)v=users.get(u);if(c>=0)cur=users.get(c);
        if(!logged(c)||u<0||(c!=u&&cur.privilege<=v.privilege)||(cmd=="modify_profile"&&!arg('g').empty()&&atoi(arg('g').c_str())>=cur.privilege))puts("-1");else{if(cmd=="modify_profile"){if(!arg('p').empty())strcpy(v.pw,arg('p').c_str());if(!arg('n').empty())strcpy(v.name,arg('n').c_str());if(!arg('m').empty())strcpy(v.mail,arg('m').c_str());if(!arg('g').empty())v.privilege=atoi(arg('g').c_str());users.put(u,v);}profile(v);}}
    else if(cmd=="add_train"){
        if(tm.find(arg('i').c_str())>=0)puts("-1");else{Train t{};strcpy(t.id,arg('i').c_str());t.n=atoi(arg('n').c_str());t.capacity=atoi(arg('m').c_str());t.type=arg('y')[0];t.start=atoi(arg('x').c_str())*60+atoi(arg('x').c_str()+3);Array<std::string>s,p,tt,o,d;split(arg('s'),s);split(arg('p'),p);split(arg('t'),tt);split(arg('o'),o);split(arg('d'),d);t.first=date(d[0]);t.last=date(d[1]);t.dep[0]=t.start;for(int i=0;i<t.n;++i){t.station[i]=station(s[i]);if(i){t.price[i]=t.price[i-1]+atoi(p[i-1].c_str());t.arr[i]=t.dep[i-1]+atoi(tt[i-1].c_str());t.dep[i]=t.arr[i]+(i+1<t.n?atoi(o[i-1].c_str()):0);}}int id=trains.add(t);tm.set(t.id,id);puts("0");}}
    else if(cmd=="release_train"||cmd=="delete_train"||cmd=="query_train"){
        int id=tm.find(arg('i').c_str());Train t{};if(id>=0)t=train(id);int day=arg('d').empty()?0:date(arg('d'));
        if(id<0||(cmd!="query_train"&&t.released)||(cmd=="query_train"&&(day<t.first||day>t.last)))puts("-1");
        else if(cmd=="delete_train"){t.deleted=true;saveTrain(id,t);tm.set(t.id,-1);puts("0");}
        else if(cmd=="release_train"){t.released=true;t.base=runs.n;Run r{};r.head=r.tail=-1;for(int j=0;j<t.n-1;++j)r.seat[j]=t.capacity;for(int j=t.first;j<=t.last;++j)runs.add(r);saveTrain(id,t);indexTrain(id,t);puts("0");}
        else{Run r{};if(t.released)r=runs.get(runId(t,day));printf("%s %c\n",t.id,t.type);for(int i=0;i<t.n;++i){printf("%s %s -> %s %d ",stationName(t.station[i]),i?timeText(day*1440+t.arr[i]).c_str():"xx-xx xx:xx",i+1<t.n?timeText(day*1440+t.dep[i]).c_str():"xx-xx xx:xx",t.price[i]);if(i+1==t.n)puts("x");else printf("%d\n",t.released?r.seat[i]:t.capacity);}}}
    else if(cmd=="query_ticket"){
        int s=sm.find(arg('s').c_str()),e=sm.find(arg('t').c_str()),d=date(arg('d'));Array<Ticket> result;
        if(s>=0&&e>=0)for(int k=heads[s];k!=-1;k=posts[k].next){Post p=posts[k];Train t=train(p.train);int day=d-t.dep[p.pos]/1440;if(day<t.first||day>t.last)continue;for(int j=p.pos+1;j<t.n;++j)if(t.station[j]==e){result.push(ticket(p.train,t,p.pos,j,day));break;}}
        costSort=arg('p')=="cost";qsort(result.p,result.n,sizeof(Ticket),compareTicket);printf("%d\n",result.n);for(int i=0;i<result.n;++i)printTicket(result[i]);}
    else if(cmd=="query_transfer"){
        struct Leg{int id,a,b,next;};Array<Leg> legs;int *lh=new int[stations.n];for(int i=0;i<stations.n;++i)lh[i]=-1;
        int s=sm.find(arg('s').c_str()),e=sm.find(arg('t').c_str()),d=date(arg('d'));bool found=false;Ticket best1{},best2{};int bestCost=0,bestTime=0;
        if(s>=0&&e>=0){for(int k=heads[e];k!=-1;k=posts[k].next){Post p=posts[k];Train t=train(p.train);for(int j=0;j<p.pos;++j){int st=t.station[j];legs.push({p.train,j,p.pos,lh[st]});lh[st]=legs.n-1;}}
        for(int k=heads[s];k!=-1;k=posts[k].next){Post p=posts[k];Train t=train(p.train);int day=d-t.dep[p.pos]/1440;if(day<t.first||day>t.last)continue;
        for(int j=p.pos+1;j<t.n;++j){int arrival=day*1440+t.arr[j];for(int z=lh[t.station[j]];z!=-1;z=legs[z].next){Leg l=legs[z];if(l.id==p.train)continue;Train v=train(l.id);int day2=(arrival-v.dep[l.a]+1439)/1440;if(day2<v.first)day2=v.first;if(day2>v.last)continue;Ticket x=ticket(p.train,t,p.pos,j,day),y=ticket(l.id,v,l.a,l.b,day2);int cost=x.cost+y.cost,duration=day2*1440+v.arr[l.b]-(day*1440+t.dep[p.pos]);int primary=arg('p')=="cost"?cost:duration,old=arg('p')=="cost"?bestCost:bestTime;
        if(!found||primary<old||(primary==old&&x.duration<best1.duration)){found=true;best1=x;best2=y;bestCost=cost;bestTime=duration;}}}}}
        delete[] lh;if(!found)puts("0");else{printTicket(best1);printTicket(best2);}}
    else if(cmd=="buy_ticket"){
        int id=tm.find(arg('i').c_str()),num=atoi(arg('n').c_str());Train t{};if(id>=0)t=train(id);int a=-1,b=-1;int s=sm.find(arg('f').c_str()),e=sm.find(arg('t').c_str());for(int i=0;i<t.n;++i){if(t.station[i]==s)a=i;if(t.station[i]==e)b=i;}int day=a<0?-1:date(arg('d'))-t.dep[a]/1440;
        if(!logged(u)||id<0||!t.released||a<0||b<=a||day<t.first||day>t.last||num<=0||num>t.capacity)puts("-1");else{int rid=runId(t,day);Run r=runs.get(rid);bool ok=minimum(r,a,b)>=num;if(!ok&&arg('q')!="true")puts("-1");else{User v=users.get(u);Order o{id,day,a,b,num,ok?0:1,v.head,-1};int oi=orders.add(o);v.head=oi;++v.count;users.put(u,v);if(ok){for(int i=a;i<b;++i)r.seat[i]-=num;printf("%lld\n",1LL*num*(t.price[b]-t.price[a]));}else{if(r.tail>=0){Order tail=orders.get(r.tail);tail.next=oi;orders.put(r.tail,tail);}else r.head=oi;r.tail=oi;puts("queue");}runs.put(rid,r);}}}
    else if(cmd=="query_order"){
        if(!logged(u))puts("-1");else{User v=users.get(u);printf("%d\n",v.count);for(int i=v.head;i!=-1;){Order o=orders.get(i);printf("[%s] ",o.status==0?"success":o.status==1?"pending":"refunded");Train t=train(o.train);printTicket(ticket(o.train,t,o.from,o.to,o.day),o.num);i=o.prev;}}}
    else if(cmd=="refund_ticket"){
        int nth=arg('n').empty()?1:atoi(arg('n').c_str());User v{};if(u>=0)v=users.get(u);if(!logged(u)||nth<=0||nth>v.count)puts("-1");else{int oi=v.head;for(int j=1;j<nth;++j)oi=orders.get(oi).prev;Order o=orders.get(oi);if(o.status==2)puts("-1");else{int old=o.status;o.status=2;orders.put(oi,o);if(old==0){Train t=train(o.train);int rid=runId(t,o.day);Run r=runs.get(rid);for(int j=o.from;j<o.to;++j)r.seat[j]+=o.num;int prev=-1;for(int k=r.head;k!=-1;){Order q=orders.get(k);int next=q.next;if(q.status==1&&minimum(r,q.from,q.to)>=q.num){for(int j=q.from;j<q.to;++j)r.seat[j]-=q.num;q.status=0;orders.put(k,q);}if(q.status!=1){if(prev<0)r.head=next;else{Order pr=orders.get(prev);pr.next=next;orders.put(prev,pr);}if(r.tail==k)r.tail=prev;}else prev=k;k=next;}runs.put(rid,r);}puts("0");}}}
    else puts("-1");
    flush();
}
}
