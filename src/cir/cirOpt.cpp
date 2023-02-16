/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <algorithm>
#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
void
CirMgr::sweep(){
  //利用包含未定義Gate的DFS來建立不在DFS中且不包含未定義Gate的GateList,notInDFS_withoutUNDEF，
  notInDFS_withoutUNDEF.clear();
  for(unsigned int i=1;i<=M;++i){
    if(gates[i]){
      notInDFS_withoutUNDEF.insert(i);
    }
  }
  for(unsigned int i = 0;i < dfsOrderWithUNDEF.size();++i){
    notInDFS_withoutUNDEF.erase(dfsOrderWithUNDEF[i]);
  }
  vector<bool> removeList;
  removeList.reserve(M+O+1);
  for(unsigned int i = 0;i < M+O+1;++i){
    removeList.push_back(false);
  }
  //利用notInDFS_withoutUNDEF建立removeList(DFS外的所有Gate),
  for(set<unsigned int>::iterator it = notInDFS_withoutUNDEF.begin();it != notInDFS_withoutUNDEF.end();++it){
    if(gates[*it]->gateType == PI_GATE){
      removeList[*it] = false;
    }else{
        cout << "Sweeping: " << gates[*it]->getTypeStr() << "(" <<  reinterpret_cast<CirAndGate*>(gates[*it])->pin[0] << ") removed...\n";
        removeList[*it] = true;
    }
  }
  //更新未定義Gate（UNDEF）,floating的fanin,跟定義但未使用Gate,
  removeTable(removeList,1,1,1,0,1);
  //將removeList中的Gate刪除
  for(unsigned int i=0;i<=M;++i){
    if(removeList[i]){
        delete gates[i];
        gates[i] = NULL;
    }
  }
  //因不能刪除PI，所以更新DFS外無法使用的PI Gate,
  for(unsigned int i = 0; i< PI.size();++i){
    if(gates[ PI[i]/2]->fanout.empty()){
        notInDFS_DfButNtUs.push_back(PI[i]/2);
    }
  }
  //將重複的PI Gate刪除
  sort(notInDFS_DfButNtUs.begin(), notInDFS_DfButNtUs.end());
  notInDFS_DfButNtUs.erase(unique(notInDFS_DfButNtUs.begin(), notInDFS_DfButNtUs.end()), notInDFS_DfButNtUs.end());
}




// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void
CirMgr::optimize(){
  for(vector<unsigned int>::iterator it = AIGinDFSOrder.begin();it != AIGinDFSOrder.end();++it){
    if(gates[*it]->fanin[0] == 1 || gates[*it]->fanin[1] == 1){//optimize()分為四種AND Gate的fanin型式，merge涵式為merge(欲結合其他人的Gate ID,被結合（將消失）的Gate ID)
      if(gates[*it]->fanin[0] == 1){
        merging((gates[*it]->fanin[1]/2)*2,2*(*it)+gates[*it]->fanin[1]%2,1);//[1]+[..]
      }else{
        merging((gates[*it]->fanin[0]/2)*2,2*(*it)+gates[*it]->fanin[0]%2,1);//[..]+[1]
      }
    }else if(gates[*it]->fanin[0] == 0 || gates[*it]->fanin[1] == 0){
      if(gates[*it]->fanin[0] == 0){
        merging(0, 2*(*it),1);//[0]+[..]
      }else{
        merging(0, 2*(*it),1);//[..]+[0]
      }
    }else if(gates[*it]->fanin[0] == gates[*it]->fanin[1]){
      merging((gates[*it]->fanin[1]/2)*2,2*(*it)+gates[*it]->fanin[1]%2,1);//[..]+[..],[!..]+[!..]
    }else if(gates[*it]->fanin[0]/2 == gates[*it]->fanin[1]/2){
      merging(0, 2*(*it),1);//[!..]+[..]
    }
  }//optimize需處理UNDEF Gate，並將其刪除
  for(vector<unsigned int>::iterator it = undefs.begin();it != undefs.end();){
    if(gates[*it]->fanout.empty()){
      delete gates[*it];
      gates[*it] = NULL;
      it = undefs.erase(it);
    }else{
      ++it;
    }
  }//因merge會改變DFS，重新建立DFS
  dealWithDFS(1,0);
  cleanDFS_flag(1,0);
  floatingFanin.clear();//Floating Fanin會因UNDEF的刪除，所以需更新Floating Fanin
  for(vector<unsigned int>::iterator it = undefs.begin();it != undefs.end();++it){
    vector<unsigned int>& fanoutList = gates[*it]->fanout;
    for(vector<unsigned int>::iterator it2 = fanoutList.begin();it2 != fanoutList.end();++it2){
        floatingFanin.push_back(*it2);
    }
  }
  sort(floatingFanin.begin(), floatingFanin.end());
  floatingFanin.erase(unique(floatingFanin.begin(), floatingFanin.end()), floatingFanin.end());
  notInDFS_DfButNtUs.clear();//merge也會導致產生一些PO連不到的Gate，所以也需更新notInDFS_DfButNtUs
  for(unsigned int i = 1;i <= M;i++){
    if(gates[i]){
      if(gates[i]->gateType != UNDEF_GATE && gates[i]->fanout.empty()){
        notInDFS_DfButNtUs.push_back(i);
      }
    }
  }
  sort(notInDFS_DfButNtUs.begin(), notInDFS_DfButNtUs.end());
  notInDFS_DfButNtUs.erase(unique(notInDFS_DfButNtUs.begin(), notInDFS_DfButNtUs.end()), notInDFS_DfButNtUs.end());
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/
void CirMgr::merging(unsigned int _merge, unsigned int _merged, int _mergedID){
  if(_mergedID==1)//處理merge型式
  cout << "Simplifying" << ": " << inv(_merge) << _merge/2 << " merging " << inv(_merged) << _merged/2 << "..." << endl;
  else if(_mergedID==2)
  cout << "Strashing" << ": " << inv(_merge) << _merge/2 << " merging " << inv(_merged) << _merged/2 << "..." << endl;
  if(_merge == 0){//[0]+[..] or [0]+[..]
    if(gates[_merged/2]->fanin[0] == 0 || gates[_merged/2]->fanin[1] == 0){
      unsigned int target;
      if(gates[_merged/2]->fanin[0] == 0){
         target = gates[_merged/2]->fanin[1] ;//[0]+[..]
      }else{
         target =  gates[_merged/2]->fanin[0];}//[..]+[0]
      rematchFanout(_merged/2, 0, &gates[_merged/2]->fanout);//重新建立Fanout,連接到即將刪除Gate的Fanin
      rematchFanin(_merged/2, _merged%2, &gates[_merged/2]->fanout);//重新建立Fanin,連接即將刪除Gate的Fanout的Fanin
      if(target/2!=0){//若非[0]+[0]
        rematchFanout(_merged/2,target, NULL);//重新建立Fanout,連接到即將刪除Gate的Fanin
      }
    }else {//![..]+[..]
      for(vector<unsigned int>::iterator it = gates[_merged/2]->fanout.begin();it != gates[_merged/2]->fanout.end();++it){
        gates[0]->fanout.push_back(*it); // CONST 0 is not in original fanout, so add directly
      }
      rematchFanin(_merged/2, 0,&gates[_merged/2]->fanout);
      rematchFanout(_merged/2, gates[_merged/2]->fanin[0], NULL);
      if(gates[_merged/2]->fanin[1] != gates[_merged/2]->fanin[0]){
        rematchFanout(_merged/2, gates[_merged/2]->fanin[1], NULL);
      }
    }//[1]+[..]or[..]+[1]or[..]+[..]or![..]+![..]
  }else if(gates[_merged/2]->fanin[0]/2 == _merge/2 || gates[_merged/2]->fanin[1]/2 == _merge/2){
    unsigned int target;
      if(gates[_merged/2]->fanin[0] == _merge/2){
         target = gates[_merged/2]->fanin[1] ;
      }else{
         target =  gates[_merged/2]->fanin[0];}
    rematchFanout(_merged/2, _merge, &gates[_merged/2]->fanout);
    if(target == 1){
      rematchFanout(_merged/2, 0, NULL);
    }else if(gates[_merged/2]->fanin[0]/2 != gates[_merged/2]->fanin[1]/2){
      rematchFanout(_merged/2,target,NULL);
    }
    rematchFanin(_merged/2, _merge ^ (_merge%2 != _merged%2),&gates[_merged/2]->fanout);
  }else{//strash
    rematchFanout(_merged/2,gates[_merged/2]->fanin[0],NULL);
    if(gates[_merged/2]->fanin[0]/2 != gates[_merged/2]->fanin[1]/2){
      rematchFanout(_merged/2,gates[_merged/2]->fanin[1],NULL);
    }
    rematchFanin(_merged/2, _merge ,&gates[_merged/2]->fanout);
    for(vector<unsigned int>::iterator it = gates[_merged/2]->fanout.begin();it != gates[_merged/2]->fanout.end();++it){
      gates[_merge/2]->fanout.push_back(*it); 
    }
  }
  delete gates[_merged/2];
  gates[_merged/2] = NULL;
}

void 
CirMgr::rematchFanin(unsigned int originGtId,unsigned int replaceId,vector<unsigned int>* originGt_fanout){
  if(originGtId == replaceId/2){
      return;
  }
  for(vector<unsigned int>::iterator it = originGt_fanout->begin();it!=originGt_fanout->end();++it){
    if(gates[*it]->gateType == AIG_GATE){
      if(gates[*it]->fanin[0]/2==originGtId){
        reinterpret_cast<CirAndGate*>(gates[*it])->pin[1] = replaceId/2; // pin and inv are o, i1, i2
        reinterpret_cast<CirAndGate*>(gates[*it])->inv[1] = (replaceId+reinterpret_cast<CirAndGate*>(gates[*it])->inv[1])%2;
        gates[*it]->fanin[0] = 2*reinterpret_cast<CirAndGate*>(gates[*it])->pin[1]+reinterpret_cast<CirAndGate*>(gates[*it])->inv[1];
      }
      else{
        reinterpret_cast<CirAndGate*>(gates[*it])->pin[2] = replaceId/2; // pin and inv are o, i1, i2
        reinterpret_cast<CirAndGate*>(gates[*it])->inv[2] = (replaceId+reinterpret_cast<CirAndGate*>(gates[*it])->inv[2])%2;
        gates[*it]->fanin[1] = 2*reinterpret_cast<CirAndGate*>(gates[*it])->pin[2]+reinterpret_cast<CirAndGate*>(gates[*it])->inv[2];
      }
    }
    else if(gates[*it]->gateType == PO_GATE)
    {
      if(gates[*it]->fanin[0]/2 == originGtId){
        reinterpret_cast<CirIOGate*>(gates[*it])->id = replaceId/2; // pin and inv are o, i1, i2
        reinterpret_cast<CirIOGate*>(gates[*it])->inverted = (replaceId+reinterpret_cast<CirIOGate*>(gates[*it])->inverted)%2;
        gates[*it]->fanin[0] = 2*reinterpret_cast<CirIOGate*>(gates[*it])->id+reinterpret_cast<CirIOGate*>(gates[*it])->inverted;
      }
    }
  }
}
void CirMgr::rematchFanout(unsigned int originGtId, unsigned int replaceId, vector<unsigned int>* originGt_fanout)
{
  for(vector<unsigned int>::iterator it = gates[replaceId/2]->fanout.begin();it != gates[replaceId/2]->fanout.end();){
    if(*it == originGtId){
      gates[replaceId/2]->fanout.erase(it);
    }else{
        ++it;
    }
  }
  if(originGt_fanout != NULL){
    for(vector<unsigned int>::iterator it = originGt_fanout->begin();it != originGt_fanout->end();++it){
        gates[replaceId/2]->fanout.push_back(*it);
    }
  }
}