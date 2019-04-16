#ifndef FILE_HCURLDIVFE
#define FILE_HCURLDIVFE

/*********************************************************************/
/* File:   hcurldivfe.hpp                                            */
/* Author: Philip Lederer                                            */
/* Date:   2017/2018                                                 */
/*********************************************************************/

//include recursive_pol.hpp;
#include "recursive_pol_tet.hpp"
#include "hcurldivfe_impl.hpp"
//#include "l2orth3d.hpp"

namespace ngfem
{

  template <int DIM>
  class HCurlDivFiniteElement : public FiniteElement
  {
  public:
    using FiniteElement::FiniteElement;
    using FiniteElement::ndof;
    using FiniteElement::order;


    // old style
    virtual void CalcShape (const IntegrationPoint & ip, 
                            BareSliceMatrix<double> shape) const = 0;

    virtual void CalcDivShape (const IntegrationPoint & ip, 
                               BareSliceMatrix<double> divshape) const = 0;

    virtual void CalcCurlShape (const IntegrationPoint & ip, 
                               BareSliceMatrix<double> divshape) const = 0;
    
    virtual void CalcMappedShape (const MappedIntegrationPoint<DIM,DIM> & mip,
      BareSliceMatrix<double> shape) const = 0;    

    virtual void CalcMappedDivShape (const MappedIntegrationPoint<DIM,DIM> & mip,
      BareSliceMatrix<double> shape) const = 0;

    virtual void CalcMappedCurlShape (const MappedIntegrationPoint<DIM,DIM> & mip,
      BareSliceMatrix<double> shape) const = 0;

    virtual void CalcMappedShape (const SIMD<MappedIntegrationPoint<DIM,DIM>> & mip,
                                         BareSliceMatrix<SIMD<double>> shapes) const = 0;

    virtual void CalcMappedShape (const SIMD_BaseMappedIntegrationRule & ir,
				      BareSliceMatrix<SIMD<double>> shapes) const = 0;
    
    virtual void Evaluate (const SIMD_BaseMappedIntegrationRule & ir,
                                  BareSliceVector<> coefs,
                                  BareSliceMatrix<SIMD<double>> values) const = 0;

