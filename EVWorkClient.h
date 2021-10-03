#pragma once

#include <QMap>
#include "EVWorkItemManager.h"

class EVWorkClient : public  QObject
{
  Q_OBJECT
public:
  EVWorkClient();
  ~EVWorkClient();


  bool CreateWorkItem(const std::string& seriesPath);
  EVWorkItemManager* GetWorkItemManger() { return m_pWorkItemManager; }

public slots:
  void handleResults(const QString &);


signals:
  void operate(const QString &);

private:
  EVWorkItemManager* m_pWorkItemManager;

};

