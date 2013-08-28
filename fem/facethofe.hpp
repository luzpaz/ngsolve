#ifndef FILE_FACETHOFE
#define FILE_FACETHOFE

/*********************************************************************/
/* File:   facethofe.hpp                                             */
/* Author: A. Sinwel, H. Egger, J. Schoeberl                         */
/* Date:   2008                                                      */
/*********************************************************************/

// #include "tscalarfe.cpp"

namespace ngfem
{




  
  template <ELEMENT_TYPE ET> class FacetFE;


  template <ELEMENT_TYPE ET> 
  class FacetFiniteElement_Family :
    public FacetVolumeFiniteElement<ET_trait<ET>::DIM>,
    public ET_trait<ET> 
  {
  protected:
    enum { DIM = ET_trait<ET>::DIM };
    using ET_trait<ET>::N_FACET;

    // using FiniteElement::ndof;
    // using FiniteElement::eltype;
    using FacetVolumeFiniteElement<ET_trait<ET>::DIM>::ndof;
    using FacetVolumeFiniteElement<ET_trait<ET>::DIM>::eltype;

    using FacetVolumeFiniteElement<ET_trait<ET>::DIM>::first_facet_dof;
    using FacetVolumeFiniteElement<ET_trait<ET>::DIM>::facet_order;

  public:
    FacetFiniteElement_Family ()
    { 
      eltype = ET;
    }

    virtual ELEMENT_TYPE ElementType() const { return ET; }

    virtual void ComputeNDof() 
    {
      ndof = 0;
      for (int i = 0; i < N_FACET; i++)
	{
	  first_facet_dof[i] = ndof;
	  int fo = facet_order[i];
	  switch (ElementTopology::GetFacetType (ET, i))
	    {
	    case ET_SEGM: ndof += fo+1; break;
	    case ET_TRIG: ndof += ( (fo+1) * (fo+2) ) / 2; break;
	    case ET_QUAD: ndof += sqr (fo+1); break;
	    default: ;
	    }
	}
      first_facet_dof[N_FACET] = ndof;
    }
    

    virtual void CalcFacetShapeVolIP(int fnr, const IntegrationPoint & ip, 
				     SliceVector<> shape) const
    {
      double pt[DIM];
      for (int i = 0; i < DIM; i++) pt[i] = ip(i);
      static_cast<const FacetFE<ET>*>(this)->T_CalcShapeFNr (fnr, pt, shape); 
    }
  };

  template <ELEMENT_TYPE ET>
  class FacetFE : public FacetFiniteElement_Family<ET>
  {
  public:
    template<typename Tx, typename TFA>  
    void T_CalcShapeFNr (int fnr, Tx x[ET_trait<ET>::DIM], TFA & shape) const;
  };


#ifdef FILE_FACETHOFE_CPP
#else
  extern template class FacetFE<ET_SEGM>;
  extern template class FacetFE<ET_TRIG>;
  extern template class FacetFE<ET_QUAD>;
  extern template class FacetFE<ET_TET>;
  extern template class FacetFE<ET_HEX>;
  extern template class FacetFE<ET_PRISM>;
  extern template class FacetFE<ET_PYRAMID>;

  extern template class FacetFiniteElement_Family<ET_SEGM>;
  extern template class FacetFiniteElement_Family<ET_TRIG>;
  extern template class FacetFiniteElement_Family<ET_QUAD>;
  extern template class FacetFiniteElement_Family<ET_TET>;
  extern template class FacetFiniteElement_Family<ET_HEX>;
  extern template class FacetFiniteElement_Family<ET_PRISM>;
  extern template class FacetFiniteElement_Family<ET_PYRAMID>;

#endif

