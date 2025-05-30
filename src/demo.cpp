#pragma GCC optimize(3,"Ofast","inline")

#include <numeric>
#include <algorithm>
#include "dynamic_bf_single.h"

using namespace std;

vector< pair<uint32_t, int> > test_data;
ofstream fout;

int read_data(string fname){
    cout << "Read Data From " << fname << endl;
    std::ifstream inFile(fname);
    if(!inFile){
        cout << "Open File Failed...\n";
        return 0;
    }
    else{
        cout << "Read Data Successfully." << endl;
    }
    int cnt = 0;
    uint32_t a, b;
    while(inFile >> a >> b){
        cnt ++;
        test_data.push_back(make_pair(a,b));
    }
    return cnt;
}

void test(){
    // nn = number of KV pairs
    constexpr int nn = 100000;
    auto DBF = new DynamicBloomierFilter_Single< (uint32_t)(nn * 1.70), 8u>();
    srand((int)time(0));
    srand(time(0));
    DBF->hash_byte = 4;
    DBF->hash_seed = rand();
    cout<<"range="<<DBF->Hash_Mask<<endl;
    
    timespec dtime1, dtime2;
    clock_gettime(CLOCK_MONOTONIC, &dtime1);
    int t;
    for (t = 0; t < nn; ++t){   
        if(!DBF->insert_pair(test_data[t].first, test_data[t].second)){
            cout << "Failed at key " << t << endl;
            break;
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &dtime2);
    long long delay = (long long)(dtime2.tv_sec - dtime1.tv_sec) * 1000000000LL + (dtime2.tv_nsec - dtime1.tv_nsec);
    double dth = (double)1000.0 * nn / delay;
    printf("Inserted %d KV Pairs. Update Throughput: %.5lf MOPS.\n", t, dth);

    DBF->exportDP();
    int error_cnt = 0;
    int sum = 0;
    clock_gettime(CLOCK_MONOTONIC, &dtime1);
    for(int i = 0; i < nn; i++){
        uint32_t vv;
        DBF->queryDP(test_data[i].first, vv);
        sum += vv;
        if(vv != test_data[i].second){
            error_cnt ++ ;
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &dtime2);
    delay = (long long)(dtime2.tv_sec - dtime1.tv_sec) * 1000000000LL + (dtime2.tv_nsec - dtime1.tv_nsec);
    dth = (double)1000.0 * nn / delay;
    printf("Lookup Throughput: %.5lf MOPS.\n", dth);

    cout << "Error Cnt: " << error_cnt << endl;

    size_t total_keys = 0;
    size_t total_buckets = 0;
    size_t total_memory = 0;
    for (int i = 0; i < (int)(nn * 1.70); ++i) {
        auto &b = DBF->B[i];
        total_keys += b.counter;
        if(b.counter>0) {
            total_buckets++;
        }
        total_memory += sizeof(b.counter) + sizeof(b.A) + sizeof(b.ifModified) + sizeof(b.layer1mem);
        if (b.layer2mem != nullptr) {
            total_memory += sizeof(uint32_t) * (b.counter - 4); // layer2mem里存储的元素
        }
    }
    cout<<"Memory per key="<<1.0*total_buckets/nn<<endl;
}



int main(){
    read_data("./test_data");

    test();

    return 0;
}