    virtual void AddTrans (const SIMD_BaseMappedIntegrationRule & ir,
                                  BareSliceMatrix<SIMD<double>> values,
                                  BareSliceVector<> coefs) const = 0;   
  };
  

  
  template <ELEMENT_TYPE ET> class HCurlDivFE;

  
  template <ELEMENT_TYPE ET>
  class T_HCurlDivFE : public HCurlDivFiniteElement<ET_trait<ET>::DIM>,
    public VertexOrientedFE<ET>
  {
  protected:
    enum { DIM = ET_trait<ET>::DIM };
    enum { DIM_STRESS = DIM*DIM };
    
    using VertexOrientedFE<ET>::vnums;
    using HCurlDivFiniteElement<ET_trait<ET>::DIM>::ndof;
    using HCurlDivFiniteElement<ET_trait<ET>::DIM>::order;

    int order_facet[ET_trait<ET>::N_FACET];
    int order_inner;
    int order_trace;
    //bool curlbubbles;
    bool GGbubbles;

    //L2orthTet l2orth;
    
  public:          
    T_HCurlDivFE (int aorder, bool aGGbubbles = false)
    {
      order = aorder;
      for (auto & of : order_facet) of = aorder;
      order_inner = aorder;
      order_trace = -1;
      //curlbubbles = acurlbubbles;
      GGbubbles = aGGbubbles;
    }   
    
    using VertexOrientedFE<ET>::SetVertexNumbers;

    /*template <typename TA>
    void SetVertexNumbers(const TA & avnums)
    {
      //FlatArray<int> aavnums;
      for (int i=0; i<avnums.Size(); i++)
	{
	  //aavnums[i] = avnums[i];
	  vnums[i] = avnums[i];
	}

      l2orth.SetVertexNumbers(avnums);
      }*/
    
    
    virtual ELEMENT_TYPE ElementType() const { return ET; }
    const HCurlDivFE<ET> * Cast() const { return static_cast<const HCurlDivFE<ET>*> (this); } 
    
    INLINE void SetOrderFacet (int nr, int order) { order_facet[nr] = order; }
    INLINE void SetOrderInner (int order) { order_inner = order; }
    INLINE void SetOrderTrace (int order) { order_trace = order; }

    virtual void ComputeNDof()
    {
      cout << "Error, T_HCurlDivFE<ET>:: ComputeNDof not available, only for ET == TRIG" << endl;
    }

    virtual void CalcShape (const IntegrationPoint & ip, 
                            BareSliceMatrix<double> shape) const
    {
      Vec<DIM, AutoDiffDiff<DIM>> adp;
      for ( int i=0; i<DIM; i++)
      {
        adp(i) = AutoDiffDiff<DIM>(ip(i),i);
      }

      Cast() -> T_CalcShape (TIP<DIM, AutoDiffDiff<DIM>> (adp), SBLambda([&] (int nr, auto val)
                                          {
                                            shape.Row(nr).AddSize(DIM_STRESS) = val.Shape();
                                          }));
    }

    virtual void CalcDivShape (const IntegrationPoint & ip,
                               BareSliceMatrix<double> shape) const
    {
      Vec<DIM, AutoDiffDiff<DIM>> adp;
      for ( int i=0; i<DIM; i++)
      {
        adp[i] = AutoDiffDiff<DIM>(ip(i),i);
      }
      
      Cast() -> T_CalcShape (TIP<DIM, AutoDiffDiff<DIM>> (adp), SBLambda([&] (int nr, auto val)
                                          {
                                            shape.Row(nr).AddSize(DIM) = val.DivShape();
                                          }));
    }

    virtual void CalcCurlShape (const IntegrationPoint & ip,
                               BareSliceMatrix<double> shape) const
    {
      Vec<DIM, AutoDiffDiff<DIM>> adp;
      for ( int i=0; i<DIM; i++)
      {
        adp[i] = AutoDiffDiff<DIM>(ip(i),i);
      }
      
      Cast() -> T_CalcShape (TIP<DIM, AutoDiffDiff<DIM>> (adp), SBLambda([&] (int nr, auto val)
                                          {
                                            shape.Row(nr).AddSize(DIM) = val.CurlShape();
                                          }));
    }
    


    virtual void CalcMappedShape (const MappedIntegrationPoint<DIM,DIM> & mip,
                            BareSliceMatrix<double> shape) const
    {
      Vec<DIM, AutoDiff<DIM>> adp = mip;
      Vec<DIM, AutoDiffDiff<DIM>> addp;
      for (int i=0; i<DIM; i++)
      {
        addp[i] = adp[i].Value();
        addp[i].LoadGradient(&adp[i].DValue(0));
      }
      Cast() -> T_CalcShape (TIP<DIM,AutoDiffDiff<DIM>> (addp),SBLambda([&](int nr,auto val)
      {
	shape.Row(nr).AddSize(DIM_STRESS) = val.Shape();
      }));
    }

    virtual void CalcMappedShape (const SIMD_BaseMappedIntegrationRule & bmir, 
                                         BareSliceMatrix<SIMD<double>> shapes) const override
    {
      auto & mir = static_cast<const SIMD_MappedIntegrationRule<DIM,DIM>&> (bmir);
      for (size_t i = 0; i < mir.Size(); i++)
        {
          Vec<DIM,AutoDiff<DIM,SIMD<double>>> adp = mir[i];
          TIP<DIM,AutoDiffDiff<DIM,SIMD<double>>> addp(adp);
          
          this->Cast() -> T_CalcShape (addp,
                                       SBLambda ([i,shapes] (size_t j, auto val) 
                                                 {

                                                   Vec<DIM*DIM,SIMD<double>> vecshape = val.Shape();
                                                   for (size_t k = 0; k < DIM*DIM; k++)
                                                     shapes(j*(DIM*DIM)+k,i) = vecshape(k);
                                                 }));
        }
    }

     virtual void CalcMappedShape (const SIMD<MappedIntegrationPoint<DIM,DIM>> & mip, 
                                         BareSliceMatrix<SIMD<double>> shapes) const override
    {
      Vec<DIM,AutoDiff<DIM,SIMD<double>>> adp = mip;
      TIP<DIM,AutoDiffDiff<DIM,SIMD<double>>> addp(adp);
          
      this->Cast() -> T_CalcShape (addp,
				   SBLambda ([shapes] (size_t j, auto val) 
					     {

					       Vec<DIM*DIM,SIMD<double>> vecshape = val.Shape();
					       for (size_t k = 0; k < DIM*DIM; k++)
						 shapes(j*(DIM*DIM)+k,0) = vecshape(k);
					     }));
    }


    virtual void Evaluate (const SIMD_BaseMappedIntegrationRule & bmir,
                                  BareSliceVector<> coefs,
                                  BareSliceMatrix<SIMD<double>> values) const override
    {
      auto & mir = static_cast<const SIMD_MappedIntegrationRule<DIM,DIM>&> (bmir);
      for (size_t i = 0; i < bmir.Size(); i++)
        {
          double *pcoefs = &coefs(0);
          const size_t dist = coefs.Dist();
          
          Vec<DIM_STRESS,SIMD<double>> sum(0.0);
          Vec<DIM,AutoDiff<DIM,SIMD<double>>> adp = mir[i];
          TIP<DIM,AutoDiffDiff<DIM,SIMD<double>>> addp(adp);
          
          Cast() -> T_CalcShape (addp,
                                 SBLambda ([&sum,&pcoefs,dist] (size_t j, auto val)
                                           {
                                             sum += (*pcoefs)*val.Shape();
                                             pcoefs += dist;
                                           }));

	  for (size_t k = 0; k < DIM*DIM; k++)
	    values(k,i) = sum(k);
        }
    }

    virtual void AddTrans (const SIMD_BaseMappedIntegrationRule & bmir,
                                  BareSliceMatrix<SIMD<double>> values,
                                  BareSliceVector<> coefs) const override
    {
       for (size_t i = 0; i < bmir.Size(); i++)
        {
          Mat<DIM,DIM,SIMD<double>> mat;
	  
	  auto & mir = static_cast<const SIMD_MappedIntegrationRule<DIM,DIM>&> (bmir);

	  for (size_t k = 0; k < DIM*DIM; k++)
	    mat(k) = values(k,i);
	  
	  Vec<DIM,AutoDiff<DIM,SIMD<double>>> adp = mir[i];
          TIP<DIM,AutoDiffDiff<DIM,SIMD<double>>> addp(adp);
          double *pcoefs = &coefs(0);
          const size_t dist = coefs.Dist();

          Cast() -> T_CalcShape (addp,
                                 SBLambda ([mat,&pcoefs,dist] (size_t j, auto val)
                                           {                                          
					     Vec<DIM*DIM,SIMD<double>> vecshape = val.Shape();
                                             
                                             SIMD<double> sum = 0.0;
                                             for (size_t k = 0; k < DIM*DIM; k++)
                                               sum += mat(k) * vecshape(k);
                                             
                                             *pcoefs += HSum(sum);
                                             pcoefs += dist;
                                           }));
        }
    }

    virtual void CalcMappedDivShape (const MappedIntegrationPoint<DIM,DIM> & mip,
                            BareSliceMatrix<double> shape) const
    {
      Vec<DIM, AutoDiff<DIM>> adp = mip;
      Vec<DIM, AutoDiffDiff<DIM>> addp;
      for (int i=0; i<DIM; i++)
      {
        addp[i] = adp[i].Value();
        addp[i].LoadGradient(&adp[i].DValue(0));
      }

      if(!mip.GetTransformation().IsCurvedElement()) // non-curved element
      {
        Cast() -> T_CalcShape (TIP<DIM,AutoDiffDiff<DIM>> (addp),SBLambda([&](int nr,auto val)
        {
          shape.Row(nr).AddSize(DIM) = val.DivShape();
        }));
      }
      else // curved element
      {	
        Mat<DIM> jac = mip.GetJacobian();
        Mat<DIM> inv_jac = mip.GetJacobianInverse();        
	Mat<DIM> hesse_FinvT[3], F_HFinvT_Finv[3];
		
	double eps = 1e-4;
		
	Mat<DIM> jacrinv, jaclinv,jacrrinv, jacllinv;
	for (int dir = 0; dir < DIM; dir++)
	  {
	    IntegrationPoint ipr = mip.IP();
	    IntegrationPoint ipl = mip.IP();
	    IntegrationPoint iprr = mip.IP();
	    IntegrationPoint ipll = mip.IP();
	    
	    ipr(dir) += eps;
	    ipl(dir) -= eps;
	    iprr(dir) += 2*eps;
	    ipll(dir) -= 2*eps;
	    
	    MappedIntegrationPoint<DIM,DIM> mipr(ipr, mip.GetTransformation());
	    MappedIntegrationPoint<DIM,DIM> mipl(ipl, mip.GetTransformation());
	    MappedIntegrationPoint<DIM,DIM> miprr(iprr, mip.GetTransformation());
	    MappedIntegrationPoint<DIM,DIM> mipll(ipll, mip.GetTransformation());
	    
	    jacrinv = Trans(mipr.GetJacobianInverse());    
	    jaclinv = Trans(mipl.GetJacobianInverse());

	    jacrrinv = Trans(miprr.GetJacobianInverse());    
	    jacllinv = Trans(mipll.GetJacobianInverse());
	    
	    for (int j = 0; j < DIM; j++)
	      {
		hesse_FinvT[0](j,dir) = (8.0*jacrinv(0,j) - 8.0*jaclinv(0,j) - jacrrinv(0,j) + jacllinv(0,j) ) / (12.0*eps);
		hesse_FinvT[1](j,dir) = (8.0*jacrinv(1,j) - 8.0*jaclinv(1,j) - jacrrinv(1,j) + jacllinv(1,j) ) / (12.0*eps);
		hesse_FinvT[2](j,dir) = (8.0*jacrinv(2,j) - 8.0*jaclinv(2,j) - jacrrinv(2,j) + jacllinv(2,j) ) / (12.0*eps);

	      }
	  }
	
	for(int i=0; i<DIM; i++)
	  F_HFinvT_Finv[i] = jac * hesse_FinvT[i] * inv_jac;

        Cast() -> T_CalcShape (TIP<DIM,AutoDiffDiff<DIM>> (addp),SBLambda([&](int nr,auto val)
                                  {
                                    shape.Row(nr).AddSize(DIM) = val.DivShape();
                                    BareVector<double> divshape = shape.Row(nr);				    
                                    Vec<DIM*DIM> matshape = val.Shape();				    
				    
                                    for(int k=0; k<DIM; k++)
                                    {
                                      for(int j=0; j<DIM*DIM; j++)
                                      {
					divshape(k) += F_HFinvT_Finv[k](j) * matshape(j);
                                      }
                                    }
                                    
                                  }));
	
      }
    }

     virtual void CalcMappedCurlShape (const MappedIntegrationPoint<DIM,DIM> & mip,
                            BareSliceMatrix<double> shape) const
    {
      Vec<DIM, AutoDiff<DIM>> adp = mip;
      Vec<DIM, AutoDiffDiff<DIM>> addp;
      for (int i=0; i<DIM; i++)
      {
        addp[i] = adp[i].Value();
        addp[i].LoadGradient(&adp[i].DValue(0));
      }

      if(!mip.GetTransformation().IsCurvedElement()) // non-curved element
      {
        Cast() -> T_CalcShape (TIP<DIM,AutoDiffDiff<DIM>> (addp),SBLambda([&](int nr,auto val)
        {
          shape.Row(nr).AddSize(DIM) = val.CurlShape();
        }));	
      }
      else
	{	
        Mat<DIM> jac = mip.GetJacobian();
        Mat<DIM> inv_jac = mip.GetJacobianInverse();        
	Mat<DIM> curl_FT[2], F_curlFT_Finv[2];
	Vec<DIM> curl_Jinv;
		
	double eps = 1e-6;
	
	Mat<DIM> jacr, jacl;
	for (int dir = 0; dir < DIM; dir++)
	  {
	    IntegrationPoint ipr = mip.IP();
	    IntegrationPoint ipl = mip.IP();
    
	    ipr(dir) += eps;
	    ipl(dir) -= eps;    	    

	    mip.GetTransformation().CalcJacobian(ipr, jacr);
	    mip.GetTransformation().CalcJacobian(ipl, jacl);
	    
	    jacr = Trans(jacr);
	    jacl = Trans(jacl);

	    for (int j = 0; j < DIM; j++)
	     {	       
	       curl_FT[0](DIM-1-dir,j) = pow(-1.0,dir) * (jacr(j,0) - jacl(j,0)) / (2.0*eps);
	       curl_FT[1](DIM-1-dir,j) = pow(-1.0,dir)  * (jacr(j,1) - jacl(j,1)) / (2.0*eps);	      
	     }
	  }

	F_curlFT_Finv[0] = jac * curl_FT[0] * inv_jac;
	F_curlFT_Finv[1] = jac * curl_FT[1] * inv_jac;
	
	Mat<DIM> hesse[3];
        mip.CalcHesse (hesse[0],hesse[1],hesse[2]);
	
	Mat<DIM,DIM,AutoDiff<DIM> > f_tilde;
	for(int i = 0; i < DIM; i++)
        {
          for(int j = 0; j < DIM; j++)
          {
            f_tilde(i,j).Value() = jac(i,j);
            for(int k = 0; k < DIM; k++)
              f_tilde(i,j).DValue(k) = hesse[i](j,k);
          }
        }
	
	AutoDiff<DIM> ad_det = Det (f_tilde);
        AutoDiff<DIM> iad_det = 1.0 / ad_det;	
	curl_Jinv(0) = -iad_det.DValue(1);
	curl_Jinv(1) = iad_det.DValue(0);
	
	Vec<DIM> curl_Jinv_FT;
	curl_Jinv_FT(0) = curl_Jinv(0) * Trans(jac)(0,0) + curl_Jinv(1) * Trans(jac)(1,0);
	curl_Jinv_FT(1) = curl_Jinv(0) * Trans(jac)(0,1) + curl_Jinv(1) * Trans(jac)(1,1);

        Cast() -> T_CalcShape (TIP<DIM,AutoDiffDiff<DIM>> (addp),SBLambda([&](int nr,auto val)
                                  {
                                    shape.Row(nr).AddSize(DIM) = val.CurlShape();
                                    BareVector<double> curlshape = shape.Row(nr);				    
                                    Vec<DIM*DIM> matshape = val.Shape();				    				    
                                    for(int k=0; k<DIM; k++)
                                    {
                                      for(int j=0; j<DIM*DIM; j++)
				      {
					curlshape(k) += 1.0/mip.GetJacobiDet() * F_curlFT_Finv[k](j) * matshape(j);
                                      }
				      for(int j=0; j<DIM; j++)
				      {
					curlshape(k) += curl_Jinv_FT(j) * matshape(k+j*DIM);
				      }
                                    }
                                    
                                  }));
	
      }
    }

  };

 
  
