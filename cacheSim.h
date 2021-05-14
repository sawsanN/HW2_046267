
#include <vector>
#include <time.h> 

#define WRITE_ALLOCATE 1
#define NO_WRITE_ALLOCATE 0

void DecodeAddrs(int addrs , int& set , int& tag, int SetSize ){

    std::bitset<32> ADDRS(addrs);
    set  = 0;
    for(int i=5 ; i<5+SetSize ;i++) set+= ADDRS[i]*pow(2,i);
    tag = 0;
    for(int i = 5+SetSize ; i< 32 ;i++ ) tag += ADDRS[i]*pow(2,i);



}

class Block{

    public:
        Block():
            dirty(0),valid(0){
                time(&timer);

            }
        void validate(){
            valid = 1;
        }
        void Invalidate(){
            valid = 0;
        }
        void set_accessed(){
            time(&timer);
        }
        void set_dirty(){
            dirty = 1;
        }


    bool dirty;
    bool valid;
    time_t timer;

}
class line{
    public:
        line(int tag=0):
            Tag(tag){}
        
        bool IsValid(){ return Block.valid ;}
        bool IsDirty(){return Block.dirty;}
        void validate(){
            Block.validate();
            Block.set_accessed();
        }
        void read_line(){
            Block.set_accessed();
        }
        void write_line(){
            Block.set_dirty();
        }
        void pop_line(){
            Block.Invalidate();
        }
        void update_line(){Block.set_accessed();}
        time_t& get_time(){return Block.timer;}
        int Tag;
        Block;
}
class Way{

    public:
        Way(int SIZE = 1 ):
            Waysize(SIZE){
                lines.resize(SIZE);
            }
        
        void push(int addrs);
        bool try_push(int addrs);
        bool look(int addrs);
        bool look(int set,int tag) { return lines[set].Tag == tag;}
        void read(int addrs);
        int write(int addrs);
        time_t& get_time(int addrs);
        line& get_line(int set);
        bool IsDirty(int set){ return lines[set].IsDirty();}
        void update(int set){ lines[set].update();}


        vector<line> lines;
        int Waysize;
        friend void DecodeAddrs(int addrs , int& set , int& tag , int SetSize);


};
class Cache{

    public:
        Cache(int CacheSize_=0 , int BlockSize_ =0, int ways_num_=0 , int AccessTime_=0 , int MissPolicy_=0):
            CacheSize(CacheSize_),BlockSize(BlockSize_),ways_num(ways_num_),AccessTime(AccessTime_),MissPolicy(MissPolicy_),Misses(0){
                Ways.resize(ways_num_);
                int Way_Capacity = CacheSize_/(BlockSize_*ways_num_) ;
                for(int i=0 ; i<ways_num_ ; i++)
                    Ways[i] = Way(Way_Capacity);

            }
        
        void ReadFrom(int addrs);
        void WriteTo(int addrs);
        void Insert(int addrs);
        void Invalidate(int set, int victim);
        bool update(int set , int tag);

    
    int CacheSize ; //cache size in bytes
    int BlockSize; //block size in bytes
    int ways_num; //assosotivity of cache
    int AccessTime;
    int MissPolicy;
    int Misses;

    vector<Way> Ways;



};

class Victim{
    public :
        int tag;
        int set;
        bool dirty;
};

class CacheControl{

    public:
        CacheControl(int CacheSize_ , int BlockSize_ , int ways_num_ , int AccessTime_ , int MissPolicy_){

            L1 = Cache(CacheSize_ ,BlockSize_ ,ways_num_ ,AccessTime_ ,MissPolicy_);
            L2 = Cache(CacheSize_ ,BlockSize_ ,ways_num_ ,AccessTime_ ,MissPolicy_);
            MissPolicy = MissPolicy_;
        }
        void Read(int addrs);
        void Write(int addrs);




