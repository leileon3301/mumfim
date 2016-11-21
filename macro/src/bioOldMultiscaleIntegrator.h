#ifndef BIO_MULTISCALE_INTEGRATOR_H_
#define BIO_MULTISCALE_INTEGRATOR_H_
#include "bioNonlinearTissue.h"
#include <apf.h>
#include <apfDynamicMatrix.h>
#include <apfNumbering.h>
#include <apfShape.h>
#include <apfSIM.h>
namespace bio
{
  class MultiscaleIntegrator : public apf::Integrator
  {
  public:
  MultiscaleIntegrator(Biotissue::NonLinTissue * nlt,
                       apf::Mesh * mesh,
                       apf::Field * field,
                       int o) :
    apf::Integrator(o),
      analysis(nlt),
      m(mesh),
      Ke(NULL),
      fe(NULL),
      f(field)
      {
        fs = apf::getShape(f);
      }
    void inElement(apf::MeshElement * ME)
    {
      me = ME;
      e = apf::createElement(f,me);
      es = fs->getEntityShape(m->getType(apf::getMeshEntity(me)));
      num_element_nodes = apf::countNodes(e);
      num_field_components = apf::countComponents(f);
      int new_num_element_eqs = num_element_nodes * num_field_components;
      bool reallocate = new_num_element_eqs != num_element_eqs;
      num_element_eqs = new_num_element_eqs;
      current_integration_point = 0;
      if(reallocate)
      {
        delete [] Ke;
        delete [] fe;
        Ke = new double[num_element_eqs * num_element_eqs];
        fe = new double[num_element_eqs];
      }
      memset(Ke,0,num_element_eqs*num_element_eqs*sizeof(double));
      memset(fe,0,num_element_eqs*sizeof(double));
    }
    void outElement(apf::MeshElement * e)
    { }
    // todo: pull initialization of things that don't change between integration pts into the inElement function
    void atPoint(apf::Vector3 const &p, double w, double dV)
    {
      // Retrieve the material law info from microscale corresponding to this integration point
      RVE_Info * rve_info = analysis->Analysis_RetrieveMatLawFromRVE(apf::getMeshEntity(me),current_integration_point);
      // Note: Macro and micro solvers index stress tensor differently
      //
      //   micro    ----->    macro
      //
      // [ 0 1 2 ]          [ 0 3 5 ]
      // [   3 4 ]  ----->  [   1 4 ]
      // [     5 ]          [     2 ]
      //
      apf::Matrix3x3 Jac;
      apf::getJacobian(me,p,Jac);
      double wxdetjac = w * apf::getJacobianDeterminant(Jac,3);
      int & nen = num_element_nodes; // = 4 (tets)
      int & nedof = num_element_eqs; // = 12 (tets)
      int offset = 9;
      double *stress_deriv[6];
      stress_deriv[0] = &(rve_info->derivS[offset]);
      stress_deriv[1] = &(rve_info->derivS[offset + 3*nedof]);
      stress_deriv[2] = &(rve_info->derivS[offset + 5*nedof]);
      stress_deriv[3] = &(rve_info->derivS[offset +   nedof]);
      stress_deriv[4] = &(rve_info->derivS[offset + 4*nedof]);
      stress_deriv[5] = &(rve_info->derivS[offset + 2*nedof]);
      apf::NewArray<apf::Vector3> grads;
      apf::getShapeGrads(e,p,grads);
      // todo: change back to standard layout
      apf::DynamicMatrix BL(6,nedof); // linear strain disp
      for(int ii = 0; ii < nen; ii++)
      {
        BL(0,3*ii) = grads[ii][0]; // N_(ii,1)
        BL(0,3*ii+1) = BL(0,3*ii+2) = 0.0;
        BL(1,3*ii+1) = grads[ii][1]; // N_(ii,2)
        BL(1,3*ii) = BL(1,3*ii+2) = 0.0;
        BL(2,3*ii+2) = grads[ii][2]; // N_(ii,3)
        BL(2,3*ii) = BL(2,3*ii+1) = 0.0;
        BL(3,3*ii) = grads[ii][1]; // N_(ii,2)
        BL(3,3*ii+1) = grads[ii][0]; // N_(ii,1)
        BL(3,3*ii+2) = 0.0;
        BL(4,3*ii) = 0.0;
        BL(4,3*ii+1) = grads[ii][2]; // N_(ii,3)
        BL(4,3*ii+2) = grads[ii][1]; // N_(ii,2)
        BL(5,3*ii) = grads[ii][2]; // N_(ii,3)
        BL(5,3*ii+1) = 0.0;
        BL(5,3*ii+2) = grads[ii][0];  // N_(ii,1)
      }
      apf::DynamicMatrix K0(nedof,nedof);
      // K0 = BL^T * stress_deriv
      for(int ii = 0; ii < nedof; ii++)
        for(int jj = 0; jj < nedof; jj++)
        {
          K0(ii,jj) = 0.0;
          for(int kk = 0; kk < 6; kk++)
            K0(ii,jj) += BL(kk,ii) * stress_deriv[kk][jj];
        }
      apf::DynamicMatrix BNL(9,nedof); //nonlinear strain disp
      double Bp[3][10];
      for(int ii = 0; ii < 9; ii++)
        for(int jj = 0; jj < nedof; jj++)
          BNL(ii,jj) = 0.0;
      for(int ii = 0; ii < 3; ii++)
      {
        for(int jj = 0; jj < 10; jj++)
          Bp[ii][jj] = 0.0;
        for(int jj = 0; jj < 4; jj++)
          Bp[ii][jj*3] = grads[jj][ii];
      }
      for(int ii = 0; ii < 3; ii++)
        for(int jj = 0; jj < 10; jj++)
        {
          BNL(ii,jj) = Bp[ii][jj];
          BNL(3+ii,jj+1) = Bp[ii][jj];
          BNL(6+ii,jj+2) = Bp[ii][jj];
        }
      // Fill S matrix - in block notation contains stress tensor along diagonal, zero elsewhere
      double S[9][9];
      for(int ii = 0; ii < 9; ii++)
        for(int jj = 0; jj < 9; jj++)
          S[ii][jj] = 0.0;
      double SV[6];
      SV[0] = rve_info->derivS[0];
      SV[1] = rve_info->derivS[3];
      SV[2] = rve_info->derivS[5];
      SV[3] = rve_info->derivS[1];
      SV[4] = rve_info->derivS[4];
      SV[5] = rve_info->derivS[2];
      S[0][0] = S[0+3][0+3] = S[0+6][0+6] = rve_info->derivS[0];
      S[0][1] = S[0+3][1+3] = S[0+6][1+6] = rve_info->derivS[1];
      S[0][2] = S[0+3][2+3] = S[0+6][2+6] = rve_info->derivS[2];
      S[1][0] = S[1+3][0+3] = S[1+6][0+6] = rve_info->derivS[1];
      S[1][1] = S[1+3][1+3] = S[1+6][1+6] = rve_info->derivS[3];
      S[1][2] = S[1+3][2+3] = S[1+6][2+6] = rve_info->derivS[4];
      S[2][0] = S[2+3][0+3] = S[2+6][0+6] = rve_info->derivS[2];
      S[2][1] = S[2+3][1+3] = S[2+6][1+6] = rve_info->derivS[4];
      S[2][2] = S[2+3][2+3] = S[2+6][2+6] = rve_info->derivS[5];
      apf::DynamicMatrix BNLTxS(nedof,9);
      // BNLTxS = BNL^T * S
      for(int ii = 0; ii < nedof; ii++)
        for(int jj = 0; jj < 9; jj++)
        {
          BNLTxS(ii,jj) = 0.0;
          for(int kk = 0; kk < 9; kk++)
            BNLTxS(ii,jj) += BNL(kk,ii) * S[kk][jj];
        }
      // for force vector calculation
      apf::DynamicMatrix BLTxSV(nedof,1);
      for(int ii = 0; ii < nedof; ii++)
      {
        BLTxSV(ii,0) = 0.0;
        for(int jj = 0; jj < 6; jj++)
        {
          BLTxSV(ii,0) += BL(jj,ii) * SV[jj];
        }
      }
      // retrieve virtual strain/stress for force vector calc
      double Q[3];
      Q[0] = rve_info->derivS[6];
      Q[1] = rve_info->derivS[7];
      Q[2] = rve_info->derivS[8];
      apf::DynamicMatrix N(num_field_components,nedof);
      for(int ii = 0; ii < num_field_components; ii++)
        for(int jj = 0; jj < nedof; jj++)
          N(ii,jj) = 0.0;
      for(int ii = 0; ii < num_field_components; ii++)
        for(int jj = 0; jj < nen; jj++)
          N(ii,ii+(jj*num_field_components)) = p[ii]; // assumption for linear tet... should be all shape function values...
      apf::DynamicMatrix NTxQ(nedof,1);
      for(int ii = 0; ii < nedof; ii++)
      {
        NTxQ(ii,0) = 0;
        for(int jj = 0; jj < num_field_components; jj++)
          NTxQ(ii,0) += N(jj,ii) * Q[jj];
      }
      apf::DynamicMatrix K1;
      apf::multiply(BNLTxS,BNL,K1);
      for(int ii = 0; ii < nedof; ii++)
      {
        fe[ii] += wxdetjac * (NTxQ(ii,0) - BLTxSV(ii,0)); // P - F
        for(int jj = 0; jj < nedof; jj++)
          Ke[ii * nedof + jj] += wxdetjac * (K0(ii,jj) + K1(ii,jj));
      }
      current_integration_point++;
    }
    void parallelReduce()
    {
    }
    double * Ke;
    double * fe;
    int current_integration_point;
    int num_element_eqs;
    int num_field_components;
    int num_element_nodes;
    apf::Element * e;
  private:
    apf::Field * f;
    apf::FieldShape * fs;
    apf::EntityShape * es;
    apf::MeshElement * me;
    apf::Mesh * m;
    Biotissue::NonLinTissue * analysis;
  };
}
#endif
