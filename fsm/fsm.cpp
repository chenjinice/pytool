#include "fsm.h"
#include <unistd.h>


#define ClassName "Fsm"


FsmItem::FsmItem()
{
//    m_curState      = StateNone;
//    m_nextState     = StateNone;
//    m_errState      = StateErr;
//    m_fun           = nullptr;
}

FsmItem::FsmItem(ItemFun fun, FsmState cur, FsmState next, FsmState err)
{
    m_fun           = fun;
    m_curState      = cur;
    m_nextState     = next;
    m_errState      = err;
}

FsmItem::~FsmItem()
{

}


// --------------------------------------------------------

Fsm::Fsm()
{
    this->init();
}


Fsm::~Fsm()
{

}

void Fsm::init()
{

}

bool Fsm::handle(FsmState state)
{
    bool ret        = false;
    int count       = m_table.size();
    int index       = -1;
    ItemFun fun     = nullptr;

    for(int i=0;i<count;i++){
        if(m_table[i].m_curState == state){
            index   = i;
            break;
        }
    }
    if(-1 == index){
        printf(ClassName":state %d not exist\n",state);
        return  ret;
    }

    fun = m_table[index].m_fun;
    if(nullptr == fun){
        printf(ClassName":state %d, fun = nullptr\n",state);
        return  ret;
    }

    if(fun()){
        m_state     = m_table[index].m_nextState;
    }else{
        m_state     = m_table[index].m_errState;
    }

    sleep(1);
    ret = this->handle(m_state);
    return ret;
}