        Cache L1;
        Cache L2; //LLC
        int MissPolicy;
};
/*******************************Way**********************************/
void Way::push(int addrs ){
    int  set , tag;
    DecodeAddrs(addrs , set ,tag, log2(Waysize) );
    lines[set].validate();
    lines.tag = tag;

}
bool Way::try_push(int addrs){

    int  set , tag;
    DecodeAddrs(addrs , set ,tag, log2(Waysize) );
    if(!lines[set].IsValid()) 
        return true;
    
    return false;

}
bool Way::look(int addrs){
    int  set , tag;
    DecodeAddrs(addrs , set ,tag, log2(Waysize) );
    if(lines[set].valid && !lines[set].dirty && lines.tag == tag)
        return true;
    

    return false;
}

void Way::read(int addrs){
    int  set , tag;
    DecodeAddrs(addrs , set ,tag, log2(Waysize) );
    lines[set].read_line();
}
void Way::write(int addrs){
    int  set , tag;
    DecodeAddrs(addrs , set ,tag, log2(Waysize) );
    lines[set].write_line();
}

time_t& Way::get_time(int addrs){
    int  set , tag;
    DecodeAddrs(addrs , set ,tag, log2(Waysize) );
    
    
    return lines[set].get_time();  
}
line& Way::get_line(int set){
    
    return lines[set];    
}
/***********************************Cache**************************************/
bool Cache::ReadFrom(int addrs){

    bool found = false;
    for(int i = 0 ; i<ways_num ; i++){
        if(Ways[i].look(addrs)){
            Ways[i].read(addrs);
            return true;
        }
    }

    Misses++;
    return false ; //miss

}
bool Cache::WriteTo(int addrs){

    bool found = false;
    for(int i = 0 ; i<ways_num ; i++){
        if(Ways[i].look(addrs)){
            Ways[i].write(addrs);
            return true;
        }
    }
    Misses++;
    return false ; //miss

}
void Cache::Insert(int addrs , Victim& victim){
    

    bool pushed = false;
    int  set , tag;
    DecodeAddrs(addrs , set ,tag, log2(Waysize) );

    for(int i = 0 ;i<ways_num;i++){
        if(Ways[i].try_push(addrs)){
            Ways[i].push(addrs);
            victim.tag = -1;
            victim.set = -1;
            victim.dirty = 0;
            pushed = true;
        }
    }

    if(pushed == false){
        double seconds , max = 0;
        int victim;
        time_t now;
        time(&now);

        for(int i=0 ; i<ways_num ;i++){
            if(difftime(now,Ways[i].get_time(addrs))>max){
                max = difftime(now,Ways[i].get_time(addrs));
                victim = i;

            }
        }
        victim.tag = Ways[victim].get_line(set).Tag;
        victim.set = set;
        victim.dirty = Ways[victim].IsDirty(set);
        Ways[victim].push(addrs);
    }
}
void Cache::update(int set,int tag){

    for(int i=0 ;i<ways_num ;i++){
        if(Ways[i].look(set , tag)){
            Ways[i].update(set);

        }
    }
}
/********************************************CacheControl**********************************/

void CacheControl::read(int addrs){
    
    bool state = false;
    Victim victim;

    state = L1.ReadFrom(addrs);
    if(MissPolicy == WRITE_ALLOCATE){
        if(state == false){
            state = L2.ReadFrom(addrs);
            if(state == false){
                L2.Insert(addrs , victim);
                if(victim.tag != -1 ){
                    L1.Invalidate(victim.set,.victim.tag);
                }
            }

            L1.Insert(addrs , victim);
            if(victim.dirty)
                L2.update(victim.set,victim.tag);
            L1.ReadFrom(addrs);
    }
    }

}
void CacheControl::write(int addrs){
    
    bool state = false;
    Victim victim;
    state = L1.WriteTo(addrs);
    if(state == false){
        state = L2.WriteTo(addrs);

        if(state == false){
            if(MissPolicy == WRITE_ALLOCATE){
                L2.Insert(addrs , victim);
                L1.Invalidate(victim.set,victim.tag);
            }
        }
        if(MissPolicy == WRITE_ALLOCATE){
            L1.Insert(addrs,victim);
            if(victim.dirty){
                L2.update(victim.set,victim.tag);
            }
            L1.WriteTo(addrs);
        } 
    
    }
}

