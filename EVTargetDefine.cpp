#include "EVFirst.h"
#include "EVTargetDef.h"
#include "EVTargetDefine.h"
#include "EVMsgDefine.h"
#include "EVWorkItem.h"

#define tr  

EVTargetDefine::EVTargetDefine(EVWorkItem* pWorkItem)
{
  m_pWorkItem = pWorkItem;
  m_pTargetCurrent = NULL;
 
  viableTargetsCount = 0;
  definingRootFlag = true;
  editingSegmentation = false;
}

EVTargetDefine::~EVTargetDefine()
{
}

ebiVesselPipeline* EVTargetDefine::GetVesselPipeine()
{
  EVWorkItem* pWorkItem = GetWorkItem();
  ebAssert(pWorkItem);
  return pWorkItem->GetVesselPipeline();
}

vtkSmartPointer<EVLinkedViewers> EVTargetDefine::GetEVLinkedViewers()
{
  EVWorkItem* pWorkItem = GetWorkItem();
  ebAssert(pWorkItem);
  return pWorkItem->GetEVLinkerViewers();
}

EVImageSeries* EVTargetDefine::GetImageSeries()
{
  EVWorkItem* pWorkItem = GetWorkItem();
  ebAssert(pWorkItem);
  return pWorkItem->GetImageSeries();
}

ebID EVTargetDefine::GetImage4ID()
{
  EVImageSeries* pImageSeries = GetImageSeries();
  ebAssert(pImageSeries);
  return pImageSeries->image4IDdefine;
}

QList<EVTargetDef>* EVTargetDefine::GetTargets()
{
  EVWorkItem* pWorkItem = GetWorkItem();
  ebAssert(pWorkItem);
  return pWorkItem->GetTargetDefs();
}

void EVTargetDefine::preloadDefinePre(QString product)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;

  setDefiningRootFlag(false);

  //client should update UI states
  PromptMessage(EV_MSG_PRELOAD_DEFINE_PRE_FINISHED);
}

void EVTargetDefine::processingParametersSettingsChanged()
{
  EVTargetDef* currentTarget = GetCurrentTarget();
  ebAssert(currentTarget);

  EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
  ebAssert(pEVLineedViewers);

  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  clearTargetBeyondCurrentStage(currentTarget);
  currentTarget->unsavedChanges = true;
  logUpdateWithStageSettings("EVTargetDefine::processingParametersSettingsChanged", currentTarget, currentTarget->getID());
  pEVLineedViewers->SyncViewersToScene();
  pEVLineedViewers->Render();
}

void EVTargetDefine::presentTarget(EVTargetDef *def)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;

  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);

  EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
  ebAssert(pEVLineedViewers)

  // work forward, as deep as can go based on what is in place (except not as far as patientAnalyze goes)
  if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetInitialization()) {
    def->initializerIDdefine = pEVLineedViewers->AddVesselTarget(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetInitialization(), "initializer");
    if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenSegmentation()) { 
      def->lumenSegIDdefine = pEVLineedViewers->AddSegmentation4(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenSegmentation(), 0, "lumenSegmentation");
      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetPath()) {
        pEVLineedViewers->RemoveVesselTarget(def->initializerIDdefine); // if have a path, don't want to display the initializer too
        def->initializerIDdefine = null_ebID;
        def->pathIDdefine = pEVLineedViewers->AddVesselTarget(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetPath(), "path");
        if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenPartition()) {
          pEVLineedViewers->RemoveSegmentation4(def->lumenSegIDdefine); // if have partitions, don't want to display the segmentation too
          def->lumenSegIDdefine = null_ebID;
          def->lumenPartIDsDefine.clear();
          int i = 0;
          for (auto partition : *pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenPartition())
            def->lumenPartIDsDefine.insert(pEVLineedViewers->AddSegmentation4(partition.GetPointer(), 0, "lumenPartition"+std::to_string(i++)));
          if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenAndWallSegmentation()) {
            def->wallSegIDdefine = pEVLineedViewers->AddSegmentation4(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenAndWallSegmentation(), 0, "wallSegmentation");
            pEVLineedViewers->GetScene()->GetSegmentation4(def->wallSegIDdefine)->GetInteriorProperty()->SetOpacity(0);
            if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenAndWallPartition()) {
              pEVLineedViewers->RemoveSegmentation4(def->wallSegIDdefine); // if have partitions, don't want to display the segmentation too
              def->wallSegIDdefine = null_ebID;
              def->wallPartIDsDefine.clear();
              int i = 0;
              for (auto partition : *pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenAndWallPartition()) {
                ebID partID = pEVLineedViewers->AddSegmentation4(partition.GetPointer(), 0, "wallPartition"+std::to_string(i++));
                def->wallPartIDsDefine.insert(partID);
                pEVLineedViewers->GetScene()->GetSegmentation4(partID)->GetInteriorProperty()->SetOpacity(0);
              }
            }
          }
        }
      }
    }
  } 
  pEVLineedViewers->SyncViewersToScene();
  pEVLineedViewers->Render();
}

void EVTargetDefine::logUpdateWithStageSettings(QString step, EVTargetDef *def, QString ID)
{
}


void EVTargetDefine::UpdateSenes()
{
  EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
  ebAssert(pEVLineedViewers);
  pEVLineedViewers->SyncViewersToScene();
  pEVLineedViewers->Render();
}

int EVTargetDefine::PromptMessage(int iIDMsg)
{
  // client respond this message ID

  return 0;
}

bool EVTargetDefine::computeRemainingStages(EVTargetDef *def)
{
  ebLog eblog(Q_FUNC_INFO); eblog << def->getID().toStdString() << std::endl;
  
  EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
  ebAssert(pEVLineedViewers);

  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);

  //need progress indicated on angular side
  UpdateSenes();

  // verify the starting point and take it through to completion
  if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LumenSegmentationPreConditions()) {
    if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenSegmentation() == NULL) {
      pEVLineedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("Delineating Overall Luminal Surfaces");
      pEVLineedViewers->SyncViewersToScene();
      pEVLineedViewers->Render();
      try {
        pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLumenSegmentation();
      }
      catch (std::exception &e) {
        eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
        PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
        return false;
      }
      logUpdateWithStageSettings("EVTargetDefine::ComputeLumenSegmentation", def, def->getID());
      def->unsavedChanges = true;
      def->lumenSegIDdefine = GetEVLinkedViewers()->AddSegmentation4(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenSegmentation(), 0, "lumenSegmentation");
      clearTargetBeyondCurrentStage(def);
    }

    UpdateSenes();
    if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->PathPreConditions()) {
      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetPath() == NULL) {
        pEVLineedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("Computing Detailed Cross-section Positions");
        pEVLineedViewers->SyncViewersToScene();
        pEVLineedViewers->Render();
        try {
          pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputePath();
        }
        catch (std::exception &e) {
          eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
          PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
          return false;
        }
        logUpdateWithStageSettings("EVTargetDefine::ComputeVesselTargetPath", def, def->getID());
        def->unsavedChanges = true;
        if (def->initializerIDdefine) {
          pEVLineedViewers->RemoveVesselTarget(def->initializerIDdefine); // if have a path, don't want to display the initializer too
          def->initializerIDdefine = null_ebID;
        }
        def->pathIDdefine = pEVLineedViewers->AddVesselTarget(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetPath(), "path");
        clearTargetBeyondCurrentStage(def);
      }
    }

    UpdateSenes();
    if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LumenPartitionPreConditions()) {
      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenPartition() == NULL) {
        pEVLineedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("Partitioning Lumen Into Vessels");
        pEVLineedViewers->SyncViewersToScene();
        pEVLineedViewers->Render();
        try {
          pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLumenPartition();
        }
        catch (std::exception &e) {
          eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
          PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
          return false;
        }
        logUpdateWithStageSettings("EVTargetDefine::ComputeLumenPartition", def, def->getID());
        def->unsavedChanges = true;
        pEVLineedViewers->RemoveSegmentation4(def->lumenSegIDdefine); // if have partitions, don't want to display the segmentation too
        def->lumenSegIDdefine = null_ebID;
        def->lumenPartIDsDefine.clear();
        int j = 0;
        for (auto partition : *pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenPartition()) {
          def->lumenPartIDsDefine.insert(pEVLineedViewers->AddSegmentation4(partition.GetPointer(), 0, "lumenPartition" + std::to_string(j++)));
        }
        clearTargetBeyondCurrentStage(def);
      }
    }

    UpdateSenes();
    if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->RegistrationPreConditions()) {
      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetResampledRegisteredImages() == NULL) {
        pEVLineedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("Registering Images");
        pEVLineedViewers->SyncViewersToScene();
        pEVLineedViewers->Render();
        try {
          pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeRegistration();
        }
        catch (std::exception &e) {
          eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
          PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
          return false;
        }
        logUpdateWithStageSettings("EVTargetDefine::ComputeRegistration", def, def->getID());
        def->unsavedChanges = true;
        clearTargetBeyondCurrentStage(def);
      }
    }

    UpdateSenes();
    if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LumenAndWallSegmentationPreConditions()) {
      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenAndWallSegmentation() == NULL) {
        pEVLineedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("Delineating Outer Wall Surface");
        pEVLineedViewers->SyncViewersToScene();
        pEVLineedViewers->Render();
        try {
          pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLumenAndWallSegmentation();
        }
        catch (std::exception &e) {
          eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
          PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
          return false;
        }
        logUpdateWithStageSettings("EVTargetDefine::ComputeLumenAndWallSegmentation", def, def->getID());
        def->unsavedChanges = true;
        def->wallSegIDdefine = pEVLineedViewers->AddSegmentation4(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenAndWallSegmentation(), 0, "wallSegmentation");
        pEVLineedViewers->GetScene()->GetSegmentation4(def->wallSegIDdefine)->GetInteriorProperty()->SetOpacity(0);
        clearTargetBeyondCurrentStage(def);
      }
    }

    UpdateSenes();
    if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LumenAndWallPartitionPreConditions()) {
      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenAndWallPartition() == NULL) {
        pEVLineedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("Partitioning Wall Into Vessels");
        pEVLineedViewers->SyncViewersToScene();
        pEVLineedViewers->Render();
        try {
          pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLumenAndWallPartition();
        }
        catch (std::exception &e) {
          eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
          PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
          return false;
        }
        logUpdateWithStageSettings("EVTargetDefine::ComputeLumenAndWallPartition", def, def->getID());
        def->unsavedChanges = true;
        pEVLineedViewers->RemoveSegmentation4(def->wallSegIDdefine); // if have partitions, don't want to display the segmentation too
        def->wallSegIDdefine = null_ebID;
        def->wallPartIDsDefine.clear();
        int j = 0;
        for (auto partition : *pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenAndWallPartition()) {
          ebID partID = pEVLineedViewers->AddSegmentation4(partition.GetPointer(), 0, "wallPartition" + std::to_string(j++));
          def->wallPartIDsDefine.insert(partID);
          pEVLineedViewers->GetScene()->GetSegmentation4(partID)->GetInteriorProperty()->SetOpacity(0);
        }
        clearTargetBeyondCurrentStage(def);
      }
    }

    UpdateSenes();
    if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->WallThicknessPreConditions()) {
      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetWallThickness() == NULL) {
        pEVLineedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("Computing Wall Thickness Distribution");
        pEVLineedViewers->SyncViewersToScene();
        pEVLineedViewers->Render();
        try {
          pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeWallThickness();
        }
        catch (std::exception &e) {
          eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
          PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
          return false;
        }
        logUpdateWithStageSettings("EVTargetDefine::ComputeWallThickness", def, def->getID());
        def->unsavedChanges = true;
        clearTargetBeyondCurrentStage(def);
      }
    }

    UpdateSenes();
    if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->PerivascularRegionPreConditions()) {
      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetPerivascularRegion() == NULL) {
        pEVLineedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("Establishing Perivascular EVRegion");
        pEVLineedViewers->SyncViewersToScene();
        pEVLineedViewers->Render();
        try {
          pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputePerivascularRegion();
        }
        catch (std::exception &e) {
          eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
          PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
          return false;
        }
        logUpdateWithStageSettings("EVTargetDefine::ComputePerivascularRegion", def, def->getID());
        def->unsavedChanges = true;
        clearTargetBeyondCurrentStage(def);
      }
    }
    UpdateSenes();
    if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->PerivascularRegionPartitionPreConditions()) {
      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetPerivascularRegionPartition() == NULL) {
        pEVLineedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("Partitioning Perivascular EVRegion Into Vessels");
        pEVLineedViewers->SyncViewersToScene();
        pEVLineedViewers->Render();
        try {
          pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputePerivascularRegionPartition();
        }
        catch (std::exception &e) {
          eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
          PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
          return false;
        }
        logUpdateWithStageSettings("EVTargetDefine::ComputePerivascularRegionPartition", def, def->getID());
        def->unsavedChanges = true;
        clearTargetBeyondCurrentStage(def);
      }

    UpdateSenes();
    if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->CompositionPreConditions()) {
      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetComposition() == NULL) {

          //JCG condition
		  //  EV_MSG_CONFIRM_
		//QMessageBox msgBox(this);
        //msgBox.setText(tr("The current target definitions now enable completion of the calculations."));
        //msgBox.setInformativeText(tr("Would you like to compute locally, or on the CAPgraph server? Yes means compute locally, No means you prefer to compute on the server."));
        //msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
        //msgBox.setDefaultButton(QMessageBox::Yes);
        //msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
         // for ZENITH COMPUTE_ON_PROSERVERALWAYS
		 //		 
		 if (true) {
       pEVLineedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("Determining Composition of Wall and Plaque Tissues");
       pEVLineedViewers->SyncViewersToScene();
       pEVLineedViewers->Render();
            try {
              // NO LONGER NEED THIS BUT THE PATTERN MIGHT BE USEFUL FOR OTHER PURPOSES LATERif (owner->clinicalJurisdiction != "") // any of the clinical editiuons
              //   pipeline->GetVesselTargetPipeline(def->targetPipelineID)->SetCompositionAnalytes(true,true,false,true,false);
              // else
              pipeline->GetVesselTargetPipeline(def->targetPipelineID)->SetCompositionAnalytes(true, true, true, true, true);
              pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetCompositionFilter()->UpdateCalFactorBykVp(GetImageSeries()->kvp.toDouble());
              pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeComposition();
            }
            catch (std::exception &e) {
              eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
              PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
				      return false;
            }
            logUpdateWithStageSettings("EVTargetDefine::ComputeComposition", def, def->getID());
            def->unsavedChanges = true;

            //FROM A.3 and later releases, it is no longer needed to display the composition control dialog for the following two reasons.
            //1) MultiKVP calibration (scheduled for A.3 release) will lead to improved composition analysis and hence may not be needed to provide range hint for parameters.
            //2) If still necessary, user can always set the parameter range from the Processing Parameter Dialog. 
            //def->parameters->presentCompositionControl();
            clearTargetBeyondCurrentStage(def);
          }
          else {

            // stop the transition, user will do the computation on the server
            pEVLineedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("");
            pEVLineedViewers->SyncViewersToScene();
            pEVLineedViewers->Render();
            return false;
          }
        }
      }

      UpdateSenes();
      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->CapThicknessPreConditions()) {
        if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetCapThickness() == NULL) {
          pEVLineedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("Computing Cap Thickness Distribution");
          pEVLineedViewers->SyncViewersToScene();
          pEVLineedViewers->Render();
          try {
            pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeCapThickness();
          }
          catch (std::exception &e) {
            eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
            PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
            return false;
          }
          logUpdateWithStageSettings("EVTargetDefine::ComputeCapThickness", def, def->getID());
          def->unsavedChanges = true;
          clearTargetBeyondCurrentStage(def);
        }
      }

      UpdateSenes();
      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ReadingsPreConditions()) {
        if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetReadings() == NULL) {
          pEVLineedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("Performing Quantitative Calculations");
          pEVLineedViewers->SyncViewersToScene();
          pEVLineedViewers->Render();
          try {
            pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeReadings();
          }
          catch (std::exception &e) {
            eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
            PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
            return false;
          }
          logUpdateWithStageSettings("EVTargetDefine::ComputeVesselTargetReadings", def, def->getID());
          def->unsavedChanges = true;
          def->keyImages.clear(); // anything that caused readings to be re-computed invalidates any saved key images
          clearTargetBeyondCurrentStage(def);
        }
        pEVLineedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("Target Calculations Completed");
        pEVLineedViewers->SyncViewersToScene();
        pEVLineedViewers->Render();
      }

      UpdateSenes();
      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LesionLumenPartitionPreConditions()) {
        if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLesionLumenPartition() == NULL) {
          pEVLineedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("Partitioning Lumen Into Lesions");
          pEVLineedViewers->SyncViewersToScene();
          pEVLineedViewers->Render();
          try {
            pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLesionLumenPartition();
          }
          catch (std::exception &e) {
            eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
            PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
            return false;
          }
          logUpdateWithStageSettings("EVTargetDefine::ComputeLesionLumenPartition", def, def->getID());
          def->unsavedChanges = true;
          clearTargetBeyondCurrentStage(def);
        }
      }

      UpdateSenes();
      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LesionLumenAndWallPartitionPreConditions()) {
        if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLesionLumenAndWallPartition() == NULL) {
          pEVLineedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("Partitioning Wall Into Lesions");
          pEVLineedViewers->SyncViewersToScene();
          pEVLineedViewers->Render();
          try {
            pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLesionLumenAndWallPartition();
          }
          catch (std::exception &e) {
            eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
            PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
            return false;
          }
          logUpdateWithStageSettings("EVTargetDefine::ComputeLesionLumenAndWallPartition", def, def->getID());
          def->unsavedChanges = true;
          clearTargetBeyondCurrentStage(def);
        }
      }
      
      UpdateSenes();
      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LesionPerivascularRegionPartitionPreConditions()) {
        if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLesionPerivascularRegionPartition() == NULL) {
          pEVLineedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("Partitioning PerivascularRegion Into Lesions");
          pEVLineedViewers->SyncViewersToScene();
          pEVLineedViewers->Render();
          try {
            pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLesionPerivascularRegionPartition();
          }
          catch (std::exception &e) {
            eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
            PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
            return false;
          }
          logUpdateWithStageSettings("EVTargetDefine::ComputeLesionPerivascularRegionPartition", def, def->getID());
          def->unsavedChanges = true;
          clearTargetBeyondCurrentStage(def);
        }
      }

      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LesionReadingsPreConditions()) {
        if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLesionReadings() == NULL) {
          pEVLineedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("Performing Quantitative Calculations for Lesions");
          pEVLineedViewers->SyncViewersToScene();
          pEVLineedViewers->Render();
          try {
            pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLesionReadings();
          }
          catch (std::exception &e) {
            eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
            PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
            return false;
          }
          logUpdateWithStageSettings("EVTargetDefine::ComputeLesionReadings", def, def->getID());
          def->unsavedChanges = true;
          clearTargetBeyondCurrentStage(def);

        }
        pEVLineedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("Lesion Calculations Completed");
        pEVLineedViewers->SyncViewersToScene();
        pEVLineedViewers->Render();
        //progressIndicator.setMaximum(100); // any value will take it down
        return true;
      }
      else {
        ///qWarning() << "compute remaining stages was not able to make it all the way through to the lesion readings.";
        pEVLineedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("");
        pEVLineedViewers->SyncViewersToScene();
        pEVLineedViewers->Render();
        //progressIndicator.setMaximum(100); // any value will take it down
        return false;
        }
      return true;
    }
    else {
      qWarning() << "compute remaining stages was invoked on a target that is not yet viable, skipping.";
      pEVLineedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("");
      pEVLineedViewers->SyncViewersToScene();
      pEVLineedViewers->Render();
      //progressIndicator.setMaximum(100); // any value will take it down
      return false;
    }
  }


  return false;
}



