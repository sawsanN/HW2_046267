
#include <vector>
#include <time.h>
#include <bitset>
#include <math.h>

#define WRITE_ALLOCATE 1
#define NO_WRITE_ALLOCATE 0

void DecodeAddrs(int addrs , int& set , int& tag, int SetSize,int blockSize_ ){

    double num = (double)log2(blockSize_);
    int blockSize = ceil(num);
    std::bitset<32> ADDRS(addrs);
   // std::cout<<"in binary this address: "<<addrs<<" is: ";
    //for(int i=0 ;i<32;i++) std::cout<<ADDRS[i];
    //std::cout<<std::endl;
    //if(SetSize == 0) set = 0;
    //else{
        set  = 0;
        //std::cout<<"range is: "<<"["<<blockSize<<" "<<blockSize+SetSize<<"]"<<std::endl;
    //std::cout<<"offset is: "<<blockSize<<" setsize is: "<<SetSize<<std::endl;
        for(int i=blockSize ; i<blockSize+SetSize ;i++) set+= ADDRS[i]*pow(2,i-blockSize);
    //}
    tag = 0;
    for(int i = blockSize+SetSize ; i<32 ;i++ ) tag += ADDRS[i]*pow(2,i-blockSize-SetSize);
    //std::cout<<"the tag is: "<<tag<<"set is: "<<set<<std::endl;



}

class LRU{
    
    public:
        LRU(int ways_num_=1,int setsNum_=1):
            ways_num(ways_num_),setsNum(setsNum_){
                Counters.resize(ways_num);
                for(int i=0 ;i<ways_num;i++)
                    Counters[i].resize(setsNum);

            }
        void CountUpdate(int set, int way_idx){
            //std::cout<<"in counterupdate set is: "<<set<<" way_idx is: "<<way_idx<<"waynum is: "<<ways_num<<std::endl;
            unsigned x = Counters[way_idx][set];
            Counters[way_idx][set] = ways_num - 1;

            for(int i=0 ; i<ways_num ;i++){

                if(i!=way_idx && Counters[i][set] > x)
                    Counters[i][set] --;
            }
        }
        int GetLRU(int set){

            for(int i=0 ; i<ways_num ;i++){

                if(Counters[i][set] == 0)
                    return i;
            } 

            return -1; 
        }



        int ways_num;
        int setsNum;
        std::vector<std::vector<unsigned>> Counters;
};
class Block{

    public:
        Block():
            dirty(0),valid(0){
               // time=timer.CountUp();
                

            }
        void validate(){
            valid = 1;
        }
        void Invalidate(){
            valid = 0;
        }
        /*void set_accessed(){
            time=timer.CountUp();
        }*/
        void set_dirty(){
            dirty = 1;
        }

    bool dirty;
    bool valid;
    //Counter timer;
    //unsigned time;

};

class line{
    public:
        line():
        Tag(-1){}
        bool IsValid(){ return block.valid ;}

        bool IsDirty(){return block.dirty;}
        void validate(){
            block.validate();
            //block.set_accessed();
        }
        /*void read_line(){
            block.set_accessed();
        }*/
        void write_line(){
            block.set_dirty();
            //block.set_accessed();
        }
        void pop_line(){
            block.Invalidate();
        }
        //void update_line(){block.set_accessed();}
        //time_t& get_time(){return block.timer;}
        //unsigned get_time(){ return block.time;}
        int Tag;
        Block block;
};

class Way{

    public:
        Way(int SIZE = 1 , int BlockSize_=1):
            Waysize(SIZE),BlockSize(BlockSize_){
                lines.resize(SIZE);

            }
        
        void push(int addrs);
        bool try_push(int addrs);
        bool look(int addrs);
        bool look(int set,int tag) { return lines[set].Tag == tag;}
        //void read(int addrs);
        void write(int addrs);
        void write(int set,int tag);
       // unsigned get_time(int set){ return lines[set].get_time();}
        void set_line_tag(int set,int tag){lines[set].Tag = tag ; }
        int get_line_tag(int set){
            return lines[set].Tag ; 
            }
        line& get_line(int set);
        bool IsDirty(int set){ return lines[set].IsDirty();}
        //void update(int set){ lines[set].update_line();}


        std::vector<line> lines;
        int Waysize;
        int BlockSize;
        friend void DecodeAddrs(int addrs , int& set , int& tag , int SetSize , int blockSize_);


};

