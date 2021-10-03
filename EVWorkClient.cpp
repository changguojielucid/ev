#include "EVFirst.h"
#include "EVWorkClient.h"
#include "EVLinkedViewers.h"
#include "EVTargetDefine.h"
#include "EVWorkItem.h"
#include "EVMacros.h"
#include "EVWorkItemThread.h"
#include "EVWorkItemManager.h"


EVWorkClient::EVWorkClient()
{
  m_pWorkItemManager = new EVWorkItemManager();
}

bool EVWorkClient::CreateWorkItem(const std::string& seriesPath)
{

  EVWorkItemManager* pWorkItemManager = GetWorkItemManger();
  ebAssert(pWorkItemManager);

  EVWorkItemThread* pWorkItemThread = pWorkItemManager->AddWorkItemThread();
  ebAssert(pWorkItemThread);


  EVWorkItem* pWorkItem = pWorkItemThread->GetWorkItem();
  ebAssert(pWorkItem);

  QThread* pThread = pWorkItemThread->GetThread();
  ebAssert(pThread);

  pWorkItem->moveToThread(pThread);
  pWorkItem->LoadImageFromSeriesPath(seriesPath);
  pWorkItem->RenderImage();

  connect(pThread, &QThread::finished, pWorkItem, &QObject::deleteLater);
  connect(this, &EVWorkClient::operate, pWorkItem, &EVWorkItem::doWork);
  connect(pWorkItem, &EVWorkItem::resultReady, this, &EVWorkClient::handleResults);

  pThread->start();

  return true;
}

EVWorkClient::~EVWorkClient()
{
  SAFE_REMOVAL(m_pWorkItemManager)
}

void EVWorkClient::handleResults(const QString &)
{

}