void EVTargetDefine::ClearTargetBeyondStageInitialization(EVTargetDef *def, int currentStage)
{
  EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
  ebAssert(pEVLineedViewers);

  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);

  if (def->initializerIDdefine != null_ebID) {
    pEVLineedViewers->RemoveVesselTarget(def->initializerIDdefine);
    def->initializerIDdefine = null_ebID;
    def->unsavedChanges = true;
  }
  def->setTargetInitializerFileName("");
}

void EVTargetDefine::ClearTargetBeyondStageLumenSegmentation(EVTargetDef *def, int currentStage)
{
  EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
  ebAssert(pEVLineedViewers);

  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);

  if (def->lumenSegIDdefine != null_ebID) {
    pEVLineedViewers->RemoveSegmentation4(def->lumenSegIDdefine);
    def->lumenSegIDdefine = null_ebID;
    def->unsavedChanges = true;
  }
  def->lumenSegIDdefine = null_ebID;
  for (int j = 0; j < def->regions.size(); j++) {
    EVRegion *reg = &((def->regions)[j]);
    if (reg->regName.contains("lumenSegmentation")) {
      def->regions.removeAt(j);
      break;
    }
  }

}

void EVTargetDefine::ClearTargetBeyondStagePath(EVTargetDef *def, int currentStage)
{
  EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
  ebAssert(pEVLineedViewers);

  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);

  if (def->pathIDdefine != null_ebID) {
    pEVLineedViewers->RemoveVesselTarget(def->pathIDdefine);
    def->pathIDdefine = null_ebID;
    // if there is an initializer, need to restore it
    if ((currentStage > 0) && (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetInitialization() != NULL)) {
      def->initializerIDdefine = pEVLineedViewers->AddVesselTarget(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetInitialization(), "initializer");
    }
    def->unsavedChanges = true;
  }
  def->setTargetPathFileName("");

}

void EVTargetDefine::ClearTargetBeyondStageLumenPartition(EVTargetDef *def, int currentStage)
{
  EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
  ebAssert(pEVLineedViewers);

  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);

  if (!def->lumenPartIDsDefine.empty()) {
    for (auto partitionID : def->lumenPartIDsDefine) {
      pEVLineedViewers->RemoveSegmentation4(partitionID);
    }
    def->unsavedChanges = true;
  }
  def->lumenPartIDsDefine.clear();
  for (int j = 0; j < def->regions.size(); j++) {
    EVRegion *reg = &((def->regions)[j]);
    if (reg->regName.contains("lumenPartition")) {
      def->regions.removeAt(j);
      break;
    }
  }

}

void EVTargetDefine::ClearTargetBeyondStageResampledRegisteredImages(EVTargetDef *def, int currentStage)
{
  EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
  ebAssert(pEVLineedViewers);

  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);

  def->setTargetRegistrationTransforms("");
}

void EVTargetDefine::ClearTargetBeyondStageLumenAndWallSegmentation(EVTargetDef *def, int currentStage)
{
  EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
  ebAssert(pEVLineedViewers);

  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);

  if (def->wallSegIDdefine != null_ebID) {
    pEVLineedViewers->RemoveSegmentation4(def->wallSegIDdefine);
    def->unsavedChanges = true;
  }
  def->wallSegIDdefine = null_ebID;
  for (int j = 0; j < def->regions.size(); j++) {
    EVRegion *reg = &((def->regions)[j]);
    if (reg->regName.contains("wallSegmentation")) {
      def->regions.removeAt(j);
      break;
    }
  }
}

void EVTargetDefine::ClearTargetBeyondStageLumenAndWallPartition(EVTargetDef *def, int currentStage)
{
  EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
  ebAssert(pEVLineedViewers);

  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);

  if (!def->wallPartIDsDefine.empty()) {
    for (auto partitionID : def->wallPartIDsDefine) {
      pEVLineedViewers->RemoveSegmentation4(partitionID);
    }
    def->unsavedChanges = true;
  }
  def->wallPartIDsDefine.clear();
  for (int j = 0; j < def->regions.size(); j++) {
    EVRegion *reg = &((def->regions)[j]);
    if (reg->regName.contains("wallPartition")) {
      def->regions.removeAt(j);
      break;
    }
  }
}

void EVTargetDefine::ClearTargetBeyondStageWallThickness(EVTargetDef *def, int currentStage)
{
  EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
  ebAssert(pEVLineedViewers);

  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);

  if (def->wallThickIDdefine != null_ebID) {
    pEVLineedViewers->RemoveImage4(def->wallThickIDdefine);
    def->unsavedChanges = true;
  }
  def->wallThickIDdefine = null_ebID;
  for (int j = 0; j < def->valueMaps.size(); j++) {
    EVValueMap *map = &((def->valueMaps)[j]);
    if (map->valueName.contains("wallThickness")) {
      def->valueMaps.removeAt(j);
      break;
    }
  }
}

void EVTargetDefine::ClearTargetBeyondStagePerivascularRegion(EVTargetDef *def, int currentStage)
{
  EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
  ebAssert(pEVLineedViewers);

  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);

  if (def->periRegIDdefine != null_ebID) {
    pEVLineedViewers->RemoveSegmentation4(def->periRegIDdefine);
    def->unsavedChanges = true;
  }
  def->periRegIDdefine = null_ebID;
  for (int j = 0; j < def->regions.size(); j++) {
    EVRegion *reg = &((def->regions)[j]);
    if (reg->regName.contains("perivascularRegion")) {
      def->regions.removeAt(j);
      break;
    }
  }
}

void EVTargetDefine::ClearTargetBeyondStagePerivascularRegionPartition(EVTargetDef *def, int currentStage)
{
  EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
  ebAssert(pEVLineedViewers);

  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);

  if (!def->periRegPartIDsDefine.empty()) {
    for (auto partitionID : def->periRegPartIDsDefine) {
      pEVLineedViewers->RemoveSegmentation4(partitionID);
    }
    def->unsavedChanges = true;
  }
  def->periRegPartIDsDefine.clear();
  for (int j = 0; j < def->regions.size(); j++) {
    EVRegion *reg = &((def->regions)[j]);
    if (reg->regName.contains("periRegPartition")) {
      def->regions.removeAt(j);
      break;
    }
  }

}

void EVTargetDefine::ClearTargetBeyondStageComposition(EVTargetDef *def, int currentStage)
{
  EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
  ebAssert(pEVLineedViewers);

  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);

  for (int j = 0; j < def->probabilityMaps.size(); j++) {
    EVProbabilityMap *map = &((def->probabilityMaps)[j]);
    if (map->probabilityName.contains("composition")) {
      def->probabilityMaps.removeAt(j);
      break;
    }
  }
}

void EVTargetDefine::ClearTargetBeyondStageCapThickness(EVTargetDef *def, int currentStage)
{
  EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
  ebAssert(pEVLineedViewers);

  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);

  if (def->capThickIDdefine != null_ebID) {
    pEVLineedViewers->RemoveImage4(def->capThickIDdefine);
    def->unsavedChanges = true;
  }
  def->capThickIDdefine = null_ebID;
  for (int j = 0; j < def->valueMaps.size(); j++) {
    EVValueMap *map = &((def->valueMaps)[j]);
    if (map->valueName.contains("capThickness")) {
      def->valueMaps.removeAt(j);
      break;
    }
  }
}

void EVTargetDefine::ClearTargetBeyondStageReadings(EVTargetDef *def, int currentStage)
{
  EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
  ebAssert(pEVLineedViewers);

  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);

  if (def->readingsIDdefine != null_ebID) {
    pEVLineedViewers->RemoveVesselTarget(def->readingsIDdefine);
    def->readingsIDdefine = null_ebID;
    // if there is either a path, or even an initializer, need to restore accordingly
    if ((currentStage > ebiVesselTargetPipeline::PATH_STAGE) && (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetPath() != NULL)) {
      def->pathIDdefine = pEVLineedViewers->AddVesselTarget(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetPath(), "path");
    }
    else if ((currentStage > ebiVesselTargetPipeline::INITIALIZATION_STAGE) && (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetInitialization() != NULL)) {
      def->initializerIDdefine = pEVLineedViewers->AddVesselTarget(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetInitialization(), "initializer");
    }
  }
  def->setTargetReadingsFileName("");
}

void EVTargetDefine::ClearTargetBeyondStageLesionLumenPartition(EVTargetDef *def, int currentStage)
{
  EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
  ebAssert(pEVLineedViewers);

  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);

  if (!def->lesionLumenPartIDsDefine.empty()) {
    for (auto partitionID : def->lesionLumenPartIDsDefine) {
      pEVLineedViewers->RemoveSegmentation4(partitionID);
    }
    def->unsavedChanges = true;
  }
  def->lesionLumenPartIDsDefine.clear();
  for (int j = 0; j < def->regions.size(); j++) {
    EVRegion *reg = &((def->regions)[j]);
    if (reg->regName.contains("lesionLumenPartition")) {
      def->regions.removeAt(j);
      break;
    }
  }
}

void EVTargetDefine::ClearTargetBeyondStageLesionLumenAndWallPartition(EVTargetDef *def, int currentStage)
{
  EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
  ebAssert(pEVLineedViewers);

  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);

  if (!def->lesionLumenAndWallPartIDsDefine.empty()) {
    for (auto partitionID : def->lesionLumenAndWallPartIDsDefine) {
      pEVLineedViewers->RemoveSegmentation4(partitionID);
    }
    def->unsavedChanges = true;
  }
  def->lesionLumenAndWallPartIDsDefine.clear();
  for (int j = 0; j < def->regions.size(); j++) {
    EVRegion *reg = &((def->regions)[j]);
    if (reg->regName.contains("lesionLumenAndWallPartition")) {
      def->regions.removeAt(j);
      break;
    }
  }
}

void EVTargetDefine::ClearTargetBeyondStageLesionPerivascularRegionPartition(EVTargetDef *def, int currentStage)
{
  EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
  ebAssert(pEVLineedViewers);

  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);

  if (!def->lesionPeriRegPartIDsDefine.empty()) {
    for (auto partitionID : def->lesionPeriRegPartIDsDefine) {
      std::cerr << "line 1214" << std::endl;
      pEVLineedViewers->RemoveSegmentation4(partitionID);
    }
    def->unsavedChanges = true;
  }
  def->lesionPeriRegPartIDsDefine.clear();
  for (int j = 0; j < def->regions.size(); j++) {
    EVRegion *reg = &((def->regions)[j]);
    if (reg->regName.contains("lesionPeriRegPartition")) {
      def->regions.removeAt(j);
      break;
    }
  }
}

void EVTargetDefine::ClearTargetBeyondStageLesionReadings(EVTargetDef *def, int currentStage)
{
  EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
  ebAssert(pEVLineedViewers);

  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);

  if (def->lesionReadingsIDdefine != null_ebID) {
    pEVLineedViewers->RemoveVesselTarget(def->lesionReadingsIDdefine);
    def->lesionReadingsIDdefine = null_ebID;
    // if there is either readings, a path, or even an initializer, need to restore accordingly
    if ((currentStage > ebiVesselTargetPipeline::READINGS_STAGE) && (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetReadings() != NULL)) {
      def->readingsIDdefine = pEVLineedViewers->AddVesselTarget(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetReadings(), "readings");
    }
    else if ((currentStage > ebiVesselTargetPipeline::PATH_STAGE) && (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetPath() != NULL)) {
      def->pathIDdefine = pEVLineedViewers->AddVesselTarget(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetPath(), "path");
    }
    else if ((currentStage > ebiVesselTargetPipeline::INITIALIZATION_STAGE) && (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetInitialization() != NULL)) {
      def->initializerIDdefine = pEVLineedViewers->AddVesselTarget(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetInitialization(), "initializer");
    }
  }
  def->setLesionReadingsFileName("");
}

