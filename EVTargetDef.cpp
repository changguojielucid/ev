

#include "EVFirst.h"
#include "EVTargetDef.h"

void EVTargetDef::closeOutCompletedSession()
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // we keep all the file-oriented objects, but null out the pipeline IDs
  pipeline = NULL;
  isViable = false;

  unsavedChanges = false;
  targetPipelineID = null_ebID;
  
  initializerIDdefine = null_ebID; 
  initializerIDanalyze = null_ebID;
  
  pathIDdefine = null_ebID; 
  pathIDanalyze = null_ebID;
  
  readingsIDdefine = null_ebID; 
  readingsIDanalyze = null_ebID;
  
  lesionReadingsIDdefine = null_ebID; 
  lesionReadingsIDanalyze = null_ebID;
  
  lumenSegIDdefine = null_ebID; 
  lumenSegIDanalyze = null_ebID;
  
  wallSegIDdefine = null_ebID; 
  wallSegIDanalyze = null_ebID;
  
  periRegIDdefine = null_ebID; 
  periRegIDanalyze = null_ebID;
  
  wallThickIDdefine = null_ebID; 
  wallThickIDanalyze = null_ebID;
  
  capThickIDdefine = null_ebID; 
  capThickIDanalyze = null_ebID;
  
  lumenPartIDsDefine.clear(); 
  lumenPartIDsAnalyze.clear();
  
  wallPartIDsDefine.clear(); 
  wallPartIDsAnalyze.clear();
  
  periRegPartIDsDefine.clear(); 
  periRegPartIDsAnalyze.clear();
  
  lesionLumenPartIDsDefine.clear(); 
  lesionLumenAndWallPartIDsDefine.clear(); 
  
  lesionPeriRegPartIDsDefine.clear();
  
  lesionLumenPartIDsAnalyze.clear(); 
  lesionLumenAndWallPartIDsAnalyze.clear(); 
  lesionPeriRegPartIDsAnalyze.clear();
}

EVTargetDef::EVTargetDef() :
  ID(QString("")),
  bodySite(QString(""))
{
  closeOutCompletedSession();
  probabilityMaps.clear();
  regions.clear();
  valueMaps.clear();
  keyImages.clear();
  targetFolder = "";
  readingsLocalFileName = "";
  lesionReadingsLocalFileName = "";
  targetPathFileName = "";
  registrationTransforms = "";
}

EVTargetDef::EVTargetDef(const QString &id, const QString &site) :
  ID(id),
  bodySite(site)
{
  closeOutCompletedSession();
  probabilityMaps.clear();
  regions.clear();
  valueMaps.clear();
  keyImages.clear();
  targetFolder = "";
  readingsLocalFileName = "";
  lesionReadingsLocalFileName = "";
  targetPathFileName = "";
  registrationTransforms = "";
}

EVTargetDef::~EVTargetDef()
{
  regions.clear();
  probabilityMaps.clear();
  valueMaps.clear();
}