  /*

  //------------------------------------------------------------
  template <>
  class FacetFE<ET_SEGM> : public FacetFiniteElement_Family<ET_SEGM>
  {
  public:
    template<typename Tx, typename TFA>  
    void T_CalcShapeFNr (int fnr, Tx x[2], TFA & shape) const
    {
      shape[0] = 1.0;
    }
  };

  template <>
  class FacetFE<ET_TRIG> : public FacetFiniteElement_Family<ET_TRIG>
  {
  public:
    template<typename Tx, typename TFA>  
    void T_CalcShapeFNr (int fnr, Tx x[2], TFA & shape) const
    {
      Tx lam[3] = { x[0], x[1], 1-x[0]-x[1] };
      
      INT<2> e = GetEdgeSort (fnr, vnums);
      int p = facet_order[fnr];

      LegendrePolynomial::Eval (p, lam[e[1]]-lam[e[0]], shape);
    }
  };


  // --------------------------------------------------------
  template <>
  class FacetFE<ET_QUAD> : public FacetFiniteElement_Family<ET_QUAD>
  {
  public:
    template<typename Tx, typename TFA>  
    void T_CalcShapeFNr (int fnr, Tx hx[2], TFA & shape) const
    {
      Tx x = hx[0], y = hx[1];
      Tx sigma[4] = {(1-x)+(1-y),x+(1-y),x+y,(1-x)+y};  
      
      INT<2> e = GetEdgeSort (fnr, vnums);
      int p = facet_order[fnr];

      LegendrePolynomial::Eval (p, sigma[e[1]]-sigma[e[0]], shape);
    }
  };


  // --------------------------------------------------------
  template <>
  class FacetFE<ET_TET> : public FacetFiniteElement_Family<ET_TET>
  {
  public:
    template<typename Tx, typename TFA>  
    void T_CalcShapeFNr (int fnr, Tx hx[3], TFA & shape) const
    {
      Tx lam[4] = { hx[0], hx[1], hx[2], 1-hx[0]-hx[1]-hx[2] };

      INT<4> f = GetFaceSort (fnr, vnums);
      int p = facet_order[fnr];
      
      DubinerBasis::Eval (p, lam[f[0]], lam[f[1]], shape);
    }
  };


  // --------------------------------------------------------
  template <>
  class FacetFE<ET_HEX> : public FacetFiniteElement_Family<ET_HEX>
  {
  public:
    template<typename Tx, typename TFA>  
    void T_CalcShapeFNr (int fnr, Tx hx[3], TFA & shape) const
    {
      Tx x = hx[0], y = hx[1], z = hx[2];
      Tx sigma[8]={(1-x)+(1-y)+(1-z),x+(1-y)+(1-z),x+y+(1-z),(1-x)+y+(1-z),
		   (1-x)+(1-y)+z,x+(1-y)+z,x+y+z,(1-x)+y+z}; 
      
      int p = facet_order[fnr];

      INT<4> f = GetFaceSort (fnr, vnums);	  
	    
      Tx xi  = sigma[f[0]] - sigma[f[1]]; 
      Tx eta = sigma[f[0]] - sigma[f[3]];

      ArrayMem<Tx,20> polx(p+1), poly(p+1);
      
      LegendrePolynomial::Eval(p, xi, polx);
      LegendrePolynomial::Eval(p, eta, poly);

      for (int i = 0, ii = 0; i <= p; i++)
	for (int j = 0; j <= p; j++)
	  shape[ii++] = polx[i] * poly[j];
    }
  };

  // --------------------------------------------------------
  template <>
  class FacetFE<ET_PRISM> : public FacetFiniteElement_Family<ET_PRISM>
  {
  public:
    template<typename Tx, typename TFA>  
    void T_CalcShapeFNr (int fnr, Tx hx[3], TFA & shape) const
    {
      Tx x = hx[0], y = hx[1], z = hx[2];
      Tx lam[6] = { x, y, 1-x-y, x, y, 1-x-y };
      Tx muz[6]  = { 1-z, 1-z, 1-z, z, z, z };
      

      INT<4> f = GetFaceSort (fnr, vnums);

      int p = facet_order[fnr];

      if (fnr < 2)
	DubinerBasis::Eval (p, lam[f[0]], lam[f[1]], shape);
      else
	{
	  Tx xi  = lam[f[0]]+muz[f[0]] - lam[f[1]]-muz[f[1]];
	  Tx eta = lam[f[0]]+muz[f[0]] - lam[f[3]]-muz[f[3]];

	  ArrayMem<Tx,20> polx(p+1), poly(p+1);

	  LegendrePolynomial::Eval (p, xi, polx);
	  LegendrePolynomial::Eval (p, eta, poly);

	  for (int i = 0, ii = 0; i <= p; i++)
	    for (int j = 0; j <= p; j++)
	      shape[ii++] = polx[i] * poly[j];
	}
    }

  };


  // --------------------------------------------------------
  template <>
  class FacetFE<ET_PYRAMID> : public FacetFiniteElement_Family<ET_PYRAMID>
  {
  public:
    template<typename Tx, typename TFA>  
    void T_CalcShapeFNr (int fnr, Tx hx[3], TFA & shape) const
    {
      Tx x = hx[0], y = hx[1], z = hx[2];
      
      if (z == 1.) z -= 1e-10;
      
      Tx xt = x / (1-z);
      Tx yt = y / (1-z);
      
      Tx sigma[4]  = { (1-xt)+(1-yt), xt+(1-yt), xt+yt, (1-xt)+yt };
      Tx lambda[4] = { (1-xt)*(1-yt), xt*(1-yt), xt*yt, (1-xt)*yt };
      Tx lam[5];
      
      for (int i = 0; i < 4; i++)  
	lam[i] = lambda[i] * (1-z);
      lam[4] = z;


      INT<4> f = GetFaceSort (fnr, vnums);

      int p = facet_order[fnr];

      if (fnr < 4)
	DubinerBasis::Eval (p, lam[f[0]], lam[f[1]], shape);
      else
	{
	  Tx xi  = sigma[f[0]]-sigma[f[1]];
	  Tx eta = sigma[f[0]]-sigma[f[3]];

	  ArrayMem<Tx,20> polx(p+1), poly(p+1);

	  LegendrePolynomial::Eval (p, xi, polx);
	  LegendrePolynomial::Eval (p, eta, poly);

	  for (int i = 0, ii = 0; i <= p; i++)
	    for (int j = 0; j <= p; j++)
	      shape[ii++] = polx[i] * poly[j];
	}
    }

  };

  */




}

#endif