#define CHECK_STAGE(__ptrstage__, __stage__)  if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->__ptrstage__ == NULL) currentStage = ebiVesselTargetPipeline::__stage__;

int EVTargetDefine::GetStageOfClearTargetBeyondCurrentStage(EVTargetDef *def)
{
  ebLog eblog(Q_FUNC_INFO); eblog << def->getID().toStdString() << std::endl;

  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);

  int currentStage = -1;
  
  CHECK_STAGE(GetInitialization(), INITIALIZATION_STAGE)
  CHECK_STAGE(GetLumenSegmentation(), LUMENSEGMENTATION_STAGE)
  CHECK_STAGE(GetPath(), PATH_STAGE)
  CHECK_STAGE(GetLumenPartition(), LUMENPARTITION_STAGE)
  CHECK_STAGE(GetResampledRegisteredImages(), IMAGEREGISTRATION_STAGE)
  CHECK_STAGE(GetLumenAndWallSegmentation(), LUMENANDWALLSEGMENTATION_STAGE)
  CHECK_STAGE(GetLumenAndWallPartition(), LUMENANDWALLPARTITION_STAGE)
  CHECK_STAGE(GetWallThickness(), WALLTHICKNESS_STAGE)
  CHECK_STAGE(GetPerivascularRegion(), PERIVASCULARREGION_STAGE)
  CHECK_STAGE(GetPerivascularRegionPartition(), PERIVASCULARREGIONPARTITION_STAGE)
  CHECK_STAGE(GetComposition(), COMPOSITION_STAGE)
  CHECK_STAGE(GetCapThickness(), CAPTHICKNESS_STAGE)
  CHECK_STAGE(GetReadings(), READINGS_STAGE)
  CHECK_STAGE(GetLesionLumenPartition(), LESIONLUMENPARTITION_STAGE)
  CHECK_STAGE(GetLesionLumenAndWallPartition(), LESIONLUMENANDWALLPARTITION_STAGE)
  CHECK_STAGE(GetLesionPerivascularRegionPartition(), LESIONPERIVASCULARREGIONPARTITION_STAGE)
  CHECK_STAGE(GetLesionReadings(), LESIONREADINGS_STAGE)

  return currentStage;
}

void EVTargetDefine::clearTargetBeyondCurrentStage(EVTargetDef *def)
{
  int currentStage = GetStageOfClearTargetBeyondCurrentStage(def);
  if (-1 != currentStage)
  {
    // we use a switch statement because we want the fall-through behaviour
    switch (currentStage)
    {
    case ebiVesselTargetPipeline::INITIALIZATION_STAGE:
      ClearTargetBeyondStageInitialization(def, currentStage);
    case ebiVesselTargetPipeline::LUMENSEGMENTATION_STAGE:
      ClearTargetBeyondStageLumenSegmentation(def, currentStage);
    case ebiVesselTargetPipeline::PATH_STAGE:
      ClearTargetBeyondStagePath(def, currentStage);
    case ebiVesselTargetPipeline::LUMENPARTITION_STAGE:
      ClearTargetBeyondStageLumenPartition(def, currentStage);
    case ebiVesselTargetPipeline::IMAGEREGISTRATION_STAGE:
      ClearTargetBeyondStageResampledRegisteredImages(def, currentStage);
    case ebiVesselTargetPipeline::LUMENANDWALLSEGMENTATION_STAGE:
      ClearTargetBeyondStageLumenAndWallSegmentation(def, currentStage);
    case ebiVesselTargetPipeline::LUMENANDWALLPARTITION_STAGE:
      ClearTargetBeyondStageLumenAndWallPartition(def, currentStage);
    case ebiVesselTargetPipeline::WALLTHICKNESS_STAGE:
      ClearTargetBeyondStageWallThickness(def, currentStage);
    case ebiVesselTargetPipeline::PERIVASCULARREGION_STAGE:
      ClearTargetBeyondStagePerivascularRegion(def, currentStage);
    case ebiVesselTargetPipeline::PERIVASCULARREGIONPARTITION_STAGE:
      ClearTargetBeyondStagePerivascularRegionPartition(def, currentStage);
    case ebiVesselTargetPipeline::COMPOSITION_STAGE:
      ClearTargetBeyondStageComposition(def, currentStage);
    case ebiVesselTargetPipeline::CAPTHICKNESS_STAGE:
      ClearTargetBeyondStageCapThickness(def, currentStage);
    case ebiVesselTargetPipeline::READINGS_STAGE:
      ClearTargetBeyondStageReadings(def, currentStage);
    case ebiVesselTargetPipeline::LESIONLUMENPARTITION_STAGE:
      ClearTargetBeyondStageLesionLumenPartition(def, currentStage);
    case ebiVesselTargetPipeline::LESIONLUMENANDWALLPARTITION_STAGE:
      ClearTargetBeyondStageLesionLumenAndWallPartition(def, currentStage);
    case ebiVesselTargetPipeline::LESIONPERIVASCULARREGIONPARTITION_STAGE:
      ClearTargetBeyondStageLesionPerivascularRegionPartition(def, currentStage);
    case ebiVesselTargetPipeline::LESIONREADINGS_STAGE:
      ClearTargetBeyondStageLesionReadings(def, currentStage);
    }
  }
}

