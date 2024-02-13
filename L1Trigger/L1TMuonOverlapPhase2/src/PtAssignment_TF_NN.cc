/*
 * PtAssignment_TF_NN.cc
 *
 *  Created on: Thu Jun 10 10:56:42 CEST 2021
 *      Author: akalinow
 */

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
  auto& aStubs = algoMuon->getStubResultsConstr();
  int nInputs = 2*aStubs.size() +1;
  if(!nInputs) return pts;

  tensorflow::Tensor inputs(tensorflow::DT_FLOAT, tensorflow::TensorShape({1, nInputs}));  
  std::vector<tensorflow::Tensor> outputs; 
  const auto& get = [&](int var_index) -> float& { return inputs.matrix<float>()(0, var_index); };

  int pdfMiddle = 1<<(this->omtfConfig->nPdfAddrBits()-1);
  int input = 0;
  int nHits = 0;
  for(unsigned int iLogicLayer=0; iLogicLayer < aStubs.size(); ++iLogicLayer) {
    input = aStubs[iLogicLayer].getPdfBin() - pdfMiddle;
    get(iLogicLayer) = input*(input<5000);
    get(iLogicLayer+aStubs.size()) = 16*(input>0);
    nHits += input<5000;
  }
  get(2*aStubs.size()) = nHits;     
     
tensorflow::Status status = model.session->Run( {{inputs_name, inputs}},		     
		                                            {outputs_name}, {}, &outputs);

if (!status.ok()) {
    std::cerr << "Inference failed: " << status;
    return pts;
}  

  double pt = outputs.at(0).matrix<float>()(0,0);
  pts.at(0) = 2.0*pt+1;

  algoMuon->setPtNNConstr(pts.at(0));
  return pts;
}
////////////
