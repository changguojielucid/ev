#pragma once

#include <QMap>
#include "EVWorkItemThread.h"

typedef QMap<int, EVWorkItemThread*> EVWorkItemThreadMap;

class EVWorkItemManager
{
public:
  EVWorkItemManager();
  ~EVWorkItemManager();

  EVWorkItemThread* AddWorkItemThread();
  EVWorkItemThreadMap* GetWorkItemThreadMap() { return &m_mapWorkItemThread;  }

private:
  EVWorkItemThreadMap m_mapWorkItemThread;

};