void EVTargetDefine::preloadDefinePost()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  selectCurrentTarget(NULL); // start out with no target selected as current focus

  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);

  QList<EVTargetDef>* targets = GetTargets();
 
  // loop through initial description to setup predefined targets each according to how well specified they are
  for (int i=0; i < targets->size(); i++) 
  {
    EVTargetDef *def = &((*targets)[i]);
    
    if ((def->getID() != "") && (def->getBodySite() != "") && (def->getTargetFolder() != "")) 
    {
      qInfo() << "EVTargetDefine::preloadDefinePost: preloading a target of bodySite: " << def->getBodySite();
      def->unsavedChanges = false;
      def->setParentPipeline(pipeline);
      def->targetPipelineID = pipeline->AddVesselTargetPipeline();
      pipeline->GetVesselTargetPipeline(def->targetPipelineID)->SetBodySite(def->getBodySite().toStdString());
      pipeline->GetVesselTargetPipeline(def->targetPipelineID)->SetTargetFolder(def->getTargetFolder().toStdString());

      //def->parameters = new processingParameters(this, owner->systemPreferencesObject, owner->clinicalJurisdiction, pipeline, def->getBodySite(), def->targetPipelineID);

      PromptMessage(EV_MSG_PROCESSING_PARAMETERS_SETTINGS_CHANGED);
      def->pushTargetParametersToPipeline(pipeline->GetVesselTargetPipeline(def->targetPipelineID));



      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->InitializationPreConditions()) {
        if (def->getTargetInitializerFileName() != "") {
          QFileInfo checkFile(def->getTargetInitializerFileName());
          if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable()) {
            try {
              pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenInitialization(def->getTargetInitializerFileName().toStdString());
            } catch (std::exception &e) {
              eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
              PromptMessage(EV_MSG_ERROR_OPENING_RESET_TARGET);
              break; 
            }
          }
          else {
            PromptMessage(EV_MSG_ERROR_LIST_ENDING_PRELOAD_EARLY_FILE_NOT_EXIST);
            break;
          }
        }
        else {
          pipeline->GetVesselTargetPipeline(def->targetPipelineID)->CreateInitialization();
        }
        




        // now we continue as deep as the target definition allows
        // start with lumenSegmentation
        int j;
        EVRegion *reg;
        for (j=0; j < def->regions.size(); j++) {
          reg = &((def->regions)[j]);
          if (reg->regName.contains("lumenSegmentation")) {
            break;
          }
        }
        if (j < def->regions.size()) {
          if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LumenSegmentationPreConditions()) {
            if (reg->regFileName != "") {
              QFileInfo checkFile(reg->regFileName);
              if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable()) {
                try {
                  pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenLumenSegmentation(reg->regFileName.toStdString());
                  qInfo() << "...OpenLumenSegmentation done";
                } catch (std::exception &e) {
                  eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                  PromptMessage(EV_MSG_ERROR_OPENING_RESET_TARGET);
                  break; 
                }
              }
              else {
                PromptMessage(EV_MSG_ERROR_LIST_ENDING_PRELOAD_EARLY_FILE_NOT_EXIST);
                break; 
              }
            }
          }
          else if (reg->regFileName != "") {
            qWarning() << "target in list position" << i << "has a lumen segmentation EVRegion, but pipeline state doesn't allow it.";
            return;
          }
        }




        // proceed to path
        if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->PathPreConditions()) {
          if (def->getTargetPathFileName() != "") {
            QFileInfo checkFile(def->getTargetPathFileName());
            if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable())
              if (false/* replace with test for whether the object shoudl be recomputed because runnign a later version from that which created the object */) { 
                try {
                  pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputePath();
                } catch (std::exception &e) {
                  eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                  PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
                  break;
                }
                logUpdateWithStageSettings("EVTargetDefine::ComputeVesselTargetPath", def, def->getID());
                def->unsavedChanges = true;
              }
              else {
                try {
                  pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenPath(def->getTargetPathFileName().toStdString());
                  qInfo() << "...OpenPath done";
                } catch (std::exception &e) {
                  eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                  PromptMessage(EV_MSG_ERROR_OPENING_RESET_TARGET);
                  break; 
                }
              }
            else {
              PromptMessage(EV_MSG_ERROR_LIST_ENDING_PRELOAD_EARLY_FILE_NOT_EXIST);
              break; 
            }
          }
        }
        else if (def->getTargetPathFileName() != "") {
          qWarning() << "target in list position" << i << "has a path, but pipeline state doesn't allow it.";
          return;
        }




        // lumenPartition
        for (j=0; j < def->regions.size(); j++) {
          reg = &((def->regions)[j]);
          if (reg->regName.contains("lumenPartition"))
            break;
        }
        if (j < def->regions.size()) {
          if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LumenPartitionPreConditions()) {
            if (reg->regFileName != "") {
              QFileInfo checkFile(reg->regFileName);
              if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable()) {
                if (false/* replace with test for whether the object should be recomputed because running a later version from that which created the object */) { 
                  try {
                    pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLumenPartition();
                  } catch (std::exception &e) {
                    eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                    PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
                    break; 
                  }
                  logUpdateWithStageSettings("EVTargetDefine::ComputeLumenPartition", def, def->getID());
                  def->unsavedChanges = true;
                }
                else {
                  try {
                    pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenLumenPartition(reg->regFileName.toStdString());
                    qInfo() << "...OpenLumenPartition done";
                  } catch (std::exception &e) {
                    eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                    PromptMessage(EV_MSG_ERROR_OPENING_RESET_TARGET);
                    break; 
                  }
                }
                // given that we have a partitioned lumen, the target is considered viable.
                def->isViable = true;
                viableTargetsCount++;
              }
              else {
                PromptMessage(EV_MSG_ERROR_LIST_ENDING_PRELOAD_EARLY_FILE_NOT_EXIST);
                break; 
              }              
            }
          }
          else if (reg->regFileName != "") {
            qWarning() << "target in list position" << i << "has a partitioned lumen, but pipeline state doesn't allow it.";
            return;
          }
        }





        // registration
        if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->RegistrationPreConditions()) {
          if (def->getTargetRegistrationTransforms() != "") {
            QFileInfo checkFile(def->getTargetRegistrationTransforms());
            if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable())
              if (false/* replace with test for whether the object shoudl be recomputed because runnign a later version from that which created the object */) { 
                try {
                  pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeRegistration();
                } catch (std::exception &e) {
                  eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
				          PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
                  break; 
                }
                logUpdateWithStageSettings("EVTargetDefine::ComputeRegistration", def, def->getID());
                def->unsavedChanges = true;
              }
              else {
                try {
                  pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenRegistration(def->getTargetRegistrationTransforms().toStdString());
                  qInfo() << "...OpenRegistration done";
                } catch (std::exception &e) {
                  eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                  PromptMessage(EV_MSG_ERROR_OPENING_RESET_TARGET);
                  break; 
                }
              }
            else {
              PromptMessage(EV_MSG_ERROR_LIST_ENDING_PRELOAD_EARLY_FILE_NOT_EXIST);
              break; 
            }
          }
        }
        else if (def->getTargetRegistrationTransforms() != "") {
          qWarning() << "target in list position" << i << "has registration transforms, but pipeline state doesn't allow it.";
          return;
        }





        // wallSegmentation
        for (j=0; j < def->regions.size(); j++) {
          reg = &((def->regions)[j]);
          if (reg->regName.contains("wallSegmentation"))
            break;
        }
        if (j < def->regions.size()) {
          if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LumenAndWallSegmentationPreConditions()) {
            if (reg->regFileName != "") {
              QFileInfo checkFile(reg->regFileName);
              if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable()) {
                try {
                  pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenLumenAndWallSegmentation(reg->regFileName.toStdString());
                  qInfo() << "...OpenLumenAndWallSegmentation done";
                } catch (std::exception &e) {
                  eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                  PromptMessage(EV_MSG_ERROR_OPENING_RESET_TARGET);
                  break; 
                }
              }
              else {
                PromptMessage(EV_MSG_ERROR_LIST_ENDING_PRELOAD_EARLY_FILE_NOT_EXIST);
                break; 
              }
            }
          }
          else if (reg->regFileName != "") {
            qWarning() << "target in list position" << i << "has a wall segmentation EVRegion, but pipeline state doesn't allow it.";
            return;
          }
        }






        // wallPartition
        for (j=0; j < def->regions.size(); j++) {
          reg = &((def->regions)[j]);
          if (reg->regName.contains("wallPartition"))
            break;
        }
        if (j < def->regions.size()) {
          if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LumenAndWallPartitionPreConditions()) {
            if (reg->regFileName != "") {
              QFileInfo checkFile(reg->regFileName);
              if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable()) {
                if (false/* replace with test for whether the object should be recomputed because running a later version from that which created the object */) { 
                  try {
                    pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLumenAndWallPartition();
                  } catch (std::exception &e) {
                    eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                    PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
                    break; 
                  }
                  logUpdateWithStageSettings("EVTargetDefine::ComputeLumenAndWallPartition", def, def->getID());
                  def->unsavedChanges = true;
                }
                else {
                  try {
                    pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenLumenAndWallPartition(reg->regFileName.toStdString());
                    qInfo() << "...OpenLumenAndWallPartition done";
                  } catch (std::exception &e) {
                    eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                    PromptMessage(EV_MSG_ERROR_OPENING_RESET_TARGET);
                    break; 
                  }
                }
              }
              else {
                PromptMessage(EV_MSG_ERROR_LIST_ENDING_PRELOAD_EARLY_FILE_NOT_EXIST);
                break; 
              }              
            }
          }
          else if (reg->regFileName != "") {
            qWarning() << "target in list position" << i << "has a partitioned wall, but pipeline state doesn't allow it.";
            return;
          }
        }





        // wallThickness
        EVValueMap *WTmap;
        for (j=0; j < def->valueMaps.size(); j++) {
          WTmap = &((def->valueMaps)[j]);
          if (WTmap->valueName.contains("wallThickness"))
            break;
        }
        if (j < def->valueMaps.size()) {
          if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->WallThicknessPreConditions()) {
            if (WTmap->valueMapFileName != "") {
              QFileInfo checkFile(WTmap->valueMapFileName);
              if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable())
                if (false/* replace with test for whether the object should be recomputed because running a later version from that which created the object */) { 
                  try {
                    pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeWallThickness();
                  } catch (std::exception &e) {
                    eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
					
                    PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
                    break; 
                  }
                  logUpdateWithStageSettings("EVTargetDefine::ComputeWallThickness", def, def->getID());
                  def->unsavedChanges = true;
                }
                else {
                  try {
                    pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenWallThickness(WTmap->valueMapFileName.toStdString());
                    qInfo() << "...OpenWallThickness done";
                  } catch (std::exception &e) {
                    eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                    PromptMessage(EV_MSG_ERROR_OPENING_RESET_TARGET);
                    break; 
                  }
                }
              else {
                PromptMessage(EV_MSG_ERROR_LIST_ENDING_PRELOAD_EARLY_FILE_NOT_EXIST);
                break; 
              }              
            }
          }
          else if (WTmap->valueMapFileName != "") {
            qWarning() << "target in list position" << i << "has a wall thickness value map, but pipeline state doesn't allow it.";
            return;
          }
        }





        // perivascularRegion
        for (j=0; j < def->regions.size(); j++) {
          reg = &((def->regions)[j]);
          if (reg->regName.contains("perivascularRegion"))
            break;
        }
        if (j < def->regions.size()) {
          if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->PerivascularRegionPreConditions()) {
            if (reg->regFileName != "") {
              QFileInfo checkFile(reg->regFileName);
              if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable()) {
                try {
                  pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenPerivascularRegion(reg->regFileName.toStdString());
                  qInfo() << "...OpenPerivascularRegion done";
                } catch (std::exception &e) {
                  eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                  PromptMessage(EV_MSG_ERROR_OPENING_RESET_TARGET);
                  break; 
                }
              }
              else {
                PromptMessage(EV_MSG_ERROR_LIST_ENDING_PRELOAD_EARLY_FILE_NOT_EXIST);
                break; 
              }
            }
          }
          else if (reg->regFileName != "") {
            qWarning() << "target in list position" << i << "has a perivascular EVRegion, but pipeline state doesn't allow it.";
            return;
          }
        }





        // periRegPartition
        for (j=0; j < def->regions.size(); j++) {
          reg = &((def->regions)[j]);
          if (reg->regName.contains("periRegPartition"))
            break;
        }
        if (j < def->regions.size()) {
          if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->PerivascularRegionPartitionPreConditions()) {
            if (reg->regFileName != "") {
              QFileInfo checkFile(reg->regFileName);
              if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable()) {
                if (false/* replace with test for whether the object should be recomputed because running a later version from that which created the object */) { 
                  try {
                    pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputePerivascularRegionPartition();
                  } catch (std::exception &e) {
                    eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                    PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
                    break; 
                  }
                  logUpdateWithStageSettings("EVTargetDefine::ComputePerivascularRegionPartition", def, def->getID());
                  def->unsavedChanges = true;
                }
                else {
                  try {
                    pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenPerivascularRegionPartition(reg->regFileName.toStdString());
                    qInfo() << "...OpenPerivascularRegionPartition done";
                  } catch (std::exception &e) {
                    eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                    PromptMessage(EV_MSG_ERROR_OPENING_RESET_TARGET);
                    break; 
                  }
                }
              }
              else {
                PromptMessage(EV_MSG_ERROR_LIST_ENDING_PRELOAD_EARLY_FILE_NOT_EXIST);
                break; 
              }              
            }
          }
          else if (reg->regFileName != "") {
            qWarning() << "target in list position" << i << "has a partitioned perivascular EVRegion, but pipeline state doesn't allow it.";
            return;
          }
        }





        // composition
        EVProbabilityMap *pmap;
        for (j=0; j < def->probabilityMaps.size(); j++) {
          pmap = &((def->probabilityMaps)[j]);
          if (pmap->probabilityName.contains("composition"))
            break;
        }
        if (j < def->probabilityMaps.size()) {
          if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->CompositionPreConditions()) {
            if (pmap->probabilityMapFileName != "") {
              QFileInfo checkFile(pmap->probabilityMapFileName);
              if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable())
                if (false/* replace with test for whether the object should be recomputed because running a later version from that which created the object */) { 
                  try {
                    /*NOT NEEDED NOW BUT LEFT FOR THE PATTERNif (owner->clinicalJurisdiction != "") // any of the clinical editions
                      pipeline->GetVesselTargetPipeline(def->targetPipelineID)->SetCompositionAnalytes(true,true,false,true,false);
                    else*/
                      pipeline->GetVesselTargetPipeline(def->targetPipelineID)->SetCompositionAnalytes(true,true,true,true,true);
                    pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeComposition();
                  } catch (std::exception &e) {
                    eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                    PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
                    break; 
                  }
                  logUpdateWithStageSettings("EVTargetDefine::ComputeComposition", def, def->getID());
                  def->unsavedChanges = true;
                }
                else {
                  try {
                    pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenComposition(pmap->probabilityMapFileName.toStdString());
                    qInfo() << "...OpenComposition done";
                  } catch (std::exception &e) {
                    eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                    PromptMessage(EV_MSG_ERROR_OPENING_RESET_TARGET);
                    break; 
                  }
                }
              else {
                PromptMessage(EV_MSG_ERROR_LIST_ENDING_PRELOAD_EARLY_FILE_NOT_EXIST);
                break; 
              } 
            }
          }
          else if (pmap->probabilityMapFileName != "") {
            qWarning() << "target in list position" << i << "has a composition probability map, but pipeline state doesn't allow it.";
            //return;
          }
        }






        // capThickness
        EVValueMap *CTmap;
        for (j=0; j < def->valueMaps.size(); j++) {
          CTmap = &((def->valueMaps)[j]);
          if (CTmap->valueName.contains("capThickness"))
            break;
        }
        if (j < def->valueMaps.size()) {
          if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->CapThicknessPreConditions()) {
            if (CTmap->valueMapFileName != "") {
              QFileInfo checkFile(CTmap->valueMapFileName);
              if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable())
                if (false/* replace with test for whether the object should be recomputed because running a later version from that which created the object */) { 
                  try {
                    pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeCapThickness();
                  } catch (std::exception &e) {
                    eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                    PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
                    break; 
                  }
                  logUpdateWithStageSettings("EVTargetDefine::ComputeCapThickness", def, def->getID());
                  def->unsavedChanges = true;
                }
                else {
                  try {
                    pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenCapThickness(CTmap->valueMapFileName.toStdString());
                    qInfo() << "...OpenCapThickness done";
                  } catch (std::exception &e) {
                    eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                    PromptMessage(EV_MSG_ERROR_OPENING_RESET_TARGET);
                    break; 
                  }
                }
              else {
                PromptMessage(EV_MSG_ERROR_LIST_ENDING_PRELOAD_EARLY_FILE_NOT_EXIST);
                break; 
              }              
            }
          }
          else if (CTmap->valueMapFileName != "") {
            qWarning() << "target in list position" << i << "has a cap thickness value map, but pipeline state doesn't allow it.";
            //return;
          }
        }








        // readings
        if (def->getTargetReadingsFileName() != "") {
          if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ReadingsPreConditions()) {
            QFileInfo checkFile(def->getTargetReadingsFileName());
            if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable())
              if (false/* replace with test for whether the object should be recomputed because running a later version from that which created the object */) { 
                try {
                  pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeReadings();
                } catch (std::exception &e) {
                  eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                  PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
                  break; 
                }
                logUpdateWithStageSettings("EVTargetDefine::ComputeVesselTargetReadings", def, def->getID());
                def->unsavedChanges = true;
              }
              else {
                try {
                  pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenReadings(def->getTargetReadingsFileName().toStdString());
                  qInfo() << "...OpenReadings done";
                } catch (std::exception &e) {
                  eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                  PromptMessage(EV_MSG_ERROR_OPENING_RESET_TARGET);
                  break; 
                }
              }
            else {
              PromptMessage(EV_MSG_ERROR_LIST_ENDING_PRELOAD_EARLY_FILE_NOT_EXIST);
              break; 
            }
          }
          else {
            qWarning() << "target in list position" << i << "has readings, but pipeline state doesn't allow it.";
            //return;
          }
        } // end-if there are readings








        // lesionLumenPartition
        for (j=0; j < def->regions.size(); j++) {
          reg = &((def->regions)[j]);
          if (reg->regName.contains("lesionLumenPartition"))
            break;
        }
        if (j < def->regions.size()) {
          if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LesionLumenPartitionPreConditions()) {
            if (reg->regFileName != "") {
              QFileInfo checkFile(reg->regFileName);
              if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable()) {
                if (false/* replace with test for whether the object should be recomputed because running a later version from that which created the object */) { 
                  try {
                    pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLesionLumenPartition();
                  } catch (std::exception &e) {
                    eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                    PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
                    break; 
                  }
                  logUpdateWithStageSettings("EVTargetDefine::ComputeLesionLumenPartition", def, def->getID());
                  def->unsavedChanges = true;
                }
                else {
                  try {
                    pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenLesionLumenPartition(reg->regFileName.toStdString());
                    qInfo() << "...OpenLesionLumenPartition done";
                  } catch (std::exception &e) {
                    eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
	                  PromptMessage(EV_MSG_ERROR_OPENING_RESET_TARGET);
	                  break; 
                  }
                }
              }
              else {
                PromptMessage(EV_MSG_ERROR_LIST_ENDING_PRELOAD_EARLY_FILE_NOT_EXIST);
                break; 
              }              
            }
          }
          else if (reg->regFileName != "") {
            qWarning() << "target in list position" << i << "has a partitioned lumen for lesions, but pipeline state doesn't allow it.";
            //return;
          }
        }






        // lesionLumenAndWallPartition
        for (j=0; j < def->regions.size(); j++) {
          reg = &((def->regions)[j]);
          if (reg->regName.contains("lesionLumenAndWallPartition"))
            break;
        }
        if (j < def->regions.size()) {
          if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LesionLumenAndWallPartitionPreConditions()) {
            if (reg->regFileName != "") {
              QFileInfo checkFile(reg->regFileName);
              if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable()) {
                if (false/* replace with test for whether the object should be recomputed because running a later version from that which created the object */) { 
                  try {
                    pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLesionLumenAndWallPartition();
                  } catch (std::exception &e) {
                    eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                    PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
                    break; 
                  }
                  logUpdateWithStageSettings("EVTargetDefine::ComputeLesionLumenAndWallPartition", def, def->getID());
                  def->unsavedChanges = true;
                }
                else {
                  try {
                    pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenLesionLumenAndWallPartition(reg->regFileName.toStdString());
                    qInfo() << "...OpenLesionLumenAndWallPartition done";
                  } catch (std::exception &e) {
                    eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                    PromptMessage(EV_MSG_ERROR_OPENING_RESET_TARGET);
                    break; 
                  }
                }
              }
              else {
                PromptMessage(EV_MSG_ERROR_LIST_ENDING_PRELOAD_EARLY_FILE_NOT_EXIST);
                break; 
              }              
            }
          }
          else if (reg->regFileName != "") {
            qWarning() << "target in list position" << i << "has a partitioned lumenAndWall for lesions, but pipeline state doesn't allow it.";
            //return;
          }
        }







        // lesionPeriRegPartition
        for (j=0; j < def->regions.size(); j++) {
          reg = &((def->regions)[j]);
          if (reg->regName.contains("lesionPeriRegPartition"))
            break;
        }
        if (j < def->regions.size()) {
          if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LesionPerivascularRegionPartitionPreConditions()) {
            if (reg->regFileName != "") {
              QFileInfo checkFile(reg->regFileName);
              if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable()) {
                if (false/* replace with test for whether the object should be recomputed because running a later version from that which created the object */) { 
                  try {
                    pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLesionPerivascularRegionPartition();
                  } catch (std::exception &e) {
                    eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                    PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
                    break; 
                  }
                  logUpdateWithStageSettings("EVTargetDefine::ComputeLesionPerivascularRegionPartition", def, def->getID());
                  def->unsavedChanges = true;
                }
                else {
                  try {
                    pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenLesionPerivascularRegionPartition(reg->regFileName.toStdString());
                    qInfo() << "...OpenLesionPerivascularRegionPartition done";
                  } catch (std::exception &e) {
                    eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                    PromptMessage(EV_MSG_ERROR_OPENING_RESET_TARGET);
                    break; 
                  }
                }
              }
              else {
                PromptMessage(EV_MSG_ERROR_LIST_ENDING_PRELOAD_EARLY_FILE_NOT_EXIST);
                break; 
              }              
            }
          }
          else if (reg->regFileName != "") {
            qWarning() << "target in list position" << i << "has a partitioned perivascular EVRegion for lesions, but pipeline state doesn't allow it.";
            //return;
          }
        }








        // lesion readings
        if (def->getLesionReadingsFileName() != "") 
        {
          if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LesionReadingsPreConditions()) {
            QFileInfo checkFile(def->getLesionReadingsFileName());
            if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable())
              if (false/* replace with test for whether the object should be recomputed because running a later version from that which created the object */) 
              { 
                try 
                {
                  pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLesionReadings();
                } catch (std::exception &e) 
                {
                  eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                  PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
                  break; 
                }
                logUpdateWithStageSettings("EVTargetDefine::ComputeLesionReadings", def, def->getID());
                def->unsavedChanges = true;
              }
              else 
              {
                try 
                {
                  pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenLesionReadings(def->getLesionReadingsFileName().toStdString());
                  qInfo() << "...OpenLesionReadings done";
                } catch (std::exception &e) 
                {
                  eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                  PromptMessage(EV_MSG_ERROR_OPENING_RESET_TARGET);
                  break; 
                }
              }
            else {
              PromptMessage(EV_MSG_ERROR_LIST_ENDING_PRELOAD_EARLY_FILE_NOT_EXIST);
              break; 
            }
          }
          else 
          {
            qWarning() << "target in list position" << i << "has lesion readings, but pipeline state doesn't allow it.";
            //return;
          }
        } // end-if there are readings
      } // end-if the target can be initialized
      else
        qWarning() << "target in list position" << i << "cannot be initialized, skipping.";



      presentTarget(def);

    } // end-if the minimum spec is in place (ID, bodySite, and folder all being non-blank) 
    else 
      qWarning() << "target in list position" << i << "does not have the minimum specification, skipping.";
  } // end-for each listed target
  
}

void EVTargetDefine::selectCurrentTarget(EVTargetDef *newCurrentDef)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  std::cerr << GetEVLinkedViewers() << std::endl;

  SetCurrentTarget(newCurrentDef);

  EVTargetDef* currentTarget = GetCurrentTarget();
  //ebAssert(currentTarget);

  QList<EVTargetDef>* targets = GetTargets();
  ebAssert(targets);

 
  QString label;
  if (currentTarget != NULL) {
    if ((currentTarget->lesionReadingsIDdefine != null_ebID) || (currentTarget->readingsIDdefine != null_ebID) ||
              (currentTarget->pathIDdefine != null_ebID) || (currentTarget->initializerIDdefine != null_ebID)) {
      ebID id = null_ebID;
      if (currentTarget->lesionReadingsIDdefine != null_ebID)
        id = currentTarget->lesionReadingsIDdefine;
      else if (currentTarget->readingsIDdefine != null_ebID)
        id = currentTarget->readingsIDdefine;
      else if (currentTarget->pathIDdefine != null_ebID)
        id = currentTarget->pathIDdefine;
      else if (currentTarget->initializerIDdefine != null_ebID)
        id = currentTarget->initializerIDdefine;
      GetEVLinkedViewers()->GetScene()->SetSelectedVesselTarget(id);
      ebAssert(GetEVLinkedViewers() && GetEVLinkedViewers()->GetScene() && GetEVLinkedViewers()->GetScene()->GetSelectedVesselTarget());
      GetEVLinkedViewers()->GetScene()->GetSelectedVesselTarget()->GetVesselTarget()->SetName(currentTarget->getID().toStdString());
    }
    label = "Current target being defined: ";

    QString displayName = currentTarget->getID();
    // override the display name if it is a coronary target (because the greater branching complexity makes their screen labelling confusing if we don't do this)
    if (displayName.contains("oronary") && currentTarget->rootVesselName != "") {
      displayName = currentTarget->rootVesselName;
    }
    label.append(displayName);
    bool atLeastOneAlreadyInTheParens = false;
    if (targets->size() > 1) {
      label.append(" (other available targets include: ");
      for (int k=0; k < targets->size(); k++) {
        EVTargetDef *def = &((*targets)[k]);
        if (def->getID() != currentTarget->getID()) {
          QString otherTargetDisplayName = def->getID();
          // override the display name if it is a coronary target (because the greater branching complexity makes their screen labelling confusing if we don't do this)
          if (otherTargetDisplayName.contains("oronary") && def->rootVesselName != "") {
            otherTargetDisplayName = def->rootVesselName;
          }
          if (atLeastOneAlreadyInTheParens && (k < targets->size()))
            label.append(", ");
          label.append(otherTargetDisplayName);
          atLeastOneAlreadyInTheParens = true;
        } // end-if this isn't the current one  
      } // end-for each target
      label.append(")");
    } // end-if there are more targets defined
    else {
      label.append(" (no other targets have as yet been defined)");
    }
  }
  else { // the selected target is NULL, compose label accordingly
    label = "No current target focus, press ";
    if (targets->size() > 0) {
      label.append("Switch Target button to select from among ");
      bool atLeastOneAlreadyInTheParens = false;
      for (int k=0; k < targets->size(); k++) {
        EVTargetDef *def = &((*targets)[k]);
        QString otherTargetDisplayName = def->getID();
        // override the display name if it is a coronary target (because the greater branching complexity makes their screen labelling confusing if we don't do this)
        if (otherTargetDisplayName.contains("oronary") && def->rootVesselName != "") {
          otherTargetDisplayName = def->rootVesselName;
        }
        if (atLeastOneAlreadyInTheParens && (k < targets->size()))
          label.append(", ");
        label.append(otherTargetDisplayName);
        atLeastOneAlreadyInTheParens = true;
      } // end-for each target
      label.append(" (or press Create Target to add an additional one)");

    }
    else {
      label.append("Create Target button to make one");
    }
 
  }

  UpdateSenes();

  //this is on patient analyse
  //setCurrentTarget(currentTarget);

  GetEVLinkedViewers()->Render();
}

