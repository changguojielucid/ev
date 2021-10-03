#pragma once


#include "EVMap.h"
#include "manifest.h"
#include "EVWorkItemListFileToken.h"

class EVTargetDef
{
public:
  EVTargetDef();
  EVTargetDef(const QString &id, const QString &site);
  virtual ~EVTargetDef();

  void closeOutCompletedSession();
  void saveTargetDerivedDataChanges(QString baseFolder);

  // these last across analysis tab instantiations
  QList<EVProbabilityMap> probabilityMaps;
  QList<EVRegion> regions;
  QList<EVValueMap> valueMaps;
  QList<QString> keyImages;

  // these get reset when an analysis tab closes
  QString rootVesselName;
  bool isViable;
  bool unsavedChanges;
  ebiVesselPipeline::Pointer pipeline;
  ebID targetPipelineID,
    initializerIDdefine, initializerIDanalyze,
    pathIDdefine, pathIDanalyze,
    readingsIDdefine, readingsIDanalyze,
    lesionReadingsIDdefine, lesionReadingsIDanalyze,
    lumenSegIDdefine, lumenSegIDanalyze,
    wallSegIDdefine, wallSegIDanalyze,
    wallThickIDdefine, wallThickIDanalyze,
    periRegIDdefine, periRegIDanalyze,
    capThickIDdefine, capThickIDanalyze;
  std::set<ebID> lumenPartIDsDefine, lumenPartIDsAnalyze, wallPartIDsDefine, wallPartIDsAnalyze, periRegPartIDsDefine, periRegPartIDsAnalyze, compositionSegIDsAnalyze;
  std::set<ebID> lesionLumenPartIDsDefine, lesionLumenPartIDsAnalyze, lesionLumenAndWallPartIDsDefine, lesionLumenAndWallPartIDsAnalyze, lesionPeriRegPartIDsDefine, lesionPeriRegPartIDsAnalyze;
  // processingParameters *parameters;

  void setParentPipeline(ebiVesselPipeline::Pointer p) { pipeline = p; }

