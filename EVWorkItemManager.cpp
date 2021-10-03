#include "EVWorkItemManager.h"
#include "EVMacros.h"


EVWorkItemManager::EVWorkItemManager()
{
}


EVWorkItemManager::~EVWorkItemManager()
{
  EVWorkItemThreadMap::const_iterator item = m_mapWorkItemThread.constBegin();
  while (item != m_mapWorkItemThread.constEnd())
  {
    EVWorkItemThread* pWorkerThread = item.value();
    ebAssert(pWorkerThread);

    QThread* pThread = pWorkerThread->GetThread();
    ebAssert(pThread)

    pThread->quit();
    pThread->wait();

    SAFE_REMOVAL(pWorkerThread)
  }
}

EVWorkItemThread* EVWorkItemManager::AddWorkItemThread()
{
  EVWorkItemThread* pWorkItemThread = new EVWorkItemThread();
  m_mapWorkItemThread.insert(m_mapWorkItemThread.size(), pWorkItemThread);
  return pWorkItemThread;
}



