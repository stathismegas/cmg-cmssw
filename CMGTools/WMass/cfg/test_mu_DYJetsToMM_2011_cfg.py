import copy
import os 
import CMGTools.RootTools.fwlite.Config as cfg
from CMGTools.RootTools.fwlite.Config import printComps
from CMGTools.WMass.triggerMap import pathsAndFilters
import FWCore.ParameterSet.Config as cms
import glob
jsonAna = cfg.Analyzer(
    'JSONAnalyzer',
    )

triggerAna = cfg.Analyzer(
    'TriggerAnalyzer',
     keepFailingEvents = True
    )

vertexAna = cfg.Analyzer(
    'VertexAnalyzer',
    fixedWeight = 1.,
    keepFailingEvents = True
    )

WAna = cfg.Analyzer(
    'WAnalyzer',
    recoilcut = 1000,
    pfmetcut = 0,
    jetptcut = 1000,
    pt = 30,
    eta = 2.1,
    iso = 0.5,
    savegenp = True,
    verbose = True,
    triggerMap = pathsAndFilters
    )

WtreeProducer = cfg.Analyzer(
    'WTreeProducer'
    )

ZAna = cfg.Analyzer(
    'ZAnalyzer',
    recoilcut = 1000,
    pfmetcut = 0,
    jetptcut = 1000,
    pt = 30,
    eta = 2.1,
    iso = 0.5,
    savegenp = True,
    verbose = True,
    triggerMap = pathsAndFilters
    )

ZtreeProducer = cfg.Analyzer(
    'ZTreeProducer'
    )

genAna = cfg.Analyzer(
    'GenParticleAnalyzerFSR',
    src = 'genParticlesPruned'
    )


sequence = cfg.Sequence( [

    genAna,
    jsonAna,
    triggerAna,
    vertexAna,
    WAna,
   WtreeProducer,
    ZAna,
    ZtreeProducer
   ] )

from CMGTools.H2TauTau.proto.samples.ewk import DYJets
from CMGTools.H2TauTau.proto.samples.getFiles import getFiles

DYJets.files = getFiles('/DYToMuMu_M-20_CT10_TuneZ2_7TeV-powheg-pythia/Fall11-PU_S6_START44_V9B-v1/AODSIM/V5_B/PAT_CMG_V5_6_0_B_44X', 'cmgtools', '.*.root')
#DYJets.files = getFiles('/DYJetsToLL_TuneZ2_M-50_7TeV-madgraph-tauola/Fall11-PU_S6_START44_V9B-v1/AODSIM/V5_B/PAT_CMG_V5_6_0_B', 'cmgtools', '.*.root')
#/eos/cms/store/cmst3/user/cmgtools/CMG/DYToTauTau_M-20_CT10_TuneZ2_7TeV-powheg-pythia-tauola/Fall11-PU_S6_START44_V9B-v1/AODSIM/V5_B/V5_6_0_B_44X
#DYJets.files = getFiles('/DYToTauTau_M-20_CT10_TuneZ2_7TeV-powheg-pythia-tauola/Fall11-PU_S6_START44_V9B-v1/AODSIM/V5_B/PAT_CMG_V5_6_0_B_44X', 'cmgtools', '.*root')
#DYJets.files = getFiles('/DYToTauTau_M-20_CT10_TuneZ2_7TeV-powheg-pythia-tauola/Fall11-PU_S6_START44_V9B-v1/AODSIM/V5_B/PAT_CMG_V5_6_0_B_44X_2nd', 'cmgtools', '.*root')
#DYJets.files = getFiles('/DYJetsToLL_TuneZ2_M-50_7TeV-madgraph-tauola/Fall11-PU_S6_START44_V9B-v1/AODSIM/V5_B/PAT_CMG_V5_6_0_B', 'cmgtools', '.*root')
DYJets.triggers = ["HLT_IsoMu24_v1","HLT_IsoMu24_v2","HLT_IsoMu24_v3","HLT_IsoMu24_v4","HLT_IsoMu24_v5","HLT_IsoMu24_v6","HLT_IsoMu24_v7",\
                   "HLT_IsoMu24_v8","HLT_IsoMu24_v9","HLT_IsoMu24_v10","HLT_IsoMu24_v11","HLT_IsoMu24_v12","HLT_IsoMu24_v13","HLT_IsoMu24_v14",\
                   "HLT_IsoMu24_eta2p1_v1","HLT_IsoMu24_eta2p1_v2","HLT_IsoMu24_eta2p1_v3","HLT_IsoMu24_eta2p1_v4","HLT_IsoMu24_eta2p1_v5",\
                   "HLT_IsoMu24_eta2p1_v6","HLT_IsoMu24_eta2p1_v7","HLT_IsoMu24_eta2p1_v8"
                   ]

selectedComponents = [DYJets]

DYJets.splitFactor = 1000

config = cfg.Config( components = selectedComponents,
                     sequence = sequence )
                     
printComps(config.components, True)