  template <> class HCurlDivFE<ET_TRIG> : public T_HCurlDivFE<ET_TRIG> 
  {
    
  public:
    using T_HCurlDivFE<ET_TRIG> :: T_HCurlDivFE;

    virtual void ComputeNDof()
    {     
      order = 0;
      ndof = 0;
      for (int i=0; i<3; i++)
      {
        ndof += order_facet[i]+1;
        order = max2(order, order_facet[i]);
      }      
      int ninner = 3 * ((order_inner +1) * (order_inner))/2; 
      order = max2(order, order_inner);
      
      ndof += ninner;
      if (order_trace > -1)
	{
	  ndof += (order_trace +1) * (order_trace+2)/2.0;
	  order = max2(order, order_trace);
	}

      if(GGbubbles)
	{
	  ndof += order_inner+1;
	  order +=1;
	}
      
    }
    
   template <typename Tx, typename TFA> 
    void T_CalcShape (TIP<2,Tx> ip/*AutoDiffDiff<2> hx[2]*/, TFA & shape) const
    {
      auto x = ip.x, y = ip.y;
      Tx ddlami[3] ={ x, y, 1-x-y };
      
      int ii = 0;
      
      int oi=order_inner;
      int ot=order_trace; 
      int maxorder_facet =
        max2(order_facet[0],max2(order_facet[1],order_facet[2]));      

      ArrayMem<Tx,20> ha(maxorder_facet);
      ArrayMem<Tx,20> v(oi), u(oi);

      for (int i = 0; i < 3; i++)
        {
	  INT<2> e = ET_trait<ET_TRIG>::GetEdgeSort (i, vnums);	  	  
          Tx ls = ddlami[e[0]], le = ddlami[e[1]];
	  	 
	  IntLegNoBubble::EvalMult (maxorder_facet, le-ls, 0.25*le*ls, ha);
	  
          for (int l = 0; l <= order_facet[i]; l++)	    
	    shape[ii++] = Sigma_gradv(ha[l]);	 
        }
      
      Tx ls = ddlami[0];
      Tx le = ddlami[1];
      Tx lt = ddlami[2];
      
      //if trace order is not equal -1
      if (ot>-1)
	{
	  LegendrePolynomial::Eval(ot, 2*lt-1, v);
	  for (int i = 0; i <= ot; i++)
	    shape[ii++] = type4(le, ls, v[i]);

	  IntLegNoBubble::EvalMult (ot, le-lt, 0.25*le*lt, u);
	  LegendrePolynomial::EvalMult(ot, 2*ls-1, ls, v);
	  for(int i = 0; i <= ot-1; i++)
	      for(int j = 0; j+i <= ot-1; j++)       
		    shape[ii++] = Sigma_gradu_v(u[i],v[j]);	  		  	    
	}
                  
      IntLegNoBubble::EvalMult (oi, le-lt, 0.25*le*lt, u);
      LegendrePolynomial::EvalMult(oi, 2*ls-1, ls, v);
      
      for(int i = 0; i <= oi-1; i++)
      {
        for(int j = 0; j+i <= oi-1; j++)
        {	  
	  shape[ii++] = Curlgraduv_graducurlv(u[i],v[j]);	  	  
        }	
      }
     
      IntLegNoBubble::EvalMult (oi, le-ls, 0.25*le*ls, u);
      LegendrePolynomial::EvalMult(oi, 2*lt-1, lt, v);
      
      for(int i = 0; i <= oi-1; i++)
      {
        for(int j = 0; j+i <= oi-1; j++)
        {
	  shape[ii++] = Sigma_gradv(u[i]*v[j]); //divfree!
	  shape[ii++] = Curlgraduv_graducurlv(u[i],v[j]); 	 
        }	
      }

      
      if(GGbubbles)
	{
	  /*
	  IntLegNoBubble::EvalMult (oi, le-ls, 0.25*le*ls, u);
	  LegendrePolynomial::EvalMult(oi, 2*lt-1, lt, v);

	  //in 2 dimensions u[i]*v[oi-1-i] should be a H1_0 bubble
      
	  for(int i = 0; i <= oi-1; i++)
	    {
	      shape[ii++] = CurlBubble2D(u[i]*v[oi-1-i]);
	    }
	  */
	  //Vector<AutoDiffDiff<2, T> >  l2shape( (oi+1)*(oi+2) / 2 );
	  
	  Vector<Tx>  l2shape( (oi+1)*(oi+2) / 2);
	  DubinerBasis3::Eval ( oi , ddlami[0], ddlami[1], l2shape);

	  Vector<Tx>  S(oi+1);

	  S(0) = l2shape(oi);	    
	  for ( int i = 1; i<oi+1; i++)
	      S(i) = l2shape((i+1)*oi-(i-1)*(i)/2);
	  
	  Tx b = ddlami[0]*ddlami[1]*ddlami[2];
	  
	  for (int i=0; i<oi+1;i++)
	    {	     
	      shape[ii++] = GGbubble(S(i), b);
	    }
	}
            
    };
  };