void EVTargetDef::saveTargetDerivedDataChanges(QString baseFolder)
{
  ebLog eblog(Q_FUNC_INFO); eblog << baseFolder.toStdString() << std::endl;
  if (unsavedChanges) {
    // first, scrub all target derived data objects as the correct state will be saved by virtue of what follows
    setTargetInitializerFileName("");
    setTargetPathFileName("");
    setTargetRegistrationTransforms("");
    setTargetReadingsFileName("");
    setLesionReadingsFileName("");
    regions.clear();
    valueMaps.clear();
    probabilityMaps.clear();

    // set up a file name prefix
    QString prefix = getTargetFolder();
    prefix.append("/");
    prefix.append("getLocalUserName"); // JCG function 
    prefix.append("_");
    QDateTime dt = QDateTime::currentDateTime();
    QString dtStr = dt.toString("yyyyMMdd-hhmm");
    QStringList dtSubStr = dtStr.split('-');
    prefix.append(dtSubStr.at(0));
    prefix.append("_");

    // now begin a nested sequence of save operations based on pipeline state
    if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetInitialization()) {
      QString targetInitializer = prefix;
      targetInitializer.append("_initializer.json");
      setTargetInitializerFileName(targetInitializer);
      QString targetInitializerFull = baseFolder;
      targetInitializerFull.append("/");
      targetInitializerFull.append(getTargetInitializerFileName());
      pipeline->GetVesselTargetPipeline(targetPipelineID)->SaveInitialization(targetInitializerFull.toStdString());

      if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLumenSegmentation()) {
        QString lumenSegFileName = prefix;
        lumenSegFileName.append("_lumenSegmentation.nrrd");
        EVRegion lumenSeg("lumenSegmentation", lumenSegFileName);
        QString lumenSegFileNameFull = baseFolder;
        lumenSegFileNameFull.append("/");
        lumenSegFileNameFull.append(lumenSeg.getRegFileName());
        pipeline->GetVesselTargetPipeline(targetPipelineID)->SaveLumenSegmentation(lumenSegFileNameFull.toStdString());
        regions.append(lumenSeg);

        if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetPath()) {
          QString targetPath = prefix;
          targetPath.append("_path.json");
          setTargetPathFileName(targetPath);
          QString targetPathFull = baseFolder;
          targetPathFull.append("/");
          targetPathFull.append(getTargetPathFileName());
          pipeline->GetVesselTargetPipeline(targetPipelineID)->SavePath(targetPathFull.toStdString());

          if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLumenPartition()) {
            QString lumenPartFileName = prefix;
            lumenPartFileName.append("_lumenPartition.multi.nrrd");
            EVRegion lumenPart("lumenPartition", lumenPartFileName);
            QString lumenPartFileNameFull = baseFolder;
            lumenPartFileNameFull.append("/");
            lumenPartFileNameFull.append(lumenPart.getRegFileName());
            pipeline->GetVesselTargetPipeline(targetPipelineID)->SaveLumenPartition(lumenPartFileNameFull.toStdString());
            regions.append(lumenPart);

            if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetResampledRegisteredImages()) {
              QString registrationTransforms = prefix;
              registrationTransforms.append("_transforms.json");
              setTargetRegistrationTransforms(registrationTransforms);
              QString registrationTransformsFull = baseFolder;
              registrationTransformsFull.append("/");
              registrationTransformsFull.append(getTargetRegistrationTransforms());
              pipeline->GetVesselTargetPipeline(targetPipelineID)->SaveRegistration(registrationTransformsFull.toStdString());

              if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLumenAndWallSegmentation()) {
                QString wallSegFileName = prefix;
                wallSegFileName.append("_wallSegmentation.nrrd");
                EVRegion wallSeg("wallSegmentation", wallSegFileName);
                QString wallSegFileNameFull = baseFolder;
                wallSegFileNameFull.append("/");
                wallSegFileNameFull.append(wallSeg.getRegFileName());
                pipeline->GetVesselTargetPipeline(targetPipelineID)->SaveLumenAndWallSegmentation(wallSegFileNameFull.toStdString());
                regions.append(wallSeg);

                if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLumenAndWallPartition()) {
                  QString wallPartFileName = prefix;
                  wallPartFileName.append("_wallPartition.multi.nrrd");
                  EVRegion wallPart("wallPartition", wallPartFileName);
                  QString wallPartFileNameFull = baseFolder;
                  wallPartFileNameFull.append("/");
                  wallPartFileNameFull.append(wallPart.getRegFileName());
                  pipeline->GetVesselTargetPipeline(targetPipelineID)->SaveLumenAndWallPartition(wallPartFileNameFull.toStdString());
                  regions.append(wallPart);

                  if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetWallThickness()) {
                    QString wallThickFileName = prefix;
                    wallThickFileName.append("_wallThickness.nrrd");
                    EVValueMap wallThick("wallThickness", wallThickFileName);
                    QString wallThickFileNameFull = baseFolder;
                    wallThickFileNameFull.append("/");
                    wallThickFileNameFull.append(wallThick.getValueMapFileName());
                    pipeline->GetVesselTargetPipeline(targetPipelineID)->SaveWallThickness(wallThickFileNameFull.toStdString());
                    valueMaps.append(wallThick);

                    if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetPerivascularRegion()) {
                      QString periRegFileName = prefix;
                      periRegFileName.append("_perivascularRegion.nrrd");
                      EVRegion periReg("perivascularRegion", periRegFileName);
                      QString periRegFileNameFull = baseFolder;
                      periRegFileNameFull.append("/");
                      periRegFileNameFull.append(periReg.getRegFileName());
                      pipeline->GetVesselTargetPipeline(targetPipelineID)->SavePerivascularRegion(periRegFileNameFull.toStdString());
                      regions.append(periReg);

                      if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetPerivascularRegionPartition()) {
                        QString periRegPartFileName = prefix;
                        periRegPartFileName.append("_periRegPartition.multi.nrrd");
                        EVRegion periRegPart("periRegPartition", periRegPartFileName);
                        QString periRegPartFileNameFull = baseFolder;
                        periRegPartFileNameFull.append("/");
                        periRegPartFileNameFull.append(periRegPart.getRegFileName());
                        pipeline->GetVesselTargetPipeline(targetPipelineID)->SavePerivascularRegionPartition(periRegPartFileNameFull.toStdString());
                        regions.append(periRegPart);

                        if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetComposition()) {
                          QString compositionFileName = prefix;
                          compositionFileName.append("_composition.multi.nrrd");
                          EVProbabilityMap composition("composition", compositionFileName);
                          QString compositionFileNameFull = baseFolder;
                          compositionFileNameFull.append("/");
                          compositionFileNameFull.append(composition.getProbabilityMapFileName());
                          pipeline->GetVesselTargetPipeline(targetPipelineID)->SaveComposition(compositionFileNameFull.toStdString());
                          probabilityMaps.append(composition);

                          if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetCapThickness()) {
                            QString capThickFileName = prefix;
                            capThickFileName.append("_capThickness.nrrd");
                            EVValueMap capThick("capThickness", capThickFileName);
                            QString capThickFileNameFull = baseFolder;
                            capThickFileNameFull.append("/");
                            capThickFileNameFull.append(capThick.getValueMapFileName());
                            pipeline->GetVesselTargetPipeline(targetPipelineID)->SaveCapThickness(capThickFileNameFull.toStdString());
                            valueMaps.append(capThick);

                            if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetReadings()) {
                              QString readingsLocalFileName = prefix;
                              readingsLocalFileName.append("_readings.json");
                              setTargetReadingsFileName(readingsLocalFileName);
                              QString readingsLocalFileNameFull = baseFolder;
                              readingsLocalFileNameFull.append("/");
                              readingsLocalFileNameFull.append(getTargetReadingsFileName());
                              pipeline->GetVesselTargetPipeline(targetPipelineID)->SaveReadings(readingsLocalFileNameFull.toStdString());

                              if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLesionLumenPartition()) {
                                QString lesionLumenPartitionFileName = prefix;
                                lesionLumenPartitionFileName.append("_lesionLumenPartition.multi.nrrd");
                                EVRegion lesionLumenPart("lesionLumenPartition", lesionLumenPartitionFileName);
                                QString lesionLumenPartFileNameFull = baseFolder;
                                lesionLumenPartFileNameFull.append("/");
                                lesionLumenPartFileNameFull.append(lesionLumenPart.getRegFileName());
                                pipeline->GetVesselTargetPipeline(targetPipelineID)->SaveLesionLumenPartition(lesionLumenPartFileNameFull.toStdString());
                                regions.append(lesionLumenPart);

                                if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLesionLumenAndWallPartition()) {
                                  QString lesionLumenAndWallPartitionFileName = prefix;
                                  lesionLumenAndWallPartitionFileName.append("_lesionLumenAndWallPartition.multi.nrrd");
                                  EVRegion lesionLumenAndWallPart("lesionLumenAndWallPartition", lesionLumenAndWallPartitionFileName);
                                  QString lesionLumenAndWallPartFileNameFull = baseFolder;
                                  lesionLumenAndWallPartFileNameFull.append("/");
                                  lesionLumenAndWallPartFileNameFull.append(lesionLumenAndWallPart.getRegFileName());
                                  pipeline->GetVesselTargetPipeline(targetPipelineID)->SaveLesionLumenAndWallPartition(lesionLumenAndWallPartFileNameFull.toStdString());
                                  regions.append(lesionLumenAndWallPart);

                                  if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLesionPerivascularRegionPartition()) {
                                    QString lesionPerivascularRegionPartitionFileName = prefix;
                                    lesionPerivascularRegionPartitionFileName.append("_lesionPerivascularRegionPartition.multi.nrrd");
                                    EVRegion lesionPerivascularRegionPart("lesionPerivascularRegionPartition", lesionPerivascularRegionPartitionFileName);
                                    QString lesionPerivascularRegionPartFileNameFull = baseFolder;
                                    lesionPerivascularRegionPartFileNameFull.append("/");
                                    lesionPerivascularRegionPartFileNameFull.append(lesionPerivascularRegionPart.getRegFileName());
                                    pipeline->GetVesselTargetPipeline(targetPipelineID)->SaveLesionPerivascularRegionPartition(lesionPerivascularRegionPartFileNameFull.toStdString());
                                    regions.append(lesionPerivascularRegionPart);

                                    if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLesionReadings()) {
                                      QString lesionReadingsLocalFileName = prefix;
                                      lesionReadingsLocalFileName.append("_lesionReadings.json");
                                      setLesionReadingsFileName(lesionReadingsLocalFileName);
                                      QString lesionReadingsLocalFileNameFull = baseFolder;
                                      lesionReadingsLocalFileNameFull.append("/");
                                      lesionReadingsLocalFileNameFull.append(getLesionReadingsFileName());
                                      pipeline->GetVesselTargetPipeline(targetPipelineID)->SaveLesionReadings(lesionReadingsLocalFileNameFull.toStdString());
                                    } // end-if there are lesionReadings
                                  } // end-if there are lesionPerivascularREgionPartitions
                                } // end-if there are lesionLumenAndWallPartitions
                              } // end-if there are lesionLumenPartitions
                            } // end-if there are readings
                          } // end-if cap thickness has been computed
                        } // end-if there is composition
                      } // end-if the perivascular EVRegion has been partitioned
                    } // end-if perivascular EVRegion has been established
                  } // end-if wall thickness has been computed
                } // end-if the wall has been partitioned
              } // end-if there is a wall segmentation
            } // end-if registration transforms have been computed
          } // end-if the lumen has been partitioned
        } // end-if the path has been determined
      } // end-if the lumen has been segmented
    } // end-if there is an initializer

    unsavedChanges = false;
  }
}