class Victim{
public :
    int tag;
    int set;
    bool dirty;
};

class Cache{

    public:
        Cache(int memTime=1, int CacheSize_=1 , int BlockSize_ =1, int ways_num_=0 , int AccessTime_=1 , int MissPolicy_=1):
            MemTime(memTime),CacheSize(CacheSize_),BlockSize(BlockSize_),ways_num(ways_num_),AccessTime(AccessTime_),
            MissPolicy(MissPolicy_),Misses(0),hits(0), AccCount(0)
            {   
                Ways.resize(ways_num);
                float Waysize_1 = (float)((float)CacheSize_/(BlockSize_*(ways_num))) ;
                Waysize = ceil(Waysize_1) ;
                Counter = LRU(ways_num,Waysize);
                for(int i=0 ; i<ways_num ; i++)
                    Ways[i] = Way(Waysize , BlockSize);
               // std::cout<<"waysize is: "<<Waysize<<" BlockSize is: "<<BlockSize<<" ways_num is: "<<ways_num<<" cachesize is: "<<CacheSize<<std::endl;

            }
        
        bool ReadFrom(int addrs);
        bool WriteTo(int addrs);
        bool WriteTo(int set,int tag);
        bool ReadFrom(int set,int tag);

        void Insert(int addrs, Victim &victim);
        void Invalidate(int set, int victimTag);
       // void update(int set , int tag);
        friend class Stats;

        int Waysize;
        int MemTime;
        int CacheSize ; //cache size in bytes
        int BlockSize; //block size in bytes
        int ways_num; //assosotivity of cache
        int AccessTime;
        int MissPolicy;
        int Misses;
        int hits;
        int AccCount;
        LRU Counter;

    std::vector<Way> Ways;



};

class CacheControl{

    public:
        CacheControl(int memTime, int L1Size, int L2Size, int BlockSize_, int L1ways_num_, int L2ways_num_, int L1AccessTime_, int L2AccessTime_, int MissPolicy_) :
        MemTime(memTime), tAccess(0)
        {   
            L1 = Cache(memTime, pow(2,L1Size) ,pow(2,BlockSize_) ,pow(2,L1ways_num_) ,L1AccessTime_ ,MissPolicy_);
            L2 = Cache(memTime, pow(2,L2Size) ,pow(2,BlockSize_ ),pow(2,L2ways_num_ ),L2AccessTime_ ,MissPolicy_);
            MissPolicy = MissPolicy_;
        }
        void Read(int addrs);
        void Write(int addrs);

        double L1Stats() {
         //   std::cout<<"L1 TOT acc is: "<<L1.AccCount<<std::endl;
            return (double)L1.Misses/L1.AccCount; }
        double L2Stats() {  
         //   std::cout<<"L2 misses is: "<<L2.Misses<<"L2 tot acceses: "<<L2.AccCount<<std::endl;
            return (double)L2.Misses/L2.AccCount; 
        }
        double AvgAccTime() { return (double)tAccess/(L1.AccCount); }
       // friend class Stats;

        int MemTime;
        Cache L1;
        Cache L2; //LLC
        int MissPolicy;
        int tAccess; // total time needed in accessing
        int tAccessCount; //tottal access time
};

//class Stats{
//public:
//
//    double L1Stats() { return CacheControl::L1.Misses/CacheControl::L1.AccCount; }
//    double L2Stats() { return L2.Misses/L2.AccCount; }
//    double AvgAccTime() { return tAccess/tAccessCount; }
//
//};

/*******************************Way**********************************/
void Way::push(int addrs ){
    int  set , tag , setsize;
    if(Waysize == 1 ) setsize = 0;
    else setsize = log2(Waysize);
    DecodeAddrs(addrs , set ,tag, setsize,BlockSize );
    lines[set].validate();
    lines[set].Tag = tag;

}
bool Way::try_push(int addrs){

    int  set , tag, setsize;
    if(Waysize == 1 ) setsize = 0;
    else setsize = log2(Waysize);
    DecodeAddrs(addrs , set ,tag, setsize , BlockSize );
    if(!lines[set].IsValid()) 
        return true;
    
    return false;

}
bool Way::look(int addrs){
    int  set , tag,setsize;
    if(Waysize == 1 ) setsize = 0;
    else setsize = log2(Waysize);
    DecodeAddrs(addrs , set ,tag, setsize, BlockSize );
  //  std::cout<<"tag is: "<<tag<<" set is: "<<set<<std::endl;
    if(lines[set].IsValid() && lines[set].Tag == tag){

       // std::cout<<"Tag is: "<<lines[set].Tag<<"and its valid: "<<lines[set].IsValid()<<std::endl;
        return true;
    }
        
    
    return false;
}