  QString getID() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/  return ID; }
  void setID(const QString &id) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/  ID = id; }

  QString getBodySite() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/  return bodySite; }
  void setBodySite(QString site) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/  bodySite = site; }

  QString getTargetFolder() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/  return targetFolder; }
  bool setTargetFolder(QString workItemFolder) {
    ebLog eblog(Q_FUNC_INFO); eblog << workItemFolder.toStdString() << std::endl;
    QString absTargetFolder = workItemFolder;
    absTargetFolder.append("/");
    absTargetFolder.append(getID());
    QDir targetFolderDir(QDir::current());
    if (targetFolderDir.mkpath(absTargetFolder)) {
      targetFolder = targetFolderDir.relativeFilePath(absTargetFolder);
      return true;
    }
    else {
      targetFolder = workItemFolder;
      return false;
    }
  }

  void repointTargetFiles(QString oldIndividualID, QString newIndividualID) {
    ebLog eblog(Q_FUNC_INFO); eblog << ("repointing target files from " + oldIndividualID + " to " + newIndividualID).toStdString() << std::endl;
    targetFolder.replace(oldIndividualID, newIndividualID);
    readingsLocalFileName.replace(oldIndividualID, newIndividualID);
    targetPathFileName.replace(oldIndividualID, newIndividualID);
    targetInitializerFileName.replace(oldIndividualID, newIndividualID);
    registrationTransforms.replace(oldIndividualID, newIndividualID);

    for (int i = 0; i < probabilityMaps.size(); i++) {
      EVProbabilityMap *map = &(probabilityMaps[i]);
      map->probabilityMapFileName.replace(oldIndividualID, newIndividualID);
    }

    for (int i = 0; i < regions.size(); i++) {
      EVRegion *reg = &(regions[i]);
      reg->regFileName.replace(oldIndividualID, newIndividualID);
    }

    for (int i = 0; i < valueMaps.size(); i++) {
      EVValueMap *map = &(valueMaps[i]);
      map->valueMapFileName.replace(oldIndividualID, newIndividualID);
    }

    for (int i = 0; i < keyImages.size(); i++) {
      QString *keyImage = &(keyImages[i]);
      keyImage->replace(oldIndividualID, newIndividualID);
    }
  }

  QString getTargetInitializerFileName() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/  return targetInitializerFileName; }
  void setTargetInitializerFileName(QString name) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/  targetInitializerFileName = name; }

  QString getTargetPathFileName() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/  return targetPathFileName; }
  void setTargetPathFileName(QString name) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/ targetPathFileName = name; }

  QString getTargetReadingsFileName() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/  return readingsLocalFileName; }
  void setTargetReadingsFileName(QString name) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/ readingsLocalFileName = name; }

  QString getLesionReadingsFileName() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/  return lesionReadingsLocalFileName; }
  void setLesionReadingsFileName(QString name) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/ lesionReadingsLocalFileName = name; }

  QString getTargetRegistrationTransforms() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/  return registrationTransforms; }
  void setTargetRegistrationTransforms(QString name) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/ registrationTransforms = name; }

  void readTargetDef(const QJsonObject &json) {
    ID = json[targetID_token].toString();
    bodySite = json[bodySite_token].toString();
    targetFolder = json[targetLocalFolderName_token].toString();
    readingsLocalFileName = json[readingsLocalFileName_token].toString();
    lesionReadingsLocalFileName = json[lesionReadingsLocalFileName_token].toString();
    targetPathFileName = json[targetPathFileName_token].toString();
    targetInitializerFileName = json[targetInitializerFileName_token].toString();
    registrationTransforms = json[registrationTransforms_token].toString();

    probabilityMaps.clear();
    QJsonObject list = json[probabilityMaps_token].toObject();
    foreach(const QString key, list.keys()) {
      QString mapFile = list[key].toString();
      EVProbabilityMap map(key, mapFile);
      probabilityMaps.append(map);
    }

    regions.clear();
    list = json[regions_token].toObject();
    foreach(const QString key, list.keys()) {
      QString regFile = list[key].toString();
      EVRegion reg(key, regFile);
      regions.append(reg);
    }

    valueMaps.clear();
    list = json[valueMaps_token].toObject();
    foreach(const QString key, list.keys()) {
      QString mapFile = list[key].toString();
      qDebug() << "mapFile is" << mapFile << ", for key=" << key;
      EVValueMap map(key, mapFile);
      valueMaps.append(map);
    }

    keyImages.clear();
    QJsonArray keyImageArray = json[keyImages_token].toArray();
    for (int i = 0; i < keyImageArray.size(); ++i) {
      keyImages.append(keyImageArray[i].toString());
    }
  }

  void writeTargetDef(QJsonObject &json) const {
    //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
    json[targetID_token] = ID;
    json[bodySite_token] = bodySite;
    json[targetLocalFolderName_token] = targetFolder;
    json[readingsLocalFileName_token] = readingsLocalFileName;
    json[lesionReadingsLocalFileName_token] = lesionReadingsLocalFileName;
    json[targetPathFileName_token] = targetPathFileName;
    json[targetInitializerFileName_token] = targetInitializerFileName;
    json[registrationTransforms_token] = registrationTransforms;

    QJsonObject probabilityMapsObject;
    foreach(const EVProbabilityMap map, probabilityMaps)
      probabilityMapsObject[map.probabilityName] = map.probabilityMapFileName;
    json[probabilityMaps_token] = probabilityMapsObject;

    QJsonObject regionsObject;
    foreach(const EVRegion reg, regions)
      regionsObject[reg.regName] = reg.regFileName;
    json[regions_token] = regionsObject;

    QJsonObject valueMapsObject;
    foreach(const EVValueMap map, valueMaps)
      valueMapsObject[map.valueName] = map.valueMapFileName;
    json[valueMaps_token] = valueMapsObject;

    QJsonArray keyImageArray;
    foreach(const QString im, keyImages) {
      keyImageArray.append(im);
    }
    json[keyImages_token] = keyImageArray;
  }

  void writeTargetDef(QString parentDir, QString prefix, manifestQueue *mfiles) const
  {
    //  the json of this target has already been captured in the workitem json

    QString keyPrefix = ID + ":";
    QString nameStr = "form-data; name=\"";

    if ("" != readingsLocalFileName) {
      QString name = nameStr + keyPrefix + readingsLocalFileName_token;
      mfiles->addFile(keyPrefix + QString(readingsLocalFileName_token), QString("application/json"), name, parentDir, prefix, readingsLocalFileName);
    }

    if ("" != lesionReadingsLocalFileName) {
      QString name = nameStr + keyPrefix + lesionReadingsLocalFileName_token;
      mfiles->addFile(keyPrefix + QString(lesionReadingsLocalFileName_token), QString("application/json"), name, parentDir, prefix, lesionReadingsLocalFileName);
    }

    if ("" != targetPathFileName) {
      QString name = nameStr + keyPrefix + targetPathFileName_token;
      mfiles->addFile(keyPrefix + QString(targetPathFileName_token), QString("application/json"), name, parentDir, prefix, targetPathFileName);
    }

    if ("" != targetInitializerFileName) {
      QString name = nameStr + keyPrefix + targetInitializerFileName_token;
      mfiles->addFile(keyPrefix + QString(targetInitializerFileName_token), QString("application/json"), name, parentDir, prefix, targetInitializerFileName);
    }

    if ("" != registrationTransforms) {
      QString name = nameStr + keyPrefix + registrationTransforms_token;
      mfiles->addFile(keyPrefix + QString(registrationTransforms_token), QString("application/json"), name, parentDir, prefix, registrationTransforms);
    }

    foreach(const EVProbabilityMap map, probabilityMaps) {
      if ("" != map.probabilityMapFileName) {
        QString keyname = keyPrefix + "probabilityMaps/" + map.probabilityName;
        QString name = nameStr + keyname;
        mfiles->addFile(keyname, QString("application/octet-stream"), name, parentDir, prefix, map.probabilityMapFileName);
      }
    }
    foreach(const EVRegion reg, regions) {
      if ("" != reg.regFileName) {
        QString keyname = keyPrefix + "regions/" + reg.regName;
        QString name = nameStr + keyname;
        mfiles->addFile(keyname, QString("application/octet-stream"), name, parentDir, prefix, reg.regFileName);
      }
    }

    foreach(const EVValueMap map, valueMaps) {
      if ("" != map.valueMapFileName) {
        QString keyname = keyPrefix + "valueMaps/" + map.valueName;
        QString name = nameStr + keyname;
        mfiles->addFile(keyname, QString("application/octet-stream"), name, parentDir, prefix, map.valueMapFileName);
      }
    }

    int count = 0;
    foreach(const QString im, keyImages) {
      QString keyname = keyPrefix + QString(keyImages_token) + "[" + QString::number(count) + "]";
      QString name = nameStr + keyname;
      count++;
      mfiles->addFile(keyname, QString("application/octet-stream"), name, parentDir, prefix, im);
    }
  }

  bool updateTarget(EVTargetDef *otarget)
  {
    qInfo() << "updating target " << ID;
    otarget->ID = ID;
    otarget->bodySite = bodySite;
    otarget->targetFolder = targetFolder;
    otarget->readingsLocalFileName = readingsLocalFileName;
    otarget->lesionReadingsLocalFileName = lesionReadingsLocalFileName;
    otarget->targetPathFileName = targetPathFileName;
    otarget->targetInitializerFileName = targetInitializerFileName;
    otarget->registrationTransforms = registrationTransforms;
    for (int i = 0; i < probabilityMaps.size(); i++) {
      qInfo() << "updating target pmap " << i;
      EVProbabilityMap nmap(probabilityMaps[i].probabilityName, probabilityMaps[i].probabilityMapFileName);
      if (i < otarget->probabilityMaps.size())
        otarget->probabilityMaps[i] = nmap;
      else {
        otarget->probabilityMaps.append(nmap);
      }
    }
    for (int i = 0; i < regions.size(); i++) {
      qInfo() << "updating target EVRegion " << i;
      EVRegion nreg(regions[i].regName, regions[i].regFileName);
      if (i < otarget->regions.size())
        otarget->regions[i] = nreg;
      else {
        otarget->regions.append(nreg);
      }
    }
    for (int i = 0; i < valueMaps.size(); i++) {
      qInfo() << "updating target vmap " << i;
      EVValueMap vmap(valueMaps[i].valueName, valueMaps[i].valueMapFileName);
      if (i < otarget->valueMaps.size())
        otarget->valueMaps[i] = vmap;
      else
        otarget->valueMaps.append(vmap);
    }
    for (int i = 0; i < keyImages.size(); i++) {
      qInfo() << "updating target keyimage " << i;
      if (i < otarget->keyImages.size())
        otarget->keyImages[i] = keyImages[i];
      else
        otarget->keyImages.append(keyImages[i]);
    }
    return true;
  }

  void pushTargetParametersToPipeline(ebiVesselTargetPipeline *p) const {
    ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
    SetMetaData(p, bodySite_token, bodySite.toStdString());
  }

private:
  QString ID, bodySite, targetFolder, targetInitializerFileName, targetPathFileName, readingsLocalFileName, lesionReadingsLocalFileName, registrationTransforms;
};




