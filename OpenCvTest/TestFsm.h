#ifndef TESTFSM_H
#define TESTFSM_H


#include <EwayFunc/FuncInterface.h>
#include <vector>


// 状态枚举
enum FsmState
{
    StateStart                  = 0,
    StateRun,
    StateError,
    StateEnd,
};

// --------------------------
class FsmItem
{
public:
    FsmItem();
    ~FsmItem();

private:

};

// ---------------------------
class TestFsm
{
public:
    static TestFsm *            Ins();
    ~TestFsm();


private:
    TestFsm();
    void                        Init();

    std::vector<FsmItem>        m_vTable;
    FsmState                    m_nState            = StateStart;

};



#endif // TESTFSM_H