    template <> class HCurlDivFE<ET_QUAD> : public T_HCurlDivFE<ET_QUAD> 
  {
    
  public:
    using T_HCurlDivFE<ET_QUAD> :: T_HCurlDivFE;

    virtual void ComputeNDof()
    {     
      order = 0;
      ndof = 0;
      for (int i=0; i<4; i++)
	{
	  ndof += order_facet[i]+1;
	  order = max2(order, order_facet[i]);
	}
      
      int ninner = (order_inner+1)*(order_inner+1) + (order_inner+2)*(order_inner) *2;    
      
      order = max2(order, order_inner);
      order += 4;
      ndof += ninner;

      if (order_trace > -1)
	{
	  ndof += (order_trace+1)*(order_trace+1);
	  order = max2(order, order_trace);
	}
     
    }
    
   template <typename Tx, typename TFA> 
    void T_CalcShape (TIP<2,Tx> ip/*AutoDiffDiff<2> hx[2]*/, TFA & shape) const
    {
      auto x = ip.x, y = ip.y;
      Tx lx[4] ={1-x, x, x, 1-x};
      Tx ly[4] = {1-y, 1-y, y, y};
      //Tx lam[4] = {(1-x)*(1-y),x*(1-y),x*y,(1-x)*y};
      Tx edgebubbles[4] = {(1-x)*x, x*(1-x), y*(1-y), (1-y)*y};

      typedef decltype(x.Value()+x.Value()) T;
     		  
      int ii = 0;
      
      int oi=order_inner;
      int ot = order_trace;
      
      int maxorder_facet =
        max2(order_facet[3],max2(order_facet[0],max2(order_facet[1],order_facet[2])));

      const EDGE * edges = ElementTopology::GetEdges(ET_QUAD);

      ArrayMem<Tx,20> ha(maxorder_facet);
      ArrayMem<Tx,20> v(oi), u(oi);
      for (int i = 0; i < 4; i++)
        {
	  INT<2> e = ET_trait<ET_QUAD>::GetEdgeSort (i, vnums);
	  
          Tx xi = lx[e[1]]+ly[e[1]]-lx[e[0]]-ly[e[0]];
          Tx eta = lx[e[0]]*ly[e[0]]+lx[e[1]]*ly[e[1]];
	  	 
	  IntLegNoBubble::EvalMult (maxorder_facet , xi, 0.25*edgebubbles[i], ha);	  
	  
          for (int l = 0; l <= order_facet[i]; l++)	    
	    shape[ii++] = Sigma_gradv(eta*ha[l]);	 
        }
           
      IntLegNoBubble::EvalMult (oi+2, lx[0]-lx[1], 0.25*lx[0]*lx[1], u);
      IntLegNoBubble::EvalMult (oi+2, ly[0]-ly[2], 0.25*ly[0]*ly[2], v);

      // constants in diagonal
      // off-diagonal constant functions are provided by edge functions

      
      shape[ii++] = u_Sigma_gradv(AutoDiffDiff<2,T>(1.0), lx[0]*ly[0]);
      //shape[ii++] = u_Sigma_gradv(lx[0]-lx[1], lx[0]*ly[0]);
	
      //shape[ii++] = Sigma_gradu_v(lx[0],ly[0]);

      if (ot>-1)
	shape[ii++] = Sigma_gradu_v(ly[0],lx[0]);

      //provide mixed functions in the diagonal
      for(int i = 0; i <= oi-1; i++)
      {
        for(int j = 0; j <= oi-1; j++)
        {
          shape[ii++] = Sigma_gradv(u[i]*v[j]);
	  if (ot>-1)
	    shape[ii++] = Sigma_gradu_v(u[i],v[j]);
	}
      }

      //are needed to compensate the terms in the off diagonal from the block before
      for(int i = 0; i <= oi+1; i++)
      {
        for(int j = 0; j <= oi-1; j++)
        {
          shape[ii++] = u_Sigma_gradv(u[j],v[i]);
          shape[ii++] = u_Sigma_gradv(v[j],u[i]);	  	  
        }
      }
       
      // lienar (and high order) parts in the diagonal
      for(int i = 0; i <= oi-1; i++)
       {
	 if (ot>-1)
	   {
	     shape[ii++] = Sigma_gradu_v(ly[0],u[i]);
	     shape[ii++] = Sigma_gradu_v(lx[0],v[i]);
	   }

	 shape[ii++] = Gradu_Curlv(u[i],ly[0]);
	 shape[ii++] = Gradu_Curlv(v[i],lx[0]);	 
       }
      
    };
  };
  
 
     template <> class HCurlDivFE<ET_TET> : public T_HCurlDivFE<ET_TET> 
  {
    
  public:
    using T_HCurlDivFE<ET_TET> :: T_HCurlDivFE;

    virtual void ComputeNDof()
    {     
      order = 0;
      ndof = 0;
      for (int i=0; i<4; i++)
      {
        ndof += (order_facet[i]+1)*(order_facet[i]+2);
        order = max2(order, order_facet[i]);
      }
      //first type + second type
      //int ninner = (order_inner + 1)*(order_inner + 2)*(order_inner + 3)/6.0 + 8.0/6.0* ((order_inner +2) * (order_inner +1) * (order_inner));
      int ninner = 8.0/6.0* ((order_inner +2) * (order_inner +1) * (order_inner));
      
      order = max2(order, order_inner);
      ndof += ninner;
      
      if (order_trace > -1)
	{
	  ndof += (order_trace +1) * (order_trace+2)* (order_trace+3)/6.0;
	  order = max2(order, order_trace);
	}

      if (GGbubbles)
	{
	  //highest order div functions (with zero normal comp)
	  //if(!GG)	    
	  //  ndof += 3*((order_inner+1)*order_inner + order_inner);
	  //else
	  
	  //GG bubbles of jay
	  ndof += 3*(order_inner+1)*(order_inner+2)/2;
	  //
	  order +=1;
	}
    }
    
   template <typename Tx, typename TFA> 
    void T_CalcShape (TIP<3,Tx> ip/*AutoDiffDiff<2> hx[2]*/, TFA & shape) const
    {
      auto x = ip.x, y = ip.y, z = ip.z ;     
      //Tx ddlami[4] ={ x, y, z, 1-x-y-z };

      typedef decltype(x.Value()+x.Value()) T;      
      AutoDiff<3,T> xx(x.Value(), &x.DValue(0));
      AutoDiff<3,T> yy(y.Value(), &y.DValue(0));
      AutoDiff<3,T> zz(z.Value(), &z.DValue(0));
      AutoDiff<3,T> ddlami[4] = {xx, yy, zz, 1-xx-yy-zz};
      
      int ii = 0;
      
      int maxorder_facet =
        max2(order_facet[0],max2(order_facet[1],order_facet[2]));
      

       const FACE * faces = ElementTopology::GetFaces(ET_TET);

       
       ArrayMem<AutoDiff<3,T>,20> ha((maxorder_facet+1)*(maxorder_facet+2)/2.0); 
       ArrayMem<AutoDiff<3,T>,20> v((order_inner + 1)*(order_inner + 2)*(order_inner + 3)/6.0);
       ArrayMem<AutoDiff<3,T>,20> dubb(order_inner*(order_inner+1)*(order_inner+2)/6.0);
       
      /* Edge based basis functions for tangential-normal continuity */
      for(int fa = 0; fa < 4; fa++)
        {
	  int fav[3] = {faces[fa][0], faces[fa][1], faces[fa][2]};
	  
	  int p = order_facet[fa];
	  //Sort vertices  first edge op minimal vertex
	  if(vnums[fav[0]] > vnums[fav[1]]) swap(fav[0], fav[1]);
	  if(vnums[fav[1]] > vnums[fav[2]]) swap(fav[1], fav[2]);
	  if(vnums[fav[0]] > vnums[fav[1]]) swap(fav[0], fav[1]);


          AutoDiff<3,T> ls = ddlami[fav[0]], le = ddlami[fav[1]], lt = ddlami[fav[2]];
	  
	  DubinerBasis3::Eval (maxorder_facet, ls, le, ha);

          for (int l = 0; l < (order_facet[fa]+1)*(order_facet[fa]+2)/2.0; l++)
	    {	      
	      shape[ii++] =  T_Dl1_o_Dl2xDl3_v<T>(le,ls,lt,ha[l]);	      
	      shape[ii++] =  T_Dl1_o_Dl2xDl3_v<T>(ls,lt,le,ha[l]);
	    }
        }

      int oi=order_inner;
      int ot=order_trace; 
      
      int es = 0; int ee = 1; int et = 2; int eo = 3 ;
      AutoDiff<3,T> ls = ddlami[es]; 
      AutoDiff<3,T> le = ddlami[ee];
      AutoDiff<3,T> lt = ddlami[et];
      AutoDiff<3,T> lo = ddlami[eo];

      LegendrePolynomial leg;
      JacobiPolynomialAlpha jac1(1);

      //############ type 1 ############
      if (ot>-1)
	{
	  leg.EvalScaled1Assign 
	    (ot, lt-lo, lt+lo,
	     SBLambda ([&](size_t k, AutoDiff<3,T> polz) LAMBDA_INLINE
		       {
			 JacobiPolynomialAlpha jac2(2*k+2);
 
			 jac1.EvalScaledMult1Assign
			   (ot-k, le-lt-lo, 1-ls, polz, 
			    SBLambda ([&] (size_t j, AutoDiff<3,T> polsy) LAMBDA_INLINE
				      {				    
					jac2.EvalMult(ot - k - j, 2 * ls - 1, polsy, 
						      SBLambda([&](size_t j, AutoDiff<3,T> val) LAMBDA_INLINE
							       {
								 shape[ii++] =  Id_v(val);	      							     					     							   }));
					jac2.IncAlpha2();
				      }));
			 jac1.IncAlpha2();
		       }));
	}
      
      //############ type 2 ############
            
      leg.EvalScaled1Assign 
	(oi-1, lt-lo, lt+lo,
	 SBLambda ([&](size_t k, AutoDiff<3,T> polz) LAMBDA_INLINE
		   {
		     JacobiPolynomialAlpha jac2(2*k+2);
 
		     jac1.EvalScaledMult1Assign
		       (oi-1-k, le-lt-lo, 1-ls, polz, 
			SBLambda ([&] (size_t j, AutoDiff<3,T> polsy) LAMBDA_INLINE
				  {				    
				    jac2.EvalMult(oi-1 - k - j, 2 * ls - 1, polsy, 
						  SBLambda([&](size_t j, AutoDiff<3,T> val) LAMBDA_INLINE
							   {
							     shape[ii++] =  T_Dl1_o_Dl2xDl3_v<T>(le,ls,lt,lo*val);	      
							     shape[ii++] =  T_Dl1_o_Dl2xDl3_v<T>(ls,lt,le,lo*val);	  
							     shape[ii++] =  T_Dl1_o_Dl2xDl3_v<T>(le,ls,lo,lt*val);	      
							     shape[ii++] =  T_Dl1_o_Dl2xDl3_v<T>(ls,lo,le,lt*val);	  
							     shape[ii++] =  T_Dl1_o_Dl2xDl3_v<T>(le,lo,lt,ls*val);	      
							     shape[ii++] =  T_Dl1_o_Dl2xDl3_v<T>(lo,lt,le,ls*val);	  
							     shape[ii++] =  T_Dl1_o_Dl2xDl3_v<T>(lo,ls,lt,le*val);	      
							     shape[ii++] =  T_Dl1_o_Dl2xDl3_v<T>(lt,ls,lo,le*val);							     
							   }));
				    jac2.IncAlpha2();
				  }));
		     jac1.IncAlpha2();
		   }));

      if(GGbubbles)
	{
	  //old version
	  
	  //if(!GG)
	  //  {
	  //int oc = oi+1;
	  //
	  //ArrayMem<AutoDiff<3,T>,20> adpol1(oc); 
	  //ArrayMem<AutoDiff<3,T>,20> adpol2(oc);
	  //ArrayMem<AutoDiff<3,T>,20> adpol3(oc);
	  //
	  //TetShapesInnerLegendre::CalcSplitted(oc+2, ddlami[0]-ddlami[3], ddlami[1], ddlami[2], adpol1, adpol2, adpol3 );
	  //
	  //for (int i = 0; i <3; i++)
	  //  {
	  //    for (int j = 0; j <= oc-2; j++)
	  //	for (int k = 0; k <= oc-2-j; k++)
	  //	  {
	  //	    //  [grad v  x  grad (uw)] o-times Dlami
	  //	    shape[ii++] = CurlBubble3D_type1(adpol2[j], adpol1[oc-2-j-k]*adpol3[k], ddlami[i]);        
	  //	    // grad w  x  grad (uv)
	  //	    shape[ii++] = CurlBubble3D_type1(adpol3[k], adpol1[oc-2-j-k]*adpol2[j], ddlami[i]);
	  //	  }     
	  //    // ned = [lami[0] * nabla(lami[3]) - lami[3] * nabla(lami[0])] o-times Dlami
	  //    for (int j= 0; j <= oc-2; j++)
	  //	shape[ii++] = CurlBubble3D_type2(ddlami[0], ddlami[3], adpol2[j]*adpol3[oc-2-j], ddlami[i]);
	  //  }
	  //  }
	  
	  //GGbubbles

	  AutoDiffDiff<3, T> ax[4] = { x, y, z, 1-x-y-z};
	      
	  AutoDiffDiff<3, T> b1 = ax[0]*ax[1]*ax[2] + ax[1]*ax[2]*(1-ax[0]-ax[1]-ax[2]);
	  AutoDiffDiff<3, T> b2 = ax[0]*ax[1]*ax[2] + ax[0]*ax[2]*(1-ax[0]-ax[1]-ax[2]);
	  AutoDiffDiff<3, T> b3 = ax[0]*ax[1]*ax[2] + ax[0]*ax[1]*(1-ax[0]-ax[1]-ax[2]);
	  AutoDiffDiff<3, T> b0 = ax[0]*ax[1]*ax[2];

	  leg.EvalScaled1Assign 
	    (oi, ax[2]-ax[3], ax[2]+ax[3],
	     SBLambda ([&](size_t k, Tx polz) LAMBDA_INLINE
		       {
			 // JacobiPolynomialAlpha jac(2*k+1);
			 JacobiPolynomialAlpha jac2(2*k+2);
 
			 jac1.EvalScaledMult1Assign
			   (oi-k, ax[1]-ax[2]-ax[3], 1-ax[0], polz, 
			    SBLambda ([&] (size_t j, Tx polsy) LAMBDA_INLINE
				      {
					// JacobiPolynomialAlpha jac(2*(j+k)+2);
					jac2.EvalMult(oi - k - j, 2 * ax[0] - 1, polsy, 
						      SBLambda([&](size_t l, Tx val) LAMBDA_INLINE
							       {
								 if (l == oi - k - j)
								   {
								     shape[ii++] = GGbubble_B1(val, b0, b1, b2, b3);
								     shape[ii++] = GGbubble_B2(val, b0, b1, b2, b3);
								     shape[ii++] = GGbubble_B3(val, b0, b1, b2, b3);								   
								   }
							       }));
					jac2.IncAlpha2();
				      }));
			 jac1.IncAlpha2();
		       }));

	  //old version with	      
	  /*
	    Vector<AutoDiffDiff<3, T> > S( (oi+1)*(oi+2)/2.0+1 );
	    l2orth.T_L2orthShape<AutoDiffDiff<3,T> > (ax,S);
	      
	    for ( int i = 0; i<(oi+1)*(oi+2)/2.0; i++)
	    {	    
	    shape[ii++] = GGbubble_B1(S[i], b0, b1, b2, b3);
	    shape[ii++] = GGbubble_B2(S[i], b0, b1, b2, b3);
	    shape[ii++] = GGbubble_B3(S[i], b0, b1, b2, b3);
	    }
	  */

	}
	
            
     };
  };
  
