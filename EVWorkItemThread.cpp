#include "EVWorkItemThread.h"
#include "EVMacros.h"


EVWorkItemThread::EVWorkItemThread()
{
  m_pThread = new QThread();
  m_pWorkItem = new EVWorkItem();
}


EVWorkItemThread::~EVWorkItemThread()
{
  SAFE_REMOVAL(m_pThread);
  SAFE_REMOVAL(m_pWorkItem);
}
