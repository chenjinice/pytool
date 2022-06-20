#include "TestFsm.h"


FsmItem::FsmItem()
{

}

FsmItem::~FsmItem()
{

}


// --------------------------------------------------------

TestFsm *TestFsm::Ins()
{
    static TestFsm instance;
    return &instance;
}

TestFsm::TestFsm()
{
    this->Init();
}


TestFsm::~TestFsm()
{

}

void TestFsm::Init()
{

}


