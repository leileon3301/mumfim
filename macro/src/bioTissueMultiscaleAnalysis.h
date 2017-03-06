#ifndef BIO_TISSUEMULTISCALEANALYSIS_H_
#define BIO_TISSUEMULTISCALEANALYSIS_H_
#include "bioLinearTissue.h"
#include "bioMultiscaleTissue.h"
#include <amsiMultiscale.h>
#include <amsiAnalysis.h>
#include <apfsimWrapper.h>
#include <apf.h>
#include <MeshSim.h>
#include <amsiPETScLAS.h>
#include <string>
#include <stdexcept>
namespace bio
{
  class RelativeFieldNorm : public amsi::Convergence
  {
  protected:
    apf::Field * dlta;
    apf::Field * accm;
    apf::Numbering * fxd;
    double eps;
    double dlta_nrm;
    double accm_nrm;
    amsi::Log nrms;
  public:
    RelativeFieldNorm(apf::Field * d, apf::Field * a, apf::Numbering * f, double e)
      : amsi::Convergence()
      , dlta(d)
      , accm(a)
      , fxd(f)
      , eps(e)
      , dlta_nrm(0.0)
      , accm_nrm(0.0)
      , nrms(amsi::activateLog("norm_history"))
    { }
    bool converged()
    {
      amsi::AccumOp acm;
      amsi::FreeApplyOp free_acm(fxd,&acm);
      amsi::SquareApplyOp sm_sqrd(&free_acm);
      amsi::ExtractOp dlta_nrm_op(dlta,&sm_sqrd);
      amsi::ExtractOp accm_nrm_op(accm,&sm_sqrd);
      dlta_nrm_op.run();
      accm_nrm_op.run();
      dlta_nrm = sqrt(amsi::comm_sum(dlta_nrm_op.getExtractedValue()));
      accm_nrm = sqrt(amsi::comm_sum(accm_nrm_op.getExtractedValue()));
      bool cnvrgd =  ( dlta_nrm / accm_nrm ) < eps;
      std::cout << "relative field norm convergence criteria: " << std::endl
                << "\t" << dlta_nrm << " / " << accm_nrm << " < " << eps << std::endl
                << "\t" << dlta_nrm / accm_nrm << " < " << eps << std::endl
                << "\t" << (cnvrgd ? "TRUE" : "FALSE") << std::endl;
      return cnvrgd;
    }
    bool failed()
    {
      return false;
    }
    void log(int ldstp, int iteration, int rnk)
    {
      if (rnk == 0)
        amsi::log(nrms) << ldstp << ", "
                        << iteration << ", "
                        << dlta_nrm/accm_nrm << std::endl;
    }
    double getdlta_nrm(){return dlta_nrm;}
    double getaccm_nrm(){return accm_nrm;}
  };
  class LASResidualConvergence : public amsi::Convergence
  {
  protected:
    amsi::LAS * las;
    double eps;
    double nrm;
    double accm_nrm;
    amsi::Log nrms;
  public:
    LASResidualConvergence(amsi::LAS * l, double e)
      : las(l)
      , eps(e)
      , nrm(0.0)
      , accm_nrm(0.0)
      , nrms(amsi::activateLog("norm_history"))
    { }
    bool converged()
    {
      las->GetVectorNorm(nrm);
      las->GetAccumVectorNorm(accm_nrm);
      bool cnvrgd = (nrm / accm_nrm) < eps;
      std::cout << "relative residual convergence criteria: " << std::endl
                << "\t" << nrm << " / " << accm_nrm << " < " << eps << std::endl
                << "\t" << nrm / accm_nrm << " < " << eps << std::endl
                << "\t" << (cnvrgd ? "TRUE" : "FALSE") << std::endl;
//      amsi::log(nrms) << amsi::post(nrms) << " " << nrm / accm_nrm << " ";
      return cnvrgd;
    }
    bool failed()
    {
      return false;
    }
    void log(int ldstp, int iteration, int rnk)
    {
      if (rnk == 0)
        amsi::log(nrms) << ldstp << ", "
                        << iteration << ", "
                        << nrm/accm_nrm << std::endl;
    }
  };
  /*
  class LASSubvectorConvergence : public amsi::RelativeResidualConvergence
  {
  protected:
    int ignr_cnt;
    int * ignr;
  public:
    LASSubvectorConvergence(amsi::LAS * l, double e, int ic, int * i)
      : RelativeResidualConvergence(l,e)
      , ignr_cnt(ic)
      , ignr(new int[ignr_cnt])
    {
      memcpy(&ignr[0],&i[0],ignr_cnt*sizeof(int));
    }
    bool converged()
    {
      double nrm = 0.0;
      double accm_nrm = 0.0;
      double * r_i = NULL;
      double * r = NULL;
      las->GetVector(r_i);
      las->GetAccumVector(r);
      int lcl = las->LocalDOFs();
      int ffst = las->LocalOffset();
      for(int ii = 0; ii < lcl; ii++)
      {
        nrm += r_i[ii];
        accm_nrm += r[ii];
      }
      for(int ii = 0; ii < ignr_cnt; ii++)
      {
        nrm -= r_i[ignr[ii]-ffst];
        accm_nrm -= r[ignr[ii]-ffst];
      }
      nrm = sqrt(amsi::comm_sum(nrm));
      accm_nrm = sqrt(amsi::comm_sum(accm_nrm));
      bool cnvrgd = (nrm / accm_nrm) < eps;
      std::cout << "Relative residual convergence criteria: " << std::endl
                << "\t" << nrm << " / " << accm_nrm << " < " << eps << std::endl
                << "\t" << nrm / accm_nrm << " < " << eps << std::endl
                << "\t" << (cnvrgd ? "TRUE" : "FALSE") << std::endl;
      return cnvrgd;
    }
  };
  */
  class TissueMultiScaleAnalysis
  {
  public:
    TissueMultiScaleAnalysis(pGModel imdl, pParMesh imsh, pACase pd, MPI_Comm cm);
    void initLogs();
    void deleteLogs();
    void updateTime()
    {
      t = ((double)current_step+1.0)/num_load_steps;
    }
    int run();
  private:
    int rnk;
    unsigned num_load_steps;
    unsigned current_step;
    unsigned iteration;
    double t; // [0,1] 0-initial configuration, 1-final configuration
    double initial_volume;
    std::vector<pModelItem> frc_itms;
    std::vector<apf::ModelEntity*> dsp_itms;
    std::vector<apf::ModelEntity*> vol_itms;
    amsi::Log state;
    amsi::Log cnstrnts;
    amsi::Log norms;
    amsi::Log disps;
    amsi::Log loads;
    amsi::Log vols;
    std::string state_file;
    std::string cnstrnts_file;
    std::string norms_file;
    std::string disps_file;
    std::string loads_file;
    std::string vols_file;
    pGModel model;
    pParMesh mesh;
    pMesh part;
    std::vector<amsi::Convergence*> cnvrg;
    MultiscaleTissue * tissue; // actual multiscale iterations
    amsi::LAS * las; // Linear solver used for both analyses
  };
  struct MultiscaleTissueAnalysis
  {
    amsi::LAS * linear_solver;
    MultiscaleTissue * tissue_fea;
  };
  class MultiscaleTissueIteration : public amsi::Iteration
  {
  protected:
    MultiscaleTissueAnalysis * analysis;
    int iter;
  public:
    MultiscaleTissueIteration(MultiscaleTissueAnalysis * a)
      : amsi::Iteration()
      , analysis(a)
      , iter(0)
    { }
    void iterate();
    int getIteration() {return iter;}
  };
  class MultiscaleTissueConvergence : public amsi::Convergence
  {
  protected:
    MultiscaleTissueIteration * iter;
    MultiscaleTissueAnalysis * analysis;
    double eps;
  public:
    MultiscaleTissueConvergence(MultiscaleTissueAnalysis * a,
                                MultiscaleTissueIteration * i,
                                double e = 1e-8)
      : amsi::Convergence()
      , iter(i)
      , analysis(a)
      , eps(e)
    { }
    bool converged();
    double & epsilon() {return eps;}
  };
} // end of namespace Biotissue
#endif