//void EVTargetDefine::acceptScreenControlFromAnalyze(QStackedWidget *seriesSelectionArea, EVImageSeries *series, TargetDef *def)
//{
//  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
//  selectCurrentTarget(def);
// // acceptScreenControlCommon(seriesSelectionArea, series);
//}
//
//void EVTargetDefine::acceptScreenControlFromSurvey(QStackedWidget *seriesSelectionArea, EVImageSeries *series)
//{
//  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
//  //acceptScreenControlCommon(seriesSelectionArea, series);
//}

void EVTargetDefine::CreateTargetPre(const QString& bodySite)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // compose a menu to get the root vessel name
  QList<QString> allowableVesselsGivenBodySite;

  if (bodySite == "Aorta") {
    allowableVesselsGivenBodySite = QList<QString>()
       << tr("Aorta")
       << tr("ThoracicAorta")
       << tr("AbdominalAorta")
       << tr("LeftCommonIliacArtery")
       << tr("RightCommonIliacArtery")
       ;
  } 
  else if (bodySite == "LeftCoronary") {
    allowableVesselsGivenBodySite = QList<QString>()
       << tr("MainStem")
       << tr("LeftAnteriorDescending")
       << tr("Circumflex")
       << tr("LeftCoronaryArtery")
       ;
  } 
  else if (bodySite == "RightCoronary") {
    allowableVesselsGivenBodySite = QList<QString>()
       << tr("RightCoronaryArtery")
       ;
  } 
  else if (bodySite == "LeftCarotid") {
    allowableVesselsGivenBodySite = QList<QString>()
       << tr("CommonCarotidArtery")
       << tr("InternalCarotidArtery")
       << tr("ExternalCarotidArtery")
       << tr("CarotidArtery")
       ;
  } 
  else if (bodySite == "RightCarotid") {
    allowableVesselsGivenBodySite = QList<QString>()
       << tr("CommonCarotidArtery")
       << tr("InternalCarotidArtery")
       << tr("ExternalCarotidArtery")
       << tr("CarotidArtery")
       ;
  } 
  else if ((bodySite == "LeftVertebral") || (bodySite == "RightVertebral")) {
    addVessel(tr("VertebralArtery"));
    return;
  } 
  else if ((bodySite == "LeftFemoral") || (bodySite == "RightFemoral")) {
    addVessel(tr("FemoralArtery"));
    return;
  } 
  else {
    addVessel(tr("NotSpecified"));
    return;
  }

  foreach (QString vessel, allowableVesselsGivenBodySite) {
    addRootVessel(vessel);
  }
}

void EVTargetDefine::addRootVessel(const QString& rootVesselName)
{
  EVTargetDef* currentTarget = GetCurrentTarget();
  ebAssert(currentTarget);

  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  currentTarget->rootVesselName = rootVesselName;
  addVessel(rootVesselName);
}

void EVTargetDefine::EstablishNewTarget(const QString& bodySite)
{
  ebLog eblog(Q_FUNC_INFO); eblog << bodySite.toStdString() << std::endl;

  QList<EVTargetDef>* targets = GetTargets();
  ebAssert(targets);
 

  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);

  if (coronalType == ebvViewer::CPR) {
    // can't define a target when in oblique, need to get back to axial
    toggleOblique(false);
  }

  // allocate the new target def
  EVTargetDef *newTarget = new EVTargetDef();
  newTarget->setBodySite(bodySite); // bodySite stored by createVesselPre to pass to createVesselAction
  newTarget->rootVesselName = ""; // will be set later but need empty string now for display logic

  // we establish the ID of the target to be the body site, or if this same body site has already been seen we also append an index count.  
  QString newID(bodySite);
  int countOfTargetsWithThisBodySite = 1;
  for (int i=0; i < targets->size(); i++) {
    EVTargetDef *def = &((*targets)[i]);
    if (def->getBodySite() == bodySite)
      countOfTargetsWithThisBodySite++;
  }
  if (countOfTargetsWithThisBodySite > 1)
    newID.append(QString::number(countOfTargetsWithThisBodySite)); // this will make the IDs be assigned for example as LeftCarotid, LeftCarotid2, LeftCarotid3, etc., which should work for the typical cases where a given body site isn't usually re-sued (as in LeftCarotid) but also works when it is (e.g., PulmonaryLesion)
  newTarget->setID(newID);

  // now proceed to the rest of the fields
  newTarget->setTargetReadingsFileName(""); 
  newTarget->setLesionReadingsFileName(""); 
  newTarget->setTargetPathFileName("");
  newTarget->setTargetRegistrationTransforms("");
  newTarget->setParentPipeline(pipeline);

  newTarget->targetPipelineID = pipeline->AddVesselTargetPipeline();
  pipeline->GetVesselTargetPipeline(newTarget->targetPipelineID)->SetBodySite(bodySite.toStdString());
  //newTarget->parameters = new processingParameters(this, owner->systemPreferencesObject, owner->clinicalJurisdiction, pipeline, newTarget->getBodySite(), newTarget->targetPipelineID);
  //connect(newTarget->parameters, SIGNAL(processingParametersSettingsChanged()), this, SLOT(processingParametersSettingsChanged()));
  newTarget->pushTargetParametersToPipeline(pipeline->GetVesselTargetPipeline(newTarget->targetPipelineID));
  if (pipeline->GetVesselTargetPipeline(newTarget->targetPipelineID)->InitializationPreConditions()) {
    newTarget->unsavedChanges = true;

    //// setting the targetFolder is more involved due to the need to carefully construct it
    //workItemListEntry *wi = owner->getWorkItem(sessionItemIndex)->selectedWorkItemListEntryPtr;
    //if (!wi->setWorkItemFolder()) { // this may be the first time an operation that requires the folder has been done
    //  message->showMessage(tr("Error: Cannot write work item folder."));
    //  return;
    //}
    //if (!newTarget->setTargetFolder(wi->getWorkItemFolder())) {
    //  message->showMessage(tr("Error: Cannot write target folder."));
    //  return;
    //}

    targets->append(*newTarget);


    // here we make sure we have the screen, as the create may have originated from analyze.  That would be simple enough but for a catch: a similar 
    // sequence is used when the request is to modify (rather than create) the target; and in that case, we want analyze to set the current target as 
    // what is to be modified. But this doesn't work for create, since analyze doesn't have the newly created one at this stage of the sequence yet. So
    // need to select the current target as the new one after the transition.
   // owner->getPatientAnalyze(sessionItemIndex)->giveToDefineIfNotAlreadyThere();
    selectCurrentTarget(&targets->last()); // the one we just appended
   // owner->getWorkItemProcess(sessionItemIndex)->setCurrentIndex(TARGETDEFINE);
   // qApp->processEvents();

    EVTargetDef* currentTarget = GetCurrentTarget();
    ebAssert(currentTarget);

    // now set up to collect points
    pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CreateInitialization();
    currentTarget->initializerIDdefine = GetEVLinkedViewers()->AddVesselTarget(pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetInitialization(), "initializer");
  }
  else {
    PromptMessage(EV_MSG_ERROR_NEW_TARGET_MAY_NOT_BE_INITIALIZED_SKIPPING);
  }
  
  SetCurrentTarget(newTarget);

  EVTargetDef* currentTarget = GetCurrentTarget();
  if(currentTarget);
 
  pipeline->GetVesselTargetPipeline(newTarget->targetPipelineID)->SetTargetFolder(currentTarget->getTargetFolder().toStdString());
}

void EVTargetDefine::addVessel(QString startFromName)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // enter gesture mode to collect points for root vessel
  GetEVLinkedViewers()->GetViewer3D(ebvIdVolume)->SetShowVolume(true);
  GetEVLinkedViewers()->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::AXIAL, false);
  GetEVLinkedViewers()->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CORONAL, false);
  GetEVLinkedViewers()->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::SAGITTAL, false);
  GetEVLinkedViewers()->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUE, false);
  GetEVLinkedViewers()->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUEPERP1, false);
  GetEVLinkedViewers()->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CPR, false);
  GetEVLinkedViewers()->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUEPERP2, false);

  //tools->disableToolButtons(); // almost everything needs to be disabled during this operation
  //tools->VolumeRenderTool->setChecked(true);

  //ENABLECONTROL(tools->OptimizeSubvolumeTool, tools->OptimizeSubvolumeAction, tr("Press to toggle widgets allowing optimization of subvolume"));

  //ENABLECONTROL(tools->TargetCreateTool, tools->TargetCreateAction, tr("Press to stop collecting points and process them"));
  //tools->TargetCreateTool->setChecked(true);

  //DISABLECONTROL(tools->TargetModifyTool, tools->TargetModifyAction, tr("TargetModify is disabled"));

  //ENABLECONTROL(tools->ToggleCoronalTool, tools->ToggleCoronalAction, tr("Press to toggle coronal plane"));
  //tools->ToggleCoronalTool->setChecked(false);
  //  
  //ENABLECONTROL(tools->ToggleSagittalTool, tools->ToggleSagittalAction, tr("Press to toggle sagittal plane"));
  //tools->ToggleSagittalTool->setChecked(false);
  //  
  //ENABLECONTROL(tools->ToggleAxialTool, tools->ToggleAxialAction, tr("Press to toggle axila plane"));
  //tools->ToggleAxialTool->setChecked(false);

  //ui->continueWithAnalysisButton->setEnabled(false); 
  //owner->getPatientAnalyze(sessionItemIndex)->disableMenuActions(); // propagate it downstream

  GetEVLinkedViewers()->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Sub-volume ready"));
  GetEVLinkedViewers()->SyncViewersToScene();
  GetEVLinkedViewers()->Render();
  // when adding rather than creating, proximal vessel is determined by distance rather than being explicitly given
  GetEVLinkedViewers()->StartVesselTargetInitialization(startFromName.toStdString());
  //ui->backToSurveyButton->setEnabled(false); // can't go back when in middle of operation
}

void EVTargetDefine::addDistalVessel()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;

  EVTargetDef* currentTarget = GetCurrentTarget();
  ebAssert(currentTarget);

  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);


  //owner->getPatientAnalyze(sessionItemIndex)->giveToDefineIfNotAlreadyThere();
  //owner->getWorkItemProcess(sessionItemIndex)->setCurrentIndex(TARGETDEFINE);
  // clear out any existing computations and displays
  // Keep only the initializer. This is accomplished by closing the lumen (if it exists) and then flushing.
  if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenSegmentation())
    pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseLumenSegmentation(); 
  clearTargetBeyondCurrentStage(currentTarget);
  addVessel("");
}

void EVTargetDefine::completeDistalVessel()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;


  EVTargetDef* currentTarget = GetCurrentTarget();
  ebAssert(currentTarget);

  // prompt user for distal vessel name
  // compose a menu to get the root vessel name
 // QMenu menu(this);
  QList<QString> allowableDistalVesselsGivenRoot;

  if (currentTarget->rootVesselName == "Aorta") {
    allowableDistalVesselsGivenRoot = QList<QString>()
       << tr("LeftCoronaryCusp")
       << tr("LeftCoronaryArtery")
       << tr("MainStem")
       << tr("RightCoronaryCusp")
       << tr("RightCoronaryArtery")
       << tr("NonCoronaryCusp")
       << tr("NotSpecified")
       ;
  } 
  else if (currentTarget->rootVesselName == "LeftCoronaryArtery") {
    allowableDistalVesselsGivenRoot = QList<QString>()
       << tr("LeftAnteriorDescending")
       << tr("Circumflex")
       << tr("Ramus")
       << tr("CoronaryArtery")
       << tr("NotSpecified")
       ;
  } 
  else if (currentTarget->rootVesselName == "MainStem") {
    allowableDistalVesselsGivenRoot = QList<QString>()
       << tr("LeftAnteriorDescending")
       << tr("Circumflex")
       << tr("CoronaryArtery")
       << tr("NotSpecified")
       ;
  } 
  else if (currentTarget->rootVesselName == "LeftAnteriorDescending") {
    allowableDistalVesselsGivenRoot = QList<QString>()
       << tr("Diagonal")
       << tr("Diagonal1")
       << tr("Diagonal2")
       << tr("SeptalPerforator")
       << tr("CoronaryArtery")
       << tr("NotSpecified")
       ;
  } 
  else if (currentTarget->rootVesselName == "Circumflex") {
    allowableDistalVesselsGivenRoot = QList<QString>()
       << tr("LeftMarginal")
       << tr("LeftMarginal1")
       << tr("LeftMarginal2")
       << tr("PosterioLateral")
       << tr("SinoAtrialNode")
       << tr("PosteriorDescending")
       << tr("AtrioventricularNodal")
       << tr("CoronaryArtery")
       << tr("NotSpecified")
       ;
  } 
  else if (currentTarget->rootVesselName == "RightCoronaryArtery") {
    allowableDistalVesselsGivenRoot = QList<QString>()
       << tr("AtrioventricularNodal")
       << tr("SinoAtrialNode")
       << tr("RightMarginal")
       << tr("PosteriorDescending")
       << tr("PosterioLateral")
       << tr("SeptalPerforator")
       << tr("ConusBranch")
       << tr("Ventricular")
       << tr("AcuteMarginal")
       << tr("RightVentricular")
       << tr("CoronaryArtery")
       << tr("NotSpecified")
       ;
  } 
  else if (currentTarget->rootVesselName == "CoronaryArtery") {
    allowableDistalVesselsGivenRoot = QList<QString>()
       << tr("CoronaryArtery")
       << tr("NotSpecified")
       ;
  } 
  else if (currentTarget->rootVesselName == "CommonCarotidArtery") {
    allowableDistalVesselsGivenRoot = QList<QString>()
       << tr("InternalCarotidArtery")
       << tr("ExternalCarotidArtery")
       << tr("NotSpecified")
       ;
  }
  else 
  {
    labelDistalVessel(tr("NotSpecified"));
    return;
  }

  foreach (QString vessel, allowableDistalVesselsGivenRoot) 
  {
    labelDistalVessel(vessel);
  }
}

