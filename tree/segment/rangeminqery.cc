#include<iostream>
#include<vector>
#include<memory>
#include<iterator>
#include<algorithm>
#include<utility>
#include<random>

using std::cout;
using std::cin;
using std::endl;
using std::vector;
using std::unique_ptr;
using std::istream_iterator;
using std::copy;
using std::back_inserter;
using std::pair;
using std::random_device;
using std::mt19937;
using std::uniform_int_distribution;

bool release = false;

#define DBG if(!release)

void print_vector(const vector<int>& invec){
    if(release)
        return;
    
    bool f = false;
    int i=0;
    cout<<"[";
    for(const auto& v: invec){
        if(f){
            cout<<" ,";
        }
        f=true;
        cout<<i<<":";
        cout<<v;
        i++;
    }
    cout<<"]";
    cout<<endl;
}

// A Node in the Range Tree
// containing value,
// [left,right] range covered by the node
// left tree pointer
// right tree pointer
struct RTNode{
    int val;
    struct interval_{
        int left;
        int right;
    } interval;
    unique_ptr<RTNode> left;
    unique_ptr<RTNode> right;
};

// Builds a segment Tree from the input vector.
// The elements are duplicated in the tree.
// The Segment Tree is dynamically built. It is not Array backed.
// Each Node in the the tree is a unique ptr
unique_ptr<RTNode> BuildRangeTree(vector<int>& vec){
    unique_ptr<RTNode> root;

    vector<unique_ptr<RTNode>> base_vec;
    int i=0;
    for(const auto& v : vec){
        unique_ptr<RTNode> n(new RTNode());
        n->val = v; 
        n->interval.left = n->interval.right = i;
        base_vec.push_back(move(n));
        i++;
    }

    //Keep building the tree upwards till we hit the root
    while(base_vec.size()>1){
        vector<unique_ptr<RTNode>> new_vec;
        unique_ptr<RTNode> n;
        for(int i=0; i<base_vec.size(); i++){
            if(i%2==0){
                n.reset(new RTNode);          
                n->interval.left = base_vec[i]->interval.left;
                n->interval.right = base_vec[i]->interval.right;
                n->val = base_vec[i]->val;
                n->left = move(base_vec[i]);
            }
            else{
                n->interval.right = base_vec[i]->interval.right;
                if(n->val > base_vec[i]->val){
                    n->val = base_vec[i]->val;
                }
                n->right = move(base_vec[i]);
                new_vec.push_back(move(n));   
            }
        }
        if(n){
            new_vec.push_back(move(n));
        }
        base_vec = move(new_vec);
    }

    root = move(base_vec[0]);
    return root;
}

// updating a key in the tree
bool KeyUpdate(const unique_ptr<RTNode>& RangeTreeRoot, int index, int val){
    if(!RangeTreeRoot){
        return false;
    }

    if(RangeTreeRoot->interval.right < index || RangeTreeRoot->interval.left > index){
        return false;
    }

    if(RangeTreeRoot->interval.right == index && RangeTreeRoot->interval.left == index){
        RangeTreeRoot->val = val;
        return true;
    }

    if(KeyUpdate(RangeTreeRoot->left, index, val) ||
       KeyUpdate(RangeTreeRoot->right, index, val)){
        if(RangeTreeRoot->left && RangeTreeRoot->right){
            RangeTreeRoot->val = RangeTreeRoot->left->val;
            if(RangeTreeRoot->right->val < RangeTreeRoot->left->val){
                RangeTreeRoot->val = RangeTreeRoot->right->val;
            }
        }
        else if(RangeTreeRoot->left){
            RangeTreeRoot->val = RangeTreeRoot->left->val;
        }
        else{
            RangeTreeRoot->val = RangeTreeRoot->right->val;
        }
        return true;
    }

    return false;
}

bool KeyUpdateNaive(vector<int>& invec, int index, int value){
    if(index >= invec.size()){
        return false;
    }

    invec[index] = value;
    return true;
}

// Query a minimum in the closed range
vector<int> RangeMinQuery(const unique_ptr<RTNode>& RangeTreeRoot, int l, int r){
    if(!RangeTreeRoot){
        return {};
    }

    if(RangeTreeRoot->interval.right < l || RangeTreeRoot->interval.left > r){
        return {};
    }

    if(RangeTreeRoot->interval.left>= l && RangeTreeRoot->interval.right<=r){
        return {RangeTreeRoot->val};
    }

    auto lmin = RangeMinQuery(RangeTreeRoot->left, l, r);
    auto rmin = RangeMinQuery(RangeTreeRoot->right, l, r);

    if(lmin.size() && rmin.size()){
        if(lmin[0] < rmin[0]){
            return lmin;
        }
        return rmin;
    }

    return lmin.size()?lmin:rmin;
}