  ////////////////////// SURFACE ////////////////////////////
    template <int DIM>
  class HCurlDivSurfaceFiniteElement : public FiniteElement
  {
  public:
    using FiniteElement::FiniteElement;
    using FiniteElement::ndof;
    using FiniteElement::order;

    virtual void CalcMappedShape (const MappedIntegrationPoint<DIM,DIM+1> & mip,
      BareSliceMatrix<double> shape) const = 0;

    virtual void CalcShape (const IntegrationPoint & ip, 
                            BareSliceMatrix<double> shape) const = 0;

  };
  

  template <ELEMENT_TYPE ET> class HCurlDivSurfaceFE;

  
  template <ELEMENT_TYPE ET>
  class T_HCurlDivSurfaceFE : public HCurlDivSurfaceFiniteElement<ET_trait<ET>::DIM>,
    public VertexOrientedFE<ET>
  {
  protected:
    enum { DIM = ET_trait<ET>::DIM };
    enum { DIM_STRESS = (DIM+1)*(DIM+1) }; //check this!!!
    
    using VertexOrientedFE<ET>::vnums;
    using HCurlDivSurfaceFiniteElement<ET_trait<ET>::DIM>::ndof;
    using HCurlDivSurfaceFiniteElement<ET_trait<ET>::DIM>::order;

    int order_inner;


  public:
    using VertexOrientedFE<ET>::SetVertexNumbers;
    
    T_HCurlDivSurfaceFE (int aorder)
    {
      order = aorder;
      order_inner = aorder;
    }
    
    virtual ELEMENT_TYPE ElementType() const { return ET; }
    const HCurlDivSurfaceFE<ET> * Cast() const { return static_cast<const HCurlDivSurfaceFE<ET>*> (this); } 
    
    INLINE void SetOrderInner (int order) { order_inner = order; }

    virtual void ComputeNDof()
    {
      cout << "Error, T_HCurlDivSurfaceFE<ET>:: ComputeNDof not available for base class" << endl;
    }

    virtual void CalcShape (const IntegrationPoint & ip, 
                            BareSliceMatrix<double> shape) const
    {
      Vec<DIM, AutoDiffDiff<DIM+1>> adp;
      for ( int i=0; i<DIM; i++)
      {
        adp(i) = AutoDiffDiff<DIM+1>(ip(i),i);
      }

      Cast() -> T_CalcShape (TIP<DIM, AutoDiffDiff<DIM+1>> (adp), SBLambda([&] (int nr, auto val)
                                          {
                                            //shape.Row(nr).AddSize(DIM_STRESS) = val.Shape();
					    shape.Row(nr).AddSize(DIM_STRESS) = val;
                                          }));
    }

    
    virtual void CalcMappedShape (const MappedIntegrationPoint<DIM,DIM+1> & mip,
                            BareSliceMatrix<double> shape) const
    {
      Vec<DIM, AutoDiff<DIM+1>> adp = mip;
      Vec<DIM, AutoDiffDiff<DIM+1>> addp;
      for (int i=0; i<DIM+1; i++)
      {
        addp[i] = adp[i].Value();
        addp[i].LoadGradient(&adp[i].DValue(0));
      }
      Cast() -> T_CalcShape (TIP<DIM,AutoDiffDiff<DIM+1>> (addp),SBLambda([&](int nr,auto val)
      {
	if (DIM==1)
	  shape.Row(nr).AddSize(DIM_STRESS) = val;
	else
	  shape.Row(nr).AddSize(1) = val;
      }));      
    }


  };