void EVTargetDefine::labelDistalVessel(const QString& distalVesselName)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;


  EVTargetDef* currentTarget = GetCurrentTarget();
  ebAssert(currentTarget);


  // turn off the gesture mode (with distal vessel parameter) which causes the target data structures to be established
  GetEVLinkedViewers()->StopVesselTargetInitialization(distalVesselName.toStdString());
  PromptMessage(EV_MSG_ON_LABEL_DISTAL_VESSEL);
  // log the update and set the flag
  logUpdateWithStageSettings("targetDefine::labelDistalVessel (initializer)", currentTarget, currentTarget->getID());
  currentTarget->unsavedChanges = true;

}

bool EVTargetDefine::switchTargetPath()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;

  EVTargetDef* currentTarget = GetCurrentTarget();
  ebAssert(currentTarget);

  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);

  QList<EVTargetDef>* targets = GetTargets();
  ebAssert(targets);


  GetEVLinkedViewers()->GetScene()->IncrCurrentPathSetCursor(1);
  if (GetEVLinkedViewers()->GetScene()->IsVesselTargetSelected()) {
    //ENABLECONTROL(tools->ToggleObliqueTool, tools->ToggleObliqueAction, tr("Press to toggle slice viewer alignment"));
    //ENABLECONTROL(tools->MoveProximalTool, tools->MoveProximalAction, tr("Press to move proximally"));
    //ENABLECONTROL(tools->MoveDistalTool, tools->MoveDistalAction, tr("Press to move distally"));
    ebID id = GetEVLinkedViewers()->GetScene()->GetSelectedVesselTargetID();

    if ((currentTarget == NULL) 
        || ((id != currentTarget->initializerIDdefine) || (id != currentTarget->pathIDdefine) || (id != currentTarget->readingsIDdefine))) {
      // we not only switched paths, but targets as well.  Find the new one and switch to it.
      int j;
      for (j=0; j < targets->size(); j++) {
        if ((id == targets->at(j).initializerIDdefine) || (id == targets->at(j).pathIDdefine) || (id == targets->at(j).readingsIDdefine))
          break;
      }
      if (j < targets->size()) {
        selectCurrentTarget(&((*targets)[j]));
              ebAssert(GetEVLinkedViewers() && GetEVLinkedViewers()->GetScene() && GetEVLinkedViewers()->GetScene()->GetSelectedVesselTarget());
              GetEVLinkedViewers()->GetScene()->GetSelectedVesselTarget()->GetVesselTarget()->SetName(currentTarget->getID().toStdString());

        // if the initializer has no points, then get them
        if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetInitialization()->InitializationPointsEmpty()) {
          //owner->getPatientAnalyze(sessionItemIndex)->giveToDefineIfNotAlreadyThere();
          //owner->getWorkItemProcess(sessionItemIndex)->setCurrentIndex(TARGETDEFINE);
          CreateTargetPre(currentTarget->getBodySite());
          return false; // signifies that there had to be an initializer created
        }
        else if (!currentTarget->isViable) {
          // similar to case above but without going into the initializer point collecting sequence
          //owner->getPatientAnalyze(sessionItemIndex)->giveToDefineIfNotAlreadyThere();
          //owner->getWorkItemProcess(sessionItemIndex)->setCurrentIndex(TARGETDEFINE);
        }

        // update buttons buttons now that there is a specific target in focus
        //ENABLECONTROL(tools->TargetDeleteTool, tools->TargetDeleteAction, tr("Press to delete current target"));
        //ENABLECONTROL(tools->TargetModifyTool, tools->TargetModifyAction, tr("Press to modify current target"));
        //tools->TargetModifyTool->setChecked(false);
        return true;
      }
      else {
        qWarning() << "can't find target id" << id << ", skipping.";
      }
    } // end-if there had been no prior selected target or if selected target has changed
    else { // there is no change to the target (possibly the path within the target changed, but no actual target switch)
      return true;
    }
  } // end-if there is a selected target
  // get here either if there is no selected target or if can't find the one that supposedly is
  selectCurrentTarget(NULL);
 // DISABLECONTROL(tools->ToggleObliqueTool, tools->ToggleObliqueAction, tr("ToggleOblique is disabled (select a target with a path to enable)"));
  //DISABLECONTROL(tools->MoveProximalTool, tools->MoveProximalAction, tr("MoveProximal is disabled (select a target with a path to enable)"));
  //DISABLECONTROL(tools->MoveDistalTool, tools->MoveDistalAction, tr("MoveDistal is disabled (select a target to enable)"));
  return false;
}

void EVTargetDefine::resetAllTargetsDueToChangesInImages()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
 
  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);
  
  QList<EVTargetDef>* targets = GetTargets();
  ebAssert(targets);


  //this->setEnabled(false);
  for (int i=0; i < targets->size(); i++) {
    EVTargetDef *def = &((*targets)[i]);
    // Keep only the initializer. This is accomplished by closing the lumen (if it exists) and then flushing.
    if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenSegmentation())
      pipeline->GetVesselTargetPipeline(def->targetPipelineID)->CloseLumenSegmentation(); 
    clearTargetBeyondCurrentStage(def);
    def->unsavedChanges = true;
    logUpdateWithStageSettings("EVTargetDefine::resetAllTargetsDueToChangesInImages", def, def->getID());
  }
  GetEVLinkedViewers()->SyncViewersToScene();
  GetEVLinkedViewers()->Render();
  //this->setEnabled(true);
}

void EVTargetDefine::resetAllTargetsDueToDifferentImages()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;

  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);

  QList<EVTargetDef>* targets  = GetTargets();
  ebAssert(targets);

  //this->setEnabled(false);
  if (targets->size() > 0) {
    // start by clearing everything but the initializers
    resetAllTargetsDueToChangesInImages();

    // now prompt to see if want to save the initializers before scrubbing
    //QMessageBox msgBox(this);
    //msgBox.setText(tr("Resetting targets due to difference in images."));
    //msgBox.setInformativeText(tr("Do you want to save the target initializers, so they are available after the reset?"));
    //msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    //msgBox.setDefaultButton(QMessageBox::Yes);
    //msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
    //int ret = msgBox.exec();
    for (int i=0; i < targets->size(); i++) {
      EVTargetDef *def = &((*targets)[i]);
    //  if (ret == QMessageBox::Yes) {
    //    owner->save();
    //  }
      // now close the initializers and delete the pipeline
      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetInitialization())
        pipeline->GetVesselTargetPipeline(def->targetPipelineID)->CloseInitialization();  
      clearTargetBeyondCurrentStage(def);
      pipeline->RemoveVesselTargetPipeline(def->targetPipelineID);
      def->targetPipelineID = null_ebID;
    }

    // finally, select images again and re-load
    pipeline->SelectImages();
    preloadDefinePost();
  }
  //this->setEnabled(true);
}

void EVTargetDefine::deleteTarget()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  
  EVTargetDef* currentTarget = GetCurrentTarget();
  ebAssert(currentTarget);
  
  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);

  QList<EVTargetDef>* targets = GetTargets();
  ebAssert(targets);

  // this->setEnabled(false);
  if (currentTarget != NULL) {
    // determine whether there are unsaved changes and seek user confirmation if so
    if (currentTarget->unsavedChanges) {
      //QMessageBox msgBox(this);
      //msgBox.setText(tr("There are unsaved changes which will be lost if the target is deleted."));
      //msgBox.setInformativeText(tr("Do you want to save these changes before deleting?"));
      //msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
      //msgBox.setDefaultButton(QMessageBox::No);
      //msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
      //int ret = msgBox.exec();
      //if (ret == QMessageBox::Yes) {
      //  owner->save();
      //}
    }

    // start the deletion by removing all displays for the target. This is accomplished by closing the initializer and then flushing.
    //owner->getPatientAnalyze(sessionItemIndex)->giveToDefineIfNotAlreadyThere();
    //owner->getWorkItemProcess(sessionItemIndex)->setCurrentIndex(TARGETDEFINE);
    //owner->getPatientAnalyze(sessionItemIndex)->removeAllTargetDisplays();
    if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetInitialization())
      pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseInitialization(); 
    clearTargetBeyondCurrentStage(currentTarget);
    pipeline->RemoveVesselTargetPipeline(currentTarget->targetPipelineID);
    currentTarget->targetPipelineID = null_ebID;
    logUpdateWithStageSettings("EVTargetDefine::deleteTarget", currentTarget, currentTarget->getID());
    GetEVLinkedViewers()->SyncViewersToScene();
    if (currentTarget->isViable)
      --viableTargetsCount;
    // now that it is out of the displays, remove it from the targets list
    for (int i=0; i < targets->size(); i++) {
      if (&(targets->at(i)) == currentTarget) {
        targets->removeAt(i);
      }
    }
    currentTarget = NULL; // since we just deleted it!
    selectCurrentTarget(NULL); // to catch up all the labeling etc.

    // finally, perform a switch target path
    switchTargetPath();
    GetEVLinkedViewers()->SyncViewersToScene();
    GetEVLinkedViewers()->Render();
  }
  else
  {
    //message->showMessage(tr("Program logic error; can't remove a target unless it is current, yet current is NULL"));

  }
}

/*void EVTargetDefine::renameTarget()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  bool ok;
  QString name = QInputDialog::getText(this, tr("Rename Current Target"), tr("ID:"), QLineEdit::Normal, currentTarget->getID(), &ok);
  if (ok) {
    currentTarget->setID(name);
    pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetPath()->SetName(name.toStdString()); 
    currentTarget->unsavedChanges = true;
    QString step = "EVTargetDefine::renameTarget"; // log the update
    emit logUpdate(step, sessionItemIndex);
  }
}*/

void EVTargetDefine::evaluateLumen()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
 
  EVTargetDef* currentTarget = GetCurrentTarget();
  ebAssert(currentTarget);
  
  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);
  
  // proceed if there is a current target
  if ((currentTarget != NULL) 
      && (!pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetInitialization()->InitializationPointsEmpty())) {
    //this->setEnabled(false);
    if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->LumenSegmentationPreConditions()) {
      // first reset the pipeline if necessary, i.e., if a prior lumen had existed
      if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenSegmentation())
        pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseLumenSegmentation();
      clearTargetBeyondCurrentStage(currentTarget);

      //QITKProgressDialog progressIndicator(0,0);
      //progressIndicator.setWindowModality(Qt::NonModal);
      //progressIndicator.setMinimumDuration(10);
      //progressIndicator.setWindowFlags(progressIndicator.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::WindowStaysOnTopHint);
      //progressIndicator.setCancelButton(nullptr);   // no cancel button on dialog
      //progressIndicator.show();
      //progressIndicator.AddFilter(2,pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenSegmentationFilter(),2);
      //progressIndicator.AddFilter(3,pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetPathFilter(),1);
      //progressIndicator.AddFilter(4,pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenPartitionFilter(),0.5);
      //this->repaint(); // ensure progress is shown
      //qApp->processEvents();
      

      UpdateSenes();
      // and proceed step-wise with the task
      GetEVLinkedViewers()->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Delineating Luminal Surface"));
      GetEVLinkedViewers()->SyncViewersToScene();
      GetEVLinkedViewers()->Render();
      try {
              pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->ComputeLumenSegmentation();
      } catch (std::exception &e) {
              eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
        return; 
      }
      logUpdateWithStageSettings("EVTargetDefine::ComputeLumenSegmentation", currentTarget, currentTarget->getID());
      currentTarget->unsavedChanges = true;
      currentTarget->lumenSegIDdefine = GetEVLinkedViewers()->AddSegmentation4(pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenSegmentation(), 0, "lumenSegmentation");
      clearTargetBeyondCurrentStage(currentTarget);
      GetEVLinkedViewers()->SyncViewersToScene();
      GetEVLinkedViewers()->Render();

      if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->PathPreConditions()) {
        GetEVLinkedViewers()->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Computing Detailed Cross Section Positions"));
        GetEVLinkedViewers()->SyncViewersToScene();
        GetEVLinkedViewers()->Render();
        try {
          pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->ComputePath();
              } catch (std::exception &e) {
                eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
          return; 
        }

        logUpdateWithStageSettings("EVTargetDefine::ComputeVesselTargetPath", currentTarget, currentTarget->getID());
        currentTarget->unsavedChanges = true;
        GetEVLinkedViewers()->RemoveVesselTarget(currentTarget->initializerIDdefine); // now that we have a path, don't want to display the initializaer too
        currentTarget->initializerIDdefine = null_ebID;
        currentTarget->pathIDdefine = GetEVLinkedViewers()->AddVesselTarget(pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetPath(), "path");
        clearTargetBeyondCurrentStage(currentTarget);
        GetEVLinkedViewers()->SyncViewersToScene();
        GetEVLinkedViewers()->Render();
        if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->LumenPartitionPreConditions()) {
          GetEVLinkedViewers()->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Partitioning Lumen into Vessels"));
          GetEVLinkedViewers()->SyncViewersToScene();
          GetEVLinkedViewers()->Render();
          try {
            pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->ComputeLumenPartition();
                } catch (std::exception &e) {
                  eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;

            return; 
          }
          logUpdateWithStageSettings("EVTargetDefine::ComputeLumenPartition", currentTarget, currentTarget->getID());
          currentTarget->unsavedChanges = true;
          GetEVLinkedViewers()->RemoveSegmentation4(currentTarget->lumenSegIDdefine); // if have partitions, don't want to display the segmentation too
          currentTarget->lumenSegIDdefine = null_ebID;
          int i = 0;
          for (auto partition : *pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenPartition()) {
            currentTarget->lumenPartIDsDefine.insert(GetEVLinkedViewers()->AddSegmentation4(partition.GetPointer(), 0, "lumenPartition"+std::to_string(i++)));
          }
          clearTargetBeyondCurrentStage(currentTarget);
          GetEVLinkedViewers()->SyncViewersToScene();
          GetEVLinkedViewers()->Render();
        } // end-if can can compute global lumen
        else
          qWarning() << "evaluate lumen was not able to make it all the way through to the partitioned lumen.";
      } // end-if can compute vessel target path 
      
      GetEVLinkedViewers()->GetViewer(ebvIdVolume)->SetTemporaryText("");
      GetEVLinkedViewers()->SyncViewersToScene();
      GetEVLinkedViewers()->Render();
      currentTarget->isViable = true; // now that there is a lumen the target may be considered viable :-)
      viableTargetsCount++;

      //progressIndicator.setMaximum(100); // any value will take it down
    } // end-if can compute local lumen
    else
    {
    }
  }
  else
  {
  }
}