/*void Way::read(int addrs){
    int  set , tag,setsize;
    if(Waysize == 1 ) setsize = 0;
    else setsize = log2(Waysize);
    DecodeAddrs(addrs , set ,tag, setsize, BlockSize );
    std::cout<<"tag is: "<<tag<<" set is: "<<set<<std::endl;
    lines[set].read_line();
}*/
void Way::write(int addrs){
    int  set , tag,setsize;
    if(Waysize == 1 ) setsize = 0;
    else setsize = log2(Waysize);
    DecodeAddrs(addrs , set ,tag, setsize, BlockSize );
 //   std::cout<<"tag is: "<<tag<<" set is: "<<set<<std::endl;
    lines[set].write_line();
}
void Way::write(int set,int tag){

 //   std::cout<<"tag is: "<<tag<<" set is: "<<set<<std::endl;
    lines[set].write_line();
}
/*time_t& Way::get_time(int addrs){
    int  set , tag;
    DecodeAddrs(addrs , set ,tag, log2(Waysize) );
    
    return lines[set].get_time();
}*/
line& Way::get_line(int set){
    
    return lines[set];    
}

/***********************************Cache**************************************/
bool Cache::ReadFrom(int addrs){

    //bool found = false;
    int  set , tag,setsize;
    if(Waysize == 1 ) setsize = 0;
    else setsize = log2(Waysize);
    DecodeAddrs(addrs , set ,tag, setsize, BlockSize );
    for(int i = 0 ; i<ways_num ; i++){
        if(Ways[i].look(addrs)){
    //        std::cout<<"in readfrom its a hit"<<std::endl;
            //Ways[i].read(addrs);
            Counter.CountUpdate(set,i);
            hits++;
            return true;
        }
    }
    //Misses++;
    return false ; //miss

}
bool Cache::WriteTo(int addrs){

    //bool found = false;
    int  set , tag,setsize;
    if(Waysize == 1 ) setsize = 0;
    else setsize = log2(Waysize);
    DecodeAddrs(addrs , set ,tag, setsize, BlockSize );
    for(int i = 0 ; i<ways_num ; i++){
        if(Ways[i].look(addrs)){
            Ways[i].write(addrs);
            Counter.CountUpdate(set,i);
            hits++;
            return true;
        }
    }
    //Misses++;
    return false ; //miss

}
bool Cache::WriteTo(int set,int tag){



    for(int i = 0 ; i<ways_num ; i++){
        if(Ways[i].look(set,tag)){
            Ways[i].write(set,tag);
            Counter.CountUpdate(set,i);
            hits++;
            return true;
        }
    }
    //Misses++;
    return false ; //miss

}
void Cache::Invalidate(int set, int victimTag){
 //   std::cout<<"set: "<<set<<" and tag: "<<victimTag<<"has been invalidated"<<std::endl;
    for(int i = 0 ; i<ways_num ; i++) {
        if (Ways[i].look(set, victimTag)) {
            Ways[i].get_line(set).block.valid = 0;
        }
    }
}

void Cache::Insert(int addrs , Victim& victim){
    
    bool pushed = false;
    int  set , tag,setsize;
    if(Waysize == 1 ) setsize = 0;
    else setsize = log2(Waysize);
    DecodeAddrs(addrs , set ,tag, setsize, BlockSize );

    for(int i = 0 ;i<ways_num;i++){
        if(Ways[i].try_push(addrs)){
         //   std::cout<<"managed to push set: "<<set<<" and tag: "<<tag<<" into way "<<i<<"tot ways is: "<<ways_num<<std::endl;
        //    std::cout<<"in counter tot ways is: "<<Counter.ways_num<<std::endl;
            Ways[i].push(addrs);
            Counter.CountUpdate(set,i);
            victim.tag = -1;
            victim.set = -1;
            victim.dirty = 0;
            pushed = true;
            break;
        }
    }

    if(pushed == false){
       // unsigned  max = 0;
        int victim_idx = Counter.GetLRU(set);

        
        victim.tag = Ways[victim_idx].get_line_tag(set);
        victim.set = set;
        victim.dirty = Ways[victim_idx].IsDirty(set);
        Ways[victim_idx].push(addrs);
        Counter.CountUpdate(set,victim_idx);
    }
}
/*void Cache::update(int set,int tag){ //mark

    //AccCount++;
    for(int i=0 ;i<ways_num ;i++){
        if(Ways[i].look(set , tag)){
            Ways[i].update(set);
        }
    }
}*/
/********************************************CacheControl**********************************/

