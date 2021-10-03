#include "EVVesselPipeline.h"



EVVesselPipeline::EVVesselPipeline(EVWorkItem* pWorkItem)
{
    GetMultiImageReader()->SetUseThread(true);
}


EVVesselPipeline::~EVVesselPipeline()
{
}