void EVTargetDefine::evaluateWall()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;

  EVTargetDef* currentTarget = GetCurrentTarget();
  ebAssert(currentTarget);

  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);


  if ((currentTarget != NULL)
      && (!pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetInitialization()->InitializationPointsEmpty())) {
    //this->setEnabled(false);
    // start by seeing if have to do lumen first
    if ((pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->LumenSegmentationPreConditions())
        && (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenSegmentation() == NULL)) {
      //QMessageBox msgBox(this);
      //msgBox.setText(tr("A lumen is needed before computing the wall, and preconditions to compute it are met."));
      //msgBox.setInformativeText(tr("Do you want to compute the lumen now and go straight through to the wall?"));
      //msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
      //msgBox.setDefaultButton(QMessageBox::Yes);
      //msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
      //int ret = msgBox.exec();
      //if (ret == QMessageBox::Yes) 
      //  evaluateLumen();
      //else {
      //  //this->setEnabled(true);
      //  return;
      //}
    }

    if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->RegistrationPreConditions()) {
      // first reset the pipeline if necessary, i.e., if a prior lumen had existed
      if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetResampledRegisteredImages())
        pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseRegistration();
      clearTargetBeyondCurrentStage(currentTarget);

      // display a dialog for progress
      //QITKProgressDialog progressIndicator(0,0);
      //progressIndicator.setWindowModality(Qt::NonModal);
      //progressIndicator.setMinimumDuration(10);
      //progressIndicator.setWindowFlags(progressIndicator.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::WindowStaysOnTopHint);
      //progressIndicator.setCancelButton(nullptr);   // no cancel button on dialog
      //progressIndicator.show();
      //progressIndicator.AddFilter(5,pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetRegistrationFilter(),0.01);
      //progressIndicator.AddFilter(6,pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenAndWallSegmentationFilter(),3);
      //progressIndicator.AddFilter(7,pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenAndWallPartitionFilter(),0.5);
      //this->repaint(); // ensure progress is shown
      //qApp->processEvents();

      UpdateSenes();
      // and proceed step-wise with the task
      GetEVLinkedViewers()->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Registering Images"));
      GetEVLinkedViewers()->SyncViewersToScene();
      GetEVLinkedViewers()->Render();
      try {
        pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->ComputeRegistration();
      } catch (std::exception &e) {
              eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
        //QMessageBox::warning(this, tr("ebException computing"), QString(tr("object, suggest resetting target")));
        //this->setEnabled(true);
        return;
      }
      logUpdateWithStageSettings("EVTargetDefine::ComputeRegistration", currentTarget, currentTarget->getID());
      currentTarget->unsavedChanges = true;

      if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->LumenAndWallSegmentationPreConditions()) {
        GetEVLinkedViewers()->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Delineating Outer Wall Surface"));
        GetEVLinkedViewers()->SyncViewersToScene();
        GetEVLinkedViewers()->Render();
        try {
                pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->ComputeLumenAndWallSegmentation();
              } catch (std::exception &e) {
                eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
          //this->setEnabled(true);
          return; 
        }
        logUpdateWithStageSettings("EVTargetDefine::ComputeLumenAndWallSegmentation", currentTarget, currentTarget->getID());
        currentTarget->unsavedChanges = true;
        currentTarget->wallSegIDdefine = GetEVLinkedViewers()->AddSegmentation4(pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenAndWallSegmentation(), 0, "wallSegmentation");
        GetEVLinkedViewers()->GetScene()->GetSegmentation4(currentTarget->wallSegIDdefine)->GetInteriorProperty()->SetOpacity(0);
        clearTargetBeyondCurrentStage(currentTarget);
        GetEVLinkedViewers()->SyncViewersToScene();
        GetEVLinkedViewers()->Render();

        if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->LumenAndWallPartitionPreConditions()) {
          GetEVLinkedViewers()->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Partitioning Wall For Specific Vessels"));
          GetEVLinkedViewers()->SyncViewersToScene();
          GetEVLinkedViewers()->Render();
          try {
            pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->ComputeLumenAndWallPartition();
                } catch (std::exception &e) {
                  eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                 /* message->showMessage(tr("Pipeline state does not allow registration, skipping.")); this, tr("Error computing"), QString(tr(PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);)));
            this->setEnabled(true);*/
            return; 
          }
          logUpdateWithStageSettings("EVTargetDefine::ComputeLumenAndWallPartition", currentTarget, currentTarget->getID());
          currentTarget->unsavedChanges = true;
          GetEVLinkedViewers()->RemoveSegmentation4(currentTarget->wallSegIDdefine); // if have partitions, don't want to display the segmentation too
          currentTarget->wallSegIDdefine = null_ebID;
          currentTarget->wallPartIDsDefine.clear();
          int i = 0;
          for (auto partition : *pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenAndWallPartition()) {
                  ebID partID = GetEVLinkedViewers()->AddSegmentation4(partition.GetPointer(),0,"wallPartition"+std::to_string(i++));
                  currentTarget->wallPartIDsDefine.insert(partID);
                  GetEVLinkedViewers()->GetScene()->GetSegmentation4(partID)->GetInteriorProperty()->SetOpacity(0);
                }
          clearTargetBeyondCurrentStage(currentTarget);
          GetEVLinkedViewers()->SyncViewersToScene();
          GetEVLinkedViewers()->Render();
        } // end-if can can compute partitions
        else
          qWarning() << "evaluate wall was not able to make it all the way through to the local wall.";
        
        GetEVLinkedViewers()->GetViewer(ebvIdVolume)->SetTemporaryText("");
        GetEVLinkedViewers()->SyncViewersToScene();
        GetEVLinkedViewers()->Render();
 
        currentTarget->unsavedChanges = true;

        ////ENABLECONTROL(tools->ToggleWallTool, tools->ToggleWallAction, tr("Press to toggle wall display"));
        //tools->ToggleWallTool->setChecked(true);
        //tools->editSegmentationEnableState = true;
        //tools->editSegmentationActionLabel = tr("Edit Wall");

        //ui->continueWithAnalysisButton->setEnabled(true); 
        //owner->getPatientAnalyze(sessionItemIndex)->enableMenuActions(); // propagate it downstream
        
        //progressIndicator.setMaximum(100); // any value will take it down
      } // end-if can compute wall  
      else
      {
        //message->showMessage(tr("Pipeline state does not allow to compute wall, skipping."));
      }
    } // end-if can do registration
    else
    {
      //message->showMessage(tr("Pipeline state does not allow registration, skipping."));
    }

  }
  else
  {
    //message->showMessage(tr("Warning: Need a current target focus with completed initializer, skipping."));
  }
}

void EVTargetDefine::editSegmentation(bool checked)
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  //if ((currentTarget != NULL) 
  //  && (!pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetInitialization()->InitializationPointsEmpty())) {
  //   UpdateSenes();
  //  if (checked) {
  //    // start by swapping the volume and axial viewers, making the large view a slice view
  //    savedParallelScale = viewers->GetViewer2D(ebvIdAxial)->GetParallelScale();
  //    savedPerspectiveDistance = viewers->GetViewer3D(ebvIdVolume)->GetPerspectiveDistance();
  //    viewers->RemoveViewer(ebvIdAxial);
  //    viewers->RemoveViewer(ebvIdVolume);
  //    ebvIdVolume = viewers->AddViewer((tools->ToggleObliqueTool->isChecked() ? ebvViewer::OBLIQUE : ebvViewer::AXIAL),ui->volumeRender->GetRenderWindow());
  //    viewers->GetViewer(ebvIdVolume)->SetShowLogo(true);
  //    tools->resetViewer(ebvIdVolume, ebvViewer::AXIAL);
  //    ebvIdAxial = viewers->AddViewer(ebvViewer::THREED,ui->axialRender->GetRenderWindow());
  //    viewers->GetViewer(ebvIdAxial)->SetShowLogo(true);
  //    viewers->GetViewer3D(ebvIdAxial)->SetShowVolume(false);
  //    viewers->GetViewer3D(ebvIdAxial)->SetShowSlice(ebvViewer::AXIAL, false);
  //    viewers->GetViewer3D(ebvIdAxial)->SetShowSlice(ebvViewer::CORONAL, false);
  //    viewers->GetViewer3D(ebvIdAxial)->SetShowSlice(ebvViewer::SAGITTAL, false);
  //    viewers->GetViewer3D(ebvIdAxial)->SetShowSlice(ebvViewer::OBLIQUE, false);
  //    viewers->GetViewer3D(ebvIdAxial)->SetShowSlice(ebvViewer::OBLIQUEPERP1, false);
  //    viewers->GetViewer3D(ebvIdAxial)->SetShowSlice(ebvViewer::CPR, tools->ToggleObliqueTool->isChecked());
  //    viewers->PanCamerasToCursor(true, true);
  //    savedSlabThickness = viewers->GetScene()->GetSlabThickness();
  //    viewers->GetScene()->SetSlabThickness(0);
  //    viewers->UpdateCameras();

  //    double window, level;
  //    ebAssert(viewers && viewers->GetScene() && viewers->GetScene()->GetSelectedImage4());
  //    viewers->GetScene()->GetSelectedImage4()->GetWindowLevel(window, level);
  //    viewers->GetViewer2D(ebvIdVolume)->SetParallelScale(.5*savedParallelScale);
  //    currentBackingSeries->window = window;
  //    currentBackingSeries->level = level;
  //    viewers->GetScene()->GetSelectedImage4()->SetWindowLevel(currentBackingSeries->window, currentBackingSeries->level);

  //    viewers->GetViewer3D(ebvIdAxial)->SetPerspectiveDistance(.3*savedPerspectiveDistance);
  //    ui->axialLabel->setText("Surface Rendering");
  //    // start by seeing if it is wall or lumen which will be edited
  //    if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenAndWallSegmentation() != NULL) {
  //      currentTarget->wallSegIDdefine = viewers->AddSegmentation4(pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenAndWallSegmentation(), 0, "wallSegmentation");
  //            viewers->GetScene()->GetSegmentation4(currentTarget->wallSegIDdefine)->GetInteriorProperty()->SetOpacity(0);  
  //      pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseCapThickness(); 
  //      pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseComposition(); 
  //      pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseWallThickness(); 
  //      pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseLumenAndWallPartition();
  //      clearTargetBeyondCurrentStage(currentTarget); // this will take down the partitions and anything else downstream
  //      logUpdateWithStageSettings("EVTargetDefine::EditLumenAndWallSegmentationStart", currentTarget, currentTarget->getID());
  //      viewers->SyncViewersToScene();
  //      viewers->Render();
  //      viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Editing mode on").toStdString());
  //            viewers->StartSegmentationEditor();
  //      editingSegmentation = true;
  //            segmentationEditor->show();
  //      tools->editSegmentationActionLabel = tr("Exit editing mode");
  //      editingWall = true;
  //      ui->backToSurveyButton->setEnabled(false); // can't go back when in middle of operation
  //    }
  //    else if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenSegmentation() != NULL) {
  //      currentTarget->lumenSegIDdefine = viewers->AddSegmentation4(pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenSegmentation(), 0, "lumenSegmentation");
  //      pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseLumenPartition();
  //      clearTargetBeyondCurrentStage(currentTarget); // this will take down the partitions and anything else downstream
  //      //pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetInitialization()->SetVisibility(false); // since took off the path, the initializer would display but don't want it to obscure the edit
  //      if (currentTarget->initializerIDdefine) {
  //        viewers->RemoveVesselTarget(currentTarget->initializerIDdefine); // if have a path, don't want to display the initializer too
  //        currentTarget->initializerIDdefine = null_ebID;
  //      }
  //      logUpdateWithStageSettings("EVTargetDefine::EditLumenSegmentationStart", currentTarget, currentTarget->getID());
  //      viewers->SyncViewersToScene();
  //      viewers->Render();
  //      viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Editing mode on").toStdString());
  //            viewers->StartSegmentationEditor();
  //            segmentationEditor->show();
  //      tools->editSegmentationActionLabel = tr("Exit editing mode");
  //      editingWall = false;
  //      ui->backToSurveyButton->setEnabled(false); // can't go back when in middle of operation
  //    }
  //    else {
  //      qWarning() << "no lumen or wall to edit.";
  //    }
  //    //DISABLECONTROL(tools->ToggleCoronalTool, tools->ToggleCoronalAction, tr("ToggleCoronal is disabled during editing"));
  //    //DISABLECONTROL(tools->ToggleSagittalTool, tools->ToggleSagittalAction, tr("ToggleSagittal is disabled during editing"));
  //    //DISABLECONTROL(tools->ToggleAxialTool, tools->ToggleAxialAction, tr("ToggleAxial is disabled during editing"));
  //    //DISABLECONTROL(tools->ToggleLumenTool, tools->ToggleLumenAction, tr("Toggle lumen display is disabled during editing"));
  //    //DISABLECONTROL(tools->OptimizeSubvolumeTool, tools->OptimizeSubvolumeAction, tr("Optimization of subvolume is disabled during editing"));
  //    //DISABLECONTROL(tools->AnnotateTool, tools->AnnotateAction, tr("Annotation is disabled during editing"));
  //    //DISABLECONTROL(tools->TargetPathSwitchTool, tools->TargetPathSwitchAction, tr("Switch target path is disabled during editing"));
  //    //DISABLECONTROL(tools->TargetDeleteTool, tools->TargetDeleteAction, tr("Delete target is disabled during editing"));
  //    //DISABLECONTROL(tools->TargetCreateTool, tools->TargetCreateAction, tr("Create target is disabled during editing"));
  //    //DISABLECONTROL(tools->ToggleObliqueTool, tools->ToggleObliqueAction, tr("Toggle slice viewer alignment is disabled during editing"));
  //    //DISABLECONTROL(tools->VolumeRenderTool, tools->VolumeRenderAction, tr("Volume rendering is disabled during editing"));
  //    //DISABLECONTROL(tools->ShowAsMovieTool, tools->ShowAsMovieAction, tr("Show as movie is disabled during editing"));
  //    //DISABLECONTROL(tools->MoveProximalTool, tools->MoveProximalAction, tr("MoveProximal is disabled during editing"));
  //    //DISABLECONTROL(tools->MoveDistalTool, tools->MoveDistalAction, tr("MoveDistal is disabled during editing"));
  //    //ui->processingParametersButton->setEnabled(false); 
  //    // take down the parameter setting dialog if it may be up, because can't change paramters during editing
  //    //if (currentTarget->parameters != NULL)
  //    //  currentTarget->parameters->cancelParameterSetting();
  //  }
  //  else { // stop editing and re-compute partitions
  //    logUpdateWithStageSettings("EVTargetDefine::EditSegmentationStop", currentTarget, currentTarget->getID());
  //    viewers->StopSegmentationEditor();
  //    editingSegmentation = false;
  //    segmentationEditor->hide();
  //    currentTarget->unsavedChanges = true;
  //    this->setEnabled(false);
  //    ui->backToSurveyButton->setEnabled(true); // can go back now that operation is done
  //    // put viewers back to the default state
  //    viewers->RemoveViewer(ebvIdAxial);
  //    viewers->RemoveViewer(ebvIdVolume);
  //    ebvIdVolume = viewers->AddViewer(ebvViewer::THREED,ui->volumeRender->GetRenderWindow());
  //    viewers->GetViewer(ebvIdVolume)->SetShowLogo(true);
  //    tools->resetViewer(ebvIdVolume, ebvViewer::THREED);
  //    viewers->GetViewer3D(ebvIdVolume)->SetShowVolume(false);
  //    if (tools->ToggleObliqueTool->isChecked()) {
  //      viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::AXIAL, false);
  //      viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CORONAL, false);
  //      viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::SAGITTAL, false);
  //      viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUE, tools->ToggleAxialTool->isChecked());
  //      viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUEPERP1, tools->ToggleCoronalTool->isChecked());
  //      viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CPR, tools->ToggleSagittalTool->isChecked());
  //    }
  //    else {
  //      viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::AXIAL, tools->ToggleAxialTool->isChecked());
  //      viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CORONAL, tools->ToggleCoronalTool->isChecked());
  //      viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::SAGITTAL, tools->ToggleSagittalTool->isChecked());
  //      viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUE, false);
  //      viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUEPERP1, false);
  //      viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CPR, false);
  //    }
  //    //ENABLECONTROL(tools->ToggleCoronalTool, tools->ToggleCoronalAction, tr("Press to toggle coronal plane"));
  //    //ENABLECONTROL(tools->ToggleSagittalTool, tools->ToggleSagittalAction, tr("Press to toggle sagittal plane"));
  //    //ENABLECONTROL(tools->ToggleAxialTool, tools->ToggleAxialAction, tr("Press to toggle axial plane"));
  //    //ENABLECONTROL(tools->ToggleLumenTool, tools->ToggleLumenAction, tr("Press to toggle lumen display"));
  //    //ENABLECONTROL(tools->OptimizeSubvolumeTool, tools->OptimizeSubvolumeAction, tr("Press to toggle widgets allowing optimization of subvolume"));
  //    //ENABLECONTROL(tools->AnnotateTool, tools->AnnotateAction, tr("Press to annotate"));
  //    //if (targets->size() >= 1) {
  //    //  ENABLECONTROL(tools->TargetPathSwitchTool, tools->TargetPathSwitchAction, tr("Press to switch target path"));
  //    //}
  //    //else {
  //    //  DISABLECONTROL(tools->TargetPathSwitchTool, tools->TargetPathSwitchAction, tr("Switch target path is disabled"));
  //    //}
  //    //ENABLECONTROL(tools->TargetCreateTool, tools->TargetCreateAction, tr("Press to create a new target"));
  //    //ENABLECONTROL(tools->TargetDeleteTool, tools->TargetDeleteAction, tr("Press to delete current target"));
  //    //ENABLECONTROL(tools->TargetModifyTool, tools->TargetModifyAction, tr("Press to modify current target"));
  //    //ENABLECONTROL(tools->ToggleObliqueTool, tools->ToggleObliqueAction, tr("Press to toggle slice viewer alignment"));
  //    //ENABLECONTROL(tools->VolumeRenderTool, tools->VolumeRenderAction, tr("Press to toggle volume rendering"));
  //    //ENABLECONTROL(tools->ShowAsMovieTool, tools->ShowAsMovieAction, tr("Press to show as movie"));
  //    //ENABLECONTROL(tools->MoveProximalTool, tools->MoveProximalAction, tr("Press to move proximally"));
  //    //ENABLECONTROL(tools->MoveDistalTool, tools->MoveDistalAction, tr("Press to move distally"));
  //    //ui->processingParametersButton->setEnabled(true); 
  //    //ebvIdAxial = viewers->AddViewer((tools->ToggleObliqueTool->isChecked() ? ebvViewer::OBLIQUE : ebvViewer::AXIAL),ui->axialRender->GetRenderWindow());
  //    //viewers->GetViewer(ebvIdAxial)->SetShowLogo(true);
  //    ////viewers->GetViewer2D(ebvIdAxial)->SetSlabLineVisible(0,false);
  //    viewers->GetScene()->SetImageSlabType(VTK_IMAGE_SLAB_MAX);
  //    viewers->GetScene()->SetSlabThickness(savedSlabThickness);
  //    viewers->UpdateCameras();
  //    ebAssert(viewers && viewers->GetScene() && viewers->GetScene()->GetSelectedImage4());
  //    viewers->GetScene()->GetSelectedImage4()->GetWindowLevel(currentBackingSeries->window, currentBackingSeries->level);
  //    viewers->GetScene()->GetSelectedImage4()->SetWindowLevel(currentBackingSeries->window, currentBackingSeries->level);
  //    viewers->GetViewer2D(ebvIdAxial)->SetParallelScale(savedParallelScale);
  //    viewers->GetViewer3D(ebvIdVolume)->SetPerspectiveDistance(savedPerspectiveDistance);
  //    ui->axialLabel->setText(tr("Maximum-Intensity Projection enabled (set slab thickness as desired)"));
  //    viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Editing mode off").toStdString());
  //    if (editingWall) {
  //      auto newLumenAndWall = LevelSetOr(pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenSegmentation(), pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenAndWallSegmentation());
  //      ImageCopyPixels(newLumenAndWall.GetPointer(), pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenAndWallSegmentation());
  //      LevelSetReinit(pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenAndWallSegmentation(),20);
  //      if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->LumenAndWallPartitionPreConditions()) {
  //        viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Paritioning Wall For Specific Vessels").toStdString());
  //        viewers->SyncViewersToScene();
  //        viewers->Render();
  //        try {
  //          pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->ComputeLumenAndWallPartition();
  //        } catch (std::exception &e) {
  //          eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
  //          QMessageBox::warning(this, tr("Error computing"), QString(tr(PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);)));
  //          ui->continueWithAnalysisButton->setEnabled(true);
  //          ui->backToSurveyButton->setEnabled(true);
  //          this->setEnabled(true);
  //          return; 
  //        }
  //        logUpdateWithStageSettings("EVTargetDefine::ComputeLumenAndWallPartition", currentTarget, currentTarget->getID());
  //        currentTarget->unsavedChanges = true;
  //        if (currentTarget->wallSegIDdefine != null_ebID)
  //          viewers->RemoveSegmentation4(currentTarget->wallSegIDdefine); // if have partitions, don't want to display the segmentation too
  //        currentTarget->wallSegIDdefine = null_ebID;
  //        currentTarget->wallPartIDsDefine.clear();
  //        int i = 0;
  //        for (auto partition : *pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenAndWallPartition()) {
  //          ebID partID = viewers->AddSegmentation4(partition.GetPointer(),0,"wallPartition"+std::to_string(i++));
  //          currentTarget->wallPartIDsDefine.insert(partID);
  //          viewers->GetScene()->GetSegmentation4(partID)->GetInteriorProperty()->SetOpacity(0);   
  //        }
  //        clearTargetBeyondCurrentStage(currentTarget);
  //        tools->editSegmentationActionLabel = tr("Edit Wall");
  //        viewers->SyncViewersToScene();
  //        viewers->Render();
  //      } // end-if can can compute partitions
  //      else
  //        qWarning() << "evaluate wall was not able to re-partition.";
  //    }
  //    else {
  //      pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->ClosePath();
  //      if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->PathPreConditions()) {
  //        // re-compute new path
  //        viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Computing Detailed Cross-section Positions").toStdString());
  //        viewers->SyncViewersToScene();
  //        viewers->Render();
  //        try {
  //          pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->ComputePath();
  //        } catch (std::exception &e) {
  //          eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
  //          QMessageBox::warning(this, tr("Error computing"), QString(tr(PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);)));
  //          ui->continueWithAnalysisButton->setEnabled(true);
  //          ui->backToSurveyButton->setEnabled(true);
  //          this->setEnabled(true);
  //          return; 
  //        }
  //        logUpdateWithStageSettings("EVTargetDefine::ComputeVesselTargetPath", currentTarget, currentTarget->getID());
  //        currentTarget->unsavedChanges = true;
  //        if (currentTarget->initializerIDdefine) {
  //          viewers->RemoveVesselTarget(currentTarget->initializerIDdefine); // if have a path, don't want to display the initializer too
  //          currentTarget->initializerIDdefine = null_ebID;
  //        }
  //        currentTarget->pathIDdefine = viewers->AddVesselTarget(pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetPath(), "path"); 
  //        clearTargetBeyondCurrentStage(currentTarget);

  //        // re-compute new partition
  //        viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Partitioning Lumen into Vessels").toStdString());
  //        viewers->SyncViewersToScene();
  //        viewers->Render();
  //        try {
  //          pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->ComputeLumenPartition();
  //        } catch (std::exception &e) {
  //          eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
  //          QMessageBox::warning(this, tr("Error computing"), QString(tr(PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);)));
  //          ui->continueWithAnalysisButton->setEnabled(true);
  //          ui->backToSurveyButton->setEnabled(true);
  //          this->setEnabled(true);
  //          return; 
  //        }
  //        logUpdateWithStageSettings("EVTargetDefine::ComputeLumenPartition", currentTarget, currentTarget->getID());
  //        currentTarget->unsavedChanges = true;
  //        if (currentTarget->lumenSegIDdefine != null_ebID)
  //          viewers->RemoveSegmentation4(currentTarget->lumenSegIDdefine); // if have partitions, don't want to display the segmentation too
  //        currentTarget->lumenSegIDdefine = null_ebID;
  //        int i = 0;
  //        for (auto partition : *pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenPartition()) {
  //          currentTarget->lumenPartIDsDefine.insert(viewers->AddSegmentation4(partition.GetPointer(), 0, "lumenPartition"+std::to_string(i++)));
  //        }
  //        clearTargetBeyondCurrentStage(currentTarget);

  //        // sync up the display
  //        tools->editSegmentationActionLabel = tr("Edit Lumen");
  //        viewers->SyncViewersToScene();
  //        viewers->Render();
  //      } // end-if can can compute global lumen
  //      else {
  //        qWarning() << "evaluate lumen was not able to re-partition.";
  //      }
  //    }
  //    ui->continueWithAnalysisButton->setEnabled(true);
  //    ui->backToSurveyButton->setEnabled(true);
  //  }

  //  viewers->SyncViewersToScene();
  //  viewers->Render(); 
  //  this->setEnabled(true);
  //}
}

