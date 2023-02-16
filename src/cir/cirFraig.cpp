/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <algorithm>
#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashMap.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

HashKey
CirMgr::getKey(CirGate* cur)
{//取得Hashkey
  assert(cur->getTypeStr() == "AIG");
  CirAndGate* _cur = reinterpret_cast<CirAndGate*>(cur);
  unsigned int a = _cur->fanin[0], b = _cur->fanin[1];
	if(a < b){//這樣才能確保fanin[a]+[b]和[b]+[a]的Key是一樣的
    return HashKey(a, b);
  } 
	else {
    return HashKey(b, a);
  }
}
/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
void
CirMgr::strash()
{
  HashMap<HashKey, CirAndGate*> hash(getHashSize(M));//建立hashtable
  for(vector<unsigned int>::iterator it = AIGinDFSOrder.begin();it != AIGinDFSOrder.end();++it){
    HashKey k = getKey(gates[*it]);
    CirAndGate* _merge = reinterpret_cast<CirAndGate*>(gates[*it]);
    if(!hash.insert(k, _merge)) {//檢查hashkey k是否已有值
				CirAndGate* _merged;
				hash.check(k, _merged);//若已有，取得_merged（即將被取代Gate）
				merging(_merged->pin[0]*2,(*it)*2,2);//取代
		}
  }
  dealWithDFS(1,0);//因更動DFS，所以需重新建制DFS
  cleanDFS_flag(1,0);
  floatingFanin.clear();//floatingFanin有可能更動，需檢查更新
  for(vector<unsigned int>::iterator it = undefs.begin();it != undefs.end();it++){
    vector<unsigned int>& fanoutList = gates[*it]->fanout;
    for(vector<unsigned int>::iterator it2 = fanoutList.begin();it2 != fanoutList.end();it2++){
        floatingFanin.push_back(*it2);
    }
  }
  sort(floatingFanin.begin(), floatingFanin.end());
  floatingFanin.erase(unique(floatingFanin.begin(), floatingFanin.end()), floatingFanin.end());
}

void
CirMgr::fraig()
{


  
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/
