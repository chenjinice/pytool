#ifndef FSM_H
#define FSM_H


#include <iostream>
#include <vector>


typedef bool (*ItemFun)();


// 状态枚举
enum FsmState
{
    StateNone                   = 0,
    StateStart,
    StateRun,
    StateError,
    StateEnd,
    StateErr,
};


// --------------------------
class FsmItem
{
public:
    FsmItem();
    FsmItem(ItemFun fun,FsmState cur,FsmState next,FsmState err = StateErr);
    ~FsmItem();

    FsmState    m_curState      = StateNone;
    FsmState    m_nextState     = StateNone;
    FsmState    m_errState      = StateErr;
    ItemFun     m_fun           = nullptr;

private:

};


// ---------------------------
class Fsm
{
public:
    Fsm();
    ~Fsm();
    bool                        handle(FsmState state);

    std::vector<FsmItem>        m_table;
    FsmState                    m_state         = StateNone;

private:
    void                        init();


};



#endif // TESTFSM_H