void EVTargetDefine::resetWall()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  
  EVTargetDef* currentTarget = GetCurrentTarget();
  ebAssert(currentTarget);

  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);

  if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->RegistrationPreConditions()) {
    // first reset the pipeline if necessary, i.e., if a prior lumen had existed
    if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetResampledRegisteredImages()) {
      pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseRegistration();
    }
    clearTargetBeyondCurrentStage(currentTarget);
  }
  GetEVLinkedViewers()->SyncViewersToScene();
  GetEVLinkedViewers()->Render();
}

void EVTargetDefine::resetTarget()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  EVTargetDef* currentTarget = GetCurrentTarget();
  ebAssert(currentTarget);

  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);

  if (currentTarget != NULL) {
    //this->setEnabled(false);
    // determine whether there are unsaved changes and ask whether user wants to save them if so if so
    if (currentTarget->unsavedChanges) {
      //QMessageBox msgBox(this);
      //msgBox.setText(tr("There are unsaved changes which will be lost if the target is reset."));
      //msgBox.setInformativeText(tr("Do you want to save these changes before deleting?"));
      //msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
      //msgBox.setDefaultButton(QMessageBox::No);
      //msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
      //int ret = msgBox.exec();
      //if (ret == QMessageBox::Yes) {
      //  owner->save();
      //}
    }

    // close the old initializer
    if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetInitialization()) {
      pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseInitialization();  
    }
    clearTargetBeyondCurrentStage(currentTarget);
    GetEVLinkedViewers()->SyncViewersToScene();
    GetEVLinkedViewers()->Render();

    // now start the sequence for the new initializer
    if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->InitializationPreConditions()) {
      pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CreateInitialization();
      currentTarget->initializerIDdefine = GetEVLinkedViewers()->AddVesselTarget(pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetInitialization(), "initializer");
      CreateTargetPre(currentTarget->getBodySite());
    }
    else
    {
      PromptMessage(EV_MSG_PIPELN_STATE_OUT_OF_SYNC);
    }

    logUpdateWithStageSettings("EVTargetDefine::resetTarget", currentTarget, currentTarget->getID());
    //this->setEnabled(true);
  }
  else
  {
    // message->showMessage(tr("Program logic error; can't remove a target unless it is current, yet current is NULL"));
  }
}

void EVTargetDefine::toggleOblique(bool checked)
{
  ebLog eblog(Q_FUNC_INFO); eblog << (checked ? "OBLIQUE/CPR" : "AXIS-ALIGNED") << std::endl;
 // if (owner->getWorkItemProcess(sessionItemIndex)->currentIndex() != TARGETDEFINE) {
 //   return;
 // }
  GetEVLinkedViewers()->RemoveViewer(ebvIdAxial);
  GetEVLinkedViewers()->RemoveViewer(ebvIdCoronal);
  if (checked) {
    // make it oblique
    axialType = ebvViewer::OBLIQUE;
    //ebvIdAxial = viewers->AddViewer(axialType,ui->axialRender->GetRenderWindow());
    GetEVLinkedViewers()->GetViewer(ebvIdAxial)->SetShowLogo(true);
    coronalType = ebvViewer::CPR;
    ///ebvIdCoronal = viewers->AddViewer(coronalType,ui->coronalRender->GetRenderWindow());
    ebAssert(GetEVLinkedViewers()->GetViewerCPR(ebvIdCoronal));
    GetEVLinkedViewers()->GetViewer(ebvIdCoronal)->SetShowLogo(true);
    GetEVLinkedViewers()->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::AXIAL, false);
    GetEVLinkedViewers()->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CORONAL, false);
    GetEVLinkedViewers()->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::SAGITTAL, false);

    //if (tools->ToggleAxialTool->isChecked())
    //  viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUE, tools->ToggleAxialTool->isChecked());
    //else
    //  viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUE, false);
    //if (tools->ToggleCoronalTool->isChecked())
    //  viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUEPERP1, tools->ToggleCoronalTool->isChecked());
    //else
    //  viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUEPERP1, false);
    //if (tools->ToggleSagittalTool->isChecked())
    //  viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CPR, tools->ToggleSagittalTool->isChecked());
    //else
    //  viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CPR, false);
  }
  else {
    // make it aligned
    axialType = ebvViewer::AXIAL;
   // ebvIdAxial = viewers->AddViewer(axialType,ui->axialRender->GetRenderWindow());
    GetEVLinkedViewers()->GetViewer(ebvIdAxial)->SetShowLogo(true);
    coronalType = ebvViewer::CORONAL;
    //ebvIdCoronal = viewers->AddViewer(coronalType,ui->coronalRender->GetRenderWindow());
    GetEVLinkedViewers()->GetViewer(ebvIdCoronal)->SetShowLogo(true);
    //if (tools->ToggleAxialTool->isChecked())
    //  viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::AXIAL, tools->ToggleAxialTool->isChecked());
    //else
    //  viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::AXIAL, false);
    //if (tools->ToggleCoronalTool->isChecked())
    //  viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CORONAL, tools->ToggleCoronalTool->isChecked());
    //else
    //  viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CORONAL, false);
    //if (tools->ToggleSagittalTool->isChecked())
    //  viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::SAGITTAL, tools->ToggleSagittalTool->isChecked());
    //else
    //  viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::SAGITTAL, false);
    GetEVLinkedViewers()->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUE, false);
    GetEVLinkedViewers()->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUEPERP1, false);
    GetEVLinkedViewers()->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CPR, false);
  }
  //tools->ToggleObliqueTool->setChecked(checked);
  GetEVLinkedViewers()->SyncViewersToScene();
  GetEVLinkedViewers()->Render();
}

void EVTargetDefine::giveToAnalyzeIfNotAlreadyThere()
{
  ////ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  //if (ui->seriesSelectionArea->count() != 0) // if not 0, then the screen is currently on EVTargetDefine, so must give it to patientAnalyze
  //  on_continueWithAnalysisButton_clicked();

}

void EVTargetDefine::toggleSegmentation(QString regName, bool checked)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  QList<EVTargetDef>* targets = GetTargets();
  ebAssert(targets);

  // loop through targets, because the segmentation toggles effect all of them
  for (int i=0; i < targets->size(); i++) {
    // go through multiple levels of check to see if the specification is complete enough to add
    EVTargetDef *def = &((*targets)[i]);
    // have to do this in layers, since partitions override segmentations
    if (regName.contains("lumen")) {
      if (def->lumenSegIDdefine != null_ebID)
        GetEVLinkedViewers()->GetScene()->GetSegmentation4(def->lumenSegIDdefine)->SetVisibility(checked);
      if (!def->lumenPartIDsDefine.empty()) {
        for (auto partitionID : def->lumenPartIDsDefine) {
          GetEVLinkedViewers()->GetScene()->GetSegmentation4(partitionID)->SetVisibility(checked);
        }
      }
    }
    if (regName.contains("wall")) {
      if (def->wallSegIDdefine != null_ebID) 
        GetEVLinkedViewers()->GetScene()->GetSegmentation4(def->wallSegIDdefine)->SetVisibility(checked);
      if (!def->wallPartIDsDefine.empty()) {
        for (auto partitionID : def->wallPartIDsDefine) {
          GetEVLinkedViewers()->GetScene()->GetSegmentation4(partitionID)->SetVisibility(checked);
        }
      }
    }
  } // end-for each listed target
  GetEVLinkedViewers()->SyncViewersToScene();
  GetEVLinkedViewers()->Render();
}
