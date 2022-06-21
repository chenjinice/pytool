#include <QCoreApplication>
#include "fsm.h"



bool start(){
    printf("start\n");
    return true;
}

bool end(){
    printf("end\n");
    return true;
}

bool run(){
    printf("run\n");
    return false;
}

bool err(){
    printf("err\n");
    return true;
}


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Fsm  fsm;
    fsm.m_table.push_back(FsmItem(start,StateStart,StateRun));
    fsm.m_table.push_back(FsmItem(run,StateRun,StateEnd));
    fsm.m_table.push_back(FsmItem(end,StateEnd,StateNone,StateNone));
    fsm.m_table.push_back(FsmItem(err,StateErr,StateNone,StateNone));


    fsm.handle(StateStart);

    return a.exec();
}