  template <> class HCurlDivSurfaceFE<ET_SEGM> : public T_HCurlDivSurfaceFE<ET_SEGM> 
  {
    
  public:
    using T_HCurlDivSurfaceFE<ET_SEGM> :: T_HCurlDivSurfaceFE;

    virtual void ComputeNDof()
    {
      order = 0;
      ndof = 0;
      ndof += order_inner+1;
      order = max2(order,order_inner);

    }
   template <typename Tx, typename TFA> 
    void T_CalcShape (TIP<1,Tx> ip/*AutoDiffDiff<2> hx[2]*/, TFA & shape) const
    {      
      auto x = ip.x;
      AutoDiffDiff<2> ddlami[2] ={ x, 1-x };
      
      int ii = 0;
      
      ArrayMem<AutoDiffDiff<2>,20> ha(order_inner+1);
      
      int es = 0,ee = 1;
      if(vnums[es] > vnums[ee]) swap (es,ee);

      AutoDiffDiff<2> ls = ddlami[es],le = ddlami[ee];
      
      IntLegNoBubble::EvalMult (order_inner, le-ls, 0.25*le*ls, ha);
      
      for(int l = 0; l <= order_inner; l++)	
	shape[ii++] = Sigma_gradv(ha[l]).Shape();
    };
  };


  
  /* Face basis functions which are normal-tangential continuous */
  /* calculates [(grad l1) o-times (grad l2 x grad l3)] * legendre */
  /* (grad l2 x grad l3) is a scalar!! (cross product on surface */
  
