#ifndef __BRANCH_PREDICTOR_HPP__
#define __BRANCH_PREDICTOR_HPP__

#include <vector>
#include <bitset>
#include <cassert>
#include <bits/stdc++.h>
struct BranchPredictor {
    virtual bool predict(uint32_t pc) = 0;
    virtual void update(uint32_t pc, bool taken) = 0;
};

struct SaturatingBranchPredictor : public BranchPredictor {
    std::vector<std::bitset<2>> table;
    SaturatingBranchPredictor(int value) : table(1 << 14, value) {}

    bool predict(uint32_t pc) {
        // your code here
        bool branch_pred=false;         //false-not taken (0);  true- taken (1);
        int fort=pow(2,14);
        int index= pc % fort;      //last 14 LSBs value- the index for the above table
        if(table[index].to_ulong()<2){
        	branch_pred=false;       //predicting no branch taken (0)
        }else {     
        	branch_pred=true;        //predicting branch taken (1)
        }
        return branch_pred;
    }

    void update(uint32_t pc, bool taken) {
        // your code here
        int fort =pow(2,14);
        int index= pc % fort;
        if(taken== true){
        	if(table[index].to_ulong()<3){
        		table[index]=table[index].to_ulong()+1;
        	}
        }else {
        	if(table[index].to_ulong()>0){
        		table[index]=table[index].to_ulong()-1;
        	}
        }
    }
};

struct BHRBranchPredictor : public BranchPredictor {
    std::vector<std::bitset<2>> bhrTable;
    std::bitset<2> bhr;
    BHRBranchPredictor(int value) : bhrTable(1 << 2, value), bhr(value) {}

    bool predict(uint32_t pc) {
        // your code here
        int bhr_int= bhr.to_ulong();
        bool branch_pred=false;
        int sat_counter=bhrTable[bhr_int].to_ulong();
        if(sat_counter<2){
        	branch_pred=false;
        }else{
        	branch_pred=true;
        }
        return branch_pred;
    }

    void update(uint32_t pc, bool taken) {
        // your code here
        int bhr_int=bhr.to_ulong();
        int taken_int;
        if(taken==true){
        	taken_int=1;
        }else{
        	taken_int=0;
        }
        if(taken==true){
        	if(bhrTable[bhr_int].to_ulong()<3){
        		bhrTable[bhr_int]= bhrTable[bhr_int].to_ulong()+1;
        	}	
        }else{
        	if(bhrTable[bhr_int].to_ulong()>0){
        		bhrTable[bhr_int]= bhrTable[bhr_int].to_ulong()-1;
        	}
        }  //bhrTable's saturating counter updated.
        bhr= (bhr<<=1).to_ulong()+ taken_int;     //bhr updated.
    }
};

struct SaturatingBHRBranchPredictor : public BranchPredictor {
    std::vector<std::bitset<2>> bhrTable;
    std::bitset<2> bhr;
    std::vector<std::bitset<2>> table;
    std::vector<std::bitset<2>> combination;
    SaturatingBHRBranchPredictor(int value, int size) : bhrTable(1 << 2, value), bhr(value), table(1 << 14, value), combination(size, value) {
        assert(size <= (1 << 16));
    }

    bool predict(uint32_t pc) {
        // your code here
        bool branch_pred=false;         //false-not taken (0);  true- taken (1);
        int fort=pow(2,14);
        int index= pc % fort;      //last 14 LSBs value- the index for the above table
        int bhr_int= bhr.to_ulong();
        //int index_renewed=pow(2,16)-1-(index+pow(2,14)*bhr_int);
        //std::cout<<index+pow(2,14)*bhr_int<< " ";
        //std::cout<<index_renewed<<" ";
        int x=(index+pow(2,14)*bhr_int);
        int index_renewed=x%combination.size();
        	//index += pow(2,12)*bhr_int;   //16 bit index in combination vector: contcatenation of bhr+index
        	int counter=combination[index_renewed].to_ulong();
        	if(counter<2){
        		branch_pred=false;
        	}else{
        		branch_pred=true;
        	}	
        return branch_pred;
    }

    void update(uint32_t pc, bool taken) {
        // your code here
        int bhr_int=bhr.to_ulong();
        int fort =pow(2,14);
        int index= pc % fort;
        int taken_int;
        if(taken==true){
        	taken_int=1;
        }else{
        	taken_int=0;
        }
        if(taken==true){
        	if(bhrTable[bhr_int].to_ulong()<3){
        		bhrTable[bhr_int]= bhrTable[bhr_int].to_ulong()+1;
        	}	
        }else{
        	if(bhrTable[bhr_int].to_ulong()>0){
        		bhrTable[bhr_int]= bhrTable[bhr_int].to_ulong()-1;
        	}
        }  //bhrTable's saturating counter updated.
        bhr= (bhr<<=1).to_ulong()+ taken_int;     //bhr updated.
        
        
        if(taken== true){
        	if(table[index].to_ulong()<3){
        		table[index]=table[index].to_ulong()+1;
        	}
        }else {
        	if(table[index].to_ulong()>0){
  	      		table[index]=table[index].to_ulong()-1;
        	}
        }
        int t1=bhrTable[bhr_int].to_ulong();  //bhr
        int t2=table[index].to_ulong();       //sat_counter
        int t3;
        //code for t3: saturating counter of combination 
        if(t2==0){
        	t3=0;
        }else if((t1==0 && t2==1)||(t1==0 && t2==2)||(t1==1 && t2==1)||(t1==2 && t2==1)){
        	t3=1;
        }else if((t1==0 && t2==3)||(t1==1 && t2==2)||(t1==1 && t2==3)||(t1==3 && t2==1)){
        	t3=2;
        }else{
        	t3=3;
        }
        //index +=pow(2,12)*bhr_int;   //16 bit index in combination vector: contcatenation of bhr+index
        //int index_renewed=pow(2,16)-1-(index+pow(2,14)*bhr_int);
        int x=(index+pow(2,14)*bhr_int);
        int index_renewed=x%combination.size();
        //std::cout<<index_renewed<<std::endl;
        //if(index_renewed<combination.size()){
        	combination[index_renewed]=t3;        //we are updating comb[index] value after making updates to table[] and bhrTable[], although, bhr is previous
        //}
    }
};

#endif