// O(n) Min query
vector<int> RangeMinNaiveQuery(const vector<int>& inv, int l, int r){
    if(r < l || r >= inv.size()){
        return {};
    }

    int min=inv[l];
    for(int i=l+1; i<=r; i++){
        if(min>inv[i]){
            min = inv[i];
        }
    }

    return {min};
}

bool randomtest(unsigned int testsize=100000){
    if(testsize>100000){
        testsize = 100000;
    }

    random_device rd;  
    mt19937 gen(rd()); 
    uniform_int_distribution<> valuedis(1,200000000);
    uniform_int_distribution<> indexdis(0,testsize-1);

    //Generate some random input
    vector<int> invec;
    for(int i=0; i<testsize; i++){
        const auto val = valuedis(gen);
        invec.push_back(val);
    }
    print_vector(invec);

    // Build a range Tree
    unique_ptr<RTNode> RangeTreeRoot = BuildRangeTree(invec);

    //Query test 1
    for(int i=0; i<testsize/2; i++){
        int l_idx = indexdis(gen);
        int r_idx = indexdis(gen);
        const auto& r1 = RangeMinQuery(RangeTreeRoot, l_idx, r_idx);
        const auto& r2 = RangeMinNaiveQuery(invec, l_idx, r_idx);
        if(r1 != r2){
            cout<<"RangeMinQuery has a bug"<<endl;
            cout<<"Minimum in ["<<l_idx<<","<<r_idx<<"]:"<<endl;
            cout<<"RangeMinQuery returned ";
            if(r1.size())cout<<r1[0]<<endl;
            else cout<<"empty"<<endl;
            cout<<"RangeMinQueryNaive returned ";
            if(r2.size())cout<<r2[0]<<endl;
            else cout<<"empty"<<endl;            
            return false;
        }

        DBG
        if(r1.size()){
            cout<< "Minimum in ["<<l_idx<<","<<r_idx<<"] ="<<r1[0]<<endl;
        }
        else{
            cout<< "Minimum in ["<<l_idx<<","<<r_idx<<"] is empty"<<endl;
        }
    }

    //Update Test
    for(int i=0; i<testsize/2; i++){
        int idx = indexdis(gen);
        int val = valuedis(gen);
        const auto& r1 = KeyUpdate(RangeTreeRoot, idx, val);
        const auto& r2 = KeyUpdateNaive(invec, idx, val);
        if(r1 != r2){
            cout<<"A bug in update"<<endl;
            return false;
        }
        else{
            DBG cout<<"Updated "<<idx<<" to "<<val<<endl;
        }
    }

    print_vector(invec);

    //Query test 2
    for(int i=0; i<testsize/2; i++){
        int l_idx = indexdis(gen);
        int r_idx = indexdis(gen);
        const auto& r1 = RangeMinQuery(RangeTreeRoot, l_idx, r_idx);
        const auto& r2 = RangeMinNaiveQuery(invec, l_idx, r_idx);
        if(r1 != r2){
            cout<<"RangeMinQuery post update has a bug"<<endl;
            cout<<"Minimum in ["<<l_idx<<","<<r_idx<<"]:"<<endl;
            cout<<"RangeMinQuery returned ";
            if(r1.size())cout<<r1[0]<<endl;
            else cout<<"empty"<<endl;
            cout<<"RangeMinQueryNaive returned ";
            if(r2.size())cout<<r2[0]<<endl;
            else cout<<"empty"<<endl;
            return false;
        }

        DBG
        if(r1.size()){
            cout<<"Minimum in ["<<l_idx<<","<<r_idx<<"] ="<<r1[0]<<endl;
        }
        else{
            cout<<"Minimum in ["<<l_idx<<","<<r_idx<<"] is empty"<<endl;
        }        
    }

    return true;
}

int main(int argc, const char* argv[]){
    int test_num = 20;
    if(argc==2){
        int i = std::atoi(argv[1]);
        if(i==1){
            release=true;
            test_num=100000;
        }
    }

    if(randomtest(test_num)== false){
        cout<<"RangeMinQuery Test Failed"<<endl;
        return 1;
    }
    cout<<"RangeMinQuery Tested Successfully"<<endl;
}