void CacheControl::Read(int addrs){
    
    bool state = false;
    Victim victim;
    //debug code
    int  L1set , L1tag,L1setsize, L2tag,L2set,L2setsize;
    L1setsize = log2(L1.Waysize);
    L2setsize = log2(L1.Waysize);
    DecodeAddrs(addrs , L1set ,L1tag, L1setsize, L1.BlockSize );
    DecodeAddrs(addrs , L2set ,L2tag, L2setsize, L1.BlockSize );
    //tAccessCount++;
    L1.AccCount++;
    state = L1.ReadFrom(addrs);

    if (state) {
  //  std::cout<<"hit in L1"<<std::endl;
    tAccess += L1.AccessTime;
    }
        if(state == false){
            L1.Misses++;
            state = L2.ReadFrom(addrs);
     //       std::cout<<"miss in L1"<<std::endl;

            L2.AccCount++;
            if (state) {
              tAccess += L1.AccessTime + L2.AccessTime ;
         //     std::cout<<"hit in L2"<<std::endl;
             }

            if(state == false){
                L2.Misses++;
          //      std::cout<<"miss in L2"<<std::endl;
                tAccess += L1.AccessTime + L2.AccessTime + MemTime;
                L2.Insert(addrs , victim);
            //     std::cout<<"inserted addrs to L2: "<<addrs<<" tag is: "<<L2tag<<" set is: "<<L2set<<" into L2 victim set is: "<<victim.set<<" victim tag is: "<<victim.tag<<std::endl;
                if(victim.tag != -1 ){
                    L1.Invalidate(victim.set,victim.tag);
                }
            }

            L1.Insert(addrs , victim);
     //       std::cout<<"inserted addrs to L1: "<<addrs<<" tag is: "<<L1tag<<" set is: "<<L1set<<" into L1 victim set is: "<<victim.set<<" victim tag is: "<<victim.tag<<std::endl;
            if(victim.tag != -1 && victim.dirty) {
                //L2.update(victim.set, victim.tag);
                L2.WriteTo(victim.set,victim.tag);
            }
            //L1.ReadFrom(addrs);
        }
    
}
void CacheControl::Write(int addrs){
    
    bool state = false;
    Victim victim;

    //tAccessCount++;
    L1.AccCount++;

    state = L1.WriteTo(addrs);
    if (state) {
  //      std::cout<<"hit in L1"<<std::endl;
        tAccess += L1.AccessTime;
    }
    if(state == false){
        state = L2.WriteTo(addrs);
        L1.Misses++;
     //   std::cout<<"miss in L1"<<std::endl;

        L2.AccCount++;
        if (state) {
        //    std::cout<<"hit in L2"<<std::endl;
            tAccess += L1.AccessTime + L2.AccessTime ;
        }

            if (state == false) {
                L2.Misses++;
             //   std::cout<<"miss in L2"<<std::endl;
                tAccess += L1.AccessTime + L2.AccessTime + MemTime;
                if(MissPolicy == WRITE_ALLOCATE){
                    L2.Insert(addrs, victim);
              //      std::cout<<std::endl;
               //     std::cout<<"inserted addrs: "<<addrs<<" into L2 victim set is: "<<victim.set<<" victim tag is: "<<victim.tag<<std::endl;
                    if (victim.tag != -1) {
                        L1.Invalidate(victim.set, victim.tag);
                    }
                }
            }
            if(MissPolicy == WRITE_ALLOCATE){
                L1.Insert(addrs, victim);
             //   std::cout<<std::endl;
              //  std::cout<<"inserted addrs: "<<addrs<<" into L1 victim set is: "<<victim.set<<" victim tag is: "<<victim.tag<<std::endl;
                L1.WriteTo(addrs);
                if (victim.tag != -1 && victim.dirty) {
                    L2.WriteTo(victim.set,victim.tag);
                }
            }

                

        
    
    }
}