  class T_Dl1_o_Dl2xDl3_v_surf
  {
    AutoDiffDiff<3> l1,l2,l3,v;
  public:
    T_Dl1_o_Dl2xDl3_v_surf  (AutoDiffDiff<3> lam1, AutoDiffDiff<3> lam2, AutoDiffDiff<3> lam3, AutoDiffDiff<3> av) : l1(lam1), l2(lam2), l3(lam3), v(av) { ; }
    
    Vec<2> Shape() {
      double cross = l2.DValue(0)*l3.DValue(1) - l2.DValue(1)*l3.DValue(0);      
      return Vec<2> (v.Value()*l1.DValue(0) * cross,  v.Value()*l1.DValue(1) * cross);
    }

    Vec<2> DivShape()
    {
      throw Exception("not available on surface");
    }

  };
  
  template <> class HCurlDivSurfaceFE<ET_TRIG> : public T_HCurlDivSurfaceFE<ET_TRIG> 
  {
    
  public:
    using T_HCurlDivSurfaceFE<ET_TRIG> :: T_HCurlDivSurfaceFE;

    virtual void ComputeNDof()
    {
      order = 0;
      ndof = 0;
      ndof += (order_inner+1) * (order_inner+2);
      order = max2(order,order_inner);
    }
    
   template <typename Tx, typename TFA> 
    void T_CalcShape (TIP<2,Tx> ip/*AutoDiffDiff<2> hx[2]*/, TFA & shape) const
    {            
      auto x = ip.x, y= ip.y;
      AutoDiffDiff<3> ddlami[3] ={ x, y, 1-x-y };
      
      int ii = 0;

      ArrayMem<AutoDiffDiff<3>,20> ha((order_inner+1)*(order_inner+2)/2.0);
      
      int es = 0, ee = 1, et = 2;
      if(vnums[es] > vnums[ee]) swap(es, ee);
      if(vnums[ee] > vnums[et]) swap(ee, et);
      if(vnums[es] > vnums[et]) swap(es,et);
            
      AutoDiffDiff<3> ls = ddlami[es],le = ddlami[ee], lt = ddlami[et];
      
      DubinerBasis3::Eval (order_inner, ls, le, ha);

      for (int l = 0; l < (order_inner+1)*(order_inner+2)/2.0; l++)
	    {
	      shape[ii++] = T_Dl1_o_Dl2xDl3_v_surf(le,ls,lt,ha[l]).Shape();
	      shape[ii++] = T_Dl1_o_Dl2xDl3_v_surf(ls,lt,le,ha[l]).Shape();
	    }     
    };
      
  };

}


#endif
  