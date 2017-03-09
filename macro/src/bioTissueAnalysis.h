#ifndef BIO_TISSUE_ANALYSIS_H_
#define BIO_TISSUE_ANALYSIS_H_
#include "bioNonlinearTissue.h"
#include <amsiNonlinearAnalysis.h>
namespace bio
{
  template <typename O>
    void buildConvergenceOperators(pACase ss, amsi::Iteration * it, amsi::LAS * las, apf::Field * u, O out);
  class TissueIteration : public amsi::Iteration
  {
  protected:
    NonlinearTissue * tssu;
    amsi::LAS * las;
  public:
    TissueIteration(NonlinearTissue * t, amsi::LAS * l)
      : amsi::Iteration()
      , tssu(t)
      , las(l)
    {}
    virtual void iterate()
    {
      amsi::Iteration::iterate();
      LinearSolver(tssu,las);
      las->iter();
    }
  };
  class TissueAnalysis
  {
  public:
    TissueAnalysis(pGModel, pParMesh, pACase, MPI_Comm);
    ~TissueAnalysis();
    virtual void run();
  protected:
    virtual void init();
    virtual void checkpoint();
    virtual void revert();
    virtual void deinit();
  protected:
    // util
    MPI_Comm cm;
    // simmetrix
    pGModel mdl;
    pParMesh msh;
    pACase cs;
    // analysis
    double t;
    double dt;
    int stp;
    int mx_stp;
    NonlinearTissue * tssu;
    amsi::Iteration * itr;
    amsi::Convergence * cvg;
    std::vector<amsi::Convergence*> cvg_stps;
    amsi::LAS * las;
    bool completed;
    // log filenames
    std::string state_fn;
    std::string constraint_fn;
    std::string frcs_fn;
    std::string nrms_fn;
    std::string dsps_fn;
    std::string vols_fn;
    // logs
    amsi::Log state;
    amsi::Log constraints;
    amsi::Log frcs;
    amsi::Log nrms;
    amsi::Log dsps;
    amsi::Log vols;
    // track model ents
    std::vector<apf::ModelEntity*> frc_itms;
    std::vector<apf::ModelEntity*> dsp_itms;
    std::vector<apf::ModelEntity*> vol_itms;
  };
}
#include "bioTissueAnalysis_impl.h"
#endif
