#include "L1Trigger/L1TMuonOverlapPhase2/interface/PtAssignment_TF_NN.h"

/////////////////////////////
/////////////////////////////
PtAssignment_TF_NN::PtAssignment_TF_NN(const edm::ParameterSet& edmCfg, const OMTFConfiguration* omtfConfig):PtAssignmentBase(omtfConfig){

 std::string  model_path = edmCfg.getParameter<std::string>("tf_neuralNetworkFile");
 initializeModel(model_path);
  
}
/////////////////////////////
/////////////////////////////
void PtAssignment_TF_NN::initializeModel(const std::string & model_path){

  auto status = tensorflow::LoadSavedModel(session_options,
                                         run_options,
                                         model_path,
                                         {tensorflow::kSavedModelTagServe},
                                         &model);
  if (!status.ok()) {
      std::cerr << "Failed to load model: " << status;
  return;
  }

  // fill the input tensors with data
auto sig_map = model.GetSignatures();
auto model_def = sig_map.at("serving_default");

std::cout<<"Model Signature"<<std::endl;
for (auto const& p : sig_map) {
    std::cout<<"key: "<<p.first<<std::endl;
}

cout<<"Model Input Nodes"<<std::endl;
for (auto const& p : model_def.inputs()) {
    std::cout<<"key: "<<p.first<<" value name: "<<p.second.name()<<std::endl;
}

std::cout<<"Model Output Nodes"<<std::endl;
for (auto const& p : model_def.outputs()) {
    std::cout<<"key: "<<p.first<<" value name: "<<p.second.name()<<std::endl;
}

inputs_name = model_def.inputs().begin()->second.name();
outputs_name = model_def.outputs().begin()->second.name();
  
}
/////////////////////////////
/////////////////////////////
std::vector<float> PtAssignment_TF_NN::getPts(AlgoMuons::value_type& algoMuon,
                                    std::vector<std::unique_ptr<IOMTFEmulationObserver> >& observers){

  std::vector<float> pts{0};
  auto& stubResults = algoMuon->getStubResultsConstr();
  auto& gpResult = algoMuon->getGpResultConstr();
  int nInputs = 2*stubResults.size() +1;
  if(!nInputs) return pts;

  unsigned int refLayerLogicNum = omtfConfig->getRefToLogicNumber()[algoMuon->getRefLayer()];
  int phiRefHit = gpResult.getStubResults()[refLayerLogicNum].getMuonStub()->phiHw;

  double naiveBayesPt = omtfConfig->hwPtToGev(algoMuon->getPtConstr());

  tensorflow::Tensor inputs(tensorflow::DT_FLOAT, tensorflow::TensorShape({1, nInputs}));  
  std::vector<tensorflow::Tensor> outputs; 
  const auto& get = [&](int var_index) -> float& { return inputs.matrix<float>()(0, var_index); };

  for(unsigned int iLogicLayer=0; iLogicLayer < stubResults.size(); ++iLogicLayer) {
    auto& stubResult = gpResult.getStubResults()[iLogicLayer];

    //std::cout<<stubResults[iLogicLayer].getPdfBin()<<std::endl;

    int hitPhi = 5000;
    if (stubResult.getMuonStub()){
      hitPhi = stubResult.getMuonStub()->phiHw - phiRefHit;
     if (omtfConfig->isBendingLayer(iLogicLayer)) {
        hitPhi = stubResult.getMuonStub()->phiBHw;
      }
    }

    get(iLogicLayer) = hitPhi*(hitPhi<5000);
    get(iLogicLayer+stubResults.size()) = 16*(hitPhi>=5000);
  }
  //get(2*stubResults.size()) = algoMuon->getRefLayer();
  get(2*stubResults.size()) = naiveBayesPt;

tensorflow::Status status = model.session->Run( {{inputs_name, inputs}},		     
		                                            {outputs_name}, {}, &outputs);

if (!status.ok()) {
    std::cerr << "Inference failed: " << status;
    return pts;
}  

  double pt = outputs.at(0).matrix<float>()(0,0);
  double calibratedHwPt = omtfConfig->ptGevToHw(pt);
  algoMuon->setPtNNConstr(calibratedHwPt);
  algoMuon->setChargeNNConstr(algoMuon->getChargeNNConstr());
  
  return pts;
}
////////////
