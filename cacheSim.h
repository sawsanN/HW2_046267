
#include <vector>

void DecodeAddrs(int addrs , int& offset , int& set , int& tag, int SetSize ){

    std::bitset<32> ADDRS(addrs);
    offset = 0;
    for(int i=0 ; i<5 ; i++) offset += ADDRS[i]*pow(2,i);
    set  = 0;
    for(int i=5 ; i<5+SetSize ;i++) set+= ADDRS[i]*pow(2,i);
    tag = 0;
    for(int i = 5+SetSize ; i< 32 ;i++ ) tag += ADDRS[i]*pow(2,i);



}



class Way{

    public:
        Way(int SIZE):
            Waysize(SIZE){}
        void push(int addrs , int data);






        vector<int> AccTime;
        vector<int> Tag;
        vector<vector<int>> line;
        int Waysize;
        friend void DecodeAddrs(int addrs , int& offset , int& set , int& tag);


};
class Cache{







    
    int CacheSize ; //cache size in bytes
    int BlockSize; //block size in bytes
    int ways_num; //assosotivity of cache



};

void Way::push(int addrs , int data){
    int offset , set , tag;
    DecodeAddrs(addrs , offset , set ,tag, log2(Waysize) );
    auto i_line = line.begin() + set;
    line.insert(i_line,data);
    auto i_tag = Tag.begin() + set;
    Tag.insert(i_tag,tag);

}